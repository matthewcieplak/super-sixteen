#pragma once
// Display.h

extern int digit_display[3];
extern int digit_counter;

void initializeDisplay();

extern void updateSevenSegmentDisplay();

void setDisplayNum(int displayNum);

void setDisplayAlpha(char displayAlpha[3]);