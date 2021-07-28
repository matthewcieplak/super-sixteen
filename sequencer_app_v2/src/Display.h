#pragma once
// Display.h

namespace supersixteen{
class Display{
public:
    void init();

    void updateSevenSegmentDisplay();

    void blankSevenSegmentDisplay();

    void blankDigit();

    void nextDigit();
    
    void setDisplayNum(int displayNum);

    void setDisplayAlphaVar(char displayAlpha[3]);

    void setDisplayAlpha(const char displayAlpha[3]);

    void setDecimal(bool decimalOn);

    void blinkDisplay(bool blinking, int interval, int timeout);

    void startupSequence();

    void setBrightness(int brightness);

private:
    void appendDecimal();

    bool currentDigitVisible();

    int digit_display[3];
    int digit_counter;
};
}