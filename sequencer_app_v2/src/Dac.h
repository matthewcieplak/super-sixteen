#pragma once

#include <stdint.h>
namespace supersixteen{
class Dac{
    public:
        void setOutput (uint8_t channel, uint8_t gain, uint8_t shutdown, unsigned int val);
};
}
