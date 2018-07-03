#include <MCP23S17.h> //https://github.com/MajenkoLibraries/MCP23S17
#include <SPI.h>


const int DAC_CS = 9;
const uint8_t MCP_CS = 10;
int counter = 0;
int digit = 2;
int counterTime = 0;
int num_display = 0;
int digit_display = 0;
int lastAnalogValue = 0;
int analogValue = 0;

const uint8_t truth_table[10] = {
  0x7E,
  0x30,
  0x6D,
  0x79,
  0x33,
  0x5B,
  0x5F,
  0x70,
  0x7F
};

MCP23S17 Bank1(&SPI, MCP_CS, 0);

void setup() {
  for (int pin = 2; pin < 5; pin++){
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
  }
  
  Bank1.begin();
  for (int pin = 0; pin < 8; pin++){
    Bank1.pinMode(pin, OUTPUT);
    //Bank1.digitalWrite(pin, LOW);
    Bank1.writePort(0, 0xFF);
  }
}


void loop() {
  
    digitalWrite(digit, LOW);
    
    counterTime += 1;
    
    digit += 1;
    if (digit == 5) {
      digit = 2;
    }
    num_display = 0;

    if (digit == 4) {
      digit_display = num_display % 10;
    } else if (digit == 3) {
      digit_display = num_display / 10 % 10;
    } else {
      digit_display = num_display / 100 % 10;
    }

    digit_display = counterTime;
   
    Bank1.writePort(0, 0xFF - truth_table[digit_display]);

    delay(analogRead(0));

    digitalWrite(digit, HIGH);
    
}



