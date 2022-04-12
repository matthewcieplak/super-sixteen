#include <Arduino.h>
#include <SPI.h>
#include <stdint.h>

#include "Variables.h"
#include "Pinout.h"
#include "AnalogIO.h"
#include "Display.h"
#include "Sequencer.h"

namespace supersixteen {

elapsedMillis analogMultiplexorInc;

const int analog_pins[4] = { 
	ANALOG_PIN_1,
	ANALOG_PIN_2,
	ANALOG_PIN_3,
	ANALOG_PIN_4
};

#define PITCH_PARAM 0
#define OCTAVE_PARAM 1
#define DURATION_PARAM 2
#define CV_PARAM 3
#define MODE_PARAM 4

#define DISPLAY_MODE_NUMERIC 0
#define DISPLAY_MODE_NOTE_NAME 127

#define DEFAULT_CHANGE_THRESHOLD 10
#define LOW_CHANGE_THRESHOLD 2

const int analog_params[4] = { PITCH_PARAM, OCTAVE_PARAM, DURATION_PARAM, CV_PARAM };
bool editing = false;
bool audition = false;

int lastAnalogValues[4];
int analogValues[4];
int analogMultiplexor = 0;

int display_param = PITCH_PARAM;
int display_num = 0;
int display_mode = 0; //0 = note numbers, 127 = note names (C1, B3, etc)
char display_alpha[4];
bool param_changed = false;

int change_threshold = DEFAULT_CHANGE_THRESHOLD;
bool recording = false;
bool recorded_input_active = false;
char modename[5];



Sequencer* sequencerVar;

void AnalogIo::init(Sequencer& sequencer){
	pinMode(ANALOG_PIN_1, INPUT);
	pinMode(ANALOG_PIN_2, INPUT);
	pinMode(ANALOG_PIN_3, INPUT);
	pinMode(ANALOG_PIN_4, INPUT);
	sequencerVar = &sequencer;
	//initialize analog knob positions to avoid weird edits
	poll(false);
	poll(false);
	poll(false);
	poll(false);
	editing = true;
}

void AnalogIo::poll(bool shift_state) {
	analogMultiplexor += 1;
	if (analogMultiplexor > 3) {
		analogMultiplexor = 0;
	}
	
	if (display_param == analog_params[analogMultiplexor]) {
		change_threshold = LOW_CHANGE_THRESHOLD; //increase sensitivity when param is selected, decrease otherwise to reduce accidental "bump" changes
	} else {
		change_threshold = DEFAULT_CHANGE_THRESHOLD;
	}
	readInput(analogMultiplexor, shift_state, editing);
}

void AnalogIo::pollCalibration(){
	change_threshold = LOW_CHANGE_THRESHOLD;
	param_changed = readInput(3, false, false);
}

bool AnalogIo::readInput(int i, bool shift_state, bool write_values){
	// if (i > 3 || i < 0) return; //sometimes we get desynced by interrupts, and analogRead on a wrong pin is fatal
	analogValues[i] = analogRead(analog_pins[i]);
	param_changed = false;

	if (abs(analogValues[i] - lastAnalogValues[i]) > change_threshold) {
		recorded_input_active = true;
		lastAnalogValues[i] = analogValues[i];
		if (!write_values) {
			return true;
		}
		switch (i) {
		case PITCH_PARAM: shift_state ? setAudition(analogValues[0]) : setPitch(analogValues[0]); break;
		case OCTAVE_PARAM: setOctave(analogValues[1]); break;
		case DURATION_PARAM: setDuration(analogValues[2]); break;
		case CV_PARAM: shift_state ? setCVMode(analogValues[3]) : setCV(analogValues[3]); break;
		}
		return true;
	}
	return false;
}

void AnalogIo::setPitch(int analogValue) {
	display_param = PITCH_PARAM;
	//calibration_value = (float(analogValues[1]) + 1024.0) / 1500.0 ; //11.60
	int newVal = analogValue / 42.1 - 12.1; //convert from 0_1024 to 0_88 to -12_0_12
	if (recording || sequencerVar->setPitch(newVal)) {
		setDisplayNum(newVal);
		//if (display_mode == DISPLAY_MODE_NOTE_NAME)
		displayPitchName();
		if (audition) sequencerVar->auditionNote(true, 120);
	}
}

void AnalogIo::setOctave(int analogValue) {
	display_param = OCTAVE_PARAM;
	int newVal = analogValue / 120 - 4; //convert from 0-1024 to -4_0_4
	if (recording || sequencerVar->setOctave(newVal)) {
		setDisplayNum(newVal);
		//if (display_mode == DISPLAY_MODE_NOTE_NAME)
		displayPitchName();
		if (audition) sequencerVar->auditionNote(true, 120);
	}

}

void AnalogIo::setDuration(long analogValue) { //need extra bits for exponent operation
	display_param = DURATION_PARAM;
	int newVal = analogValue * analogValue / 2615; //convert from 0-1024 to 0-400 with exponential curve
	if (recording || sequencerVar->setDuration(newVal)) setDisplayNum(newVal);
}

void AnalogIo::setCV(int analogValue) {
	display_param = CV_PARAM;
	if (recording || sequencerVar->setCv2(analogValue)) {
		setDisplayNum(sequencerVar->getCv2DisplayValue());
		displayCvName();
	}
}


void AnalogIo::displaySelectedParam() {
	//update display to show currently selected step value if applicable
	switch (display_param) {
	case PITCH_PARAM: setDisplayNum(sequencerVar->getPitch()); displayPitchName(); break;		
	case OCTAVE_PARAM: setDisplayNum(sequencerVar->getOctave()); displayPitchName(); break;
	case DURATION_PARAM: setDisplayNum(sequencerVar->getDuration()); break;
	case CV_PARAM:       setDisplayNum(sequencerVar->getCv()); displayCvName(); break;
	}
}

void AnalogIo::setDisplayNum(int displayNum){
	display_num = displayNum;
	param_changed = true;
}

void AnalogIo::setDisplayAlpha(char displayAlpha[]){
	for (int i = 0; i < 3; i++) {
		display_alpha[i] = displayAlpha[i];
	}
	// display_alpha[0] = 0B00000000;
	// display_alpha[1] = 'D';
	// display_alpha[2] = '2';
	// display_alpha = "ABC";
	param_changed = true;
}

void AnalogIo::displayPitchName(){
	setDisplayAlpha(sequencerVar->getPitchName(sequencerVar->getMidiPitch(sequencerVar->getPitch(), sequencerVar->getOctave()))); 
}

void AnalogIo::displayCvName(){
	if (sequencerVar->getCvMode() == 3) //NOTE MODE
		setDisplayAlpha(sequencerVar->getPitchName(sequencerVar->getMidiPitch(sequencerVar->getCv(), -3)));
}


void AnalogIo::recordCurrentParam(){
	if (!recorded_input_active) return;
	// if (sequencerVar->currentStepActive()) {
		recording = false; //temporarily flip to enable one-step record
		change_threshold = -1; //record regardless of motion
		readInput(display_param, false, true); // read the currently selected param and write it to the sequence	
		// change_threshold = DEFAULT_CHANGE_THRESHOLD;
		recording = true;
	// }
}

void AnalogIo::setRecordMode(bool state){
	recorded_input_active = false;
	recording = state;
}

void AnalogIo::setDisplayMode(int mode){
	display_mode = mode;
}

int AnalogIo::getDisplayNum(){
	return display_num;
}

char * AnalogIo::getDisplayAlpha(){
	return display_alpha;
}

bool AnalogIo::paramChanged(){
	return param_changed;
}

bool AnalogIo::paramIsAlpha(){
	return ((display_mode == DISPLAY_MODE_NOTE_NAME) && (display_param == PITCH_PARAM || display_param == OCTAVE_PARAM)) || 
			(display_param == MODE_PARAM) || 
			(display_param == CV_PARAM && sequencerVar->getCvMode() == 3);
}

void AnalogIo::setCVMode(int analogValue){
	display_param = MODE_PARAM;
	uint8_t cv_mode = 0;
	if (analogValue < 256) {
		cv_mode = 0;
	} else if (analogValue < 512) {
		cv_mode = 1;
	} else if (analogValue < 768) {
		cv_mode = 2;
	} else {
		cv_mode = 3;
	}

	strcpy_P(modename, (char *)pgm_read_word(&(cvmode_names[cv_mode])));  // Necessary casts and dereferencing, just copy (for PROGMEM keywords in flash)


	setDisplayAlpha(modename);
	sequencerVar->setCVMode(cv_mode); 
	param_changed = true;
	return;
}

void AnalogIo::setAudition(int analogValue){

	display_param = MODE_PARAM;
	if (analogValue > 512) {
		audition = true;
		strcpy_P(modename, (char *)pgm_read_word(&(audition_names[1])));  // Necessary casts and dereferencing, just copy (for PROGMEM keywords in flash)
	} else {
		strcpy_P(modename, (char *)pgm_read_word(&(audition_names[0])));  // Necessary casts and dereferencing, just copy (for PROGMEM keywords in flash)
		audition= false;
		sequencerVar->auditionNote(false, 1);
	}
	setDisplayAlpha(modename);

	// setDisplayAlpha(modename);
	param_changed = true;
	// sequencerVar->setAudition(audition);
	return;
}

int AnalogIo::getCalibrationValue(){
	int calibration_value = (analogValues[3] - 512) / 10; //convert from 0_1024 to 0_88 to -12_0_12
	setDisplayNum(calibration_value);
	return calibration_value;
}


};
