#pragma once
#include <stdint.h>
#include <MCP23S17.h>
#include "Pinout.h"
#include <elapsedMillis.h>


namespace supersixteen{
    
extern elapsedMillis multiplex;
extern elapsedMillis stepper;
extern elapsedMillis timekeeper;
extern elapsedMillis blinker;


//float EMA_a = 0.6; //input smoothing coeff

}
