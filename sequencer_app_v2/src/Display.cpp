#include "Variables.h"
#include "Pinout.h"
#include "Font.h"

int digit_counter = 0;
int digit_display[3] = { 0, 0, 0 };


void initializeDisplay() {
	DisplayDriver.begin();
	for (int pin = 0; pin < 16; pin++) {
		if (pin >= 4 && pin < 8) {
			DisplayDriver.pinMode(pin, INPUT_PULLUP); //use these for reading in 4 top buttons
		} else {
			DisplayDriver.pinMode(pin, OUTPUT);
		}
		//DisplayDriver.writePort(0x0000);
	}
}

void updateDisplay() {
	DisplayDriver.digitalWrite(digit_counter, LOW); //turn off prev digit
	digit_counter++;
	if (digit_counter == 3) {
		digit_counter = 0;
	}
	DisplayDriver.writePort(1, 0xFF - (font[digit_display[digit_counter]] >> 1));
	DisplayDriver.digitalWrite(digit_counter, HIGH); //turn on new digit
}

void setDisplayNum() {
	digit_display[0] = abs(num_display) % 10;
	digit_display[1] = abs(num_display) / 10 % 10;
	digit_display[2] = abs(num_display) / 100 % 10;
	if (num_display < 0) {
		digit_display[2] = 16; //minus sign in font array
	}
	else if (abs(num_display) < 100) {
		digit_display[2] = 17; //blank leading zeros
	}
	if (abs(num_display) < 10) {
		digit_display[1] = 17; //blank leading zeros
	}
}