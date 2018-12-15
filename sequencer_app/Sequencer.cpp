#include "Variables.h"
#include "Pinout.h"
#include "AnalogIO.h"
#include "Calibrate.h"
#include "Sequencer.h"

bool gate_active = false;
bool clock_active = false;
int prev_note = 0;
int active_note = 0;
int glide_duration = 50;

void increment_step() {
	if (timekeeper > tempo) {
		timekeeper = 0;

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

			//CLOCK
			digitalWrite(CLOCK_OUT_PIN, HIGH);
			clock_active = true;

			//GATE
			digitalWrite(GATE_PIN, step_matrix[active_step] ? HIGH : LOW);
			gate_active = step_matrix[active_step];

		}

	} else {
		update_glide();
	}
}

void update_glide() {
	if (step_matrix[active_step] && glide_matrix[active_step]) {
		int steps_advanced = current_step - active_step;
		if (steps_advanced < 0) {
			steps_advanced = current_step + sequence_length - active_step;
		}
		int glidekeeper = timekeeper + steps_advanced * tempo;
		int glide_time = float(glide_duration) / 100.0 * tempo;
		if (glidekeeper < glide_time) {
			//double instantaneous_pitch = ((active_note * timekeeper) + prev_note * (tempo - timekeeper)) / double(tempo);
			double instantaneous_pitch = ((active_note * glidekeeper) + prev_note * (glide_time - glidekeeper)) / double(glide_time);
			setCalibratedOutput(instantaneous_pitch);
		}
	}
}

void update_gate() {
	double percent_step = timekeeper / (double)tempo * 100.0;
	int steps_advanced = current_step - active_step + 1;
	if (steps_advanced < 1) {
		steps_advanced = current_step + sequence_length - active_step + 1;
	}
	if (gate_active && duration_matrix[active_step] < percent_step * steps_advanced) {
		digitalWrite(GATE_PIN, LOW);
		gate_active == false;
	}

	if (clock_active && timekeeper > 0) {
		digitalWrite(CLOCK_OUT_PIN, LOW);
	}
}

