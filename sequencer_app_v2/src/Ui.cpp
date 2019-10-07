#include "AnalogIO.h"
#include "Buttons.h"
#include "Calibrate.h"
#include "Dac.h"
#include "Display.h"
#include "Encoder.h"
#include "LEDMatrix.h"
#include "Memory.h"
#include "Sequencer.h"
#include "Variables.h"
#include "Pinout.h"
#include "Ui.h"
#include <Arduino.h>

namespace supersixteen{

Buttons buttons;
AnalogIo analogIo;
Display display;
Encoder encoder;
LedMatrix ledMatrix;
Memory memory;

//initialized by main.cpp
Calibration *calibrationVar2;
Sequencer *sequencerVar2;
Dac *dacVar2;

bool shift_state = false;
bool record_mode = false;
bool repeat_mode = false;
bool saving = false;

const byte SEQUENCE_MODE = 0;
const byte CALIBRATE_MODE = 1;
const byte LOAD_MODE = 2;
const byte SAVE_MODE = 3 ;
const byte SCALE_MODE = 4;
byte ui_mode = SEQUENCE_MODE;

byte calibration_step = 0;


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
	if (!memory.init(sequencer)) {
		display.setDisplayAlpha("MEM");
		return;
	}
	initializeSequenceMode();
}

void Ui::poll(){
	if (saving) {
		finishSaving();
	}
    buttons.poll();
	uint16_t value = 0;
	buttons.getQueuedEvent(value);
    if (value) {
        int button_pressed = value & 0x00FF; //use last 8 bits for button number
		bool button_state = (value & 0x0100) >> 8; // use first 8 bits (one of them anyway) for button state
		onButtonToggle(button_pressed, button_state);
        //buttons.button_toggled = false; // reset state for next poll
    }

    int incrementAmount = encoder.poll();
    if (abs(incrementAmount) > 0) {
        onEncoderIncrement(incrementAmount); //encoder.getIncrementAmount());
    }

	if (!record_mode) {
	    analogIo.poll();
		if (analogIo.paramChanged()){
			display.setDisplayNum(analogIo.getDisplayNum());
		}
	}
}

void Ui::multiplex(){
	ledMatrix.multiplexLeds(); //this also updates the seven segment display on a shared serial line, messy i know
	ledMatrix.blinkStep();
}

void Ui::onSaveButton(bool state) {
	if (state == 0) { //only toggle on input
		if (ui_mode == CALIBRATE_MODE) {
			ui_mode = SEQUENCE_MODE;
			calibrationVar2->writeCalibrationValues();
			initializeSequenceMode();
		} else {
			if (saving) return;

			if (shift_state) {
				initializeCalibrationMode();
			} else {

				byte saveStatus = memory.save(01);
				if (saveStatus = 1 || saveStatus == 2) { //saved
					saving = true; 
				} else if (saveStatus == 0) {
					display.setDisplayAlpha("ERR");
				}
			}
		}
	}
}

void Ui::finishSaving(){
	if (memory.finishSaving()) {
		saving = false;
	}
}


void Ui::onLoadButton(bool state) {
	if (state == 0) { //only toggle on input
		if (saving) return;

		
		if (memory.load(01)) {
			display.setDisplayAlpha("LOD");
		} else {
			display.setDisplayAlpha("ERR");
		}
		
		ledMatrix.setMatrixFromSequencer();
	}
}

void Ui::onButtonToggle(int button, bool button_state) {
	if (button < 16) { //inside button grid
        //display.setDisplayNum(button);
        if (button_state) {
			switch(ui_mode) {
    	    	case SEQUENCE_MODE: selectStep(button); break;
        		case CALIBRATE_MODE: updateCalibration(button); break;
			}
        } else {
			        //display.setDisplayNum(button*-1);
		}
    } else {
        switch (button-8) {
		case SHIFT_PIN:  shift_state = button_state; break;
		case PLAY_PIN:   onPlayButton(button_state); break;
		case LOAD_PIN:   onLoadButton(button_state); break;
		case SAVE_PIN:   onSaveButton(button_state); break;
		case GLIDE_PIN:  onGlideButton(button_state); break;
		case RECORD_PIN: onRecButton(button_state); break;
		case REPEAT_PIN: onRepeatButton(button_state); break;
		//default: display.setDisplayNum(button);
        }
		display.setDecimal(!button_state);
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
	if (state) {
		//display.setDisplayAlpha("GLD");
		buttons.setGlideLed(sequencerVar2->toggleGlide());
	}
}

void Ui::onPlayButton(bool state){
	if (state && isSequencing()) {
		sequencerVar2->onPlayButton();
	}
}

void Ui::onRecButton(bool state){
	if (isSequencing()){
		record_mode = state;
		sequencerVar2->setRecordMode(state);
	}
}

void Ui::onRepeatButton(bool state){
	if (isSequencing()){
		repeat_mode = state;
		sequencerVar2->setRepeatMode(state);
	}
}

void Ui::selectStep(int step){
	if (repeat_mode) {
		sequencerVar2->setRepeatLength(step+1);
		return;
	}
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
	if (record_mode) {
		analogIo.recordCurrentParam();
		display.setDisplayNum(analogIo.getDisplayNum());
	}
	sequencerVar2->setActiveNote();
}

}