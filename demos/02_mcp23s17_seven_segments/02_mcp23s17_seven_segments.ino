#include <MCP23S17.h> //https://github.com/MajenkoLibraries/MCP23S17
#include <SPI.h>


const int DAC_CS = 9;
const uint8_t MCP_CS = 10;
int counter = 0;
int digit = 0;
int counterTime = 0;
int num_display = 0;
int digit_display = 0;
int lastAnalogValue = 0;
int analogValue = 0;
bool led = 0;

const uint8_t truth_table[11] = {
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
    B0011111,   // [12] => "b"
    B1001110,   // [13] => "C"
    B0111101,   // [14] => "d"
    B1001111,   // [15] => "E"
    B1000111,   // [16] => "F"
    B0000001,   // [17] => "dash"
};

uint8_t font[18];

MCP23S17 Bank1(&SPI, MCP_CS, 0);

void setup() {
  pinMode(0, OUTPUT);
  Bank1.begin();
    
  for (int pin = 0; pin < 8; pin++){
    Bank1.pinMode(pin, OUTPUT);
    Bank1.digitalWrite(pin, LOW);
  }
  

  for (int pin = 8; pin < 16; pin++){
    Bank1.pinMode(pin, OUTPUT);
    //Bank1.digitalWrite(pin, LOW);
    Bank1.writePort(1, 0xFF);
  }

  for (int pin = 0; pin < 17; pin++){
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


void loop() {
//    led = !led;
//    digitalWrite(0, led ? HIGH : LOW);   // turn the LED on (HIGH is the voltage level)

    Bank1.digitalWrite(digit, LOW);
        

    //num_display = analogRead(0);
    analogValue = analogRead(0);
    if (abs(lastAnalogValue - analogValue) > 3) {
      lastAnalogValue = analogValue;
      num_display = lastAnalogValue / 4;
    }

    digit += 1;
    if (digit == 3) {
      digit = 0;
    }

    if (digit == 0) {
      digit_display = num_display % 10;
    } else if (digit == 1) {
      digit_display = num_display / 10 % 10;
    } else {
      digit_display = num_display / 100 % 10;
    }

//    digit_display += 1;
//    if (digit_display == 17) {
//      digit_display = 0;
//      digit += 1;
//      if (digit == 3) {
//        digit = 0;
//      }
//    }
//    
    Bank1.writePort(1, 0xFF); //hard-reset previous pins
    Bank1.writePort(1, 0xFF - (font[digit_display] >> 1));
    //Bank1.writePort(1, 0xFF-(0x01 << digit_display));
    Bank1.digitalWrite(digit, HIGH);
    

    delay(1);
}
