#include "Sequencer.h"
#include "Memory.h"
#include "Pinout.h"

#include <Arduino.h>
#include <ArduinoJson.h>
#include <SerialFlash.h>

namespace supersixteen {

const uint32_t PATCH_FILE_SIZE = 4096;
const int PATCH_SEQ_LENGTH = PATCH_SEQ_LENGTH;
Sequencer *sequencerVar4;
bool active = false;
char filename[20] = "1.bin";
uint8_t durations_8bit[128];

bool Memory::init(Sequencer& sequencer){
    sequencerVar4 = &sequencer;

    if (!SerialFlash.begin(CS2_PIN)){
        return false;
    } else {
        return true;
    }

}


char* Memory::getFileName(int patch){
    char filenum[3];
    itoa(patch, filenum, 10);
    memset(&filename[0], 0, sizeof(filename));
    strcpy(filename, filenum);
    strcpy(filename, ".bin");
    return filename;
}

byte Memory::save(int patch){ 
    if (SerialFlash.ready() == false) { //still erasing?
        return 0;
    }
    getFileName(patch);
    bool fileExists = SerialFlash.exists(filename);
    if (!fileExists) {  // true if the file exists
        SerialFlash.createErasable(filename, PATCH_FILE_SIZE);
        return 1;
    } else {
        SerialFlashFile file;
        file = SerialFlash.open(filename);
        file.erase();
        file.close();
        return 2;
    }
}

bool Memory::finishSaving(){
    if (SerialFlash.ready() == false) { //still erasing?
        return false;
    }

    SerialFlashFile file;
    file = SerialFlash.open(filename);
    int8_t *pitches     =  sequencerVar4->getActiveSequence().pitch_matrix;
    int8_t *octaves     =  sequencerVar4->getActiveSequence().octave_matrix;
    uint16_t *durations =  sequencerVar4->getActiveSequence().duration_matrix;
    int8_t *cvs         =  sequencerVar4->getActiveSequence().cv_matrix;
    bool *steps         =  sequencerVar4->getActiveSequence().step_matrix;
    bool *glides        =  sequencerVar4->getActiveSequence().glide_matrix;

    for(int i = 0; i<PATCH_SEQ_LENGTH; i++){ //split 16-bit numbers into 2 bytes
        durations_8bit[i] = (durations[i] & 0xFF00) >> 8;
        durations_8bit[i+PATCH_SEQ_LENGTH] = durations[i] & 0x00FF;
    }
    
    file.write(pitches, PATCH_SEQ_LENGTH);
    file.write(octaves, PATCH_SEQ_LENGTH);
    file.write(durations_8bit, PATCH_SEQ_LENGTH*2);
    file.write(cvs, PATCH_SEQ_LENGTH);
    file.write(steps, PATCH_SEQ_LENGTH);
    file.write(glides, PATCH_SEQ_LENGTH);
    //TODO save misc settings

    file.close();
    return true;
}

bool Memory::load(int patch){
    getFileName(patch);
    SerialFlashFile file;
    file = SerialFlash.open(filename);
    if (!file) {  // true if the file exists
        return false; //TODO load blank patch?
    }
    
    int8_t *pitches     =  sequencerVar4->getActiveSequence().pitch_matrix;
    int8_t *octaves     =  sequencerVar4->getActiveSequence().octave_matrix;
    uint16_t *durations =  sequencerVar4->getActiveSequence().duration_matrix;
    int8_t *cvs         =  sequencerVar4->getActiveSequence().cv_matrix;
    bool *steps         =  sequencerVar4->getActiveSequence().step_matrix;
    bool *glides        =  sequencerVar4->getActiveSequence().glide_matrix;

    file.read(pitches, PATCH_SEQ_LENGTH);
    file.read(octaves, PATCH_SEQ_LENGTH);
    file.read(durations_8bit, PATCH_SEQ_LENGTH*2);
    file.read(cvs, PATCH_SEQ_LENGTH);
    file.read(steps, PATCH_SEQ_LENGTH);
    file.read(glides, PATCH_SEQ_LENGTH);
    //TODO load misc params

    for(int i = 0; i<PATCH_SEQ_LENGTH; i++){ //expand bytewise chars to 16-bit number
        durations[i] = durations_8bit[i] * 256 + durations_8bit[i+PATCH_SEQ_LENGTH];
    }

    file.close();
    return true;
}

}