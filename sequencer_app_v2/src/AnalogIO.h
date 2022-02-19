#pragma once

#include <Arduino.h>
#include <stdint.h>
#include "Sequencer.h"

namespace supersixteen{

const char audition_0[] PROGMEM = "OFF"; //normal behavior
const char audition_1[] PROGMEM = "AUD"; //play note when changed

const char *const audition_names[] PROGMEM = { audition_0, audition_1 };


const char cvmode_0[] PROGMEM = " CV"; //cv2 auxiliary mode
const char cvmode_1[] PROGMEM = "LFO"; //cv2 auxiliary mode
const char cvmode_2[] PROGMEM = "INT"; //cv2 auxiliary mode
const char cvmode_3[] PROGMEM = "NOT"; //cv2 auxiliary mode

const char *const cvmode_names[] PROGMEM = { cvmode_0, cvmode_1, cvmode_2, cvmode_3 };



class AnalogIo{
    public:
        void init(Sequencer& sequencer);

        void poll(bool shift_state);

        void displaySelectedParam();
        void recordCurrentParam();

        int getDisplayNum();
        char * getDisplayAlpha();
     
        int display_param;
        int display_num;
        bool paramChanged();
        bool paramIsAlpha();
        void setRecordMode(bool mode);
        void setDisplayMode(int mode);


    private:

        void readInput(int i, bool shift_state);

        void setPitch(int analogValue);

        void setOctave(int analogValue);

        void setDuration(long analogValue);

        void setDisplayNum(int displayNum);

        void setDisplayAlpha(char displayAlpha[4]);

        void setCV(int analogValue);

        void setCVMode(int analogValue);

        void setAudition(int analogValue);
};

}