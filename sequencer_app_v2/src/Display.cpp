#include "Variables.h"
#include "Pinout.h"
#include "Font.h"

extern int digit_counter = 0;
extern int digit_display[3] = { 0, 0, 0 };

void setDisplayNum() {
	digit_display[0] = abs(num_display) % 10;
	digit_display[1] = abs(num_display) / 10 % 10;
	digit_display[2] = abs(num_display) / 100 % 10;
	if (num_display < 0) {
		digit_display[2] = 37; //minus sign in font array
	}
	else if (abs(num_display) < 100) {
		digit_display[2] = 36; //blank leading zeros
	}
	if (abs(num_display) < 10) {
		digit_display[1] = 36; //blank leading zeros
	}
}