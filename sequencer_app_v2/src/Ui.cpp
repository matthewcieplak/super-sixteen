#include "AnalogIO.h"
#include "Buttons.h"
#include "Calibrate.h"
#include "Dac.h"
#include "Display.h"
#include "Encoder.h"
#include "LEDMatrix.h"
#include "Memory.h"
#include "Sequencer.h"
#include "Scales.h"
#include "Variables.h"
#include "Pinout.h"
#include "Ui.h"
#include <Arduino.h>
#include <avr/pgmspace.h>


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
bool effect_mode = false;
bool saving = false;
bool copy_state = false;

const byte SEQUENCE_MODE = 0;
const byte CALIBRATE_MODE = 1;
const byte LOAD_MODE = 2;
const byte SAVE_MODE = 3 ;
const byte EDIT_PARAM_MODE = 4;

const byte PARAM_BARS = 8;
const byte PARAM_STEPS = 9;
const byte PARAM_SCALE = 10;
const byte PARAM_SWING = 11;
const byte PARAM_TRANSPOSE = 12;
const byte PARAM_EFFECT = 21;
const byte PARAM_EFFECT_DEPTH = 25; //

byte current_param = PARAM_BARS;
byte ui_mode = SEQUENCE_MODE;
byte calibration_step = 0;
byte current_patch = 1;
byte selected_patch = 1;
byte current_bar = 0;
char scalename[4];
char effectname[4];


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
	if (buttons.getQueuedEvent(value) == 0){
        int button_pressed = value & 0x00FF; //use last 8 bits for button number
		bool button_state = (value & 0x0100) >> 8; // use first 8 bits (one of them anyway) for button state
		onButtonToggle(button_pressed, button_state);
        //buttons.button_toggled = false; // reset state for next poll
    }

    int incrementAmount = encoder.poll();
    if (abs(incrementAmount) > 0) {
        onEncoderIncrement(incrementAmount); //encoder.getIncrementAmount());
    }

	if (ui_mode == SEQUENCE_MODE || ui_mode == EDIT_PARAM_MODE) {
		analogIo.poll();
		//if (!record_mode) {
		if (analogIo.paramChanged()){
			display.setDisplayNum(analogIo.getDisplayNum());
		}
		//}
	}
}

void Ui::multiplex(){
	ledMatrix.multiplexLeds(); //this also updates the seven segment display on a shared serial line, messy i know
	ledMatrix.blinkStep();
}

void Ui::onSaveButton(bool state) {
	if (state) { //only toggle on input
		if (ui_mode == CALIBRATE_MODE) {
			ui_mode = SEQUENCE_MODE;
			calibrationVar2->writeCalibrationValues();
			digitalWrite(GATE_PIN, LOW);
			initializeSequenceMode();
		} else if (ui_mode == SAVE_MODE) {

			//actually save the patch!!
			byte saveStatus = memory.save(selected_patch);
			current_patch = selected_patch;
			if (saveStatus == 1 || saveStatus == 2) { //saved
				saving = true;
				//TODO display something, like rapid flashing?
				display.blinkDisplay(true, 100, 5);
				ui_mode = SEQUENCE_MODE;
			} else if (saveStatus == 0) {
				display.setDisplayAlpha("ERR");
			}
		} else { //sequencing, presumably
			if (saving) return; //disallow double-presses


			ui_mode = SAVE_MODE;
			selected_patch = current_patch;
			display.setDisplayNum(selected_patch);
			display.blinkDisplay(true, 300, 0);
		}
	}
}

void Ui::finishSaving(){
	if (memory.finishSaving()) {
		saving = false;
	}
}


void Ui::onLoadButton(bool state) {
	if (state) { //only toggle on input
		if (saving) return;

		if (ui_mode == LOAD_MODE) {
			//actually load patch
			if (memory.load(selected_patch)) {
				display.blinkDisplay(true, 100, 3);
			} else {
				display.setDisplayAlpha("ERR");
				display.blinkDisplay(true, 100, 3);
			}
			ui_mode = SEQUENCE_MODE;
			current_patch = selected_patch;
			current_bar = 0;
			ledMatrix.setMatrixFromSequencer(current_bar);
			//TODO set sequencer current step by length of active sequence!
		} else if (ui_mode == SEQUENCE_MODE){
			ui_mode = LOAD_MODE;
			selected_patch = current_patch;
			display.setDisplayNum(selected_patch);
			display.blinkDisplay(true, 300, 0);
		}
	}
}

