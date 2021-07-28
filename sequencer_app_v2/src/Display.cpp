#include "Variables.h"
#include "Pinout.h"
#include "Font.h"
#include "Display.h"
#include <string.h>

namespace supersixteen{

const bool COMMON_ANODE = true; //BA56-12GWA change to FALSE for common cathode display BC56-12GWA

int digit_counter = 0;
int delay_counter = 0; //used for inserting blank cycles to reduce brightness
int num_display = 999;
int digit_display[3] = { 0, 0, 0 };
int digit_pins[3] = { DIGIT_1_PIN, DIGIT_2_PIN, DIGIT_3_PIN };
uint8_t alpha_display[3] = { 0, 0, 0 };

//const uint8_t alphabet[40]= {
const uint8_t dim_mask_1 = 0B00001111;
const uint8_t dim_mask_2 = 0B11110000;

bool decimal = 0;
bool blinking = false;
bool blink_state = true;
const uint8_t decimalChar = 0x01;
elapsedMillis display_blinker;
uint16_t blink_interval;
int blink_cycles_elapsed;
int blink_cycles_timeout;

int brightness = 0;

// static const byte startup_sequence[16] = {
// 	0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x00, 0x00, 0x00 //, 0x08, 0x10, 0x20, 0x40, 0x80, 0x91, 0xA2, 0xF4, 0xF8
// };

static const byte startup_sequence[16] = {
	0x10, 0x10, 0x10, 0x20, 0x40, 0x80, 0x80, 0x80, 0x04,
	 0x08, 0x10, 0x10, 0x10, 0x00//0x02, 0x02, 0x02, 0x00
};

static const byte startup_digits[16] = {
    0x01, 0x02, 0x04, 0x04, 0x04, 0x04, 0x02, 0x01, 0x01,
	 0x01, 0x01, 0x02, 0x04, 0x00
};


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

void Display::setDisplayAlphaVar(char displayAlpha[]){
	const char displayAlphaConst[4] = {char(displayAlpha[0]), char(displayAlpha[1]), char(displayAlpha[2])};
	setDisplayAlpha(displayAlphaConst);
}

void Display::setDisplayAlpha(const char displayAlpha[]){ //turns 3-character array "MAJ" into ascii indexes that correspond to abbreviated font
	num_display = 999; //prime variable for easy reset when changed again
	for (int i = 0; i < 3; i++) {
		if (displayAlpha[i] == 98) {
			digit_display[i] = 11; //~alphabet[11]; //lowercase "b" as flat symbol
		} else if (displayAlpha[i] == 32) {
			digit_display[i] = 36; //~alphabet[36]; //space
		} else if (displayAlpha[i] > 64) { //convert a-z characters
			digit_display[i] = displayAlpha[i] - 55;
		} else if (displayAlpha[i] >= 48) {//convert 0-9 characters
			digit_display[i] = displayAlpha[i] - 48;
		} 
		alpha_display[2-i] = ~alphabet[digit_display[i]];
	}
	appendDecimal();
}	


void Display::updateSevenSegmentDisplay(){
	digitalWrite(digit_pins[digit_counter], COMMON_ANODE ? HIGH : LOW );

	if (currentDigitVisible()) {
		SPI.transfer(COMMON_ANODE ? alpha_display[digit_counter] : ~alpha_display[digit_counter]);
	} else {
		SPI.transfer(COMMON_ANODE ? 
		            ~(~(alpha_display[digit_counter]) & (delay_counter == 0 ? dim_mask_1 : dim_mask_2)) : 
					~(alpha_display[digit_counter] ^ (delay_counter == 0 ? dim_mask_1 : dim_mask_2))  
					);
		// SPI.transfer(COMMON_ANODE ? 0xFF : 0x00);
	} 



	// nextDigit();
}

bool Display::currentDigitVisible(){
	if (brightness == 0) { return true; 
	} else if (brightness == 1) {
		if (delay_counter == digit_counter) return false;
	} else if (brightness == 2) {
		if (delay_counter == 1) return false;
	} else if (brightness == 3) {
		if (delay_counter != digit_counter) return false;
	} else if (brightness == 4) {
		if (delay_counter > 0) return false;
	}
	return true;
}

void Display::blankSevenSegmentDisplay(){
	digitalWrite(digit_pins[digit_counter], COMMON_ANODE ? HIGH : LOW);
	//nextDigit();
}

void Display::nextDigit(){

	// if (brightness > 0 && delay_counter < brightness) {
	// 	delay_counter ++;
	// 	return;
	// } else {
	// 	delay_counter = 0;
	// }

	digit_counter++;
	if (digit_counter > 2) {
		digit_counter = 0;
		delay_counter++;
		if (delay_counter > 2) {
			delay_counter = 0;
		}
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
		digitalWrite(digit_pins[digit_counter], COMMON_ANODE ? LOW : HIGH); //de-activate digit
	}
}

void Display::setDecimal(bool decimalState){
	decimal = decimalState;
	appendDecimal();
}

void Display::appendDecimal(){
	if (decimal) {
		alpha_display[0] = alpha_display[0] & ~decimalChar;
	} else {
		alpha_display[0] = alpha_display[0] | decimalChar;
	}
}

void Display::blinkDisplay(bool is_blinking, int interval, int cycles){
	blinking = is_blinking;
	display_blinker = 0;
	blink_interval = interval;
	blink_cycles_timeout = cycles;
}



void Display::startupSequence(){
		for (byte i = 0; i < 14; i++) {
			digitalWrite(CS1_PIN, LOW);
			digitalWrite(digit_pins[0], (startup_digits[i] & 0x01) == 0 ? HIGH : LOW);
			digitalWrite(digit_pins[2], (startup_digits[i] & 0x02) == 0 ? HIGH : LOW);
			digitalWrite(digit_pins[1], (startup_digits[i] & 0x04) == 0 ? HIGH : LOW);
			SPI.setBitOrder(LSBFIRST); //shift registers like LSB
			SPI.transfer(~startup_sequence[i]);
			SPI.transfer(0x00); //led matrix
			digitalWrite(CS1_PIN, HIGH);
			delay(70);
		}
		delay(100);
	
}

void Display::setBrightness(int new_brightness){
	//if (new_brightness >= 0 && new_brightness <= 4) {
		brightness = new_brightness;
	//}
}

}