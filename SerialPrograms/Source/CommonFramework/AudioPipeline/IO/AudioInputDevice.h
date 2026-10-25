/*  Audio Input Device
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_AudioPipeline_AudioInputDevice_H
#define PokemonAutomation_AudioPipeline_AudioInputDevice_H

#include <QAudioSource>
using NativeAudioSource = QAudioSource;

#include <memory>
#include <QIODevice>
#include "Common/Cpp/ListenerSet.h"
#include "Common/Cpp/LifetimeSanitizer.h"
#include "Common/Cpp/StreamConverters.h"
#include "Kernels/AudioPipeline/Kernels_AudioPipeline.h"
#include "CommonFramework/AudioPipeline/AudioInfo.h"

class QAudioFormat;

namespace PokemonAutomation{

class Logger;


//
//  Raw audio input from a capture device (QAudioSource / QAudioInput).
//  Emits the raw stream to RawAudioStreamListener children (root of the new
//  pipeline tree).
//  Qt makes no contract about the frame alignment of the chunks that the
//  audio backend writes into the QIODevice, so the raw output goes through a
//  MisalignedStreamConverter / ObjectStreamListener byte-reassembly step.
//  In practice the backends emit whole-frame chunks, so no splicing happens
//  in the common case.
//
//  The reinterpreted input choices (INTERLEAVE_LR/RL_96000) open the
//  device as a physical 1ch x 96000 stream that is really the card's two
//  48 kHz channels (a misreporting capture card), so when
//  "reinterpret_96k_mono_as_stereo" is set (the choice's is_reinterpreted())
//  the raw output is that same byte stream reinterpreted as 2ch x 48000:
//  only the (rate, channels) framing changes, aligned to 2-sample
//  stereo-frame boundaries. The MONO_96000 native choice is a true 96 kHz
//  mono stream and keeps that shape through the whole pipeline; every other
//  native choice passes through unchanged.
//
class AudioInputDevice final
    : public QIODevice
    , private MisalignedStreamConverter
    , private ObjectStreamListener
{
public:
    ~AudioInputDevice();

    AudioInputDevice(
        Logger& logger,
        const NativeAudioInfo& device, const QAudioFormat& format,
        bool reinterpret_96k_mono_as_stereo
    );

    Kernels::AudioPipeline::SampleFormat sample_type() const{
        return m_sample_type;
    }
    //  The raw listener output shape (the 96 kHz reinterpretation applied).
    size_t sample_rate() const{
        return m_out_sample_rate;
    }
    size_t channels() const{
        return m_out_channels;
    }

    void add_listener(Kernels::AudioPipeline::RawAudioStreamListener& listener);
    void remove_listener(Kernels::AudioPipeline::RawAudioStreamListener& listener);


    virtual bool isSequential() const override { return true; }
    virtual qint64 readData(char* data, qint64 maxlen) override { return 0; }
    virtual qint64 writeData(const char* data, qint64 len) override;


protected:
    //  MisalignedStreamConverter: raw pass-through (in and out are the same size).
    virtual void convert(void* out, const void* in, size_t count) override;
    //  ObjectStreamListener: aligned frames out to the raw listeners.
    virtual void on_objects(const void* data, size_t objects) override;


private:
    std::unique_ptr<NativeAudioSource> m_source;
    Kernels::AudioPipeline::SampleFormat m_sample_type;
    size_t m_sample_rate;
    size_t m_channels;
    size_t m_frame_bytes;
    size_t m_out_sample_rate;
    size_t m_out_channels;
    size_t m_out_frame_bytes;
    ListenerSet<Kernels::AudioPipeline::RawAudioStreamListener> m_listeners;
    LifetimeSanitizer m_sanitizer;
};

}
#endif
