#include <MCP23S17.h> //https://github.com/MajenkoLibraries/MCP23S17
#include <SPI.h>
//#include <elapsedMillis.h>

//elapsedMillis timeElapsed;



const uint8_t chipSelect = 10;
uint8_t button = 0;
int addr = 0;
int rows[4] = {1, 2, 4, 8};
MCP23S17 Bank3(&SPI, chipSelect, 1);
bool matrix[16]; // = {1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1};
uint8_t byte1;
uint8_t byte2;
int counter = 0;


void setup() {
   
  Bank3.begin();
  for (int pin = 0; pin <= 16; pin++){
    Bank3.pinMode(pin, OUTPUT);
    Bank3.digitalWrite(pin, LOW);
    //Bank3.writePort(0, 0xFF);
  }
  Bank3.writePort(0x0F00);
  //Bank3.writePort(0, 0x00); //first 4 bits are led sink - pins 0-3
  //Bank3.writePort(1, 0x0F); //last 4 bits are led source - pins 13-16
  
}



void updateMatrix(int col) {
  byte1 = 0x00;
  for (int row = 0; row < 4; row++){
    byte1 = byte1 + (matrix[(3-row)+col*4] << row); // ^ 0xFF; //invert since we want to pull low for active drain
    //byte2 = rows[col];
    //byte2 = 0xFF;
  }
  byte1 = 0xFF - byte1 << 4;
  byte2 = rows[col];
  
  
  //turn on new light;
  uint16_t doublebyte = (byte2 << 8) + byte1;
  Bank3.writePort(0xF0F0); //hard-reset previous pins
  //Bank3.writePort(doublebyte);
  Bank3.writePort(1, byte2);
  Bank3.writePort(0, byte1);
  //button += 1;
}

void loop() {
  counter += 1;
  if (counter > 2000) {
    counter = 0;
    matrix[button] = 0;
    button += 1;
    if (button == 16) {
      button = 0;
    }
    matrix[button] = 1;
  }

  //if (counter % 1000 == 0) {
    updateMatrix(counter % 4);
  //}
}

