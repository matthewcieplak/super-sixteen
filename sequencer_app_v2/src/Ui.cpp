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
Encoder encoder;
LedMatrix ledMatrix;

//initialized by main.cpp
Calibration *calibrationVar2;
Sequencer *sequencerVar2;
Dac *dacVar2;

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

	calibrationVar2 = &calibration;
    dacVar2         = &dac;
    sequencerVar2   = &sequencer;
    analogIo.init(sequencer);
    buttons.init();
    display.init();
	encoder.init();
    ledMatrix.init(display, sequencer);
	initializeSequenceMode();


}

void Ui::poll(){
    buttons.poll();
    if (buttons.getButtonToggled()) {
        onButtonToggle(buttons.getButtonPressed(), buttons.getButtonState());
        buttons.button_toggled = false; // reset state for next poll
    }

    int incrementAmount = encoder.poll();
    if (abs(incrementAmount) > 0) {
        onEncoderIncrement(incrementAmount); //encoder.getIncrementAmount());
    }

    analogIo.poll();
	if (analogIo.paramChanged()){
		display.setDisplayNum(analogIo.getDisplayNum());
	}
}

void Ui::multiplex(){
	ledMatrix.multiplexLeds(); //this also updates the seven segment display on a shared serial line, messy i know
	//ledMatrix.blinkStep();
}

void Ui::onSaveButton(bool state) { //use as calibrate button for now
	display.setDisplayAlpha("SAV");
	return;

	if (state == 0) { //only toggle on input
		if (ui_mode == CALIBRATE_MODE) {
			ui_mode = SEQUENCE_MODE;
			calibrationVar2->writeCalibrationValues();
			initializeSequenceMode();
		}
		else {
			initializeCalibrationMode();
		}
	}
}

void Ui::onButtonToggle(int button, bool button_state) {

	if (button < 16) { //inside button grid
        //display.setDisplayNum(button);
        if (!button_state) {
			switch(ui_mode) {
    	    	case SEQUENCE_MODE: selectStep(button); break;
        		case CALIBRATE_MODE: updateCalibration(button); break;
			}
        } else {
			        //display.setDisplayNum(button*-1);
		}
    } else {
        switch (button-8) {
		case SHIFT_PIN:  display.setDisplayAlpha("SHF"); shift_state = button_state; break;
		case PLAY_PIN:   onPlayButton(button_state); break;
		case LOAD_PIN:   display.setDisplayAlpha("LOD"); break;
		case SAVE_PIN:   display.setDisplayAlpha("SAV"); break;
		case GLIDE_PIN:  onGlideButton(button_state); break;
		case RECORD_PIN: display.setDisplayAlpha("REC"); break;
		case REPEAT_PIN: display.setDisplayAlpha("REP"); break;
		//default: display.setDisplayNum(button);
        }
    }
}

void Ui::onEncoderIncrement(int increment_amount) {
	if (ui_mode == CALIBRATE_MODE) {
		display.setDisplayNum(calibrationVar2->incrementCalibration(increment_amount, calibration_step));
        updateCalibration(calibration_step);
	} else {
		display.setDisplayNum(sequencerVar2->incrementTempo(increment_amount));
	}
}

void Ui::onGlideButton(bool state){
	//display.setDisplayAlpha("GLD");
	if (state) buttons.setGlideLed(sequencerVar2->toggleGlide());
}

void Ui::onPlayButton(bool state){
	if (state && isSequencing()) {
		sequencerVar2->onPlayButton();
	}
}

void Ui::selectStep(int step){
    sequencerVar2->selectStep(step);
	ledMatrix.setMatrixFromSequencer();
	//ledMatrix.blinkLed();
    analogIo.displaySelectedParam();
	display.setDisplayNum(analogIo.getDisplayNum());
	buttons.setGlideLed(sequencerVar2->getGlide());
}


void Ui::initializeCalibrationMode() {
	ui_mode = CALIBRATE_MODE;
	calibrationVar2->readCalibrationValues();
	updateCalibration(calibration_step);
}

void Ui::updateCalibration(int step) {
    if (step > 8) return;
    calibration_step = step;
	ledMatrix.reset();
	ledMatrix.toggleLed(step);
	display.setDisplayNum(calibrationVar2->getCalibrationValue(step));
	dacVar2->setOutput(0, GAIN_2, 1, calibrationVar2->getCalibratedOutput(step * 12));
}

bool Ui::isSequencing(){
	return (ui_mode != CALIBRATE_MODE);
}

void Ui::initializeSequenceMode(){
	ledMatrix.reset();
	ledMatrix.setMatrixFromSequencer();
	analogIo.displaySelectedParam();
	display.setDisplayNum(analogIo.getDisplayNum());
}

void Ui::onStepIncremented(){
	ledMatrix.setMatrixFromSequencer();
	ledMatrix.blinkCurrentStep();
}

}