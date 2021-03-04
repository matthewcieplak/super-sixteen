#pragma once
#include <Arduino.h>
//#include <ArduinoJson.h>
#include <Sequencer.h>
#include <SerialFlash.h>
// Encoder.h

namespace supersixteen
{

class Memory{
    public:
        bool init(Sequencer& sequencer);
        char* getFileName(int patch);
        byte save(int patch); //, sequence& active_sequence);
        bool finishSaving();
        bool load(int patch);
        bool patchExists(int patch);

        bool saveSerialized(byte patch);
        void erase();


        void erasePatch(byte patch);
        byte saveTempo(byte patch);
        byte loadTempo(byte patch);
        bool saveSteps(byte patch);
        void loadSteps(byte patch);
        bool saveDuration(byte patch);
        bool loadDuration(byte patch);
        uint32_t getChipId();
        uint32_t getChipSize();
    private:
        uint32_t getPatchAddress(byte patch);
};

}