#pragma once
// Encoder.h

namespace supersixteen
{

class Encoder{
    public:
        void init();
        int poll();
        int smartPoll();
        int encoder_amount = 0;
        int increment_amount = 0;
        void toggle_inverted();
    private:
        void encoder_increment(int amt);
};

}