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
int8_t active_step = -1;
int8_t step_recording_initiated_step = 0;

uint8_t prev_sequence_length = active_sequence.sequence_length;
uint8_t repeat_step_origin = 0;
//uint8_t repeat_step_counter = 0;

bool gate_active = false;
bool clock_out_active = false;
bool clock_in_active = false;
bool reset_in_active = false;
bool step_incremented = false;
bool step_recording_mode = false;
bool first_step = true;

byte tempo_bpm = 120;
unsigned int tempo_millis = 15000 / tempo_bpm; //would be 60000 but we count 4 steps per "beat"
bool play_active = 0;
bool seq_effect_mode = false;
bool seq_record_mode = false;
bool seq_recording_effect = false;
bool mutate_button = false;

bool current_scale_tones[13];
bool note_reached;
bool skip_next_external_step = false;
bool count_next_swing_step = false;



int prev_note = 0;
int active_note = 0;
int active_pitch = 0;
int calculated_tempo = tempo_millis;
unsigned int calculated_step_length = 10;
unsigned int calculated_roll;
unsigned int calculated_stutter;
double tempo_millis_swing_odd;
double tempo_millis_swing_even;
int glide_duration = 50;
int glide_time; //
int random_octave = 0;


double current_note_value = 0;
Calibration *calibrationVar;
Dac *dacVar;

static const int ROLL_PAUSE_DURATION = 5;
const int CLOCK_PULSE_DURATION = 10; //milliseconds pulse width of clock output
elapsedMillis timekeeper;
unsigned int stepkeeper;


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

	attachInterrupt(digitalPinToInterrupt(CLOCK_IN_PIN), onClockIn, FALLING);
	// attachInterrupt(digitalPinToInterrupt(RESET_PIN), onResetIn, FALLING);

    active_sequence.scale = 0;
	prev_sequence_length = active_sequence.sequence_length;
	incrementScale(0);
	incrementTempo(0);
	updateGlideCalc();

}

void Sequencer::updateClock() {
	if (play_active || count_next_swing_step) {
		if (timekeeper > (clock_step % 2 == 1 ? tempo_millis_swing_even : tempo_millis_swing_odd)) {
			
			if (play_active) {
				incrementStep();
				timekeeper = 0;
				return;

			} else if (count_next_swing_step) {
				incrementStep();
				timekeeper = 0;
				count_next_swing_step = false;
				return;
			}
		} else {
			step_incremented = false;
		}
	}

	
	updateGlide();
	updateGate();

	bool reset = digitalRead(RESET_PIN);
	if (reset == LOW) {
		if (!reset_in_active) {
			onReset(); 
			reset_in_active = true;
		}
	} else {
		reset_in_active = false;
	}


	if(clock_in_active) {
		onClock();
		clock_in_active = false;
	} else {
		step_incremented = false;
	}

}

void Sequencer::onClock(){
	if (first_step) {
		timekeeper = tempo_millis;
		first_step = false;
	}

	clock_in_active = true;
	if (skip_next_external_step) {
		skip_next_external_step = false;
		return;
	}

	if ((active_sequence.swing > 50) && (clock_step % 2 == 0)) { //ignore external clock to implement swing
	 	skip_next_external_step = true;
		count_next_swing_step = true; //these two states *should* stay in sync but in practice microtiming requires two vars for values near 50
	} 

	
	calculated_tempo = timekeeper;
	updateSwingCalc();
	incrementStep();
	timekeeper = 0;
		
	play_active = false;
}

void Sequencer::incrementStep() {
	clock_step++;
	if (clock_step >= active_sequence.sequence_length) {
		clock_step = 0;
	}

	if (!clock_out_active) {
		clock_out_active = true;
		digitalWrite(CLOCK_OUT_PIN, HIGH);
	}

	if (seq_recording_effect) { //while recording, respect mutate button state and record active/inactive to current step
		active_sequence.effect_matrix[current_step] = mutate_button;
	} else if (active_sequence.effect_matrix[current_step]) { //if mutation data is recorded, play it back
		if (!seq_effect_mode) setEffectMode(true); //don't re-trigger effect mode, to avoid messing up timings set by prev steps
	} else if (seq_effect_mode && !mutate_button) {
		setEffectMode(false); //turn off sequenced effect after recorded activity (when not manually engaged) 
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
	} else if (seq_effect_mode && active_sequence.effect == EFFECT_REVERSE) {
		current_step--;
		if (current_step < 0) {
			current_step = active_sequence.sequence_length - 1;
		}
	} else if (seq_effect_mode && active_sequence.effect == EFFECT_FREEZE) {
		//don't increment step
	} else {
		current_step = clock_step;
	}

	// if (step_recording_mode) {
	// 	active_sequence.step_matrix[current_step] = true;
	// }



	if (active_sequence.step_matrix[current_step]) {
		active_step = current_step;
		prev_note = active_note;
	}

	step_incremented = true;
	//setActiveNote();

	

	// TEST running display number
	// setDisplayNum(current_step);
}

