#include "AnalogIO.h"
#include "Buttons.h"
#include "Calibrate.h"
#include "Dac.h"
#include "Display.h"
#include "Encoder.h"
#include "LEDMatrix.h"
#include "Sequencer.h"
#include "Variables.h"
#include "Pinout.h"
#include "Ui.h"

namespace supersixteen{

Buttons buttons;
AnalogIo analogIo;
Display display;
Dac *dacVar;
Encoder encoder;
LedMatrix ledMatrix;
Calibration *calibrationVar;
Sequencer *sequencerVar;

bool shift_state = false;

const int SEQUENCE_MODE = 0;
const int CALIBRATE_MODE = 1;
const int LOAD_MODE = 2;
const int SAVE_MODE = 3 ;
const int SCALE_MODE = 4;
int ui_mode = SEQUENCE_MODE;

int calibration_step = 0;


void Ui::init(Calibration& calibration, Dac& dac, Sequencer& sequencer){
    pinMode(CS0_PIN, OUTPUT);
	pinMode(CS1_PIN, OUTPUT);
	pinMode(CS2_PIN, OUTPUT);
	pinMode(CS3_PIN, OUTPUT);
	digitalWrite(CS0_PIN, HIGH);
	digitalWrite(CS1_PIN, HIGH);
	digitalWrite(CS2_PIN, HIGH);
	digitalWrite(CS3_PIN, HIGH);

    analogIo.init();
    buttons.init();
    display.init();
    ledMatrix.init(display);

    calibrationVar = &calibration; 
    dacVar         = &dac;
    sequencerVar   = &sequencer;
}

void Ui::poll(){
    buttons.poll();
    if (buttons.getButtonToggled()) {
        onButtonToggle(buttons.getButtonPressed(), buttons.getButtonState());
        buttons.button_toggled = false; // reset state for next poll
    }

    encoder.poll();
    if (abs(encoder.getIncrementAmount()) > 0) {
        onEncoderIncrement(encoder.getIncrementAmount());
    }

    analogIo.poll();



}

void Ui::multiplex(){
	ledMatrix.multiplex_leds(); //this also updates the seven segment display on a shared serial line, messy i know
	ledMatrix.blink_step();
}

void Ui::saveButton(bool state) { //use as calibrate button for now
	display.setDisplayAlpha("SAV");
	return;

	if (state == 0) { //only toggle on input
		if (ui_mode == CALIBRATE_MODE) {
			ui_mode = SEQUENCE_MODE;
			memcpy(led_matrix, step_matrix, sizeof led_matrix); //reset LED matrix to sequence
			analogIo.displaySelectedParam();
            display.setDisplayNum(analogIo.getDisplayNum());
			calibrationVar->writeCalibrationValues();
		}
		else {
			initializeCalibrationMode();
		}
	}
}

void Ui::onButtonToggle(int button, bool button_state) {

	if (button < 16) { //inside button grid
        display.setDisplayNum(button);
        switch(ui_mode) {
        case SEQUENCE_MODE: selectStep(button); break;
        case CALIBRATE_MODE: updateCalibration(button); break;
        }
    } else {
        switch (button) {
		case SHIFT_PIN:  display.setDisplayAlpha("SHF"); shift_state = button_state; break;
		case PLAY_PIN:   display.setDisplayAlpha("PLY"); break;
		case LOAD_PIN:   display.setDisplayAlpha("LOD"); break;
		case SAVE_PIN:   display.setDisplayAlpha("SAV"); break;
		case GLIDE_PIN:  display.setDisplayAlpha("GLD"); break;
		case RECORD_PIN: display.setDisplayAlpha("REC"); break;
		case REPEAT_PIN: display.setDisplayAlpha("REP"); break;
		default: display.setDisplayNum(button);
        }
    }
}

void Ui::onEncoderIncrement(int increment_amount) {
    if (ui_mode == CALIBRATE_MODE) {
		calibrationVar->incrementCalibration(increment_amount, calibration_step);
        updateCalibration(calibration_step);
	} else {
		sequencerVar->incrementTempo(increment_amount);
	}
}

void Ui::glideButton(){
	//display.setDisplayAlpha("GLD");
	sequencerVar->toggleGlide();
	ButtonDriver.digitalWrite(GLIDE_LED_PIN, glide_matrix[selected_step] ? HIGH : LOW); //glide LED
}


void Ui::selectStep(int step){
    sequencerVar->selectStep(step);
    analogIo.displaySelectedParam();
	ButtonDriver.digitalWrite(GLIDE_LED_PIN, glide_matrix[step]); //glide LED
}


void Ui::initializeCalibrationMode() {
	ui_mode = CALIBRATE_MODE;
	calibrationVar->readCalibrationValues();
	updateCalibration(calibration_step);
}

void Ui::updateCalibration(int step) {
    if (step > 8) return;
    calibration_step = step;
	ledMatrix.reset();
	ledMatrix.toggleLed(step);
	display.setDisplayNum(calibrationVar->getCalibrationValue(step));
	dacVar->setOutput(0, GAIN_2, 1, calibrationVar->getCalibratedOutput(step * 12));
}

bool Ui::isSequencing(){
	return (ui_mode != CALIBRATE_MODE);
}

}