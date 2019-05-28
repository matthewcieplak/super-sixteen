#include "Variables.h"
#include "Pinout.h"
#include "LEDMatrix.h"
#include "AnalogIO.h"
#include "Buttons.h"
#include "Display.h"
#include "Calibrate.h"

int button_map[16] = { 3, 2, 1, 0, 0, 1, 2, 3, 3, 2, 1, 0, 0, 1, 2, 3 }; //rows are wired symmetrically rather than sequentially
bool button_matrix[16] = { 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1 };
int saveCount = 0;

const int function_buttons[4] = { SAVE_PIN, LOAD_PIN, PLAY_PIN, SHIFT_PIN };
bool function_button_matrix[5] = { 0, 1, 1, 1, 1 }; //store status of buttons in/out  -- no idea why but first bit never toggles? works when offset by one - bad memory address?
const int aux_buttons[3] = { GLIDE_PIN, REPEAT_PIN, RECORD_PIN };
bool aux_button_matrix[3] = { 1, 1, 1 };


void initializeButtons() {
	pinMode(ENC_A_PIN, INPUT_PULLUP); //encoder A
	pinMode(ENC_B_PIN, INPUT_PULLUP); //encoder B
	pinMode(GLIDE_PIN, INPUT_PULLUP); //glide button
	pinMode(REPEAT_PIN, INPUT_PULLUP); //repeat button
	pinMode(RECORD_PIN, INPUT_PULLUP); //record button

	pinMode(GATE_PIN, OUTPUT); //gate


    pinMode(CLOCK_OUT_PIN, OUTPUT); //clock out
    pinMode(CLOCK_IN_PIN, INPUT); //clock in (external pullup)
    pinMode(RESET_PIN, INPUT); //reset in (external pullup)						   
	pinMode(LDAC_PIN, OUTPUT); //LDAC
	digitalWrite(LDAC_PIN, LOW);
}

void readButtons(int row) {
	//Bank3.writePort(0, button_rows[row]);
	//byte button_row = Bank3.readPort(0) & 0x00; // (mask since we only want the last 4 bits, pins 4-7);
	MatrixDriver.digitalWrite(row, LOW);
	for (int ii = 0; ii < 4; ii++) {
		int stepnum = row * 4 + ii;
		//    //check each bit in the byte and see if it's pulled low -- to reduce SPI overhead
		//    bool value = (button_rows[row] & (0x08 >> col)); //step_map[row*4+col])); //pins backwards again, iterate R-L by rightwards bitshift
		bool value = MatrixDriver.digitalRead(button_map[stepnum] + 4);
		if (value != button_matrix[stepnum]) { //detect when button changes state
			button_matrix[stepnum] = value; //store button state
			if (value == 0) { //toggle step status when button is pressed in
				selectStep(stepnum);
			}
		}
	}
	MatrixDriver.digitalWrite(row, HIGH);

	bool function_button = DisplayDriver.digitalRead(function_buttons[row]); //get pins 4-7 of display driver chip for buttons near display
	if (function_button_matrix[row+1] != function_button) {
		function_button_matrix[row+1] = function_button;
		//Serial.write(function_button);
		switch (function_buttons[row]) {
		case SAVE_PIN:  saveButton(function_button);  break;
		case SHIFT_PIN: shiftButton(function_button); break;
		case PLAY_PIN:  playButton(function_button); break;
		case LOAD_PIN:  loadButton(function_button); break;
		default:
			num_display = (row + 5) * 111;
			setDisplayNum();
		}
	}

	if (aux_buttons[row]) {
		int aux_button = digitalRead(aux_buttons[row]);
		if (aux_button != aux_button_matrix[row]) {
			aux_button_matrix[row] = aux_button;
			if (aux_button == 0) { //take action on pressed, not depressed
				switch (aux_buttons[row]) {
				case GLIDE_PIN:
					glide_matrix[selected_step] = !glide_matrix[selected_step];
					DisplayDriver.digitalWrite(GLIDE_LED_PIN, glide_matrix[selected_step] ? HIGH : LOW); //glide LED
					break;
				case RECORD_PIN: //todo write record fn
				case REPEAT_PIN: //todo write repeat fn
				default:
					num_display = (row + 1) * 222;
					setDisplayNum();
				}
			}
		}
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
		DisplayDriver.digitalWrite(GLIDE_LED_PIN, glide_matrix[stepnum]); //glide LED
		setDisplayNum();
	}
}

void saveButton(bool state) { //use as calibrate button for now
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
	if (state) {
		play_active = 1;
		timekeeper = 0;
		num_display = 777;
		setDisplayNum();
	}
	else {
		num_display = -77;
		setDisplayNum();
	}
}

void loadButton(bool state) {
	if (state) {
		num_display = 111;
		setDisplayNum();
	}
	else {
		num_display = -11;
		setDisplayNum();
	}
}

void shiftButton(bool state) {
		if (state) {
			num_display = 666;
			setDisplayNum();
		}
		else {
			num_display = -66;
			setDisplayNum();
		}
	

	bool shift_state = state;
}