void Sequencer::setActiveNote(){
	//PITCH/OCTAVE


	if (active_sequence.step_matrix[current_step]) {
		if (seq_effect_mode && active_sequence.effect == EFFECT_STOP) {
			updateGlide(); 
			if (!note_reached) { //stop gate after glide reaches zero
				digitalWrite(GATE_PIN, active_sequence.step_matrix[active_step] ? HIGH : LOW);
				gate_active = active_sequence.step_matrix[active_step];
			}
		} else {
			note_reached = false;
			quantizeActivePitch();
			active_note = ((active_sequence.octave_matrix[active_step] + 3) * 12) + 
							active_pitch + 
							(active_sequence.transpose - 24)  + 
						    (random_octave * 12);
			if (seq_effect_mode && active_sequence.effect == EFFECT_OCTAVE) {
				active_note += (active_sequence.effect_depth - 4) * 12;
			}
			if (active_sequence.glide_matrix[active_step]) {
				updateGlide();
			} else {
				current_note_value = calibrationVar->getCalibratedOutput(active_note);
				dacVar->setOutput(0, GAIN_2, 1, current_note_value);
			}

			digitalWrite(GATE_PIN, active_sequence.step_matrix[active_step] ? HIGH : LOW);
			gate_active = active_sequence.step_matrix[active_step];
			int cv_out =  active_sequence.cv_matrix[active_step] * 40;
			dacVar->setOutput(1, GAIN_2, 1, cv_out);
			calculated_step_length = (active_sequence.duration_matrix[active_step] / 100.0) * (double)calculated_tempo;
		}
	} else if(seq_effect_mode && active_sequence.effect == EFFECT_STUTTER) {
		digitalWrite(GATE_PIN, HIGH);
		gate_active = true;
	}
}

void Sequencer::quantizeActivePitch(){
	active_pitch = active_sequence.pitch_matrix[active_step];
	random_octave = 0;
	if (seq_effect_mode && active_sequence.effect == EFFECT_RANDOM) {
		active_pitch += rand() % active_sequence.effect_depth;
	} else if (seq_effect_mode && active_sequence.effect == EFFECT_TRANSPOSE) {
		active_pitch += active_sequence.effect_depth - 24;
	}

	//normalize to 2 octaves
	if (active_pitch > 12) {
		if (active_pitch > 36) random_octave = 3;
		else random_octave = active_pitch >= 24 ? 2 : 1;
		active_pitch = active_pitch % 12;
	} else if (active_pitch < -12) {
		if (active_pitch <= -36) random_octave = -3;
		random_octave = active_pitch <= -24 ? -2 : -1;
		active_pitch = active_pitch % -12;
	}

	//quantize to scale
	if (current_scale_tones[active_pitch >= 0 ? active_pitch : active_pitch + 12] == false) {
		if (active_sequence.scale == 3) { //major pentatonic
			active_pitch += quantize_pen[active_pitch >= 0 ? active_pitch : active_pitch + 12];
		} else if (active_sequence.scale == 4) { //minor pentatonic
			active_pitch += quantize_pem[active_pitch >= 0 ? active_pitch : active_pitch + 12];
		} else {//all others (diatonics)
			active_pitch += quantize_map[active_pitch >= 0 ? active_pitch : active_pitch + 12];
		}
		
	}
}

int Sequencer::getCurrentStep(){
	return current_step;
}

bool Sequencer::stepWasIncremented(){
	return step_incremented;
}


