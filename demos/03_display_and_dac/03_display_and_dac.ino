#include <MCP23S17.h> //https://github.com/MajenkoLibraries/MCP23S17
#include <SPI.h>
#include <elapsedMillis.h>

elapsedMillis timeElapsed;
elapsedMillis timeElapsed2;

const int DAC_CS = 9;
const uint8_t MCP_CS = 10;
const int SINK_PIN = 8;
int counter = 0;
int digit = 0;
int counterTime = 0;
int num_display = 0;
int digit_display = 0;
int lastAnalogValues[2];
int analogValues[2];
float calibration_value = 1.0;

#define SEG_A 2
#define SEG_B 3
#define SEG_C 4
#define SEG_D 5
#define SEG_E 6
#define SEG_F 7
#define SEG_G 8
#define DEC_P 12

#define SET1 9 // [!][!][x]
#define SET2 10// [!][x][!]
#define SET3 11// [x][!][!]

const uint8_t truth_table1[10] = {
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
    Bank1.writePort(0, 0x00);
  }

  Bank1.pinMode(SINK_PIN, OUTPUT);
  Bank1.pinMode(SINK_PIN+1, OUTPUT);
  Bank1.pinMode(SINK_PIN+2, OUTPUT);
  Bank1.writePort(1, 0xFF);

  pinMode(DAC_CS, OUTPUT);
  //SPI.begin();  
  //SPI.setClockDivider(SPI_CLOCK_DIV2);
}

//assuming dac single channel, gain=2
void setOutput(unsigned int val)
{
  if (val > 4095){
    val = 4095;
  } else if (val < 0) {
    val = 0;
  }
  byte lowByte = val & 0xff;
  byte highByte = ((val >> 8) & 0xff) | 0x10;

  digitalWrite(DAC_CS, LOW);
  PORTB &= 0xfb;
  SPI.transfer(highByte);
  SPI.transfer(lowByte);
  PORTB |= 0x4;
  digitalWrite(DAC_CS, HIGH);
}
 

void loop() {
  updateDisplay();
  readInput();
}

void readInput(){
  if (timeElapsed2 > 10) {
    timeElapsed2 = 0;
    analogValues[0] = analogRead(0);
    analogValues[1] = analogRead(1);

    for(int i = 0; i < 2; i++) {
      if (abs(analogValues[i] - lastAnalogValues[i]) > 4) {
        lastAnalogValues[i] = analogValues[i];
        num_display = analogValues[0] / 11.60;
        calibration_value = (float(analogValues[1]) + 1024.0) / 1500.0;
        setOutput(num_display * 11.63 * 4 * calibration_value);
      }
    }
  }
}

void updateDisplay(){
  if (timeElapsed > 0){
    Bank1.digitalWrite(digit + SINK_PIN, LOW);
    timeElapsed = 0;
    
    digit += 1;
    if (digit == 3) {
      digit = 0;
    }

    if (digit == 2) {
      digit_display = num_display % 10;
    } else if (digit == 1) {
      digit_display = num_display / 10 % 10;
    } else {
      digit_display = num_display / 100 % 10;
    }


    Bank1.writePort(0, 0xFF - truth_table1[digit_display]);
    
    Bank1.digitalWrite(digit + SINK_PIN, HIGH);
  }
}



