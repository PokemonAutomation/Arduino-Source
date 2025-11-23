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

std::string dump_audio_format(const QAudioFormat& format);

void set_sample_format_to_float(QAudioFormat& format);

void convert_samples_to_float(const QAudioFormat& format, const char* data, size_t len, float* out);

// Compute sum of the sample buffer.
// Used to debug whether the audio stream is always the same values or always zeros.
float audio_sample_sum(const QAudioFormat& format, const char* data, size_t len);

}

#endif
