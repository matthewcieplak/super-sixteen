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
bool load_button_state = false;
bool save_button_state = false;
bool record_mode = false;
bool effect_mode = false;
bool step_record_mode = false;
bool saving = false;
bool copy_state = false;
bool encoder_bumped = false;
byte erase_counter = 0;
bool just_selected_param = false;

const byte SEQUENCE_MODE = 0;
const byte CALIBRATE_MODE = 1;
const byte LOAD_MODE = 2;
const byte SAVE_MODE = 3 ;
const byte EDIT_PARAM_MODE = 4;

const byte PARAM_TEMPO = 8;
const byte PARAM_STEPS = 9;
const byte PARAM_SCALE = 10;
const byte PARAM_SWING = 11;
const byte PARAM_TRANSPOSE = 12;
const byte PARAM_SONG  = 15;
const byte PARAM_LOOPS = 16; 
const byte PARAM_EFFECT = 21;
const byte PARAM_EFFECT_DEPTH = 25; //
const byte PARAM_GLIDE = 26;

byte current_param = PARAM_TEMPO;
byte ui_mode = SEQUENCE_MODE;
byte calibration_step = 0;
byte current_patch = 1;
byte selected_patch = 1;
byte current_bar = 0;
char scalename[5];
char effectname[5];
char notename[5];


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

	analogIo.setDisplayMode(calibration.readDisplayModeValue());


	display.startupSequence();

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
			displaySequenceParam();
			cancelSaveOrLoad(); //TODO remove if this is too jittery to use
		}
		//}
	}
}

void Ui::displaySequenceParam(){
	if (analogIo.paramIsAlpha()) {
		display.setDisplayAlphaVar(analogIo.getDisplayAlpha());
	} else {
		display.setDisplayNum(analogIo.getDisplayNum());
	}
}

void Ui::multiplex(){
	ledMatrix.multiplexLeds(); //this also updates the seven segment display on a shared serial line, messy i know
	ledMatrix.blinkStep();
}

