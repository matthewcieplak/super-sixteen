#include <SPI.h>

#include "Pinout.h"
#include "Dac.h"


namespace supersixteen {
void Dac::setOutput(uint8_t channel, uint8_t gain, uint8_t shutdown, unsigned int val)
{
	if (val < 0) {
		val = 0;
	} else if (val > 4095) {
		val = 4095;
	}
	uint8_t lowByte = val & 0xff;
	uint8_t highByte = ((val >> 8) & 0xff) | channel << 7 | gain << 5 | shutdown << 4;

	//PORTB &= 0xfb;
	digitalWrite(CS3_PIN, LOW);
	SPI.transfer(highByte);
	SPI.transfer(lowByte);
	digitalWrite(CS3_PIN, HIGH);
}
}