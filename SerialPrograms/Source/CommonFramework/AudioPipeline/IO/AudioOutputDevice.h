/*  Audio Output Device
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *      AudioOutputDevice wraps an open QAudioSink and plays back the raw
 *  byte stream pushed into it. It does no sample conversion: the pipeline
 *  tree's sink branch (WorkingAudio2RawAudio) delivers samples in the
 *  device's sample format, and this class just writes them to the device.
 *
 */

#ifndef PokemonAutomation_AudioPipeline_AudioOutputDevice_H
#define PokemonAutomation_AudioPipeline_AudioOutputDevice_H

#include <QAudioSink>
#include "Common/Cpp/LifetimeSanitizer.h"
#include "CommonFramework/AudioPipeline/AudioInfo.h"
#include "Kernels/AudioPipeline/Kernels_AudioPipeline.h"

namespace PokemonAutomation{

class Logger;

class AudioOutputDevice : public Kernels::AudioPipeline::RawAudioStreamListener{
public:
    ~AudioOutputDevice();
    AudioOutputDevice(
        Logger& logger,
        const NativeAudioInfo& device,
        const QAudioFormat& format,
        Kernels::AudioPipeline::SampleFormat sample_format,
        size_t sample_rate,
        size_t channels
    );

    virtual void push_samples(size_t frames, const void* data) override;


private:
    const size_t m_bytes_per_frame;

    Logger& m_logger;
    QAudioSink m_sink;
    QIODevice* m_io_device;

    LifetimeSanitizer m_sanitizer;
};



}
#endif
