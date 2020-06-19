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
    bool step_matrix[64] = { 1,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 };
	bool glide_matrix[64];

	uint8_t glide_length = 50;
	uint8_t sequence_length = 16;
	uint8_t bars = 1;
	uint8_t scale = 1;

	uint8_t swing = 50;
	uint8_t effect = 0; //mutate = repeat (0), reverse (1), octave shift (2), auto-glide (3), hold  (4)
	uint8_t effect_depth = 4;
    uint8_t sequence_tempo = 120;

    int8_t transpose = 0;
};

class Sequencer{
    public:
        void init(Calibration& calibration, Dac& dac);

        void updateClock();

        void incrementStep();
        bool stepWasIncremented();

        void selectStep(int step);
        bool getStepOnOff(int step);
        int getCurrentStep();        
        
        int incrementTempo(int amount);
        int incrementBars(int amount);
        int incrementSteps(int amount, bool shiftState);
        int incrementSwing(int amount);
        int incrementScale(int amount);
        int incrementTranspose(int amount);
        int incrementEffect(int amount);
        int incrementEffectDepth(int amount);
        int incrementGlide(int amount);
        

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
        void setEffectMode(bool state);
        
        void setTempoFromSequence();
        void loadScale(uint8_t scale);
        void onBarSelect(byte bar);
        void clearSequence();
        void setActiveNote();
        void memoizeSequenceLength();
        void pickupPositionInNewSequence();
        void paste(byte bar1, byte bar2);
        bool currentStepActive();
        void setStepRecordingMode(bool state);


        sequence& getActiveSequence();
        sequence * getSequence();
        
    private:
        int getMinMaxParam(int param, int increment_amount, int min, int max);
        uint8_t setMinMaxParamUnsigned(uint8_t& param, int8_t increment_amount, uint8_t min, uint8_t max);
        int8_t setMinMaxParam(int8_t& param, int8_t increment_amount, int8_t min, int8_t max);
        void updateGlide();
        void updateGate();
        uint8_t editedStep();
        void quantizeActivePitch();
        void initializeSerializedSequence();
        void updateSwingCalc();
        void updateGlideCalc();
        void updateRollCalc();
        void updateStutterCalc();
        int getGlideKeeper(int step);
        void onClock();
        static void onClockIn();
        static void onResetIn();

};

}