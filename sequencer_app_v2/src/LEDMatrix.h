#pragma once
// LEDMatrix.h

void initializeMatrix();

void updateMatrix(int row);

void multiplex_leds();

void blink_step();

void blink_led();