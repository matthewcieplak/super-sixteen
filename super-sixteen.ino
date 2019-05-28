#include "Sequencer.h"
#include "Encoder.h"
#include "AnalogIO.h"
#include "LEDMatrix.h"
#include <MCP23S17.h> //https://github.com/MajenkoLibraries/MCP23S17
#include <SPI.h>
#include <elapsedMillis.h>

elapsedMillis multiplex;
elapsedMillis stepper;
elapsedMillis timekeeper;
elapsedMillis blinker;

#define ENC_PORT PINC
#define GATE_PIN 0
#define LDAC_PIN 1
#define CLOCK_OUT_PIN 2
#define CLOCK_IN_PIN 3
#define RESET_PIN 4

#define GLIDE_PIN 5
#define REPEAT_PIN 6
#define RECORD_PIN 7

#define GLIDE_LED_PIN 3 //this one's on display driver chip, not mcu
#define SAVE_PIN 4 //these four on display driver chip, not MCU
#define LOAD_PIN 5 //ditto
#define PLAY_PIN 6 //ditto
#define SHIFT_PIN 7 //ditto

const int function_buttons[4] = {SAVE_PIN, LOAD_PIN, PLAY_PIN, SHIFT_PIN};
int function_button_matrix[4] = {1, 1, 1, 1}; //store status of buttons in/out
const int aux_buttons[3] = {GLIDE_PIN, REPEAT_PIN, RECORD_PIN};
int aux_button_matrix[3] = {1, 1, 1};

const int CS0_PIN = 10;
const int CS1_PIN = 9;
const int GAIN_1 = 0x1;
const int GAIN_2 = 0x0;


uint8_t current_step = 0;
uint8_t active_step = 0;
uint8_t sequence_length = 16;
const int led_rows[4] = {0x80, 0x40, 0x20, 0x10};
//const int current_step_rows[4] = {0xE0, 0xD0, 0xB0, 0x70};
int step_map[16]       = {3, 2, 1, 0, 0, 1, 2, 3, 3, 2, 1, 0, 0, 1, 2, 3}; //rows are wired symmetrically rather than sequentially
int button_map[16]   = {3, 2, 1, 0, 0, 1, 2, 3, 3, 2, 1, 0, 0, 1, 2, 3}; //rows are wired symmetrically rather than sequentially

int pitch_matrix[16];
int octave_matrix[16];
int duration_matrix[16];
int cv_matrix[16];

bool step_matrix[16]   = {1,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0};
bool led_matrix[16]    = {1,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0};
bool button_matrix[16] = {1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1};
bool glide_matrix[16];
//bool current_step_matrix[16] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
bool gate_active = false;
bool clock_active = false;
bool blinking = false;

uint8_t byte1;
uint8_t byte2;
int row_counter = 0;
int num_display = 0;
int digit_counter = 0;
int digit_display[3] = {0, 0, 0};

#define TEMPO_PARAM 0
#define PITCH_PARAM 1
#define OCTAVE_PARAM 2
#define DURATION_PARAM 3
#define CV_PARAM 4

int analog_params[4] = {PITCH_PARAM, OCTAVE_PARAM, DURATION_PARAM, CV_PARAM};

int display_param = PITCH_PARAM;


int col        = 0;
int stepnumber = 1;
int selected_step = 0;

int lastAnalogValues[4];
int analogValues[4];
int analogMultiplexor = 0;
int tempo = 300;
float calibration_value = 1.0;
float EMA_a = 0.6; //input smoothing coeff

int enc_a;
int enc_b;
int enc_a_prev;

byte font_table[] = {
  B1111110,   // [0] => "0"
  B0110000,   // [1] => "1"
  B1101101,   // [2] => "2"
  B1111001,   // [3] => "3"
  B0110011,   // [4] => "4"
  B1011011,   // [5] => "5"
  B1011111,   // [6] => "6"
  B1110000,   // [7] => "7"
  B1111111,   // [8] => "8"
  B1111011,   // [9] => "9"
  B1110111,   // [10] => "A"
  B0011111,   // [11] => "b"
  B1001110,   // [12] => "C"
  B0111101,   // [13] => "d"
  B1001111,   // [14] => "E"
  B1000111,   // [15] => "F"
  B0000001,   // [16] => "dash"
  B0000000,   // [17] => "blank"
};

uint8_t font[18];

MCP23S17 DisplayDriver(&SPI, CS0_PIN, 0);
MCP23S17 MatrixDriver(&SPI, CS0_PIN, 1);