void Sequencer::updateGlide() {
	if (seq_effect_mode && active_sequence.effect == EFFECT_STOP) {
		if (note_reached) return;
		double glidekeeper = getGlideKeeper(repeat_step_origin);
		double stopTime = active_sequence.effect_depth * calculated_tempo;

		double instantaneous_pitch = active_note * (stopTime - glidekeeper) / stopTime;
		note_reached = (instantaneous_pitch < 1);
		current_note_value = calibrationVar->getCalibratedOutput(instantaneous_pitch);
		dacVar->setOutput(0, GAIN_2, 1, current_note_value);
		if (note_reached) {
			digitalWrite(GATE_PIN, LOW);
			gate_active = false;
		}
	} else if ((active_sequence.step_matrix[active_step] && active_sequence.glide_matrix[active_step]) || (seq_effect_mode && active_sequence.effect == EFFECT_GLIDE)) {
		int glidekeeper = getGlideKeeper(active_step);
		if (glidekeeper < glide_time) {
			//if (!note_reached) {
				note_reached = false;
				double instantaneous_pitch = ((active_note * glidekeeper) + prev_note * (glide_time - glidekeeper)) / double(glide_time);
				current_note_value = calibrationVar->getCalibratedOutput(instantaneous_pitch);
				dacVar->setOutput(0, GAIN_2, 1, current_note_value);
			// }
		} else if (!note_reached) {
			current_note_value = calibrationVar->getCalibratedOutput(active_note);
			dacVar->setOutput(0, GAIN_2, 1, current_note_value);
			note_reached = true;
		}
	} 
}

int Sequencer::getGlideKeeper(int step){
	int steps_advanced = current_step - step;
	if (steps_advanced < 0) {
		steps_advanced = current_step + active_sequence.sequence_length - step;
	}
	return(timekeeper + steps_advanced * calculated_tempo);
}

