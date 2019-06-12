#pragma once

namespace supersixteen{

#define GATE_PIN PD0
//#define LDAC_PIN 1
#define CLOCK_OUT_PIN PD2
#define CLOCK_IN_PIN PD3
#define RESET_PIN PD4


#define GLIDE_LED_PIN 15 //these 8 are on the button driver chip, not the MCU
#define GLIDE_PIN 14
#define REPEAT_PIN 13
#define RECORD_PIN 12
#define SAVE_PIN 11 
#define LOAD_PIN 10 
#define PLAY_PIN 9 
#define SHIFT_PIN 8

#define CS0_PIN  10 //MCP23S17 for buttons, aux LEDS
#define CS1_PIN  9  //Display/matrix shift registers
#define CS2_PIN  8  //Flash memory
#define CS3_PIN  PD1  //DAC

  //DIGIT DISPLAY PINS
#define DIGIT_1_PIN PD5
#define DIGIT_2_PIN PD7
#define DIGIT_3_PIN PD6 

#define GAIN_1  0x1
#define GAIN_2  0x0

#define ANALOG_PIN_1 A0
#define ANALOG_PIN_2 A1
#define ANALOG_PIN_3 A2
#define ANALOG_PIN_4 A3

#define ENC_A_PIN A4
#define ENC_B_PIN A5
#define ENC_PORT PINC

}