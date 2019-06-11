#include "Variables.h"
#include "Pinout.h"
#include "Font.h"
#include "Display.h"
#include <string.h>

namespace supersixteen{

int digit_counter = 0;
int num_display = 0;
int digit_display[3] = { 0, 0, 0 };
int digit_pins[3] = { DIGIT_1_PIN, DIGIT_2_PIN, DIGIT_3_PIN };
uint8_t alpha_display[3] = { 0, 0, 0 };

bool decimal = 0;
const uint8_t decimalChar = 0x01;


void Display::init(){
	pinMode(DIGIT_1_PIN, OUTPUT);
	pinMode(DIGIT_2_PIN, OUTPUT);
	pinMode(DIGIT_3_PIN, OUTPUT);
	digitalWrite(DIGIT_1_PIN, LOW);
	digitalWrite(DIGIT_2_PIN, LOW);
	digitalWrite(DIGIT_3_PIN, LOW);
}

void Display::setDisplayNum(int displayNum){
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
}

void Display::nextDigit(){
	digit_counter++;
	if (digit_counter == 3) {
		digit_counter = 0;
	}
	digitalWrite(digit_pins[digit_counter], LOW);
}

void Display::setDecimal(bool decimalState){
	decimal = decimalState;
}

void Display::appendDecimal(){
	if (decimal) {
		alpha_display[0] = alpha_display[0] - decimalChar;
	} else {
		alpha_display[0] = alpha_display[0] | decimalChar;
	}
}
}