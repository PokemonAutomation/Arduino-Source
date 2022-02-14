/*  Audio Constants
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_AudioConstants_H
#define PokemonAutomation_AudioConstants_H

const int AUDIO_SAMPLE_RATE = 44000;

const int FFT_LENGTH_POWER_OF_TWO = 14;

const int NUM_FFT_SAMPLES = 1 << FFT_LENGTH_POWER_OF_TWO;
const int NUM_FFT_WINDOWS = 20;
const int FFT_SLIDING_WINDOW_STEP = NUM_FFT_SAMPLES/4;


#endif