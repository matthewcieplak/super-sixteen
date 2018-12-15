#include "Variables.h"
#include "Pinout.h"
#include "AnalogIO.h"
#include "Encoder.h"
#include "Display.h"
#include "Calibrate.h"

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
	if (control_mode == CALIBRATE_MODE) {
		incrementCalibration(amt);
	} else {
		tempo += amt;
		if (tempo < 20) tempo = 20;
		if (tempo > 500) tempo = 500;
		num_display = tempo;
		display_param = TEMPO_PARAM;
		setDisplayNum();
	}
}
