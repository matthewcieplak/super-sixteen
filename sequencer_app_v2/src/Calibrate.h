#pragma once

namespace supersixteen{

class Calibration {
    public:
        void initializeCalibrationMode();

        void updateCalibration();

        int getCalibratedOutput(double pitch);

        int incrementCalibration(int amt, int step);

        void readCalibrationValues();

        void writeCalibrationValues();

        int getCalibrationValue(int step);

        int readDisplayModeValue();

        void writeDisplayModeValue(int displayMode);
};

}
