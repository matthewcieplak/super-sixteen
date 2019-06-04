#include "Variables.h"
#include "Pinout.h"
#include "LEDMatrix.h"
#include "AnalogIO.h"
#include "Buttons.h"
#include "Display.h"
#include "Calibrate.h"
#include "Sequencer.h"

int button_map[16] = { 12, 13, 14, 15, 11, 10, 9, 8, 4, 5, 6, 7, 3, 2, 1, 0 }; //rows are wired symmetrically rather than sequentially
bool button_matrix[16] = { 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1 };
int saveCount = 0;

const int function_buttons[7] = { SHIFT_PIN, PLAY_PIN, LOAD_PIN, SAVE_PIN, RECORD_PIN, REPEAT_PIN, GLIDE_PIN };
bool function_button_matrix[8] = { 0, 0, 0, 0, 0, 0, 0 }; //store status of buttons in/out  -- no idea why but first bit never toggles? works when offset by one - bad memory address?

int row = 0;
uint16_t buttons_state;
uint16_t buttons_mask;
bool shift_mode = 0;


void initializeButtons() {


	pinMode(ENC_A_PIN, INPUT_PULLUP); //encoder A
	pinMode(ENC_B_PIN, INPUT_PULLUP); //encoder B


	pinMode(GATE_PIN, OUTPUT); //gate
    pinMode(CLOCK_OUT_PIN, OUTPUT); //clock out
    pinMode(CLOCK_IN_PIN, INPUT_PULLUP); //clock in (external pullup)
    pinMode(RESET_PIN, INPUT_PULLUP); //reset in (external pullup)
	pinMode(ANALOG_PIN_1, INPUT);
	pinMode(ANALOG_PIN_2, INPUT);
	pinMode(ANALOG_PIN_3, INPUT);
	pinMode(ANALOG_PIN_4, INPUT);

	SPI.setBitOrder(MSBFIRST);
	ButtonDriver.begin();
	for (int i = 0; i < 4; i++){
		ButtonDriver.pinMode(i+4, OUTPUT);
		ButtonDriver.digitalWrite(i+4, HIGH);
		ButtonDriver.pinMode(i, INPUT_PULLUP);
		ButtonDriver.pinMode(i+8, INPUT_PULLUP);
		ButtonDriver.pinMode(i+12, INPUT_PULLUP);
	}
	ButtonDriver.pinMode(15, OUTPUT);
}

void read_buttons() {
	SPI.setBitOrder(MSBFIRST); //MCP23S17 is picky
	row++;
	if (row > 3) row = 0;
	ButtonDriver.digitalWrite(row+4, LOW);
	buttons_state = ButtonDriver.readPort(); //read all pins at once to reduce spi overhead (prevents display flicker)
	for (int ii = 0; ii < 4; ii++) {
		int stepnum = button_map[row * 4 + ii];
		//bool value = ButtonDriver.digitalRead(button_map[stepnum]); //read one pin at a time
		buttons_mask = buttons_state & 0x000F; //mask off only bits 1-4, the button matrix columns
		buttons_mask = buttons_mask & 0x01 << ii; //select one bit to scan
		bool value = buttons_mask >> ii; //shift to 0 position
		if (value != button_matrix[stepnum]) { //detect when button changes state
			button_matrix[stepnum] = value; //store button state
			if (value == 0) { //toggle step status when button is pressed in
				selectStep(stepnum);
				// TEST running display number
				setDisplayNum(stepnum);
			}
		}
	}
	ButtonDriver.digitalWrite(row+4, HIGH);
	//return;
	//bool function_button = ButtonDriver.digitalRead(function_buttons[row]); //get pins 4-7 of display driver chip for buttons near display
	buttons_mask = (~buttons_state & 0xFF00); // >> 8;
	//return;
	if (buttons_mask > 0) {
		for(int ii = 0; ii<7; ii++){
			bool function_button_state = (buttons_mask & 0x01 << (ii+8)) >> (ii+8);

			if (function_button_state != function_button_matrix[ii+1]){
				function_button_matrix[ii+1] = function_button_state;
				//Serial.write(function_button);
				switch (function_buttons[ii]) {
				case SHIFT_PIN:   shiftButton(function_button_state); break;
				case PLAY_PIN:     playButton(function_button_state); break;
				case LOAD_PIN:     loadButton(function_button_state); break;
				case SAVE_PIN:     saveButton(function_button_state); break;
				case GLIDE_PIN:   glideButton(function_button_state); break;
				case RECORD_PIN: recordButton(function_button_state); break;
				case REPEAT_PIN: repeatButton(function_button_state); break;
				default:
					if (function_button_state) {
						setDisplayNum((ii + 5) * 111);
						// digit_display[0] += 1;
						// digit_display[1] += 1;
						// digit_display[2] += 1;
						// ButtonDriver.digitalWrite(15, HIGH);
					}
				}
			}
		}
	} else {
		//ButtonDriver.digitalWrite(15, LOW);
	}
}

void selectStep(unsigned int stepnum) {
	if (control_mode == CALIBRATE_MODE) {
		if (stepnum < 9) {
			selected_step = stepnum;
			updateCalibration();
		}
	} else { //sequence mode
		if (selected_step == stepnum || !step_matrix[stepnum]) { //require 2 presses to turn active steps off, so they can be selected/edited without double-tapping //TODO maybe implement hold-to-deactivate
			step_matrix[stepnum] = !step_matrix[stepnum];
			led_matrix[stepnum] = step_matrix[stepnum];
		}
		selected_step = stepnum;
		displaySelectedParam();
		ButtonDriver.digitalWrite(GLIDE_LED_PIN, glide_matrix[stepnum]); //glide LED
		//setDisplayNum();
	}
}

void saveButton(bool state) { //use as calibrate button for now
	setDisplayAlpha("SAV");
	return;

	if (state == 0) { //only toggle on input
		if (control_mode == CALIBRATE_MODE) {
			control_mode = SEQUENCE_MODE;
			memcpy(led_matrix, step_matrix, sizeof led_matrix); //reset LED matrix to sequence
			displaySelectedParam();
			writeCalibrationValues();
		}
		else {
			initializeCalibrationMode();
		}
	}
}

void playButton(bool state) {
	setDisplayAlpha("PLY");
	if (state) {
		on_play_button();
	}
}

void loadButton(bool state) {
	setDisplayAlpha("LOD");
	if (state) {
	}
}

void shiftButton(bool state) {
	setDisplayAlpha("SHF");
	shift_mode = state;
}

void glideButton(bool state){
	setDisplayAlpha("GLD");
	glide_matrix[selected_step] = !glide_matrix[selected_step];
	ButtonDriver.digitalWrite(GLIDE_LED_PIN, glide_matrix[selected_step] ? HIGH : LOW); //glide LED
}

void recordButton(bool state){
	setDisplayAlpha("REC");
}

void repeatButton(bool state){
	setDisplayAlpha("RPT");
}