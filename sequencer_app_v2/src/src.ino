// Copyright 2019 Extralife Instruments.
//
// Author: Matthew Cieplak (extralifedisco@gmail.com)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
// See http://creativecommons.org/licenses/MIT/ for more information.
//
// -----------------------------------------------------------------------------
//
// User interface.

#include <Arduino.h>
#include <string.h>
#include <MCP23S17.h> //https://github.com/MajenkoLibraries/MCP23S17
#include <SPI.h>
#include <elapsedMillis.h>



#include "Main.h"
#include "Buttons.h"
#include "Encoder.h"
#include "AnalogIO.h"
#include "Dac.h"
#include "Display.h"
#include "Font.h"
#include "LEDMatrix.h"
#include "Main.h"
#include "Sequencer.h"
#include "Variables.h"
#include "Ui.h"
#include "Dac.h"
#include "Calibrate.h"
#include "Pinout.h"

using namespace supersixteen;

void init_placeholder(){
    //placeholder arduino
}