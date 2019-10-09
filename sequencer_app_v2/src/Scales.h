#include <avr/pgmspace.h> 

namespace supersixteen {           //C   D   E F   G   A   B C
const bool scaletones_0[] PROGMEM = {1,1,1,1,1,1,1,1,1,1,1,1,1}; //chromatic
const bool scaletones_1[] PROGMEM = {1,0,1,0,1,1,0,1,0,1,0,1,1}; //major
const bool scaletones_2[] PROGMEM = {1,0,1,1,0,1,0,1,1,0,1,0,1}; //minor
const bool scaletones_3[] PROGMEM = {1,0,1,0,1,0,0,1,0,1,0,0,1}; //major pentatonic
const bool scaletones_4[] PROGMEM = {1,0,0,1,0,1,0,1,0,0,1,0,1}; //minor pentatonic
const bool scaletones_5[] PROGMEM = {1,0,1,0,1,1,1,1,0,1,1,1,1}; //blues major
const bool scaletones_6[] PROGMEM = {1,0,1,1,0,1,1,1,1,0,1,0,1}; //blues minor
const bool scaletones_7[] PROGMEM = {1,1,0,0,1,1,0,1,1,0,1,1,1}; //phrygian
const bool scaletones_8[] PROGMEM = {1,0,1,1,0,1,0,1,0,1,1,0,1}; //dorian
const bool scaletones_9[] PROGMEM = {1,0,1,0,1,0,1,0,1,0,1,0,1};// whole tone

const bool *const scale_tones[] = {scaletones_0, scaletones_1, scaletones_2, scaletones_3, scaletones_4, scaletones_5, scaletones_6, scaletones_7, scaletones_8, scaletones_9 };

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

                              //  C  Db  D  Eb  E   F  Gb G  Ab   A  Bb  B  C
const int8_t quantize_map[13] = { 0, 1, -1, 1, -1, -1, 1, 0, 1,  -1, 1, -1, 0 }; //steps required to move to altered/diatonic scale degree when out of quantization
}