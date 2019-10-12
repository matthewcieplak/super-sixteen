#include <Arduino.h>
#include "Variables.h"
#include "Pinout.h"
#include "Dac.h"
#include "Calibrate.h"
#include "Sequencer.h"
#include "Scales.h"
namespace supersixteen{

const byte SEQUENCE_MAX_LENGTH = 64;

sequence active_sequence;

int8_t step_presets[] = { 4, 8, 12, 16, 24, 32, 48, 64 };

int selected_step = 0;
int8_t clock_step = -1;
int8_t current_step = -1;
int8_t active_step;

uint8_t repeat_step_origin = 0;
//uint8_t repeat_step_counter = 0;

bool gate_active = false;
bool clock_out_active = false;
bool clock_in_active = false;
bool step_incremented = false;

byte tempo_bpm = 120;
unsigned int tempo_millis = 15000 / tempo_bpm; //would be 60000 but we count 4 steps per "beat"
bool play_active = 0;
bool seq_effect_mode = false;
bool seq_record_mode = false;
bool current_scale_tones[13];


int prev_note = 0;
int active_note = 0;
int active_pitch = 0;
int glide_duration = 50;
int calculated_tempo = tempo_millis;
double tempo_millis_swing_odd;
double tempo_millis_swing_even;

double current_note_value = 0;
Calibration *calibrationVar;
Dac *dacVar;

const int CLOCK_PULSE_DURATION = 10; //milliseconds pulse width of clock output
elapsedMillis timekeeper;


void Sequencer::init(Calibration& calibration, Dac& dac) {
	calibrationVar = &calibration;

	dacVar = &dac;
	for (byte i = 0; i < SEQUENCE_MAX_LENGTH; i++) {
		active_sequence.duration_matrix[i] = 80;
	}

	pinMode(GATE_PIN, OUTPUT);
	pinMode(CLOCK_OUT_PIN, OUTPUT);
	pinMode(CLOCK_IN_PIN, INPUT_PULLUP);
	pinMode(RESET_PIN, INPUT_PULLUP);

    active_sequence.scale = 0;
	incrementScale(0);
	incrementTempo(0);
}

void Sequencer::updateClock() {
	if (play_active) {
		if (timekeeper > (clock_step % 2 == 1 ? tempo_millis_swing_even : tempo_millis_swing_odd)) {
			//CLOCK
			//increment_step();
			if (clock_out_active) {
				digitalWrite(CLOCK_OUT_PIN, LOW);
				clock_out_active = false;
			}
			else {
				digitalWrite(CLOCK_OUT_PIN, HIGH);
				clock_out_active = true;
			}
			incrementStep();
			timekeeper = 0;
		} else {
			step_incremented = false;
		}
		updateGlide();
		updateGate();
	}

	//todo enable clock in
	// if (digitalRead(CLOCK_IN_PIN) == LOW) {
	// 	if(!clock_in_active) {
	// 		clock_in_active = true;
	// 		play_active = false; //read the hardware input, which is normally connected to the hardware output, as the internal clock
	// 		calculated_tempo = timekeeper;
	// 		timekeeper = 0;
	// 		incrementStep();
	// 	}
	// } else {
	// 	clock_in_active = false;
	// }
}

void Sequencer::incrementStep() {
	clock_step++;
	if (clock_step >= active_sequence.sequence_length) {
		clock_step = 0;
	}
	//prev_step = current_step;

	if (seq_effect_mode && active_sequence.effect == EFFECT_REPEAT) {
		//repeat_step_counter++;
		if (current_step == repeat_step_origin){
			current_step = current_step - active_sequence.effect_depth + 1;
			if (current_step < 0) {
				current_step = active_sequence.sequence_length + current_step;
			}
		} else {
			current_step++;
			if (current_step == active_sequence.sequence_length) {
				current_step = 0;
			}
		}
	} else {
		current_step = clock_step;
	}

	if (active_sequence.step_matrix[current_step]) {
		active_step = current_step;
		prev_note = active_note;
	}

	step_incremented = true;
	

	// TEST running display number
	// setDisplayNum(current_step);
}

void Sequencer::setActiveNote(){
	//PITCH/OCTAVE
	if (active_sequence.step_matrix[current_step]) {
		quantizeActivePitch();
		active_note = (active_sequence.octave_matrix[active_step] + 2) * 12 + active_pitch + active_sequence.transpose;
		if (seq_effect_mode && active_sequence.effect == EFFECT_OCTAVE) {
			active_note += (active_sequence.effect_depth - 4) * 12;
		}
		if (active_sequence.glide_matrix[active_step]) {
			updateGlide();
		} else {
			current_note_value = calibrationVar->getCalibratedOutput(active_note);
			dacVar->setOutput(0, GAIN_2, 1, current_note_value);
		}

		//GATE
		digitalWrite(GATE_PIN, active_sequence.step_matrix[active_step] ? HIGH : LOW);
		gate_active = active_sequence.step_matrix[active_step];
	}
}

void Sequencer::quantizeActivePitch(){
	active_pitch = active_sequence.pitch_matrix[active_step];
	if (current_scale_tones[active_pitch >= 0 ? active_pitch : active_pitch + 12] == false) {
		active_pitch += quantize_map[active_pitch >= 0 ? active_pitch : active_pitch + 12];
	}
}

int Sequencer::getCurrentStep(){
	return current_step;
}

bool Sequencer::stepWasIncremented(){
	return step_incremented;
}


void Sequencer::updateGlide() {
	if (active_sequence.step_matrix[active_step] && active_sequence.glide_matrix[active_step]) {
		int steps_advanced = current_step - active_step;
		if (steps_advanced < 0) {
			steps_advanced = current_step + active_sequence.sequence_length - active_step;
		}
		int glidekeeper = timekeeper + steps_advanced * calculated_tempo;
		int glide_time = float(glide_duration) / 100.0 * calculated_tempo;
		if (glidekeeper < glide_time) {
			//double instantaneous_pitch = ((active_note * timekeeper) + prev_note * (tempo - timekeeper)) / double(tempo);
			double instantaneous_pitch = ((active_note * glidekeeper) + prev_note * (glide_time - glidekeeper)) / double(glide_time);
			current_note_value = calibrationVar->getCalibratedOutput(instantaneous_pitch);
			dacVar->setOutput(0, GAIN_2, 1, current_note_value);

		}
	}
}

void Sequencer::updateGate() {
	double percent_step = timekeeper / (double)calculated_tempo * 100.0;
	int steps_advanced = current_step - active_step + 1;
	if (steps_advanced < 1) {
		steps_advanced = current_step + active_sequence.sequence_length - active_step + 1;
	}
	if (gate_active && active_sequence.duration_matrix[active_step] < percent_step * steps_advanced) {
		digitalWrite(GATE_PIN, LOW);
		gate_active = false;
	}

	if (clock_out_active && timekeeper > CLOCK_PULSE_DURATION) {
		digitalWrite(CLOCK_OUT_PIN, LOW);
	}
}

void Sequencer::onPlayButton(){
	play_active = !play_active;
	if (!play_active) { 	
		digitalWrite(GATE_PIN, LOW);
		gate_active = false;
	}
	timekeeper = 0;
	calculated_tempo = tempo_millis;
}

void Sequencer::onReset(){
	timekeeper = tempo_millis;
	clock_step = -1;
	step_incremented = false;
}

void Sequencer::onBarSelect(byte bar){

}

int Sequencer::incrementTempo(int amount){
	tempo_bpm = getMinMaxParam(tempo_bpm, amount, 20, 250);
	tempo_millis = 15000 / tempo_bpm;
	tempo_millis_swing_odd = tempo_millis * float(active_sequence.swing) / 50.0;
	tempo_millis_swing_even = tempo_millis * 2 - tempo_millis_swing_odd;
	active_sequence.sequence_tempo = tempo_bpm;
	return tempo_bpm;
}

int Sequencer::incrementScale(int amount){
	active_sequence.scale = getMinMaxParam(active_sequence.scale, amount, 0, 9);
	loadScale(active_sequence.scale);
	return active_sequence.scale;
}

int Sequencer::incrementEffect(int amount){
	uint8_t &effect = active_sequence.effect;
	setMinMaxParamUnsigned(effect, amount, 0, 5);	
	incrementEffectDepth(0);
	return active_sequence.effect;
}


int Sequencer::incrementEffectDepth(int amount){
	uint8_t &depth = active_sequence.effect_depth;
	switch(active_sequence.effect) {
		case EFFECT_REPEAT:  setMinMaxParamUnsigned(depth, amount, 1, 16); break;
		case EFFECT_OCTAVE:  setMinMaxParamUnsigned(depth, amount, 0, 8); return active_sequence.effect_depth - 4; break;//this is displayed as -4/+4 in UI
		case EFFECT_GLIDE:   setMinMaxParamUnsigned(depth, amount, 1, 100); break;
		case EFFECT_REVERSE: setMinMaxParamUnsigned(depth, amount, 0, 1); break;
		case EFFECT_STOP:    setMinMaxParamUnsigned(depth, amount, 10, 100); break;
		case EFFECT_FREEZE:  setMinMaxParamUnsigned(depth, amount, 0, 1); break;

	}
	return active_sequence.effect_depth;
}

int Sequencer::incrementSteps(int amount, bool shift_state){
	if (shift_state && amount != 0) {
		byte i = 1;
		while (active_sequence.sequence_length > step_presets[i] && i < sizeof(step_presets)/sizeof(step_presets[0])-2) { i++; }
		return active_sequence.sequence_length = step_presets[amount > 0 ? i+1 : i-1];
	} else {
		return active_sequence.sequence_length = getMinMaxParam(active_sequence.sequence_length, amount, 1, SEQUENCE_MAX_LENGTH);
	}
}

int Sequencer::incrementBars(int amount){
	return active_sequence.bars = getMinMaxParam(active_sequence.bars, amount, 1, 4);
}

int Sequencer::incrementSwing(int amount){
	active_sequence.swing = getMinMaxParam(active_sequence.swing, amount, 10, 90);
	incrementTempo(0);
	return active_sequence.swing;
}

int Sequencer::incrementTranspose(int amount){
	return active_sequence.transpose = getMinMaxParam(active_sequence.transpose, amount, -36, 36);
}


void Sequencer::selectStep(int stepnum){
	if (selected_step == stepnum || !active_sequence.step_matrix[stepnum]) { //require 2 presses to turn active steps off, so they can be selected/edited without double-tapping //TODO maybe implement hold-to-deactivate
        active_sequence.step_matrix[stepnum] = !active_sequence.step_matrix[stepnum];
    }
    selected_step = stepnum;
}

int Sequencer::getMinMaxParam(int param, int increment, int min, int max) {
	param += increment;
	if (param > max) param = max;
	else if (param < min) param = min;
	return param;
}

uint8_t Sequencer::setMinMaxParamUnsigned(uint8_t& param, int8_t increment, uint8_t min, uint8_t max) {
	if (param == 0 && increment < 0) return param;
	param += increment;
	if (param > max) param = max;
	else if (param < min) param = min;
	return param;
}

int8_t Sequencer::setMinMaxParam(int8_t& param, int8_t increment, int8_t min, int8_t max) {
	param += increment;
	if (param > max) param = max;
	else if (param < min) param = min;
	return param;
}


bool Sequencer::getStepOnOff(int stepnum){
	return active_sequence.step_matrix[stepnum];
}

bool Sequencer::toggleGlide(){
	active_sequence.glide_matrix[selected_step] = !active_sequence.glide_matrix[selected_step];
	return active_sequence.glide_matrix[selected_step];
}

bool Sequencer::setPitch(int newVal){
	//quantize pitches to scale
	if (current_scale_tones[newVal >= 0 ? newVal : newVal + 12] == false) return false;

	bool changed = active_sequence.pitch_matrix[editedStep()] != newVal;
	active_sequence.pitch_matrix[editedStep()] = newVal;
	return changed;
}
bool Sequencer::setOctave(int8_t newVal){
	bool changed = active_sequence.octave_matrix[editedStep()] != newVal;
	active_sequence.octave_matrix[editedStep()] = newVal;
	return changed;
}
bool Sequencer::setDuration(uint16_t newVal){
	bool changed = active_sequence.duration_matrix[editedStep()] != newVal;
	active_sequence.duration_matrix[editedStep()] = newVal;
	return changed;
}
bool Sequencer::setCv(int newVal){
	bool changed = active_sequence.cv_matrix[editedStep()] != newVal;
	active_sequence.cv_matrix[editedStep()] = newVal;
	return changed;
}

void Sequencer::setTempoFromSequence(){
	if (play_active) return;
	tempo_millis = 15000 / active_sequence.sequence_tempo;
}

uint8_t Sequencer::editedStep(){
	return (seq_record_mode ? active_step : selected_step);
}

bool Sequencer::getGlide(){
	return active_sequence.glide_matrix[selected_step];
}

int Sequencer::getPitch(){
	return active_sequence.pitch_matrix[selected_step];
}
int Sequencer::getOctave(){
	return active_sequence.octave_matrix[selected_step];
}
int Sequencer::getDuration(){
	return active_sequence.duration_matrix[selected_step];
}
int Sequencer::getCv(){
	return active_sequence.cv_matrix[selected_step];
}

bool *Sequencer::getStepMatrix(){
	return active_sequence.step_matrix;
}

int Sequencer::getSelectedStep(){
	return selected_step;
}

void Sequencer::setEffectMode(bool state){
	seq_effect_mode = state;
	repeat_step_origin  = current_step;
}


void Sequencer::setRecordMode(bool state){
	seq_record_mode = state;
}

sequence& Sequencer::getActiveSequence(){
	return active_sequence;
}

sequence * Sequencer::getSequence(){
	return &active_sequence;
}

void Sequencer::clearSequence(){
	for (byte i = 0; i < SEQUENCE_MAX_LENGTH; i++) {
		active_sequence.pitch_matrix[i] = 0;
		active_sequence.octave_matrix[i] = 0;
		active_sequence.duration_matrix[i] = 80;
		active_sequence.cv_matrix[i] = 0;
		active_sequence.step_matrix[i] = false;
		active_sequence.glide_matrix[i] = false;
	}
	active_sequence.glide_length = 50;
	active_sequence.sequence_length = 16;
	active_sequence.bars = 1;
	active_sequence.scale = 1;

	active_sequence.swing = 50;
	active_sequence.effect = 0; //mutate = repeat (0), reverse (1), octave shift (2), auto-glide (3), hold  (4)
	active_sequence.effect_depth = 4;
    //active_sequence.sequence_tempo = 120; //might be done in real time? probably not a good idea to change
    active_sequence.transpose = 0;
}

void Sequencer::loadScale(uint8_t scale){
	for (byte k = 0; k < 13; k++) {
    	current_scale_tones[k] = (bool)pgm_read_byte_near(scale_tones[scale] + k);
  	}
}

}