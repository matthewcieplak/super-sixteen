#include "Buttons.h"
#include "Encoder.h"
#include "AnalogIO.h"
#include "Display.h"
#include "LEDMatrix.h"
#include "Variables.h"
#include "Pinout.h"

namespace supersixteen{

void Ui::init(){
    buttons.init();
    analogIo.init();
    display.init();
    ledMatrix.init();
}

void Ui::poll(){

}

void Ui::multiplex(){

}

}