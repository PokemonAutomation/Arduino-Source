/*  Audio Source Reader
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_AudioPipeline_AudioSourceReader_H
#define PokemonAutomation_AudioPipeline_AudioSourceReader_H

#include "Common/Cpp/StreamConverters.h"
#include "Common/Cpp/AlignedVector.h"
#include "AudioInfo.h"

class QIODevice;

namespace PokemonAutomation{




//  Listen in on a byte stream that contains audio samples.
//  Even if the stream splits frames, this class will reconstruct them
//  and present them as whole frames.
class AudioSourceReader : public MisalignedStreamConverter{
public:
    AudioSourceReader(
        AudioStreamFormat format,
        size_t samples_per_frame,   //  Typically the # of channels. Can be higher if you want to group more into each frame.
        bool reverse_channels       //  Only valid if "samples_per_frame == 2".
    );

private:
    virtual void convert(void* out, const void* in, size_t count) override;

private:
    AudioStreamFormat m_format;
    size_t m_samples_per_frame;
    bool m_reverse_channels;
    size_t m_sample_size;
    size_t m_frame_size;
};



class AudioSinkWriter : public StreamListener{
public:
    AudioSinkWriter(QIODevice& audio_sink, AudioStreamFormat format, size_t channels);
    ~AudioSinkWriter();
    virtual void on_objects(const void* data, size_t frames) override;

private:
    QIODevice& m_audio_sink;
    AudioStreamFormat m_format;
    size_t m_channels;
    size_t m_sample_size;
    size_t m_frame_size;
    size_t m_buffer_size;
    AlignedVector<char> m_buffer;
};


class AudioIODevice;

class FFTRunner : public StreamListener{
public:
    FFTRunner(
        AudioIODevice& device, size_t sample_rate,
        size_t samples_per_frame, bool average_pairs
    );
    ~FFTRunner();
    virtual void on_objects(const void* data, size_t frames) override;

private:
    void convert(float* fft_input, const float* audio_stream, size_t frames);
    void run_fft();
    void drop_from_front(size_t frames);

private:
    AudioIODevice& m_device;
    size_t m_sample_rate;

    bool m_average;
    size_t m_fft_sample_size;

    AlignedVector<float> m_buffer;
    size_t m_buffered = 0;
    size_t m_start = 0;
    size_t m_end = 0;

    AlignedVector<float> m_fft_input;
};



}
#endif
