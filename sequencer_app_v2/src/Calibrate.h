#pragma once

namespace supersixteen{

class Calibration {
    public:
        void initializeCalibrationMode();

        void updateCalibration();

        int getCalibratedOutput(double pitch, bool output);

        int incrementCalibration(int amt, int step);

        void setCalibration2Value(int value, int step);

        int incrementBrightness(int amt);

        void readCalibrationValues();

        void writeCalibrationValues();

        int getCalibrationValue(int step);

        int getBrightness();

        int readDisplayModeValue();

        void writeDisplayModeValue(int displayMode);

        bool readMutateOnReset();

        void writeMutateOnReset(bool val);
};

}
