/*  Audio Passthrough Pair (Qt separate thread)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *      Same as AudioPassthroughPairQt, but with the audio running
 *  on a separate thread to avoid UI thread noise.
 *
 */

#ifndef PokemonAutomation_AudioPipeline_AudioPassthroughPairQtThread_H
#define PokemonAutomation_AudioPipeline_AudioPassthroughPairQtThread_H

#include <QThread>
#include "Common/Cpp/LifetimeSanitizer.h"
#include "CommonFramework/AudioPipeline/AudioPassthroughPair.h"

namespace PokemonAutomation{

class Logger;
class AudioPassthroughPairQt;


class AudioPassthroughPairQtThread : private QThread, public AudioPassthroughPair{
public:
    virtual void add_listener(AudioFloatStreamListener& listener) override;
    virtual void remove_listener(AudioFloatStreamListener& listener) override;

    virtual void add_listener(FFTListener& listener) override;
    virtual void remove_listener(FFTListener& listener) override;

public:
    AudioPassthroughPairQtThread(Logger& logger);
    ~AudioPassthroughPairQtThread();

    virtual void reset(
        const std::string& file,
        const AudioDeviceInfo& output, double output_volume
    ) override;
    virtual void reset(
        const AudioDeviceInfo& input, AudioChannelFormat format,
        const AudioDeviceInfo& output, double output_volume
    ) override;

    virtual void clear_audio_source() override;
    virtual void set_audio_source(const std::string& file) override;
    virtual void set_audio_source(const AudioDeviceInfo& device, AudioChannelFormat format) override;

    virtual void clear_audio_sink() override;
    virtual void set_audio_sink(const AudioDeviceInfo& device, double volume) override;

    virtual void set_sink_volume(double volume) override;

private:
    virtual void run() override;

private:
    Logger& m_logger;
    std::atomic<AudioPassthroughPairQt*> m_body;
    LifetimeSanitizer m_sanitizer;
};




}
#endif
