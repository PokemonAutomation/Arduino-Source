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

class AudioIODevice;



class AudioFloatStreamListener{
public:
    AudioFloatStreamListener(size_t p_samples_per_frame)
        : samples_per_frame(p_samples_per_frame)
    {}
    virtual ~AudioFloatStreamListener() = default;
    virtual void on_samples(const float* data, size_t frames) = 0;

    const size_t samples_per_frame;
};



//  Convert a stream of raw audio and output to aligned frames of float samples.
class AudioStreamToFloat : private MisalignedStreamConverter, private StreamListener{
public:
    void add_listener(AudioFloatStreamListener& listener);
    void remove_listener(AudioFloatStreamListener& listener);

public:
    AudioStreamToFloat(
        AudioSampleFormat input_format,
        size_t samples_per_frame,   //  Typically the # of channels. Can be higher if you want to group more into each frame.
        bool reverse_channels       //  Only valid if "samples_per_frame == 2".
    );
    virtual ~AudioStreamToFloat();

    using MisalignedStreamConverter::push_bytes;

private:
    virtual void on_objects(const void* data, size_t objects) override;
    virtual void convert(void* out, const void* in, size_t count) override;

private:
    AudioSampleFormat m_format;
    size_t m_samples_per_frame;
    bool m_reverse_channels;
    size_t m_sample_size;
    size_t m_frame_size;
    std::set<AudioFloatStreamListener*> m_listeners;
};



//  Convert a stream of float samples to raw audio.
class AudioFloatToStream : public AudioFloatStreamListener{
public:
    void add_listener(StreamListener& listener);
    void remove_listener(StreamListener& listener);

public:
    AudioFloatToStream(QIODevice* audio_sink, AudioSampleFormat output_format, size_t samples_per_frame);
    virtual ~AudioFloatToStream();
    virtual void on_samples(const float* data, size_t frames) override;

private:
    QIODevice* m_audio_sink;
    AudioSampleFormat m_format;
    size_t m_samples_per_frame;
    size_t m_sample_size;
    size_t m_frame_size;
    size_t m_buffer_size;
    AlignedVector<char> m_buffer;
    std::set<StreamListener*> m_listeners;
};



//  Listen to an audio stream and compute FFTs on it.
class FFTRunner : public AudioFloatStreamListener{
public:
    FFTRunner(
        AudioIODevice& device, size_t sample_rate,
        size_t samples_per_frame, bool average_pairs
    );
    virtual ~FFTRunner();
    virtual void on_samples(const float* data, size_t frames) override;

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
