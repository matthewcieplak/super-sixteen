#include <SPI.h>
 
const int PIN_CS = 1;
const int GAIN_1 = 0x1;
const int GAIN_2 = 0x0;
int output_v = 0;
int tempo = 10000;
int pattern = 0;
int stepper = 0;
uint32_t counter = 0;
    

void setup()
{
  pinMode(PIN_CS, OUTPUT);
  pinMode(3, OUTPUT); //LDAC
  digitalWrite(3, LOW);
  SPI.begin();  
  SPI.setClockDivider(SPI_CLOCK_DIV2);
}
 
//assuming single channel, gain=2
void setOutput(unsigned int val)
{
  byte lowByte = val & 0xff;
  byte highByte = ((val >> 8) & 0xff) | 0x10;
   
  PORTB &= 0xfb;
  SPI.transfer(highByte);
  SPI.transfer(lowByte);
  PORTB |= 0x4;
}
 
void setOutput(byte channel, byte gain, byte shutdown, unsigned int val)
{
  byte lowByte = val & 0xff;
  byte highByte = ((val >> 8) & 0xff) | channel << 7 | gain << 5 | shutdown << 4;
   
  //PORTB &= 0xfb;
  digitalWrite(PIN_CS, LOW);
  SPI.transfer(highByte);
  SPI.transfer(lowByte);
  digitalWrite(PIN_CS, HIGH);
}


void loop()
{
 //high-res triangular wave
 for (int i=0; i < 4096; i+=32)   
 {
  setOutput(0, GAIN_2, 1, i);
  setOutput(1, GAIN_2, 1, i);
  //setOutput(i);
  delay(10);
 }
}
