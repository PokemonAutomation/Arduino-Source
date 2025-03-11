/*  Audio Source Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_AudioPipeline_AudioSourceReader_H
#define PokemonAutomation_AudioPipeline_AudioSourceReader_H

#include "Common/Cpp/ListenerSet.h"
#include "Common/Cpp/LifetimeSanitizer.h"
#include "Common/Cpp/StreamConverters.h"
#include "Common/Cpp/Containers/AlignedVector.h"
#include "AudioInfo.h"

namespace PokemonAutomation{


struct AudioFloatStreamListener{
    AudioFloatStreamListener(size_t p_expected_samples_per_frame)
        : expected_samples_per_frame(p_expected_samples_per_frame)
    {}
    virtual ~AudioFloatStreamListener() = default;
    virtual void on_samples(const float* data, size_t frames) = 0;

    size_t expected_samples_per_frame;
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
        float volume_multiplier,    //  Multiply every sample by this constant.
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
    float m_volume_multiplier;
    bool m_reverse_channels;
    size_t m_sample_size;
    size_t m_frame_size;
    std::set<AudioFloatStreamListener*> m_listeners;

    LifetimeSanitizer m_sanitizer;
};



//  Convert a stream of float samples to raw audio.
class AudioFloatToStream : public AudioFloatStreamListener{
public:
    void add_listener(StreamListener& listener);
    void remove_listener(StreamListener& listener);

public:
    AudioFloatToStream(AudioSampleFormat output_format, size_t samples_per_frame);
    virtual ~AudioFloatToStream();
    virtual void on_samples(const float* data, size_t frames) override;

private:
    AudioSampleFormat m_format;
    size_t m_samples_per_frame;
    size_t m_sample_size;
    size_t m_frame_size;
    size_t m_buffer_size;
    AlignedVector<char> m_buffer;
    ListenerSet<StreamListener> m_listeners;

    LifetimeSanitizer m_sanitizer;
};



}
#endif