void Ui::onButtonToggle(int button, bool button_state) {
	if (button < 16) { //inside button grid
        //display.setDisplayNum(button);
        if (button_state) {
			if (ui_mode == CALIBRATE_MODE) {
				updateCalibration(button);
			} else {
				if (shift_state) {
					shiftFunction(button);
				} else {
					selectStep(button);
				}
			}
        } else {
			copy_state = false;
			//display.setDisplayNum(button*-1);
		}
    } else {
        switch (button-8) {
		case SHIFT_PIN:  onShiftButton(button_state); break;
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

void Ui::onShiftButton(bool button_state){
	shift_state = button_state;
	if (button_state) {
		cancelSaveOrLoad();
	} else {
		copy_state = false;
	}
}

void Ui::shiftFunction(int button) {
	if (button < 8) {
		if (button < 4)	selectBar(button);
	} else {
		switch (button) {
			case 14: clearSequence(); break;
			case 13: initializeCalibrationMode(); break;
			case PARAM_BARS: //select bars?
			case PARAM_STEPS:
			case PARAM_SCALE:
			case PARAM_SWING:
			case PARAM_TRANSPOSE:
			  ui_mode = EDIT_PARAM_MODE;
			  current_param = button;
			  onEncoderIncrement(0);


		}
	}
}

void Ui::selectBar(byte bar){
	if (copy_state) {
		sequencerVar2->paste(current_bar, bar);
		display.setDisplayAlpha("CPY");
		display.blinkDisplay(true, 100, 1);
		copy_state = false;
	} else {
		const char barname[4] = {char(36+55), char(11+55), char(bar+1+55)}; //goofy way of writing " b4" with ad-hoc ascii table conversion
		display.setDisplayAlpha(barname);
		copy_state = true;
	}
	
	current_bar = bar;
	sequencerVar2->onBarSelect(current_bar);
	ledMatrix.setMatrixFromSequencer(current_bar);
}

void Ui::onEncoderIncrement(int increment_amount) {
	if (ui_mode == CALIBRATE_MODE) {
		display.setDisplayNum(calibrationVar2->incrementCalibration(increment_amount, calibration_step));
        updateCalibration(calibration_step);
	} else if (ui_mode == SAVE_MODE || ui_mode == LOAD_MODE) {
		selected_patch += increment_amount;
		if (selected_patch < 1) selected_patch = 99;
		if (selected_patch > 99) selected_patch = 1;
		display.setDisplayNum(selected_patch);
	} else if (ui_mode == EDIT_PARAM_MODE) {
		int param = 0;
		if (current_param == PARAM_SCALE) {
			param = sequencerVar2->incrementScale(increment_amount);
			strcpy_P(scalename, (char *)pgm_read_word(&(scale_names[param])));  // Necessary casts and dereferencing, just copy (for PROGMEM keywords in flash)
			display.setDisplayAlpha(scalename);
		} else if (current_param == PARAM_EFFECT) {
			param = sequencerVar2->incrementEffect(increment_amount);
			strcpy_P(effectname, (char *)pgm_read_word(&(effect_names[param])));  // Necessary casts and dereferencing, just copy (for PROGMEM keywords in flash)
			display.setDisplayAlpha(effectname);
		} else {
			switch(current_param) {
				case PARAM_EFFECT_DEPTH: param = sequencerVar2->incrementEffectDepth(increment_amount); break;
				case PARAM_BARS:  param = sequencerVar2->incrementBars(increment_amount); break;
				case PARAM_STEPS: param = sequencerVar2->incrementSteps(increment_amount, shift_state); break;
				case PARAM_SWING: param = sequencerVar2->incrementSwing(increment_amount); break;
				case PARAM_TRANSPOSE: param = sequencerVar2->incrementTranspose(increment_amount); break;
			}
			display.setDisplayNum(param);
		}
	} else {//sequencing, presumably
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
	if (shift_state) {
		sequencerVar2->onReset(false);
		return;
	}
	if (state && isSequencing()) {
		cancelSaveOrLoad();
		sequencerVar2->onPlayButton();
	}
}

void Ui::onRecButton(bool state){
	if (isSequencing()){
		record_mode = state;
		sequencerVar2->setRecordMode(state);
		analogIo.setRecordMode(state);
	}
}

void Ui::onRepeatButton(bool state){
	if (isSequencing()){
		if (shift_state) {
			ui_mode = EDIT_PARAM_MODE;
			current_param = PARAM_EFFECT;
			onEncoderIncrement(0);
		} else {
			effect_mode = state;
			if (effect_mode) {
				ui_mode = EDIT_PARAM_MODE;
				current_param = PARAM_EFFECT_DEPTH;
				onEncoderIncrement(0);
			} else {
				ui_mode = SEQUENCE_MODE;
			}
			sequencerVar2->setEffectMode(state);
		} 
	}
}

void Ui::selectStep(int step){
	cancelSaveOrLoad();
	sequencerVar2->selectStep(step+current_bar*16);
	ledMatrix.setMatrixFromSequencer(current_bar);
	//ledMatrix.blinkLed();
    analogIo.displaySelectedParam();
	display.setDisplayNum(analogIo.getDisplayNum());
	buttons.setGlideLed(sequencerVar2->getGlide());
}


void Ui::initializeCalibrationMode() {
	cancelSaveOrLoad();
	ui_mode = CALIBRATE_MODE;
	calibrationVar2->readCalibrationValues();
	updateCalibration(calibration_step);
	display.setDisplayAlpha("CAL");
	digitalWrite(GATE_PIN, HIGH); //to make signals audible
}

void Ui::clearSequence(){
	display.setDisplayAlpha("CLR");
	sequencerVar2->clearSequence();
	current_bar = 0;
	ledMatrix.setMatrixFromSequencer(current_bar);
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
	ui_mode = SEQUENCE_MODE;
	ledMatrix.reset();
	ledMatrix.setMatrixFromSequencer(current_bar);
	analogIo.displaySelectedParam();
	display.setDisplayNum(analogIo.getDisplayNum());
}

void Ui::onStepIncremented(){
	ledMatrix.setMatrixFromSequencer(current_bar);
	ledMatrix.blinkCurrentStep();
	if (record_mode) {
		analogIo.recordCurrentParam();
	}
	sequencerVar2->setActiveNote(); //takes place here to enable real-time recording to be heard immediately
}

bool Ui::cancelSaveOrLoad(){
	if (ui_mode == LOAD_MODE || ui_mode == SAVE_MODE || ui_mode == EDIT_PARAM_MODE || ui_mode == CALIBRATE_MODE) {
		if (ui_mode == CALIBRATE_MODE) {
			digitalWrite(GATE_PIN, LOW);
		}
		initializeSequenceMode();
		display.blinkDisplay(true, 100, 1);
		
		
		return true;
		
	}
	return false;
}

}