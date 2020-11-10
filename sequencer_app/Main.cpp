//#include <Arduino.h>
//#include <string.h>
//#include <MCP23S17.h> //https://github.com/MajenkoLibraries/MCP23S17
//#include <SPI.h>
//#include <elapsedMillis.h>
//
//#include "Analogio.h"
//#include "Buttons.h"
//#include "Calibrate.h"
//#include "Display.h"
//#include "Encoder.h"
//#include "Font.h"
//#include "LEDMatrix.h"
//#include "Sequencer.h"
//#include "Variables.h"
//#include "Pinout.h"
//
//void setup() {
//	analogReference(EXTERNAL); // use AREF for reference voltage
//	pinMode(CS0_PIN, OUTPUT); //enable CS for DAC
//	pinMode(CS1_PIN, OUTPUT); //enable CS for DAC
//
//	initializeMatrix();
//	initializeDisplay();
//	initializeFont();
//	initializeButtons();
//
//	for (int i = 0; i < 16; i++) {
//		duration_matrix[i] = 80;
//	}
//}
//
//void loop() {
//	increment_step();
//	multiplex_leds();
//	update_gate();
//
//	if (stepper > 10) { //save cycles and do these less frequently
//		stepper = 0;
//		read_encoder();
//		blink_step();
//		read_input();
//	}
//
//}
