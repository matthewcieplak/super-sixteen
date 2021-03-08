#include "Pinout.h"
#include "LEDMatrix.h"
#include "Buttons.h"
#include "Calibrate.h"
#include "Sequencer.h"
#include <MCP23S17.h>
#include "queue_ino.h"
#include <elapsedMillis.h>

namespace supersixteen{	
	
const int function_buttons[7] = { SHIFT_PIN, PLAY_PIN, LOAD_PIN, SAVE_PIN, RECORD_PIN, REPEAT_PIN, GLIDE_PIN };

MCP23S17 ButtonDriver(&SPI, CS0_PIN, 0);


//define a queue called queue_example 
//with elements of type int and size 8
QUEUE(events, uint16_t, 8);

//create an instance of queue_example
volatile struct queue_events queue;
bool editing_buttons = false;
const byte BUTTON_DEBOUNCE_TIME = 10;
elapsedMillis debounce_timer;
volatile uint16_t bouncing_button = 0x00;


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

	//init button states
	poll();
	poll();
	poll();
	poll();
	editing_buttons = true;
}

void Buttons::poll() {
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
		bool value = !(buttons_mask >> ii); //shift to 0 position
		if (value != button_matrix[stepnum]) { //detect when button changes state
			button_matrix[stepnum] = value; //store button state
			uint16_t event = stepnum | value << 8;  //condense button number and state into one 16-bit variable
			onButtonPush(event);
		}
	}
	ButtonDriver.digitalWrite(row+4, HIGH);
	
	buttons_mask = (~buttons_state >> 8 & B01111111); // exclude glide LED bit
	for(int ii = 0; ii<7; ii++){
		bool value = (buttons_mask & 0x01 << (ii)); // >> (ii+8);

		if (value != function_button_matrix[ii]){
			function_button_matrix[ii] = value;
			uint16_t event = (function_buttons[ii]+8) | (value << 8); //condense button number and state into one 16-bit variable
			onButtonPush(event);
		}
	}
}

void Buttons::onButtonPush(uint16_t& event){
	if (editing_buttons) {
		if (debounce_timer < BUTTON_DEBOUNCE_TIME && (event & 0x00FF) == bouncing_button) {
			return;
		}
		queue_events_push(&queue, &event);
		bouncing_button = event & 0x00FF;
		debounce_timer = 0;
	}
}

int Buttons::getQueuedEvent(uint16_t& value){
    return queue_events_pop(&queue, &value);
}

void Buttons::setGlideLed(bool glide){
	SPI.setBitOrder(MSBFIRST); //MCP23S17 is picky
	ButtonDriver.digitalWrite(GLIDE_LED_PIN, glide ? HIGH : LOW);
}

}
