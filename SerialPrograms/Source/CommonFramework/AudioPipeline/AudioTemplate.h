/*  Audio Template
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_AudioPipeline_AudioTemplate_H
#define PokemonAutomation_AudioPipeline_AudioTemplate_H

#include <cstddef>
#include <string>
#include <vector>
#include "Common/Cpp/Containers/AlignedVector.h"

namespace PokemonAutomation{


// Hold the spectrogram of an audio, used as a template to
// match sounds from an audio stream.
class AudioTemplate{
public:
    ~AudioTemplate();
    AudioTemplate(AudioTemplate&& x);
    AudioTemplate& operator=(AudioTemplate&& x);
    AudioTemplate(const AudioTemplate&);
    AudioTemplate& operator=(const AudioTemplate&);

public:
    AudioTemplate();
    AudioTemplate(size_t frequencies, size_t windows);

    size_t num_windows() const{ return m_num_windows; }
    size_t num_frequencies() const{ return m_num_frequencies; }

    //  Size of the buffer that holds this template.
    //  This is "num_frequencies()", rounded up to the SIMD size.
    size_t buffer_size() const{ return m_spectrogram.size(); }

    const float* get_window(size_t window_index) const{
        return (const float*)((const char*)m_spectrogram.data() + window_index * m_bytes_per_spectrum);
    }
    float* get_window(size_t window_index){
        return (float*)((char*)m_spectrogram.data() + window_index * m_bytes_per_spectrum);
    }

//    void scale(float s){ for (auto& v: m_spectrogram) v *= s; }

private:
    size_t m_num_windows = 0;
    size_t m_num_frequencies = 0;
    size_t m_bytes_per_spectrum;
    AlignedVector<float> m_spectrogram;
};

// Load AudioTemplate from disk. Accept .wav format on any OS.
// Loading .mp3 format however is dependent on Qt's platform-dependent backend.
AudioTemplate load_audio_template(const std::string& filename, size_t sample_rate = 48000);









}

#endif
