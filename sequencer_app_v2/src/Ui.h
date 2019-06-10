#pragma once
#include <stdint.h>
#include "Calibrate.h"
#include "Dac.h"

namespace supersixteen{
class Ui{
    public:
        void init(Calibration& calibration, Dac& dac, Sequencer& sequencer);
        void poll();
        void multiplex();
        bool isSequencing();

    private:
        void saveButton(bool state);
        void onButtonToggle(int button, bool button_state);
        void onEncoderIncrement(int increment_amount);
        void selectStep(int step);
        void glideButton();
        void initializeCalibrationMode();
        void updateCalibration(int step);
};
}