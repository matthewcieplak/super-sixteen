#include <Arduino.h>
#include "Variables.h"
#include "Pinout.h"
#include "Dac.h"
#include "Calibrate.h"
#include "Sequencer.h"

namespace supersixteen{

int pitch_matrix[16];
int octave_matrix[16];
int duration_matrix[16];
int cv_matrix[16];

bool step_matrix[16] = { 1,0,0,0, 1,1,0,0, 1,1,1,0, 1,1,1,1 };
bool glide_matrix[16];

int selected_step;
uint8_t current_step = 15;
uint8_t prev_step = 14;
uint8_t active_step;
uint8_t sequence_length;

bool gate_active = false;
bool clock_out_active = false;
bool clock_in_active = false;
bool step_incremented = false;

double tempo_bpm = 120;
unsigned int tempo_millis = 15000 / tempo_bpm; //would be 60000 but we count 4 steps per "beat"
bool play_active = 1;


int prev_note = 0;
int active_note = 0;
int glide_duration = 50;
int calculated_tempo = tempo_millis;
double current_note_value = 0;
Calibration *calibrationVar;
Dac *dacVar;

const int CLOCK_PULSE_DURATION = 10; //milliseconds pulse width of clock output
elapsedMillis timekeeper;


void Sequencer::init(Calibration& calibration, Dac& dac) {
	calibrationVar = &calibration;
	dacVar = &dac;
	for (int i = 0; i < 16; i++) {
		duration_matrix[i] = 80;
	}

	pinMode(GATE_PIN, OUTPUT);
	pinMode(CLOCK_OUT_PIN, OUTPUT);
	pinMode(CLOCK_IN_PIN, INPUT_PULLUP);
	pinMode(RESET_PIN, INPUT_PULLUP);
}

void Sequencer::updateClock() {
	if (play_active && timekeeper > tempo_millis) {
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
	//TODO chase leds
	//led_matrix[current_step] = step_matrix[current_step]; //reset previous LED
	prev_step = current_step;
	current_step++;
	if (current_step == 16) {
		current_step = 0;
	}
	//led_matrix[current_step] = !step_matrix[current_step]; //set current led

	if (step_matrix[current_step]) {
		//PITCH/OCTAVE
		active_step = current_step;
		prev_note = active_note;
		active_note = (octave_matrix[active_step] + 2) * 12 + pitch_matrix[active_step];

		if (glide_matrix[active_step]) {
			updateGlide();
		} else {
			current_note_value = calibrationVar->getCalibratedOutput(active_note);
			dacVar->setOutput(0, GAIN_2, 1, current_note_value);

		}
	
		//GATE
		digitalWrite(GATE_PIN, step_matrix[active_step] ? HIGH : LOW);
		gate_active = step_matrix[active_step];
	}

	step_incremented = true;

	// TEST running display number
	// setDisplayNum(current_step);
}

int Sequencer::getCurrentStep(){
	return current_step;
}

int Sequencer::getPrevStep(){
	return prev_step;
}

bool Sequencer::stepWasIncremented(){
	return step_incremented;
}


void Sequencer::updateGlide() {
	if (step_matrix[active_step] && glide_matrix[active_step]) {
		int steps_advanced = current_step - active_step;
		if (steps_advanced < 0) {
			steps_advanced = current_step + sequence_length - active_step;
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
		steps_advanced = current_step + sequence_length - active_step + 1;
	}
	if (gate_active && duration_matrix[active_step] < percent_step * steps_advanced) {
		digitalWrite(GATE_PIN, LOW);
		gate_active = false;
	}

	if (clock_out_active && timekeeper > CLOCK_PULSE_DURATION) {
		digitalWrite(CLOCK_OUT_PIN, LOW);
	}
}

void Sequencer::onPlayButton(){
	play_active = !play_active;
	timekeeper = 0;
	calculated_tempo = tempo_millis;
}

int Sequencer::incrementTempo(int amount){
	tempo_bpm += amount;
	if (tempo_bpm < 20) tempo_bpm = 20;
	if (tempo_bpm > 500) tempo_bpm = 500;
	//display_param = TEMPO_PARAM;
	//setDisplayNum(tempo_bpm);
	tempo_millis = 15000 / tempo_bpm;
	return tempo_bpm;
}

void Sequencer::selectStep(int stepnum){
	if (selected_step == stepnum || !step_matrix[stepnum]) { //require 2 presses to turn active steps off, so they can be selected/edited without double-tapping //TODO maybe implement hold-to-deactivate
        step_matrix[stepnum] = !step_matrix[stepnum];
        //todo chase leds
		//led_matrix[stepnum] = step_matrix[stepnum];
    }
    selected_step = stepnum;
}

bool Sequencer::getStepOnOff(int stepnum){
	return step_matrix[stepnum];
}

bool Sequencer::toggleGlide(){
	glide_matrix[selected_step] = !glide_matrix[selected_step];
	return glide_matrix[selected_step];
}

bool Sequencer::setPitch(int newVal){
	bool changed = pitch_matrix[selected_step] != newVal;
	pitch_matrix[selected_step] = newVal;
	return changed;
}
bool Sequencer::setOctave(int newVal){
	bool changed = octave_matrix[selected_step] != newVal;
	octave_matrix[selected_step] = newVal;
	return changed;
}
bool Sequencer::setDuration(int newVal){
	bool changed = duration_matrix[selected_step] != newVal;
	duration_matrix[selected_step] = newVal;
	return changed;
}
bool Sequencer::setCv(int newVal){
	bool changed = cv_matrix[selected_step] != newVal;
	cv_matrix[selected_step] = newVal;
	return changed;
}

bool Sequencer::getGlide(){
	return glide_matrix[selected_step];
}

int Sequencer::getPitch(){
	return pitch_matrix[selected_step];
}
int Sequencer::getOctave(){
	return octave_matrix[selected_step];
}
int Sequencer::getDuration(){
	return duration_matrix[selected_step];
}
int Sequencer::getCv(){
	return cv_matrix[selected_step];
}

bool *Sequencer::getStepMatrix(){
	return step_matrix;
}

int Sequencer::getSelectedStep(){
	return selected_step;
}

}