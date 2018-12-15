#pragma once

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

#define CS0_PIN  10
#define CS1_PIN  9
#define GAIN_1  0x1
#define GAIN_2  0x0

#define ENC_A_PIN A4
#define ENC_B_PIN A5