void Ui::onSaveButton(bool state) {
	save_button_state = state;
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
	load_button_state = state;
	if (state) { //only toggle on input
		if (saving) return;

		if (ui_mode == LOAD_MODE) {
			//actually load patch
			if (memory.load(selected_patch)) {
				display.blinkDisplay(true, 100, 3);
			} else {
				sequencerVar2->clearSequence();
				display.blinkDisplay(false, 100, 1);
			}
			ui_mode = SEQUENCE_MODE;
			current_patch = selected_patch;
			current_bar = 0;
			ledMatrix.setMatrixFromSequencer(current_bar);
			//TODO set sequencer current step by length of active sequence!
		} else if (ui_mode == SEQUENCE_MODE){
			if (shift_state) {
				loadNextSequence();
				return;
			}
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
				} else if (load_button_state) {
					selected_patch = button + 1;
					display.setDisplayNum(selected_patch);
					onLoadButton(load_button_state);
				} else if (save_button_state) {
					selected_patch = button + 1;
					display.setDisplayNum(selected_patch);
					onSaveButton(save_button_state);
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
		display.setDecimal(button_state);
    }
}

void Ui::onShiftButton(bool button_state){
	shift_state = button_state;
	if (button_state) {
		cancelSaveOrLoad();
	} else {
		copy_state = false;
		erase_counter = 0;
	}
}

void Ui::shiftFunction(int button) {
	if (button < 8) {
		if (button < 4)	selectBar(button);
	} else {
		switch (button) {
			case PARAM_SONG: 
				ui_mode = EDIT_PARAM_MODE;
				just_selected_param = true;
				if (current_param == PARAM_SONG) {
					current_param = PARAM_LOOPS;
					display.setDisplayAlpha("LPS");
				} else {
					current_param = PARAM_SONG;
					display.setDisplayAlpha("SNG");
				}
				break; 
			case 14: 
				clearSequence();
				erase_counter += 1;
				switch (erase_counter) {
					case 2: display.setDisplayAlpha("E  "); break;
					case 3: display.setDisplayAlpha("ER "); break;
					case 4: display.setDisplayAlpha("ERS"); display.blinkDisplay(true, 500, 0); break;
					case 5:  memory.erase(); display.setDisplayNum(0); erase_counter = 0; display.blinkDisplay(false, 100, 0); break;
				}
				break;
			case 13: initializeCalibrationMode(); break;
			case PARAM_TEMPO: //select bars?
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
		char barname[4] = {char(36+55), char(11+55), char(bar+1+48)}; //goofy way of writing " b4" with ad-hoc ascii table conversion
		display.setDisplayAlpha(barname);
		copy_state = true;
	}

	current_bar = bar;
	sequencerVar2->onBarSelect(current_bar);
	ledMatrix.setMatrixFromSequencer(current_bar);
}

//const char brightness_names[4][3] = {"---", "8--", "88-", "888"};

void Ui::onEncoderIncrement(int increment_amount) {
	if (ui_mode == CALIBRATE_MODE) {
		if (calibration_step == 8) { //step 8 is brightness mode
			calibrationVar2->incrementBrightness(-1*increment_amount);
			display.setBrightness(calibrationVar2->getBrightness());
			switch(calibrationVar2->getBrightness()) {
				case 4: display.setDisplayAlpha("BR1"); break;
				case 3: display.setDisplayAlpha("BR2"); break;
				case 2: display.setDisplayAlpha("BR3"); break;
				case 1: display.setDisplayAlpha("BR4"); break;
				default: display.setDisplayAlpha("BR5");
			}
		} else {
			display.setDisplayNum(calibrationVar2->incrementCalibration(increment_amount, calibration_step));
    	    updateCalibration(calibration_step);
		}
	} else if (encoder_bumped || ui_mode == SAVE_MODE || ui_mode == LOAD_MODE) {
		if (ui_mode == SEQUENCE_MODE) ui_mode = LOAD_MODE;
		selected_patch += increment_amount;
		if (selected_patch < 1) selected_patch = 99;
		if (selected_patch > 99) selected_patch = 1;
		display.setDisplayNum(selected_patch);
		display.setDecimal(memory.patchExists(selected_patch));
		display.blinkDisplay(true, 300, 0);
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
				case PARAM_TEMPO: param = sequencerVar2->incrementTempo(increment_amount); break; //sequencerVar2->incrementBars(increment_amount); break;
				case PARAM_STEPS: param = sequencerVar2->incrementSteps(increment_amount, shift_state); break;
				case PARAM_SWING: param = sequencerVar2->incrementSwing(increment_amount); break;
				case PARAM_GLIDE: param = sequencerVar2->incrementGlide(increment_amount); break;
				case PARAM_TRANSPOSE: param = sequencerVar2->incrementTranspose(increment_amount); break;
				case PARAM_LOOPS: param = sequencerVar2->incrementSongLoops(just_selected_param ? 0 : increment_amount); just_selected_param = false; break;
				case PARAM_SONG:  
					param = sequencerVar2->incrementSongNextSeq(just_selected_param ? 0 : increment_amount);
					just_selected_param = false;
					display.setDecimal(memory.patchExists(param));
					break;
			}
			display.setDisplayNum(param);
		}
	} else if (shift_state) {
		sequencerVar2->incrementClock(increment_amount);
	} else {
		//by default when encoder is turned
		//show current patch number and select another if moved further
		selected_patch = current_patch;
		display.setDisplayNum(selected_patch);
		encoder_bumped = true;
	}
}

void Ui::onGlideButton(bool state){
	if (state) {
		if (shift_state) {
			ui_mode = EDIT_PARAM_MODE;
			current_param = PARAM_GLIDE;
			onEncoderIncrement(0);
		} else {
		//display.setDisplayAlpha("GLD");
			buttons.setGlideLed(sequencerVar2->toggleGlide());
		}
	}
}

void Ui::invertEncoder(){
	encoder.toggle_inverted();
	display.setDisplayAlpha("INV");
}

void Ui::onPlayButton(bool state){
	if (state && shift_state) {
		ledMatrix.reset();
		ledMatrix.setMatrixFromSequencer(current_bar);
		sequencerVar2->onReset();
		return;
	}
	if (state && isSequencing()) {
		cancelSaveOrLoad();
		ledMatrix.reset();
		ledMatrix.setMatrixFromSequencer(current_bar);
		sequencerVar2->onPlayButton();
	}
}