void Sequencer::updateGate() {
	if (clock_out_active && timekeeper > CLOCK_PULSE_DURATION) {
		digitalWrite(CLOCK_OUT_PIN, LOW);
		clock_out_active = false;
	}
	
	if (seq_effect_mode) {
		if (active_sequence.effect == EFFECT_FREEZE) return;
		if (note_reached && active_sequence.effect == EFFECT_STOP) return;
		if (active_sequence.effect == EFFECT_ROLL) {
			for (byte i = 1; i <= active_sequence.effect_depth; i++) {
				if (gate_active && timekeeper > (calculated_roll * i) - ROLL_PAUSE_DURATION && timekeeper < (calculated_roll * i)) {
					digitalWrite(GATE_PIN, LOW);
					gate_active = false;
				} else if (!gate_active && timekeeper >= calculated_roll * i) {
					digitalWrite(GATE_PIN, HIGH);
					gate_active = true;
				}
			}
			return;
		}
	}
	if (!gate_active) return;

	//double percent_step = timekeeper / (double)calculated_tempo * 100.0;
	int steps_advanced = current_step - active_step + 1;
	if (steps_advanced < 1) {
		steps_advanced = current_step + active_sequence.sequence_length - active_step + 1;
	}

	if (seq_effect_mode && active_sequence.effect == EFFECT_STUTTER && !active_sequence.step_matrix[current_step]) {
		//if (active_sequence.effect_depth < percent_step * steps_advanced) {
		if (timekeeper > calculated_stutter) {
			digitalWrite(GATE_PIN, LOW);
			gate_active = false;
		}
	//} else if (active_sequence.duration_matrix[active_step] < percent_step * steps_advanced) {
	} else if (timekeeper + calculated_tempo * (steps_advanced-1) > calculated_step_length) {
		digitalWrite(GATE_PIN, LOW);
		gate_active = false;
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
	if (first_step && play_active) {
		incrementStep();
		setActiveNote();
	}
}

void Sequencer::onResetIn(){
	reset_in_active = true;
}

void Sequencer::onClockIn(){
	clock_in_active = true;
}

void Sequencer::onReset(){
	timekeeper = tempo_millis;
	clock_step =  -1; //clock_active ? -1 : 0;
	current_step = clock_step;
	step_incremented = false;
	first_step = true;
}

void Sequencer::onBarSelect(byte bar){

}

int Sequencer::incrementTempo(int amount){
	tempo_bpm = getMinMaxParam(tempo_bpm, amount, 20, 250);
	tempo_millis = 15000 / tempo_bpm;
	if (play_active) {
		calculated_tempo = tempo_millis;
	}
	active_sequence.sequence_tempo = tempo_bpm;
	updateSwingCalc();
	updateRollCalc();
	updateStutterCalc();
	return tempo_bpm;
}

void Sequencer::updateSwingCalc(){
	tempo_millis_swing_odd = calculated_tempo * float(active_sequence.swing) / 50.0;
	tempo_millis_swing_even = calculated_tempo * 2 - tempo_millis_swing_odd;
}

int Sequencer::incrementScale(int amount){
	active_sequence.scale = getMinMaxParam(active_sequence.scale, amount, 0, 9);
	loadScale(active_sequence.scale);
	return active_sequence.scale;
}

int Sequencer::incrementEffect(int amount){
	uint8_t &effect = active_sequence.effect;
	setMinMaxParamUnsigned(effect, amount, 0, 9);	
	switch (active_sequence.effect) {
		case EFFECT_GLIDE: active_sequence.effect_depth = active_sequence.glide_length; break; //set useful default rather than zero 
		case EFFECT_TRANSPOSE: active_sequence.effect_depth = 24; break;
		case EFFECT_OCTAVE: active_sequence.effect_depth = 5; break;//actually zero with offset
		case EFFECT_REPEAT: active_sequence.effect_depth = 4; break;
		case EFFECT_STOP: active_sequence.effect_depth = 8; break;
		case EFFECT_STUTTER: active_sequence.effect_depth = 80; break;
		case EFFECT_ROLL: active_sequence.effect_depth = 2; break;
	}
	incrementEffectDepth(0);
	return active_sequence.effect;
}


int Sequencer::incrementEffectDepth(int amount){
	uint8_t &depth = active_sequence.effect_depth;
	switch(active_sequence.effect) {
		case EFFECT_REPEAT:  setMinMaxParamUnsigned(depth, amount, 1, 16); break;
		case EFFECT_OCTAVE:  setMinMaxParamUnsigned(depth, amount, 0, 8); return active_sequence.effect_depth - 4; break;//this is displayed as -4/+4 in UI
		case EFFECT_TRANSPOSE:setMinMaxParamUnsigned(depth, amount, 0, 48); return active_sequence.effect_depth - 24; break; //this is displayed as -24/+24 in the ui
		case EFFECT_GLIDE:   setMinMaxParamUnsigned(depth, amount, 1, 100); updateGlideCalc(); return active_sequence.effect_depth * 4; break; //multiply to get bigger range
		case EFFECT_REVERSE: setMinMaxParamUnsigned(depth, amount, 0, 1); break;
		case EFFECT_STOP:    setMinMaxParamUnsigned(depth, amount, 1, 16); break;
		case EFFECT_FREEZE:  setMinMaxParamUnsigned(depth, amount, 0, 1); break;
		case EFFECT_RANDOM:  setMinMaxParamUnsigned(depth, amount, 1, 50); break;
		case EFFECT_STUTTER: setMinMaxParamUnsigned(depth, amount, 1, 100); updateStutterCalc(); break;
		case EFFECT_ROLL:    setMinMaxParamUnsigned(depth, amount, 1, 8); updateRollCalc(); break;
	}
	return active_sequence.effect_depth;
}

int Sequencer::incrementSteps(int amount, bool shift_state){
	if (shift_state && amount != 0) {
		byte i = 1;
		while (active_sequence.sequence_length > step_presets[i] && i < sizeof(step_presets)/sizeof(step_presets[0])-2) { i++; }
		active_sequence.sequence_length = step_presets[amount > 0 ? i+1 : i-1];
	} else {
		active_sequence.sequence_length = getMinMaxParam(active_sequence.sequence_length, amount, 1, SEQUENCE_MAX_LENGTH);
	}
	return prev_sequence_length = active_sequence.sequence_length;
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
	active_sequence.transpose = getMinMaxParam(active_sequence.transpose, amount, 0, 48);
	return active_sequence.transpose - 24;
}

int Sequencer::incrementGlide(int amount){
	active_sequence.glide_length = getMinMaxParam(active_sequence.glide_length, amount, 1, 255);
	updateGlideCalc();
	return active_sequence.glide_length;
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
	if (!play_active) { //if sequence is already playing, continue in time
		tempo_millis = 15000 / active_sequence.sequence_tempo;
		calculated_tempo = tempo_millis;
	}
	incrementTempo(0); //sets swing params
	updateGlideCalc();
	incrementScale(0);
	incrementEffectDepth(0);
}

void Sequencer::updateGlideCalc(){
	if (seq_effect_mode && active_sequence.effect == EFFECT_GLIDE) {
		glide_duration = active_sequence.effect_depth * 4;
	} else {
		glide_duration = active_sequence.glide_length;
	}
	glide_time = float(glide_duration) / 100.0 * calculated_tempo;
}

void Sequencer::updateRollCalc(){
	calculated_roll = calculated_tempo / active_sequence.effect_depth;
}

void Sequencer::updateStutterCalc(){
	calculated_stutter = calculated_tempo * float(active_sequence.effect_depth) / 100.0;
}

uint8_t Sequencer::editedStep(){
	return (seq_record_mode ? active_step : selected_step);
}

bool Sequencer::currentStepActive(){
	return current_step == active_step;
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
	if (active_sequence.effect == EFFECT_GLIDE) {
		updateGlideCalc();
	} else if (active_sequence.effect == EFFECT_FREEZE) {
		digitalWrite(GATE_PIN, state ? HIGH : LOW);
		gate_active = state;
	}  else if (active_sequence.effect == EFFECT_STOP) {
		note_reached = false;
	} 
}

void Sequencer::onMutateButton(bool state){
	mutate_button = state;
	setEffectMode(state);
	if (seq_record_mode && state) {
		active_sequence.effect_matrix[current_step] = state;
		seq_recording_effect = state;
	}
}


void Sequencer::setRecordMode(bool state){
	seq_record_mode = state;
	if (!state)	seq_recording_effect = false; 
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
		active_sequence.effect_matrix[i] = false;
	}
	active_sequence.glide_length = 50;
	active_sequence.sequence_length = 16;
	active_sequence.bars = 1;
	active_sequence.scale = 0;

	active_sequence.swing = 50;
	active_sequence.effect = 0; //mutate = repeat (0), reverse (1), octave shift (2), auto-glide (3), hold  (4)
	active_sequence.effect_depth = 4;
    //active_sequence.sequence_tempo = 120; //might be done in real time? probably not a good idea to change
    active_sequence.transpose = 24;
}

