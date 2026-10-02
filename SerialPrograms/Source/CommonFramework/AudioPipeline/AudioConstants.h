/*  Audio Constants
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_AudioConstants_H
#define PokemonAutomation_AudioConstants_H

//#include <cmath>

//  ~42ms of samples
const int FFT_LENGTH_POWER_OF_TWO = 11;

const size_t NUM_FFT_SAMPLES = 1 << FFT_LENGTH_POWER_OF_TWO;
//const int NUM_FFT_WINDOWS = 100;
const size_t FFT_SLIDING_WINDOW_STEP = NUM_FFT_SAMPLES / 4;

const float VISUAL_JITTER_DECAY = 0.2f;


#endif
