#pragma once

#include <Arduino.h>
#include <stdint.h>

namespace supersixteen{

class AnalogIo{
    public:
        void init();

        void poll();

        void displaySelectedParam();

        int getDisplayNum();
     
        int display_param;
        int display_num;
        bool param_changed;

    private:

        void setPitch(int analogValue);

        void setOctave(int analogValue);

        void setDuration(long analogValue);

        void setDisplayNum(int displayNum);

        void setCV(int analogValue);


        int lastAnalogValues[4];
        int analogValues[4];
        int analogMultiplexor = 0;
};

}