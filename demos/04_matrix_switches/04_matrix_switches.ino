#include <MCP23S17.h> //https://github.com/MajenkoLibraries/MCP23S17
#include <SPI.h>
#include <elapsedMillis.h>

elapsedMillis multiplex;
elapsedMillis stepper;


const uint8_t chipSelect = 10;
uint8_t button = 0;
const int led_rows[4] = {0x01, 0x02, 0x04, 0x08};
const int button_rows[4] = {0xE0, 0xD0, 0xB0, 0x70};
MCP23S17 Bank3(&SPI, chipSelect, 1);
bool step_matrix[16]   = {1,0,0,0, 1,1,0,0, 1,1,1,0, 1,1,1,1};
bool led_matrix[16]    = {1,0,0,0, 1,1,0,0, 1,1,1,0, 1,1,1,1};
bool button_matrix[16] = {1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1};
uint8_t byte1;
uint8_t byte2;
int counter    = 0;
int col        = 0;
int stepnumber = 1;
int steplength = 0;


void setup() {
  //SPI.beginTransaction(SPISettings(10000000, MSBFIRST, SPI_MODE0));
  Bank3.begin();
  for (int pin = 0; pin < 16; pin++){
    if (pin < 4) {
        Bank3.pinMode(pin, INPUT_PULLUP); //use these for reading in buttons
    } else {
      Bank3.pinMode(pin, OUTPUT);
      Bank3.digitalWrite(pin, LOW);
    }    
  }

  Bank3.writePort(0x0F00);
  //Bank3.writePort(0, 0x00); //first 4 bits are led sink - pins 0-3
  //Bank3.writePort(1, 0x0F); //last 4 bits are led source - pins 13-16
}



void updateMatrix(int row) {
  byte1 = 0x00;
  for (col = 0; col < 4; col++){
    byte1 += ( led_matrix[row*4+(3-col)] << col); //columns are wired backwards R-L on my board, hence 3-col
  }
  byte1 = 0xFF - byte1 << 4; //invert since we want to pull low for active drain
  byte2 = led_rows[row] + button_rows[row]; //while we're using the serial port, pull one output low for buttons so we can read them on the pullup inputs
  
  //turn on new light;
  uint16_t doublebyte = (byte2 << 8) + byte1;
  Bank3.writePort(0xF0F0); //hard-reset previous pins
  Bank3.writePort(doublebyte);
  //Bank3.writePort(1, byte2);
  //Bank3.writePort(0, byte1);
}

void readButtons(int row) {
  byte button_row = Bank3.readPort(0) & 0x0F; // (mask since we only want the last 4 bits, pins 4-7);
  for (int i = 0; i < 4; i++) {
    int stepnum = row*4+ i;
    // check each bit in the byte and see if it's pulled low
    bool value = (button_row & (8 >> i)); //pins backwards again, iterate R-L by rightwards bitshift
    if (value != button_matrix[stepnum]) {
      button_matrix[stepnum] = value; //store button state
      if (value == 0){ //toggle step status when button is pressed in
        step_matrix[stepnum] = !step_matrix[stepnum];
        led_matrix[stepnum] = !led_matrix[stepnum];
      }
    }
//    byte value = Bank3.digitalRead(3-i);
//    if (value != button_matrix[stepnum]) { //detect when button changes state
//      button_matrix[stepnum] = value; //store button state
//      if (value == 0){ //toggle step status when button is pressed in
//        step_matrix[stepnum] = !step_matrix[stepnum];
//        led_matrix[stepnum] = !led_matrix[stepnum];
//      }
//    }
     
  }
}

void loop() {
  if (stepper > 150) {
    stepper = 0;
    led_matrix[button] = step_matrix[button];
    button++;
    if (button == 16) {
      button = 0;
    }
    
    led_matrix[button] = !step_matrix[button];
  }
  

  if (multiplex > 1) {
    //steplength = analogRead(0);
    updateMatrix(counter);
    readButtons(3-counter);
    multiplex = 0;
    counter += 1;
    if (counter == 4) {
      counter = 0;    
    }
  }
}

