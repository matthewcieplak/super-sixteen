#include "Variables.h"
#include "AnalogIO.h"
#include "Calibrate.h"
#include "Display.h"
#include <EEPROM.h>

namespace supersixteen{

const int calibrationEEPROMAddress = 0;
const int displayModeEEPROMAddress = 20; //set whether to display numbers or note names C0, B1

unsigned int octave_values[9] = { 0,   500,  1000, 1500, 2000, 2500, 3000, 3500, 4000 };
//int calibration_values[9]   = { 0,   0,    0,    0,    0,    0,    0,    0,    0    };
int calibration_values[9]     = { 0,   -4,   -6,   -12,  -15,  -17,  -18,  -21,  -22  }; //programming the chip overwrites the EEPROM, so storing good default here for convenience

int calibration_brightness = 0;


int Calibration::getCalibratedOutput(double pitch) {
	if (pitch > 96) {
		pitch = 96;
	} else if (pitch < 0) {
		pitch = 0;
	}
	int octave1 = pitch / 12; //nearest C at or below pitch
	int octave2 = octave1 + ((int(pitch) % 12) > 0 ? 1 : 0); //nearest C above pitch, or equal if first is C
	double point1 = octave_values[octave1] + calibration_values[octave1];
	double point2 = octave_values[octave2] + calibration_values[octave2];

	unsigned int calibratedPitch;
	if (octave2 == octave1) {
		calibratedPitch = point1;
	} else {
		calibratedPitch = point1 + double(pitch - octave1 * 12.0) * double(point2 - point1) / double(12);
		//TESTMODE use display for output
		//setDisplayNum(calibratedPitch / 10);
	}

	return calibratedPitch;
}

int Calibration::getCalibrationValue(int step){
	return calibration_values[step];
}

int Calibration::incrementCalibration(int amt, int step) {
	if (abs(calibration_values[step] + amt) < 100) { //calibration values stored as 0.0 - 2.0 but displayed as -99 to +99
		calibration_values[step] += amt;
	} 
	return calibration_values[step];
}

int Calibration::incrementBrightness(int amt) {
	if (calibration_brightness + amt <= 4 && calibration_brightness + amt >= 0) { //brightness values stored as 0 - 20
		calibration_brightness += amt;
	}
	return calibration_brightness;
}

int Calibration::getBrightness(){
	return calibration_brightness;
}

void Calibration::readCalibrationValues() {
	for (int i = 0; i < 9; i++) {
		calibration_values[i] = EEPROM.read(i) - 100; //convert 0-255 to +/-99
		if (abs(calibration_values[i]) > 99) { //discard garbage
			calibration_values[i] = 0;
		}
	}
}

void Calibration::writeCalibrationValues() {
	for (int i = 0; i < 9; i++) {
		EEPROM.update(i, calibration_values[i] + 100); //convert +/-99 to 0-255
	}
}

int Calibration::readDisplayModeValue(){
	return EEPROM.read(displayModeEEPROMAddress);
}

void Calibration::writeDisplayModeValue(int displayMode){
	EEPROM.update(displayModeEEPROMAddress, displayMode);
}

}