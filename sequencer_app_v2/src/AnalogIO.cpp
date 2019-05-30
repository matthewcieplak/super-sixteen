#include <Arduino.h>
#include <SPI.h>
#include <stdint.h>

#include "Variables.h"
#include "Pinout.h"
#include "AnalogIO.h"
#include "Display.h"
#include "AnalogIO.h"

int analog_params[4] = { PITCH_PARAM, OCTAVE_PARAM, DURATION_PARAM, CV_PARAM };

int display_param = PITCH_PARAM;

int lastAnalogValues[4];
int analogValues[4];
int analogMultiplexor = 0;


//assuming dac single channel, gain=2
//void setOutput(unsigned int val) {
//	if (val > 4096) {
//		val = 4095;
//	}
//	byte lowByte = val & 0xff;
//	byte highByte = ((val >> 8) & 0xff) | 0x10;
//
//	digitalWrite(CS1_PIN, LOW);
//	PORTB &= 0xfb;
//	SPI.transfer(highByte);
//	SPI.transfer(lowByte);
//	PORTB |= 0x4;
//	digitalWrite(CS1_PIN, HIGH);
//}

void setOutput(uint8_t channel, uint8_t gain, uint8_t shutdown, unsigned int val)
{
	if (val < 0) {
		val = 0;
	} else if (val > 4095) {
		val = 4095;
	}
	uint8_t lowByte = val & 0xff;
	uint8_t highByte = ((val >> 8) & 0xff) | channel << 7 | gain << 5 | shutdown << 4;

	//PORTB &= 0xfb;
	digitalWrite(CS2_PIN, LOW);
	SPI.transfer(highByte);
	SPI.transfer(lowByte);
	digitalWrite(CS2_PIN, HIGH);
}


void read_input() {
	analogMultiplexor += 1;
	if (analogMultiplexor > 4) {
		analogMultiplexor = 0;
	}
	int i = analogMultiplexor;
	if (i > 3 || i < 0) return; //sometimes we get desynced by interrupts, and analogRead on a wrong pin is fatal
	analogValues[i] = analogRead(i);

	int change_threshold = 100;
	if (display_param == analog_params[i]) {
		change_threshold = 4; //increase sensitivity when param is selected, decrease otherwise to reduce accidental "bump" changes
	}
	if (abs(analogValues[i] - lastAnalogValues[i]) > change_threshold) {
		lastAnalogValues[i] = analogValues[i];
		switch (i) {
		case 0: setPitch(analogValues[0]); break;
		case 1: setOctave(analogValues[1]); break;
		case 2: setDuration(analogValues[2]); break;
		case 3: setCV(analogValues[3]); break;
		}
		setDisplayNum();
	}
}

void setPitch(int analogValue) {
	display_param = PITCH_PARAM;
	//calibration_value = (float(analogValues[1]) + 1024.0) / 1500.0 ; //11.60
	int newVal = analogValue / 42.6 - 12; //convert from 0_1024 to 0_88 to -12_0_12
	if (pitch_matrix[selected_step] != newVal) num_display = newVal;
	pitch_matrix[selected_step] = newVal;
}

void setOctave(int analogValue) {
	display_param = OCTAVE_PARAM;
	int newVal = analogValue / 120 - 4; //convert from 0-1024 to -4_0_4
	if (octave_matrix[selected_step] != newVal) num_display = newVal;
	octave_matrix[selected_step] = newVal;
}

void setDuration(long analogValue) { //need extra bits for exponent operation
	display_param = DURATION_PARAM;
	int newVal = analogValue * analogValue / 2615; //convert from 0-1024 to 0-400 with exponential curve
	if (duration_matrix[selected_step] != newVal) num_display = newVal;
	duration_matrix[selected_step] = newVal;
}
void setCV(int analogValue) {
	display_param = CV_PARAM;
	int newVal = analogValue / 10.23; //convert from 0-1024 to 0-100
	if (cv_matrix[selected_step] != newVal) num_display = newVal;
	cv_matrix[selected_step] = newVal;
}

void displaySelectedParam() {
	//update display to show currently selected step value if applicable
	switch (display_param) {
		//case TEMPO_PARAM: break
	case PITCH_PARAM:    num_display = pitch_matrix[selected_step]; break;
	case OCTAVE_PARAM:   num_display = octave_matrix[selected_step]; break;
	case DURATION_PARAM: num_display = duration_matrix[selected_step]; break;
	case CV_PARAM:       num_display = cv_matrix[selected_step]; break;
	//case CALIBRATION_PARAM: num_display = calibration_values[selected_step]; break;
	}
}