void Sequencer::loadScale(uint8_t scale){
	for (byte k = 0; k < 13; k++) {
    	current_scale_tones[k] = (bool)pgm_read_byte_near(scale_tones[scale] + k);
  	}
}

void Sequencer::pickupPositionInNewSequence(){
	if (prev_sequence_length != active_sequence.sequence_length) {
		clock_step = active_sequence.sequence_length - (prev_sequence_length - clock_step);
		if (clock_step < 0) {
			clock_step = active_sequence.sequence_length + clock_step;
		}
		prev_sequence_length = active_sequence.sequence_length;
	}


}

void Sequencer::paste(byte bar1, byte bar2) {
	memcpy(active_sequence.step_matrix+bar2*16, active_sequence.step_matrix+bar1*16, 16);
	memcpy(active_sequence.octave_matrix+bar2*16, active_sequence.octave_matrix+bar1*16, 16);
	memcpy(active_sequence.pitch_matrix+bar2*16, active_sequence.pitch_matrix+bar1*16, 16);
	memcpy(active_sequence.duration_matrix+bar2*16, active_sequence.duration_matrix+bar1*16, 32);
	memcpy(active_sequence.cv_matrix+bar2*16, active_sequence.cv_matrix+bar1*16, 16);
	memcpy(active_sequence.glide_matrix+bar2*16, active_sequence.glide_matrix+bar1*16, 16);
	memcpy(active_sequence.effect_matrix+bar2*16, active_sequence.effect_matrix+bar1*16, 16);
}

void Sequencer::setStepRecordingMode(bool state){
	if (state) {
		active_sequence.step_matrix[current_step] = true;
		step_recording_initiated_step = current_step;
		active_step = current_step;
		prev_note = active_note;
		stepkeeper = timekeeper;
		setActiveNote(); //update pitch
		gate_active = false;
		digitalWrite(GATE_PIN, HIGH);

	} else {
		//make each note as long as the button was held down for
		int8_t steps_elapsed = current_step - step_recording_initiated_step;
		if (steps_elapsed < 0) {
			steps_elapsed += active_sequence.sequence_length;
		}
		//active_sequence.duration_matrix[step_recording_initiated_step] = min(1 + 100 * , 400);
		uint16_t recorded_step_duration = timekeeper - stepkeeper + (steps_elapsed * calculated_step_length);

		active_sequence.duration_matrix[step_recording_initiated_step] = min(400, recorded_step_duration * 100 / calculated_step_length);
		digitalWrite(GATE_PIN, LOW);
	}
	step_recording_mode = state;
}

}