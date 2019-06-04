#pragma once
#include <stdint.h>
#include <MCP23S17.h>
#include "Pinout.h"
#include <elapsedMillis.h>

extern elapsedMillis multiplex;
extern elapsedMillis stepper;
extern elapsedMillis timekeeper;
extern elapsedMillis blinker;

extern MCP23S17 ButtonDriver;

extern int pitch_matrix[16];
extern int octave_matrix[16];
extern int duration_matrix[16];
extern int cv_matrix[16];

extern bool step_matrix[16];
extern bool glide_matrix[16];
extern bool led_matrix[16];

extern int num_display;
extern int selected_step;
extern uint8_t current_step;
extern uint8_t active_step;
extern uint8_t sequence_length;

extern int tempo_millis;
extern double tempo_bpm;
extern bool shift_state;
extern bool play_active;

const int SEQUENCE_MODE = 0;
const int CALIBRATE_MODE = 1;
extern int control_mode;

//float EMA_a = 0.6; //input smoothing coeff


