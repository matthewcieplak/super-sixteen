#pragma once

namespace supersixteen{

class Calibration {
    public:
        void initializeCalibrationMode();

        void updateCalibration();

        int getCalibratedOutput(double pitch);

        int incrementCalibration(int amt, int step);

        int incrementBrightness(int amt);

        void readCalibrationValues();

        void writeCalibrationValues();

        int getCalibrationValue(int step);

        int getBrightness();

        int readDisplayModeValue();

        void writeDisplayModeValue(int displayMode);
};

}
