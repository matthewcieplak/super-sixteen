#pragma once
// Sequencer.h
#include "Calibrate.h"
#include "Dac.h"
#include <Arduino.h>
#include <ArduinoJson.h>

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

    int transpose = 0;


	
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
        int getPrevStep();
        
        void setActiveNote();

        int incrementTempo(int amount);

        void onPlayButton();

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

        void serialize(DynamicJsonDocument& doc);
        void deserialize(DynamicJsonDocument& doc);

        sequence& getActiveSequence();
        
    private:
        void updateGlide();
        void updateGate();
        uint8_t editedStep();
        void initializeSerializedSequence();

};

}