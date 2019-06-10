#pragma once
// Display.h

namespace supersixteen{
class Display{
public:
    void init();

    void updateSevenSegmentDisplay();

    void setDisplayNum(int displayNum);

    void setDisplayAlpha(const char displayAlpha[3]);
private:
    int digit_display[3];
    int digit_counter;
};
}