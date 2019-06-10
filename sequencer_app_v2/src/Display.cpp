#include "Variables.h"
#include "Pinout.h"
#include "Font.h"
#include "Display.h"
#include <string.h>

namespace supersixteen{

int digit_counter = 0;
int digit_display[3] = { 0, 0, 0 };

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
}

void Display::setDisplayAlpha(const char displayAlpha[]){ //turns 3-character array "MAJ" into ascii indexes
	digit_display[0] = displayAlpha[0] - 55;
	digit_display[1] = displayAlpha[1] - 55;
	digit_display[2] = displayAlpha[2] - 55;
}


void Display::updateSevenSegmentDisplay(){
	SPI.transfer(~(alphabet[digit_display[2-digit_counter]])); 
	digit_counter++;
	if (digit_counter == 3) {
		digit_counter = 0;
	}
}
}