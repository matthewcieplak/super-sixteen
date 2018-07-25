#include <MCP23S17.h> //https://github.com/MajenkoLibraries/MCP23S17
#include <SPI.h>
#include <elapsedMillis.h>

elapsedMillis multiplex;
elapsedMillis stepper;


const int CS0_PIN = 10;
const int CS1_PIN = 9;
const int DISPLAY_SINK_PIN = 8;

uint8_t current_step = 0;
const int led_rows[4] = {0x01, 0x02, 0x04, 0x08};
const int current_step_rows[4] = {0xE0, 0xD0, 0xB0, 0x70};
int pitch_matrix[16];
bool step_matrix[16]   = {1,0,0,0, 1,1,0,0, 1,1,1,0, 1,1,1,1};
bool led_matrix[16]    = {1,0,0,0, 1,1,0,0, 1,1,1,0, 1,1,1,1};
bool current_step_matrix[16] = {1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1};
uint8_t byte1;
uint8_t byte2;
int row_counter = 0;
int num_display = 0;
int digit_counter = 0;
int digit_display[3] = {0, 0, 0};

int col        = 0;
int stepnumber = 1;
int selected_step = 0;

int lastAnalogValues[2];
int analogValues[2];
float calibration_value = 1.0;

const uint8_t truth_table[10] = {
  0x3F,
  0x06,
  0x5B,
  0x4F,
  0x66,
  0x6D,
  0x7D,
  0x07,
  0x7F,
  0x6F
};

MCP23S17 DisplayDriver(&SPI, CS0_PIN, 0);
MCP23S17 MatrixDriver(&SPI, CS0_PIN, 1);


void setup() {
  initializeMatrix();
  initializeDisplay();
  pinMode(9, OUTPUT); //enable CS for DAC
}

void initializeMatrix(){
  MatrixDriver.begin();
  for (int pin = 0; pin < 16; pin++){
    if (pin < 4) {
        MatrixDriver.pinMode(pin, INPUT_PULLUP); //use these for reading in current_steps
    } else {
      MatrixDriver.pinMode(pin, OUTPUT);
      MatrixDriver.digitalWrite(pin, LOW);
    }    
  }

  MatrixDriver.writePort(0x0F00);
}

void initializeDisplay(){
  DisplayDriver.begin();
  for (int pin = 0; pin < 8; pin++){
    DisplayDriver.pinMode(pin, OUTPUT);
    DisplayDriver.writePort(0, 0x00);
  }

  DisplayDriver.pinMode(DISPLAY_SINK_PIN, OUTPUT);
  DisplayDriver.pinMode(DISPLAY_SINK_PIN+1, OUTPUT);
  DisplayDriver.pinMode(DISPLAY_SINK_PIN+2, OUTPUT);
  DisplayDriver.writePort(1, 0xFF);
}



void updateMatrix(int row) {
  byte1 = 0x00;
  for (col = 0; col < 4; col++){
    byte1 += ( led_matrix[row*4+(3-col)] << col); //columns are wired backwards R-L on my board, hence 3-col
  }
  byte1 = 0xFF - byte1 << 4; //invert since we want to pull low for active drain
  byte2 = led_rows[row] + current_step_rows[row]; //while we're using the serial port, pull one output low for current_steps so we can read them on the pullup inputs
  
  //turn on new light;
  uint16_t doublebyte = (byte2 << 8) + byte1;
  MatrixDriver.writePort(0xF0F0); //hard-reset previous pins
  MatrixDriver.writePort(doublebyte);
  //MatrixDriver.writePort(1, byte2);
  //MatrixDriver.writePort(0, byte1);
}

void readButtons(int row) {
  byte current_step_row = MatrixDriver.readPort(0) & 0x0F; // (mask since we only want the last 4 bits, pins 4-7);
  for (int i = 0; i < 4; i++) {
    int stepnum = row*4+ i;
    // check each bit in the byte and see if it's pulled low
    bool value = (current_step_row & (8 >> i)); //pins backwards again, iterate R-L by rightwards bitshift
    if (value != current_step_matrix[stepnum]) {
      current_step_matrix[stepnum] = value; //store current_step state
      if (value == 0){ //toggle step status when current_step is pressed in
        selected_step = stepnum;
        step_matrix[stepnum] = !step_matrix[stepnum];
        led_matrix[stepnum] = !led_matrix[stepnum];
      }
    }     
  }
}


//assuming dac single channel, gain=2
void setOutput(unsigned int val){
  if (val > 4096) {
     val = 4095;
  }
  byte lowByte = val & 0xff;
  byte highByte = ((val >> 8) & 0xff) | 0x10;

  digitalWrite(CS1_PIN, LOW);
  PORTB &= 0xfb;
  SPI.transfer(highByte);
  SPI.transfer(lowByte);
  PORTB |= 0x4;
  digitalWrite(CS1_PIN, HIGH);
}

void updateDisplay(){
  DisplayDriver.digitalWrite(digit_counter + DISPLAY_SINK_PIN, LOW); //turn off prev digit
  digit_counter++;
  if (digit_counter == 3) {
    digit_counter = 0;
  }
  DisplayDriver.writePort(0, 0xFF - truth_table[digit_display[digit_counter]]); //write new digit
  DisplayDriver.digitalWrite(digit_counter + DISPLAY_SINK_PIN, HIGH); //turn on new digit
}
 
void increment_step(){
  if (stepper > 150) {
    stepper = 0;
    led_matrix[current_step] = step_matrix[current_step];
    current_step++;
    if (current_step == 16) {
      current_step = 0;
    }
    
    led_matrix[current_step] = !step_matrix[current_step];
    if (step_matrix[current_step]) {
      setOutput(pitch_matrix[current_step] * 11.63 * 4 * calibration_value); //convert from 0-88 to 0-4096, scale octave by calibration value
    } else {
      setOutput(0);
    }
  }
}

void multiplex_leds(){
  if (multiplex > 1) {
    multiplex = 0;
    
    updateMatrix(row_counter);
    readButtons(3-row_counter);
    row_counter++;

    if (row_counter == 4) {
      row_counter = 0;    
    }

    updateDisplay();
  }
}

void read_input(){
  if (stepper % 50 == 0) {
    for(int i = 0; i < 2; i++) {
      analogValues[i] = analogRead(i);
      if (abs(analogValues[i] - lastAnalogValues[i]) > 3) {
        lastAnalogValues[i] = analogValues[i];
        calibration_value = (float(analogValues[1]) + 1024.0) / 1500.0;        
        num_display = analogValues[0] / 11.60; //convert from 0-1024 to 0-88
        pitch_matrix[selected_step] = num_display;
                          
        digit_display[2] = num_display % 10;
        digit_display[1] = num_display / 10 % 10;
        digit_display[0] = num_display / 100 % 10;  
      }
    }
  }
}

void loop() {
  increment_step();
  multiplex_leds();
  read_input();
}



