#include <Arduino.h>
#include <string.h>
#include <MCP23S17.h> //https://github.com/MajenkoLibraries/MCP23S17
#include <SPI.h>
#include <elapsedMillis.h>


#include "Buttons.h"
#include "Encoder.h"
#include "AnalogIO.h"
//#include "Display.h"
#include "Font.h"
#include "LEDMatrix.h"
#include "Main.h"
#include "Sequencer.h"
#include "Variables.h"
#include "Pinout.h"

void setup() {
	analogReference(EXTERNAL); // use AREF for reference voltage
	pinMode(CS0_PIN, OUTPUT); 
	pinMode(CS1_PIN, OUTPUT); 
	pinMode(CS2_PIN, OUTPUT);
	pinMode(CS3_PIN, OUTPUT);
	digitalWrite(CS0_PIN, HIGH);
	digitalWrite(CS1_PIN, HIGH);
	digitalWrite(CS2_PIN, HIGH);
	digitalWrite(CS3_PIN, HIGH);


	initializeMatrix();
	//initializeDisplay();
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
	update_clock();
	update_gate();
	multiplex_leds();
	blink_step();

	read_encoder();
	read_buttons();
	read_input();
}

void run_calibration() {
	read_input();
	multiplex_leds();
	read_encoder();
}

