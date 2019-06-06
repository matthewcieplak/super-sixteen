#pragma once
// Display.h

class Display(){
public:
    void initializeDisplay();

    void updateSevenSegmentDisplay();

    void setDisplayNum(int displayNum);

    void setDisplayAlpha(char displayAlpha[3]);
private:
    int digit_display[3];
    int digit_counter;
}