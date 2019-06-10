#include <Arduino.h>
#include "Variables.h"
#include "Pinout.h"
#include "Display.h"
#include "Sequencer.h"
#include "LEDMatrix.h"
#include "Buttons.h"
#include "Font.h"

namespace supersixteen{

elapsedMillis multiplex;
bool led_matrix[16];
int step_map[16] = { 3, 2, 1, 0, 0, 1, 2 ,3, 3, 2, 1, 0, 0, 1, 2, 3 }; //rows are wired symmetrically rather than sequentially

uint8_t byte1;
uint8_t byte2;

int row_counter = 0;
int col = 0;
int j;

byte dataToSend;

Display* displayVar3;	
Sequencer* sequencerVar3;

void LedMatrix::init(Display& display, Sequencer& sequencer) {
	pinMode(CS1_PIN, OUTPUT);
	SPI.begin();
	displayVar3 = &display;
	sequencerVar3 = &sequencer;
}

void LedMatrix::updateMatrix(int row) {
	byte1 = (1 << (7-row)); //turn on row driver
	byte2 = 0xF0;
	for (col = 0; col < 4; col++) {
		byte2 +=  (led_matrix[row * 4 + col] << step_map[row * 4 + col]);
	}
	byte1 += ~byte2;
	digitalWrite(CS1_PIN, LOW);
	SPI.setBitOrder(LSBFIRST); //shift registers like LSB
	displayVar3->updateSevenSegmentDisplay(); //has to happen HERE bc it's part of the shift register 2-byte sequence
	SPI.transfer(~byte1); //led matrix
	digitalWrite(CS1_PIN, HIGH);
	displayVar3->nextDigit();
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
	//TODO chase LED
	// if (step_matrix[selected_step]) {
	// 	if (blinker > (led_matrix[selected_step] ? 600 : 100)) { //blink long when active
	// 		blink_led();
	// 		blinker = 0;
	// 	}
	// }
	// else {
	// 	if (blinker > (led_matrix[selected_step] ? 100 : 600)) { //blink short when inactive
	// 		blink_led();
	// 		blinker = 0;
	// 	}
	// }
}

void LedMatrix::blink_led() {
	//TODO chase leds
	//led_matrix[selected_step] = !led_matrix[selected_step];
}

void LedMatrix::reset(){
	for (int i = 0; i < 16; i++) {
		led_matrix[i] = 0;
	}
}

void LedMatrix::setMatrixFromSequencer(){
	memcpy(led_matrix, sequencerVar3->getStepMatrix(), sizeof led_matrix); //reset LED matrix to sequence
}

void LedMatrix::toggleLed(int led){
	led_matrix[led] = ~led_matrix[led];
}




}