#include "Sequencer.h"
#include "Memory.h"
#include "Pinout.h"

#include <Arduino.h>
#include <ArduinoJson.h>
#include <SerialFlash.h>

namespace supersixteen {

const uint32_t PATCH_FILE_SIZE = 4096;
const int PATCH_SEQ_LENGTH = 64;
Sequencer *sequencerVar4;
bool active = false;
char filename[20] = "1.bin";
uint8_t durations_8bit[128];
SerialFlashFile file;


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
        file = SerialFlash.open(filename);
        file.erase();
        return 2;
    }
}

bool Memory::finishSaving(){
    if (SerialFlash.ready() == false) { //still erasing?
        return false;
    }
    sequence *seq = sequencerVar4->getSequence();

    int8_t *pitches     =  seq->pitch_matrix;
    int8_t *octaves     =  seq->octave_matrix;
    uint16_t *durations =  seq->duration_matrix;
    int8_t *cvs         =  seq->cv_matrix;
    bool *steps         =  seq->step_matrix;
    bool *glides        =  seq->glide_matrix;

    uint8_t misc[8]; //unsigned
    misc[0] = seq->glide_length;
	misc[1] = seq->sequence_length;
	misc[2] = seq->bars;
	misc[3] = seq->scale;
	misc[4] = seq->swing;
	misc[5] = seq->effect;
	misc[6] = seq->effect_depth;
    misc[7] = seq->sequence_tempo;

    int8_t misc2[1]; //signed
    misc2[0] = seq->transpose;


    for(int i = 0; i<PATCH_SEQ_LENGTH; i++){ //split 16-bit numbers into 2 bytes
        durations_8bit[i] = (durations[i] & 0xFF00) >> 8;
        durations_8bit[i+PATCH_SEQ_LENGTH] = durations[i] & 0x00FF;
    }


    //SerialFlashFile file;
    if (file) file.close();
    file = SerialFlash.open(filename);

    file.write(pitches, PATCH_SEQ_LENGTH);
    file.write(octaves, PATCH_SEQ_LENGTH);
    file.write(durations_8bit, PATCH_SEQ_LENGTH*2);
    file.write(cvs, PATCH_SEQ_LENGTH);
    file.write(steps, PATCH_SEQ_LENGTH);
    file.write(glides, PATCH_SEQ_LENGTH);
    file.write(misc, sizeof(misc));
    file.write(misc2, sizeof(misc2));
    //TODO save misc settings

    file.close();
    return 1;

}


bool Memory::load(int patch){
    getFileName(patch);
    if (file) file.close();
    file = SerialFlash.open(filename);
    if (!file) {  // true if the file exists
        return false; //TODO load blank patch?
    }

    sequence *seq = sequencerVar4->getSequence();
    //sequence *seq = &seq2;
    
    int8_t *pitches     =  seq->pitch_matrix;
    int8_t *octaves     =  seq->octave_matrix;
    uint16_t *durations =  seq->duration_matrix;
    int8_t *cvs         =  seq->cv_matrix;
    bool *steps         =  seq->step_matrix;
    bool *glides        =  seq->glide_matrix;
    uint8_t misc[8]; //unsigned    
    int8_t misc2[4]; //signed


    file.read(pitches, PATCH_SEQ_LENGTH);
    file.read(octaves, PATCH_SEQ_LENGTH);
    file.read(durations_8bit, PATCH_SEQ_LENGTH*2);
    file.read(cvs, PATCH_SEQ_LENGTH);
    file.read(steps, PATCH_SEQ_LENGTH);
    file.read(glides, PATCH_SEQ_LENGTH);
    file.read(misc, sizeof(misc));
    file.read(misc2, sizeof(misc2));
    
    for(int i = 0; i<PATCH_SEQ_LENGTH; i++){ //expand bytewise chars to 16-bit number
        durations[i] = durations_8bit[i] * 256 + durations_8bit[i+PATCH_SEQ_LENGTH];
    }

    seq->glide_length     = misc[0];
	seq->sequence_length  = misc[1];
	seq->bars             = misc[2];
	seq->scale            = misc[3];
	seq->swing            = misc[4];
	seq->effect           = misc[5];
	seq->effect_depth     = misc[6];
    seq->sequence_tempo   = misc[7];

    seq->transpose        = misc2[0];
    sequencerVar4->setTempoFromSequence();



    file.close();
    return true;
}

}