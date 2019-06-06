#pragma once

namespace supersixteen{

class AnalogIo{
    public:
        void setOutput(uint8_t channel, uint8_t gain, uint8_t shutdown, unsigned int val);

        void read_input();
     
        extern int display_param;

    private:

        void setPitch(int analogValue);

        void setOctave(int analogValue);

        void setDuration(long analogValue);

        void setCV(int analogValue);

        void displaySelectedParam();



        int lastAnalogValues[4];
        int analogValues[4];
        int analogMultiplexor = 0;


};

}