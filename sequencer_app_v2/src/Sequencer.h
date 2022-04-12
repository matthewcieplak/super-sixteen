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
    bool effect_matrix[64];

	uint8_t glide_length = 50;
	uint8_t sequence_length = 16;
	uint8_t bars = 1;
	uint8_t scale = 1;

	uint8_t swing = 50;
	uint8_t effect = 0; //mutate = repeat (0), reverse (1), octave shift (2), auto-glide (3), hold  (4)
	uint8_t effect_depth = 4;
    uint8_t sequence_tempo = 120;

    int8_t transpose = 24;
    int8_t song_next_seq = 0;
    int8_t song_loops = 0;
    int8_t cv_mode = 0;
};

class Sequencer{
    public:
        void init(Calibration& calibration, Dac& dac);

        void updateClock();

        void incrementStep();
        bool stepWasIncremented();
        bool timeForNextSequence();

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
        int incrementSongNextSeq(int incrementAmount);
        int incrementSongLoops(int incrementAmount);
        int getSongNextSeq();
        

        void onPlayButton();
        void onReset();

        bool toggleGlide();
        bool setPitch(int newVal);
        bool setOctave(int8_t newVal);
        bool setDuration(uint16_t newVal);
        bool setCv2(int newVal);
        void auditionNote(bool gate, int timer);

        bool getGlide();
        int getPitch();
        int getMidiPitch(int pitch, int octave);
        char *getPitchName(uint8_t note);
        int getOctave();
        int getDuration();
        int getCv();


        bool *getStepMatrix();
        void setStepMatrix();

        int getSelectedStep();

        void setRecordMode(bool state);
        void onMutateButton(bool state);
        
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
        void setMutateRecordingMode(bool state);
        void incrementClock(int steps);

        void setAudition(bool audition);
        void setCVMode(uint8_t mode);
        uint8_t getCvMode();
        int8_t getCv2DisplayValue(int analogvalue);
        //void getSongLoops(int loops);



        sequence& getActiveSequence();
        sequence * getSequence();
        
    private:
        int getMinMaxParam(int param, int increment_amount, int min, int max);
        uint8_t setMinMaxParamUnsigned(uint8_t& param, int8_t increment_amount, uint8_t min, uint8_t max);
        int8_t setMinMaxParam(int8_t& param, int8_t increment_amount, int8_t min, int8_t max);
        void setEffectMode(bool state);
        void updateGlide();
        void updateGate();
        uint8_t editedStep();
        void setPitchOutput(uint8_t step);
        void setCv2Output(uint8_t step);
        int8_t quantizePitch(int8_t pitch);
        uint8_t getCv2Value(uint8_t step);
        void initializeSerializedSequence();
        void generateTuringPitches();
        void updateSwingCalc();
        void updateGlideCalc();
        void updateRollCalc();
        void updateStutterCalc();
        int getGlideKeeper(int step);
        void onClock();
        static void onClockIn();
        static void onResetIn();
        void setLfoTarget();
        void updateLfo();

};

}