#include "Variables.h"
#include "Pinout.h"
#include "Font.h"
#include <string.h>


int digit_counter = 0;
extern int digit_display[3] = { 0, 0, 0 };

void setDisplayNum(int displayNum){
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

void setDisplayAlpha(char displayAlpha[]){ //turns 3-character array "MAJ" into ascii indexes
	digit_display[0] = displayAlpha[0] - 55;
	digit_display[1] = displayAlpha[1] - 55;
	digit_display[2] = displayAlpha[2] - 55;
}


void updateSevenSegmentDisplay(){
	SPI.transfer(~(alphabet[digit_display[2-digit_counter]])); 
	digit_counter++;
	if (digit_counter == 3) {
		digit_counter = 0;
	}
}
