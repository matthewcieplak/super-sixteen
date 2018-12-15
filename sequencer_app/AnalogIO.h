#pragma once

#define TEMPO_PARAM 0
#define PITCH_PARAM 1
#define OCTAVE_PARAM 2
#define DURATION_PARAM 3
#define CV_PARAM 4
#define CALIBRATION_PARAM 5

extern int display_param;

//assuming dac single channel, gain=2
//void setOutput(unsigned int val);

void setOutput(uint8_t channel, uint8_t gain, uint8_t shutdown, unsigned int val);

void read_input();

void setPitch(int analogValue);

void setOctave(int analogValue);

void setDuration(long analogValue);

void setCV(int analogValue);

void displaySelectedParam();