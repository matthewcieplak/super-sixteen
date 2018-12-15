#include "Calibrate.h"
#include <Arduino.h>
#include <string.h>
#include <MCP23S17.h> //https://github.com/MajenkoLibraries/MCP23S17
#include <SPI.h>
#include <elapsedMillis.h>


#include "Variables.h"
#include "Pinout.h"

#include "AnalogIO.h"
#include "Buttons.h"
#include "Display.h"
#include "Encoder.h"
#include "Font.h"
#include "LEDMatrix.h"
#include "Sequencer.h"


void setup() {
	analogReference(EXTERNAL); // use AREF for reference voltage
	pinMode(CS0_PIN, OUTPUT); //enable CS for DAC
	pinMode(CS1_PIN, OUTPUT); //enable CS for DAC

	initializeMatrix();
	initializeDisplay();
	initializeFont();
	initializeButtons();
	//readCalibrationValues(); -- disable temporarily to bypass overwriting EEPROM during programming. uncomment for typical use

	for (int i = 0; i < 16; i++) {
		duration_matrix[i] = 80;
	}
}

void loop() {
	switch (control_mode){
	case CALIBRATE_MODE:
		run_calibration();
		break;
	default: //sequencer_mode
		run_sequence();
		break;
	}
}

void run_sequence() {
	increment_step();
	multiplex_leds();
	read_input();
	blink_step();
	read_encoder();
	update_gate();
}

void run_calibration() {
	read_input();
	multiplex_leds();
	read_encoder();
}
