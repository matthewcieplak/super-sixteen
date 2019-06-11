#include "Pinout.h"
#include "LEDMatrix.h"
#include "Buttons.h"
#include "Calibrate.h"
#include "Sequencer.h"
#include <MCP23S17.h>

namespace supersixteen{	
	
const int function_buttons[7] = { SHIFT_PIN, PLAY_PIN, LOAD_PIN, SAVE_PIN, RECORD_PIN, REPEAT_PIN, GLIDE_PIN };

MCP23S17 ButtonDriver(&SPI, CS0_PIN, 0);

void Buttons::init() {
	
	SPI.setBitOrder(MSBFIRST);
	ButtonDriver.begin();
	for (int i = 0; i < 4; i++){
		ButtonDriver.pinMode(i+4, OUTPUT);
		ButtonDriver.digitalWrite(i+4, HIGH);
		ButtonDriver.pinMode(i, INPUT_PULLUP);
		ButtonDriver.pinMode(i+8, INPUT_PULLUP);
		ButtonDriver.pinMode(i+12, INPUT_PULLUP);
	}
	ButtonDriver.pinMode(GLIDE_LED_PIN, OUTPUT);
	ButtonDriver.pinMode(GLIDE_PIN, INPUT_PULLUP);
}

void Buttons::poll() {
	button_toggled = false;
	button_state = 0;
	SPI.setBitOrder(MSBFIRST); //MCP23S17 is picky
	row++;
	if (row > 3) row = 0;
	ButtonDriver.digitalWrite(row+4, LOW);
	buttons_state = ButtonDriver.readPort(); //read all pins at once to reduce spi overhead (prevents display flicker)
	for (int ii = 0; ii < 4; ii++) {
		int stepnum = button_map[row * 4 + ii];
		//bool value = ButtonDriver.digitalRead(button_map[stepnum]); //read one pin at a time
		buttons_mask = buttons_state & 0x000F; //mask off only bits 1-4, the button matrix columns
		buttons_mask = buttons_mask & 0x01 << ii; //select one bit to scan
		bool value = buttons_mask >> ii; //shift to 0 position
		if (value != button_matrix[stepnum]) { //detect when button changes state
			button_matrix[stepnum] = value; //store button state
			button_toggled = true;
			button_state   = !value; 
			button_pressed = stepnum;

		}
	}
	ButtonDriver.digitalWrite(row+4, HIGH);
	
	
	buttons_mask = (~buttons_state & 0xFF80); // exclude glide LED bit
	if (buttons_mask > 0) {
		for(int ii = 0; ii<8; ii++){
			bool function_button_state = (buttons_mask & 0x01 << (ii+8)) >> (ii+8);

			if (function_button_state != function_button_matrix[ii+8]){
				function_button_matrix[ii+8] = function_button_state;
				//Serial.write(function_button);
				button_toggled = true;
				button_state = function_button_state;
				button_pressed = function_buttons[ii]+8;
			}
		}
	}
}

bool Buttons::getButtonToggled(){
	return button_toggled;
}

bool Buttons::getButtonState(){
	return button_state;
}

int Buttons::getButtonPressed(){
	return button_pressed;
}

void Buttons::setGlideLed(bool glide){
	SPI.setBitOrder(MSBFIRST); //MCP23S17 is picky
	ButtonDriver.digitalWrite(GLIDE_LED_PIN, glide ? HIGH : LOW);
}

}
