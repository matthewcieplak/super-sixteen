#pragma once

void initializeCalibrationMode();

void updateCalibration();

int setCalibratedOutput(double pitch);

void incrementCalibration(int amt);

void readCalibrationValues();

void writeCalibrationValues();
