#pragma once

#include <Arduino.h>
#include <stdint.h>
#include "Sequencer.h"

namespace supersixteen{

class AnalogIo{
    public:
        void init(Sequencer& sequencer);

        void poll();

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

        void readInput(int i);

        void setPitch(int analogValue);

        void setOctave(int analogValue);

        void setDuration(long analogValue);

        void setDisplayNum(int displayNum);

        void setDisplayAlpha(char displayAlpha[4]);

        void setCV(int analogValue);
};

}