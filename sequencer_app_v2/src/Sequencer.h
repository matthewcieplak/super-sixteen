#pragma once
// Sequencer.h
#include "Calibrate.h"
#include "Dac.h"
#include <Arduino.h>

namespace supersixteen {


struct sequence {
	int8_t pitch_matrix[64];
	int8_t octave_matrix[64];
	uint16_t duration_matrix[64];
	int8_t cv_matrix[64];
    bool step_matrix[64] = { 1,0,0,0, 1,1,0,0, 1,1,1,0, 1,1,1,1 };
	bool glide_matrix[64];

	uint8_t glide_length = 50;
	uint8_t sequence_length = 16;
	uint8_t bars = 1;
	uint8_t scale = 0;

	uint8_t swing = 50;
	uint8_t effect = 0; //mutate = repeat (0), reverse (1), octave shift (2), auto-glide (3), hold  (4)
	uint8_t effect_depth = 4;
    uint8_t sequence_tempo = 120;

    int8_t transpose = 0;
};

const static PROGMEM bool scales[10][12] = {
	{1,1,1,1,1,1,1,1,1,1,1,1}, //chromatic
	{1,0,1,0,1,1,0,1,0,1,0,1}, //major
	{1,0,1,1,0,1,0,1,1,0,1,0}, //minor
	{1,0,1,0,1,0,0,1,0,1,0,0}, //major pentatonic
	{1,0,0,1,0,1,0,1,0,0,1,0}, //minor pentatonic
	{1,0,1,0,1,1,1,1,1,0,1,1}, //blues major
	{1,0,1,1,0,1,1,1,1,0,1,0}, //blues minor
    {1,1,0,0,1,1,0,1,1,0,1,1}, //phrygian
	{1,0,1,1,0,1,0,1,0,1,1,0}, //dorian
	{1,0,1,0,1,0,1,0,1,0,1,0} // whole tone
};

const char scale_0[] PROGMEM = "CHR"; //chromatic
const char scale_1[] PROGMEM = "MAJ"; //major
const char scale_2[] PROGMEM = "MIN"; //minor
const char scale_3[] PROGMEM = "PEN"; //major pentatonic
const char scale_4[] PROGMEM = "PE2"; //minor pentatonic
const char scale_5[] PROGMEM = "BLU"; //blues major
const char scale_6[] PROGMEM = "BL2"; //blues minor
const char scale_7[] PROGMEM = "PHR"; //phrygian
const char scale_8[] PROGMEM = "DOR"; //dorian
const char scale_9[] PROGMEM = "WHO"; //whole tone

const char *const scale_names[] PROGMEM = { scale_0, scale_1, scale_2, scale_3, scale_4, scale_5, scale_6, scale_7, scale_8, scale_9 };

// const static PROGMEM char scale_names[10][4]  = 

// };


class Sequencer{
    public:
        void init(Calibration& calibration, Dac& dac);

        void updateClock();

        void incrementStep();
        bool stepWasIncremented();

        void selectStep(int step);
        bool getStepOnOff(int step);
        int getCurrentStep();        
        void setActiveNote();
        
        int incrementTempo(int amount);
        int incrementBars(int amount);
        int incrementSteps(int amount);
        int incrementSwing(int amount);
        int incrementScale(int amount);
        int incrementTranspose(int amount);

        void onPlayButton();
        void onReset();

        bool toggleGlide();
        bool setPitch(int newVal);
        bool setOctave(int8_t newVal);
        bool setDuration(uint16_t newVal);
        bool setCv(int newVal);

        bool getGlide();
        int getPitch();
        int getOctave();
        int getDuration();
        int getCv();


        bool *getStepMatrix();
        void setStepMatrix();

        int getSelectedStep();

        void setRecordMode(bool state);
        void setRepeatMode(bool state);
        void setRepeatLength(uint8_t length);

        void setTempoFromSequence();



        sequence& getActiveSequence();
        sequence * getSequence();
        
    private:
        int setMinMaxParam(int param, int increment_amount, int min, int max);
        void updateGlide();
        void updateGate();
        uint8_t editedStep();
        void initializeSerializedSequence();

};

}