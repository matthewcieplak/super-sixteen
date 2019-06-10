#include <Arduino.h>
#include "Variables.h"
#include "Pinout.h"
#include "Display.h"
#include "LEDMatrix.h"
#include "Buttons.h"
#include "Font.h"

namespace supersixteen{

int step_map[16] = { 3, 2, 1, 0, 0, 1, 2 ,3, 3, 2, 1, 0, 0, 1, 2, 3 }; //rows are wired symmetrically rather than sequentially
const int led_rows[4] = { 0x80, 0x40, 0x20, 0x10 };
uint8_t byte1;
uint8_t byte2;
int digit_pins[3] = { DIGIT_1_PIN, DIGIT_2_PIN, DIGIT_3_PIN };
int row_counter = 0;
int col = 0;
int j;
byte dataToSend;
int digit_counter = 0;
Display* displayVar;	

void LedMatrix::init(Display& display) {
	pinMode(CS1_PIN, OUTPUT);
	SPI.begin();
	displayVar = &display;
}

void LedMatrix::updateMatrix(int row) {
	byte1 = (1 << (7-row)); //turn on row driver
	byte2 = 0xF0;
	for (col = 0; col < 4; col++) {
		byte2 +=  (led_matrix[row * 4 + col] << step_map[row * 4 + col]);
	}
	byte1 += ~byte2;
	digitalWrite(CS1_PIN, LOW);
	digitalWrite(digit_pins[digit_counter], HIGH);
	SPI.setBitOrder(LSBFIRST); //shift registers like LSB
	displayVar->updateSevenSegmentDisplay(); //has to happen HERE bc it's part of the shift register 2-byte sequence
	SPI.transfer(~byte1); //led matrix
	digitalWrite(CS1_PIN, HIGH);
	digitalWrite(digit_pins[digit_counter], LOW);

}

void LedMatrix::multiplex_leds() {
	if (multiplex > 0) {
		multiplex = 0;

		updateMatrix(row_counter);
		//readButtons(row_counter);
		row_counter++;

		if (row_counter == 4) {
			row_counter = 0;
		}

		//updateDisplay();
	}
}

void LedMatrix::blink_step() {
	if (step_matrix[selected_step]) {
		if (blinker > (led_matrix[selected_step] ? 600 : 100)) { //blink long when active
			blink_led();
			blinker = 0;
		}
	}
	else {
		if (blinker > (led_matrix[selected_step] ? 100 : 600)) { //blink short when inactive
			blink_led();
			blinker = 0;
		}
	}
}

void LedMatrix::blink_led() {
	led_matrix[selected_step] = !led_matrix[selected_step];
}

void LedMatrix::reset(){
	for (int i = 0; i < 16; i++) {
		led_matrix[i] = 0;
	}
}

void LedMatrix::toggleLed(int led){
	led_matrix[led] = ~led_matrix[led];
}



}