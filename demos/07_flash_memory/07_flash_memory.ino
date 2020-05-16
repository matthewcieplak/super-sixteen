#include <SerialFlash.h>

SerialFlashFile file;
#define CS0_PIN  10 //MCP23S17 for buttons, aux LEDS
#define CS1_PIN  9  //Display/matrix shift registers
#define CS2_PIN  8  //Flash memory
#define CS3_PIN  1  //DAC PD1

bool flash_working = false;
#define LEDPIN PD0;

void setup() {
  pinMode(CS0_PIN, OUTPUT);
  pinMode(CS1_PIN, OUTPUT);
  pinMode(CS2_PIN, OUTPUT);
  pinMode(CS3_PIN, OUTPUT);
  pinMode(PD0, OUTPUT);

  digitalWrite(CS0_PIN, HIGH);
  digitalWrite(CS1_PIN, HIGH);
  digitalWrite(CS2_PIN, LOW);
  digitalWrite(CS3_PIN, HIGH);
  
  unsigned char id[256];
  //if (SerialFlash.begin(CS0_PIN)){ //to force a failure, use this line and try to find the flash chip on the wrong pin
  
  if (SerialFlash.begin(CS2_PIN)){ //to test the flash chip, look for it on the CS2 serial pin
    
    SerialFlash.readID(id);
    if (id[0] == 0xEF && id[1] == 0x40 && id[2] == 0x14) {
      //Winbond W25Q80BV;
      flash_working = true;
    } else {
      flash_working = false;
    }
  } else {
      flash_working = false;
  }
}

void loop() {
  digitalWrite(PD0, HIGH);
  delay(flash_working ? 1000 : 100);
 
  digitalWrite(PD0, LOW);
  delay(100);
}
