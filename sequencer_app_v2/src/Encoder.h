#pragma once
// Encoder.h

namespace supersixteen
{

class Encoder{
    public:
        void init();
        void poll();
        int getIncrementAmount();
        int encoder_amount = 0;
        int increment_amount = 0;
    private:
        void encoder_increment(int amt);
};

}