void setup() {
  analogReference(EXTERNAL); // use AREF for reference voltage
  pinMode(CS0_PIN, OUTPUT); //enable CS for DAC
  pinMode(CS1_PIN, OUTPUT); //enable CS for DAC

  initializeMatrix();
  initializeDisplay();
  initializeFont();
  initializeButtons();

  for (int i = 0; i < 16; i++) {
    duration_matrix[i] = 80;
  }
}

void initializeFont() {
  for (int pin = 0; pin < 17; pin++) {
    int num = font_table[pin];
    font[pin] = ((num & 0x01) << 7)
                | ((num & 0x02) << 5)
                | ((num & 0x04) << 3)
                | ((num & 0x08) << 1)
                | ((num & 0x10) >> 1)
                | ((num & 0x20) >> 3)
                | ((num & 0x40) >> 5)
                | ((num & 0x80) >> 7);
  }
}
void initializeMatrix() {
  MatrixDriver.begin();
  for (int pin = 0; pin < 16; pin++) {
    if (pin < 4) {
      MatrixDriver.pinMode(pin, OUTPUT);
      MatrixDriver.digitalWrite(pin, LOW);
    } else if (pin >= 4 && pin < 8) {
      MatrixDriver.pinMode(pin, INPUT_PULLUP); //use these for reading in buttons
    } else {
      MatrixDriver.pinMode(pin, OUTPUT);
      MatrixDriver.digitalWrite(pin, LOW);
    }
  }

  MatrixDriver.writePort(0x0F00);
}

void initializeDisplay() {
  DisplayDriver.begin();
  for (int pin = 0; pin < 16; pin++) {
    if (pin >= 4 && pin < 8) {
      DisplayDriver.pinMode(pin, INPUT_PULLUP); //use these for reading in 4 top buttons
    } else {
      DisplayDriver.pinMode(pin, OUTPUT);
    }
    //DisplayDriver.writePort(0x0000);
  }
}

void initializeButtons() {
  pinMode(A4, INPUT_PULLUP); //encoder A
  pinMode(A5, INPUT_PULLUP); //encoder B
  pinMode(GLIDE_PIN, INPUT_PULLUP); //glide button
  pinMode(REPEAT_PIN, INPUT_PULLUP); //repeat button
  pinMode(RECORD_PIN, INPUT_PULLUP); //record button

  pinMode(GATE_PIN, OUTPUT); //gate
  
  //  pinMode(LDAC_PIN, OUTPUT); //LDAC
  //  pinMode(CLOCK_OUT_PIN, OUTPUT); //clock out
  //  pinMode(CLOCK_IN_PIN, INPUT); //clock in (external pullup)
  //  pinMode(6, INPUT); //reset in (external pullup)
  //
}



void updateMatrix(int row) {
  byte1 = 0x00;
  for (col = 0; col < 4; col++) {
    byte1 += ( led_matrix[row * 4 + col] << step_map[row * 4 + col]);
  }
  byte1 = (0x0F - byte1) | led_rows[row]; //invert since we want to pull low for active drain
  //byte2 = button_rows[row]; //while we're using the serial port, pull one output low for buttons so we can read them on the pullup inputs

  //turn on new light;
  //uint16_t doublebyte = (byte1 << 8) + byte2;
  //Bank3.writePort(0x0000); //hard-reset previous pins
  //Bank3.writePort(doublebyte);
  //Bank3.writePort(1, byte2);
  MatrixDriver.writePort(1, byte1);
}

void readButtons(int row) {
  //Bank3.writePort(0, button_rows[row]);
  //byte button_row = Bank3.readPort(0) & 0x00; // (mask since we only want the last 4 bits, pins 4-7);
  MatrixDriver.digitalWrite(row, LOW);
  for (int ii = 0; ii < 4; ii++) {
    int stepnum = row * 4 + ii;
    //    //check each bit in the byte and see if it's pulled low -- to reduce SPI overhead
    //    bool value = (button_rows[row] & (0x08 >> col)); //step_map[row*4+col])); //pins backwards again, iterate R-L by rightwards bitshift
    bool value = MatrixDriver.digitalRead(button_map[stepnum] + 4);
    if (value != button_matrix[stepnum]) { //detect when button changes state
      button_matrix[stepnum] = value; //store button state
      if (value == 0) { //toggle step status when button is pressed in
        selectStep(stepnum); 
      }
    }
  } 
  MatrixDriver.digitalWrite(row, HIGH);

  int function_button = DisplayDriver.digitalRead(function_buttons[row]); //get pins 4-7 of display driver chip for buttons near display
  if (function_button != function_button_matrix[row]) {
    function_button_matrix[row] = function_button;
    if (function_button == 0) {
      switch(function_buttons[row]) {
        case SHIFT_PIN: //todo write shift fn
        case PLAY_PIN: //todo write play fn
        case LOAD_PIN: //todo write load fn
        case SAVE_PIN: //todo write save fn
        default:
          num_display = (row+5) * 111;
          setDisplayNum();
      }
    }
  }

  if (aux_buttons[row]) {
    int aux_button = digitalRead(aux_buttons[row]);
    if (aux_button != aux_button_matrix[row]) {
      aux_button_matrix[row] = aux_button;
      if (aux_button == 0) { //take action on pressed, not depressed
        switch(aux_buttons[row]) {
          case GLIDE_PIN: 
            glide_matrix[selected_step] = !glide_matrix[selected_step];
            DisplayDriver.digitalWrite(GLIDE_LED_PIN, glide_matrix[selected_step] ? HIGH : LOW); //glide LED
            break;
          case RECORD_PIN: //todo write record fn
          case REPEAT_PIN: //todo write repeat fn
          default: 
           num_display = (row+1) * 222;
           setDisplayNum();
        }
      }
    }
  }
}

