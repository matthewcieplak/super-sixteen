#include "Variables.h"
#include "Pinout.h"
#include "Display.h"
#include "LEDMatrix.h"
#include "Buttons.h"

int step_map[16] = { 3, 2, 1, 0, 0, 1, 2, 3, 3, 2, 1, 0, 0, 1, 2, 3 }; //rows are wired symmetrically rather than sequentially
const int led_rows[4] = { 0x80, 0x40, 0x20, 0x10 };
uint8_t byte1;
uint8_t byte2;

int row_counter = 0;
int col = 0;


void initializeMatrix() {
	MatrixDriver.begin();
	for (int pin = 0; pin < 16; pin++) {
		if (pin < 4) {
			MatrixDriver.pinMode(pin, OUTPUT);
			MatrixDriver.digitalWrite(pin, LOW);
		}
		else if (pin >= 4 && pin < 8) {
			MatrixDriver.pinMode(pin, INPUT_PULLUP); //use these for reading in buttons
		}
		else {
			MatrixDriver.pinMode(pin, OUTPUT);
			MatrixDriver.digitalWrite(pin, LOW);
		}
	}

	MatrixDriver.writePort(0x0F00);
}

void updateMatrix(int row) {
	byte1 = 0x00;
	for (col = 0; col < 4; col++) {
		byte1 += (led_matrix[row * 4 + col] << step_map[row * 4 + col]);
	}
	byte1 = (0x0F - byte1) | led_rows[row]; //invert since we want to pull low for active drain
											//byte2 = button_rows[row]; //while we're using the serial port, pull one output low for buttons so we can read them on the pullup inputs

											//turn on new light;
											//uint16_t doublebyte = (byte1 << 8) + byte2;
											//Bank3.writePort(0x0000); //hard-reset previous pins
											//Bank3.writePort(doublebyte);
											//Bank3.writePort(1, byte2);
	MatrixDriver.writePort(1, byte1);
}


void multiplex_leds() {
	if (multiplex > 1) {
		multiplex = 0;

		updateMatrix(row_counter);
		readButtons(row_counter);
		row_counter++;

		if (row_counter == 4) {
			row_counter = 0;
		}

		updateDisplay();
	}
}

void blink_step() {
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

void blink_led() {
	led_matrix[selected_step] = !led_matrix[selected_step];
}
