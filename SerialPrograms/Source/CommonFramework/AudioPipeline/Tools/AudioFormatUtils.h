/*  Audio format Utils
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Host various util functions for audio format related tasks.
 */

#ifndef PokemonAutomation_AudioPipeline_AudioFormatUtils_H
#define PokemonAutomation_AudioPipeline_AudioFormatUtils_H

#include <string>

class QAudioFormat;

namespace PokemonAutomation{

std::string dumpAudioFormat(const QAudioFormat& format);

void setSampleFormatToFloat(QAudioFormat& format);

void convertSamplesToFloat(const QAudioFormat& format, const char* data, size_t len, float* out);

// Compute sum of the sample buffer.
// Used to debug whether the audio stream is always the same values or always zeros.
float audioSampleSum(const QAudioFormat& format, const char* data, size_t len);

}

#endif
