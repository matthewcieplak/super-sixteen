#include "Variables.h"
#include "Pinout.h"
#include "Font.h"
#include "Display.h"
#include <string.h>

namespace supersixteen{

int digit_counter = 0;
int num_display = 999;
int digit_display[3] = { 0, 0, 0 };
int digit_pins[3] = { DIGIT_1_PIN, DIGIT_2_PIN, DIGIT_3_PIN };
uint8_t alpha_display[3] = { 0, 0, 0 };

bool decimal = 1;
bool blinking = false;
bool blink_state = true;
const uint8_t decimalChar = 0x01;
elapsedMillis display_blinker;
int blink_interval;
int blink_cycles_elapsed;
int blink_cycles_timeout;


void Display::init(){
	pinMode(DIGIT_1_PIN, OUTPUT);
	pinMode(DIGIT_2_PIN, OUTPUT);
	pinMode(DIGIT_3_PIN, OUTPUT);
	digitalWrite(DIGIT_1_PIN, LOW);
	digitalWrite(DIGIT_2_PIN, LOW);
	digitalWrite(DIGIT_3_PIN, LOW);
	setDisplayNum(0);
}

void Display::setDisplayNum(int displayNum){
	if (num_display == displayNum) return;
	num_display	= displayNum;
	digit_display[2] = abs(num_display) % 10;
	digit_display[1] = abs(num_display) / 10 % 10;
	digit_display[0] = abs(num_display) / 100 % 10;
	if (num_display < 0) {
		digit_display[0] = 37; //minus sign in font array
	}
	else if (abs(num_display) < 100) {
		digit_display[0] = 36; //blank leading zeros
	}
	if (abs(num_display) < 10) {
		digit_display[1] = 36; //blank leading zeros
	}

	alpha_display[2] = ~alphabet[digit_display[0]];
	alpha_display[1] = ~alphabet[digit_display[1]];
	alpha_display[0] = ~alphabet[digit_display[2]];
	appendDecimal();
}

void Display::setDisplayAlpha(const char displayAlpha[]){ //turns 3-character array "MAJ" into ascii indexes that correspond to abbreviated font
	digit_display[0] = displayAlpha[0] - 55;
	digit_display[1] = displayAlpha[1] - 55;
	digit_display[2] = displayAlpha[2] - 55;

	alpha_display[2] = ~alphabet[digit_display[0]];
	alpha_display[1] = ~alphabet[digit_display[1]];
	alpha_display[0] = ~alphabet[digit_display[2]];
	appendDecimal();
}	


void Display::updateSevenSegmentDisplay(){
	digitalWrite(digit_pins[digit_counter], HIGH);
	SPI.transfer(alpha_display[digit_counter]); 
	// nextDigit();
}

void Display::blankSevenSegmentDisplay(){
	digitalWrite(digit_pins[digit_counter], HIGH);
	//nextDigit();
}

void Display::nextDigit(){
	digit_counter++;
	if (digit_counter == 3) {
		digit_counter = 0;
	}
	if (blinking && display_blinker > blink_interval) {
		if (display_blinker > blink_interval * 2) {
			display_blinker = 0;
			blink_cycles_elapsed += 1;
			
			if (blink_cycles_timeout > 0 && blink_cycles_elapsed > blink_cycles_timeout) {
				blinking = false;
				blink_cycles_elapsed = 0;
			}
		}
		//leave digit inactive
	} else {
		digitalWrite(digit_pins[digit_counter], LOW);
	}
}

void Display::setDecimal(bool decimalState){
	decimal = decimalState;
	appendDecimal();
}

void Display::appendDecimal(){
	if (decimal) {
		alpha_display[0] = alpha_display[0] | decimalChar;
	} else {
		alpha_display[0] = alpha_display[0] & ~decimalChar;
	}
}

void Display::blinkDisplay(bool is_blinking, int interval, int cycles){
	blinking = is_blinking;
	display_blinker = 0;
	blink_interval = interval;
	blink_cycles_timeout = cycles;
}
}