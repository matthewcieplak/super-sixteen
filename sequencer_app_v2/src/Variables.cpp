#include <stdint.h>
#include <MCP23S17.h>
#include <elapsedMillis.h>

#include "Pinout.h"
#include "Variables.h"

elapsedMillis stepper;
elapsedMillis blinker;

int num_display = 0;
int selected_step = 0;
uint8_t current_step = 15;
uint8_t active_step = 15;
uint8_t sequence_length = 16;

int pitch_matrix[16];
int octave_matrix[16];
int duration_matrix[16];
int cv_matrix[16];

bool step_matrix[16] = { 1,0,0,0, 1,1,0,0, 1,1,1,0, 1,1,1,1 };
bool led_matrix[16] = { 1,0,0,0, 1,1,0,0, 1,1,1,0, 1,1,1,1 };
bool glide_matrix[16];