void selectStep(unsigned int stepnum) {
  if (selected_step == stepnum || !step_matrix[stepnum]) { //require 2 presses to turn active steps off, so they can be selected/edited without double-tapping //TODO maybe implement hold-to-deactivate
    step_matrix[stepnum] = !step_matrix[stepnum];
    led_matrix[stepnum] = step_matrix[stepnum]; 
  }
  
  selected_step = stepnum;

  //update display to show currently selected step value if applicable
  switch(display_param) {
    //case TEMPO_PARAM: break
    case PITCH_PARAM:    num_display = pitch_matrix[stepnum]; break;
    case OCTAVE_PARAM:   num_display = octave_matrix[stepnum]; break;
    case DURATION_PARAM: num_display = duration_matrix[stepnum]; break;
    case CV_PARAM:       num_display = cv_matrix[stepnum]; break;
  }

  DisplayDriver.digitalWrite(GLIDE_LED_PIN, glide_matrix[stepnum]); //glide LED

  setDisplayNum();
}

//assuming dac single channel, gain=2
void setOutput(unsigned int val) {
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

void setOutput(byte channel, byte gain, byte shutdown, unsigned int val)
{
  byte lowByte = val & 0xff;
  byte highByte = ((val >> 8) & 0xff) | channel << 7 | gain << 5 | shutdown << 4;
   
  //PORTB &= 0xfb;
  digitalWrite(CS1_PIN, LOW);
  SPI.transfer(highByte);
  SPI.transfer(lowByte);
  digitalWrite(CS1_PIN, HIGH);
}


void updateDisplay() {
  DisplayDriver.digitalWrite(digit_counter, LOW); //turn off prev digit
  digit_counter++;
  if (digit_counter == 3) {
    digit_counter = 0;
  }
  DisplayDriver.writePort(1, 0xFF - (font[digit_display[digit_counter]] >> 1));
  DisplayDriver.digitalWrite(digit_counter, HIGH); //turn on new digit
}

void increment_step() {
  if (timekeeper > tempo) {
    timekeeper = 0;

    led_matrix[current_step] = step_matrix[current_step]; //reset previous LED
    current_step++;
    if (current_step == 16) {
      current_step = 0;
    }
    led_matrix[current_step] = !step_matrix[current_step]; //set current led

    if (step_matrix[current_step]) {
      //PITCH/OCTAVE
      int note = (octave_matrix[current_step] + 2) * 12 + pitch_matrix[current_step];
      setOutput(0, GAIN_2, 1, note * 11.63 * 4 * calibration_value); //convert from 0-88 to 0-4096, scale octave by calibration value
      
      //CV
      //setOutput(0, GAIN_2, 1, cv_matrix[current_step] * 40.96);
  
      //GATE
      active_step = current_step;
      //if (!gate_active) {
        digitalWrite(GATE_PIN, step_matrix[current_step] ? HIGH : LOW);
        gate_active = step_matrix[current_step];
      //}
  
      //CLOCK
      digitalWrite(CLOCK_OUT_PIN, HIGH);
      clock_active = true;
    }

  } 
}

void update_gate() {
  double percent_step = timekeeper / (double)tempo * 100.0;
  int steps_advanced = current_step - active_step + 1;
  if (steps_advanced < 1) {
    steps_advanced = current_step + sequence_length - active_step + 1;
  }
  if (gate_active && duration_matrix[active_step] < percent_step * steps_advanced) {
    digitalWrite(GATE_PIN, LOW);
    gate_active == false;
  }

  if (clock_active && timekeeper > 0) {
    digitalWrite(CLOCK_OUT_PIN, LOW);
  }
}

void multiplex_leds() {
  if (multiplex > 1) {
    multiplex = 0;

    updateMatrix(row_counter);
    readButtons(row_counter);
    row_counter++;

    if (row_counter == 4) {
      row_counter = 0;
    }

    updateDisplay();
  }
}

void blink_step() {
  if (step_matrix[selected_step]) {
    if (blinker > (led_matrix[selected_step] ? 600 : 100)) { //blink long when active
      blink_led();
      blinker = 0;
    }
  } else {
    if (blinker > (led_matrix[selected_step] ? 100 : 600)) { //blink short when inactive
      blink_led();
      blinker = 0;
    }
  }
}

void blink_led(){
  led_matrix[selected_step] = !led_matrix[selected_step];
}

void read_input() {
  if (stepper > 10) {
    stepper = 0;
    
    read_encoder();
    blink_step();
    
    
    analogMultiplexor += 1;
    if (analogMultiplexor > 4) {
      analogMultiplexor = 0;
    }
    int i = analogMultiplexor;
    if (i > 3 || i < 0) return; //sometimes we get desynced by interrupts, and analogRead on a wrong pin is fatal
    analogValues[i] = analogRead(i);

    int change_threshold = 100;
    if (display_param == analog_params[i]) {
      change_threshold = 4; //increase sensitivity when param is selected, decrease otherwise to reduce accidental "bump" changes
    }
    if (abs(analogValues[i] - lastAnalogValues[i]) > change_threshold) {
      lastAnalogValues[i] = analogValues[i];
      switch (i) {
        case 0: setPitch(analogValues[0]); break;
        case 1: setOctave(analogValues[1]); break;
        case 2: setDuration(analogValues[2]); break;
        case 3: setCV(analogValues[3]); break;
      }
      setDisplayNum();
    }

  }
}

void setDisplayNum(){
  digit_display[0] = abs(num_display) % 10;
  digit_display[1] = abs(num_display) / 10 % 10;
  digit_display[2] = abs(num_display) / 100 % 10;
  if (num_display < 0) {
    digit_display[2] = 16; //minus sign in font array
  } else if (abs(num_display) < 100) {
    digit_display[2] = 17; //blank leading zeros
  }
  if (abs(num_display) < 10) {
    digit_display[1] = 17; //blank leading zeros
  }
}

void setPitch(int analogValue) {
  display_param = PITCH_PARAM;
  //calibration_value = (float(analogValues[1]) + 1024.0) / 1500.0 ; //11.60
  int newVal = analogValue / 42.6 - 12; //convert from 0_1024 to 0_88 to -12_0_12
  if (pitch_matrix[selected_step] != newVal) num_display = newVal;
  pitch_matrix[selected_step] = newVal;
}

void setOctave(int analogValue) {
  display_param = OCTAVE_PARAM;
  int newVal = analogValue / 120 - 4; //convert from 0-1024 to -4_0_4
  if (octave_matrix[selected_step] != newVal) num_display = newVal;
  octave_matrix[selected_step] = newVal;
}

void setDuration(long analogValue) { //need extra bits for exponent operation
  display_param = DURATION_PARAM;
  int newVal = analogValue * analogValue / 2615; //convert from 0-1024 to 0-400 with exponential curve
  if (duration_matrix[selected_step] != newVal) num_display = newVal;
  duration_matrix[selected_step] = newVal;
}
void setCV(int analogValue) {
  display_param = CV_PARAM;
  int newVal = analogValue / 10.23; //convert from 0-1024 to 0-100
  if (cv_matrix[selected_step] != newVal) num_display = newVal;
  cv_matrix[selected_step] = newVal;
}

//magic numbers from https://www.circuitsathome.com/mcu/reading-rotary-encoder-on-arduino/
void read_encoder() {
///* returns change in encoder state (-1,0,1) */
//int8_t read_encoder()
//{
  static int8_t enc_states[] = {0,-1,1,0,1,0,0,-1,-1,0,0,1,0,1,-1,0};
  static uint8_t old_AB = 0;
  /**/
  old_AB <<= 2;                   //remember previous state
  old_AB |= ( ENC_PORT & 0x30 ) >> 4;  //add current state from pins A5 and A4, shifted to LSB
  encoder_increment ( enc_states[( old_AB & 0x0f )] * -1); //extract encrement/decrement value from state table
}

void encoder_increment(int amt) {
  if (amt == 0) return;
  tempo += amt;
  if (tempo < 20) tempo = 20;
  if (tempo > 500) tempo = 500;
  num_display = tempo;
  display_param = TEMPO_PARAM;
  setDisplayNum();
}

void loop() {
  increment_step();
  multiplex_leds();
  read_input();
  update_gate();
}
