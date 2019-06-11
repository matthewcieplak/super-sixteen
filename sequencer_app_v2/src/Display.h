#pragma once
// Display.h

namespace supersixteen{
class Display{
public:
    void init();

    void updateSevenSegmentDisplay();

    void nextDigit();
    
    void setDisplayNum(int displayNum);

    void setDisplayAlpha(const char displayAlpha[3]);

    void setDecimal(bool decimalOn);

private:
    void appendDecimal();

    int digit_display[3];
    int digit_counter;
};
}