void Ui::onRecButton(bool state){
	if (isSequencing()){
		if (shift_state || step_record_mode) {
			//activate current step
			sequencerVar2->setStepRecordingMode(state);
			step_record_mode = state;
		} else {
			record_mode = state;
			sequencerVar2->setRecordMode(state);
			analogIo.setRecordMode(state);
			cancelSaveOrLoad();
		}
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
				cancelSaveOrLoad();
				ui_mode = EDIT_PARAM_MODE;
				current_param = PARAM_EFFECT_DEPTH;
				onEncoderIncrement(0);
			} else {
				ui_mode = SEQUENCE_MODE;
			}
			sequencerVar2->onMutateButton(state);
			
		}
	}
}

void Ui::selectStep(int step){
	cancelSaveOrLoad();
	sequencerVar2->selectStep(step+current_bar*16);
	if (ui_mode == SEQUENCE_MODE) {
		ledMatrix.setMatrixFromSequencer(current_bar);
	}
	//ledMatrix.blinkLed();
    analogIo.displaySelectedParam();
	displaySequenceParam();
	buttons.setGlideLed(sequencerVar2->getGlide());
}

bool calibration_matrix[16] = {1,1,1,1, 1,1,1,1, 1,0,0,0, 0,0,1,1};

void Ui::initializeCalibrationMode() {
	cancelSaveOrLoad();
	ui_mode = CALIBRATE_MODE;
	calibrationVar2->readCalibrationValues();
	updateCalibration(calibration_step);
	display.setDisplayAlpha("CAL");
	ledMatrix.setMatrix(calibration_matrix);
	digitalWrite(GATE_PIN, HIGH); //to make signals audible
}

void Ui::clearSequence(){
	display.setDisplayAlpha("CLR");
	sequencerVar2->clearSequence();
	current_bar = 0;
	ledMatrix.setMatrixFromSequencer(current_bar);
}

void Ui::updateCalibration(int step) {
    //deal with special options - display mode alpha/numeric on buttons 15/16
	if (step == 15) {
		display.setDisplayAlpha("NUM");
		display.blinkDisplay(true, 100, 3);
		calibrationVar2->writeDisplayModeValue(0);
		analogIo.setDisplayMode(0);
		return;
	} else if (step == 14) {
		display.setDisplayAlpha("NOT");
		display.blinkDisplay(true, 100, 3);
		calibrationVar2->writeDisplayModeValue(127);
		analogIo.setDisplayMode(127);
		return;
	}

	if (step == 8) {
		calibration_step = step;
		display.setDisplayAlpha("BRT");
		return;
	}
	
	//deal with CV output calibration increment octaves
	if (step > 8) return;
    calibration_step = step;
	//ledMatrix.reset();
	ledMatrix.selectStep(step);
	ledMatrix.setMatrix(calibration_matrix);
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
	displaySequenceParam();
}

void Ui::onStepIncremented(){
	if (sequencerVar2->timeForNextSequence()) {
		loadNextSequence();
	}

	ledMatrix.setMatrixFromSequencer(current_bar);
	ledMatrix.blinkCurrentStep();
	if (record_mode) {
		analogIo.recordCurrentParam();
	}
	sequencerVar2->setActiveNote(); //takes place here to enable real-time recording to be heard immediately
}

void Ui::loadNextSequence(){
	int next_seq = sequencerVar2->getSongNextSeq();
	if (next_seq > 0 && memory.patchExists(next_seq)) {
		current_patch = next_seq;
		selected_patch = current_patch;
		memory.load(next_seq);
		display.setDisplayNum(current_patch);
		display.blinkDisplay(true, 100, 5);
	}
}

bool Ui::cancelSaveOrLoad(){
	encoder_bumped = false;

	if (ui_mode == LOAD_MODE || ui_mode == SAVE_MODE || ui_mode == EDIT_PARAM_MODE || ui_mode == CALIBRATE_MODE) {
		if (ui_mode == CALIBRATE_MODE) {
			digitalWrite(GATE_PIN, LOW);
		}
		initializeSequenceMode();
		display.blinkDisplay(true, 100, 1);
		return true;
	}
	erase_counter = 0;
	display.blinkDisplay(false, 1, 1);
	return false;
}

}