#pragma once

#include "Display.h"
#include "AnalogIo.h"

namespace supersixteen{

class Buttons {
  public:
    Buttons() { }
    ~Buttons() { }
  
    
    void init();
    void poll();

    bool getButtonToggled();
    bool getButtonState();
    int getButtonPressed();

    bool button_toggled = false;
    bool button_state = false;
    bool shift_mode = false;
    int button_pressed = 0;
    

  private:
    void selectStep(unsigned int stepnum);

    void saveButton(bool state);
    void loadButton(bool state);
    void playButton(bool state);
    void shiftButton(bool state);

    void recordButton(bool state);
    void repeatButton(bool state);
    void glideButton(bool state);
        
    int button_map[16] = { 12, 13, 14, 15, 11, 10, 9, 8, 4, 5, 6, 7, 3, 2, 1, 0 }; //rows are wired symmetrically rather than sequentially
    bool button_matrix[16] = { 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1 };
    int saveCount = 0;

    bool function_button_matrix[8] = { 0, 0, 0, 0, 0, 0, 0 }; //store status of buttons in/out  -- no idea why but first bit never toggles? works when offset by one - bad memory address?

    int row = 0;
    uint16_t buttons_state;
    uint16_t buttons_mask;
};

}