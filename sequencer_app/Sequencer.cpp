#include "Variables.h"
#include "Pinout.h"
#include "AnalogIO.h"
#include "Calibrate.h"
#include "Sequencer.h"

bool gate_active = false;
bool clock_out_active = false;
bool clock_in_active = false;

int prev_note = 0;
int active_note = 0;
int glide_duration = 50;
int calculated_tempo = tempo;

const int CLOCK_PULSE_DURATION = 10; //milliseconds pulse width of clock output

void update_clock() {
	if (play_active && timekeeper > tempo) {
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
		increment_step();
		timekeeper = 0;
	}

	update_glide();

	
	if (digitalRead(CLOCK_IN_PIN) == LOW) {
		if(!clock_in_active) {
			clock_in_active = true;
			play_active = false; //read the hardware input, which is normally connected to the hardware output, as the internal clock
			calculated_tempo = timekeeper;
			timekeeper = 0;
			increment_step();
		}
	} else {
		clock_in_active = false;
	}
}

void increment_step() {
	led_matrix[current_step] = step_matrix[current_step]; //reset previous LED
	current_step++;
	if (current_step == 16) {
		current_step = 0;
	}
	led_matrix[current_step] = !step_matrix[current_step]; //set current led

	if (step_matrix[current_step]) {
		//PITCH/OCTAVE
		active_step = current_step;
		prev_note = active_note;
		active_note = (octave_matrix[active_step] + 2) * 12 + pitch_matrix[active_step];

		if (glide_matrix[active_step]) {
			update_glide();
		} else {
			setCalibratedOutput(active_note);
		}

		//GATE
		digitalWrite(GATE_PIN, step_matrix[active_step] ? HIGH : LOW);
		gate_active = step_matrix[active_step];
	}
}

void update_glide() {
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
			setCalibratedOutput(instantaneous_pitch);
		}
	}
}

void update_gate() {
	double percent_step = timekeeper / (double)calculated_tempo * 100.0;
	int steps_advanced = current_step - active_step + 1;
	if (steps_advanced < 1) {
		steps_advanced = current_step + sequence_length - active_step + 1;
	}
	if (gate_active && duration_matrix[active_step] < percent_step * steps_advanced) {
		digitalWrite(GATE_PIN, LOW);
		gate_active == false;
	}

	if (clock_out_active && timekeeper > CLOCK_PULSE_DURATION) {
		//digitalWrite(CLOCK_OUT_PIN, LOW);
	}
}

