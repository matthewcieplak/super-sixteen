#include "Variables.h"
#include "Pinout.h"
#include "AnalogIO.h"
#include "Encoder.h"
#include "Display.h"
#include "Calibrate.h"
#include "Sequencer.h"

int encoder_amount = 0;
int increment_amount = 0;

//magic numbers from https://www.circuitsathome.com/mcu/reading-rotary-encoder-on-arduino/
void read_encoder() {
	///* returns change in encoder state (-1,0,1) */
	//int8_t read_encoder()
	//{
	static int8_t enc_states[] = { 0,-1,1,0,1,0,0,-1,-1,0,0,1,0,1,-1,0 };
	static uint8_t old_AB = 0;
	/**/
	old_AB <<= 2;                   //remember previous state
	old_AB |= (ENC_PORT & 0x30) >> 4;  //add current state from pins A5 and A4, shifted to LSB
	encoder_increment(enc_states[(old_AB & 0x0f)] * -1); //extract encrement/decrement value from state table
}

void encoder_increment(int amt) {
	if (amt == 0) return;
	encoder_amount += amt;
	if (abs(encoder_amount) < 4) return; //encoder detent is too coarse for 1-per step granularity
	increment_amount = encoder_amount > 0 ? 1 : -1;
	encoder_amount = 0;
	if (control_mode == CALIBRATE_MODE) {
		incrementCalibration(increment_amount);
	} else {
		increment_tempo(increment_amount);
	}
	//setDisplayAlpha("ENC");
}
