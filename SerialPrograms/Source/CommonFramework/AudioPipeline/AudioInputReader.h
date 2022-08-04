/*  Audio Input Reader
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *      AudioInputReader represents an audio input stream. Once constructed,
 *  it spits out the float samples. Listeners can attach themselves to this
 *  class to receive these audio frames.
 *
 */

#ifndef PokemonAutomation_AudioPipeline_AudioInputReader_H
#define PokemonAutomation_AudioPipeline_AudioInputReader_H

#include <memory>
#include <set>
#include "Common/Cpp/SpinLock.h"
#include "AudioInfo.h"
#include "AudioStream.h"

namespace PokemonAutomation{

class Logger;
class AudioSourceReader;
class AudioInputFile;
class AudioInputDevice;


class AudioInputReader{
public:
    void add_listener(AudioFloatStreamListener& listener);
    void remove_listener(AudioFloatStreamListener& listener);

public:
    ~AudioInputReader();

    //  Read from an audio file. (i.e. .wave or .mp3)
    AudioInputReader(Logger& logger, const std::string& file, AudioFormat format);

    //  Read from an audio input device. (i.e. capture card)
    AudioInputReader(Logger& logger, const AudioDeviceInfo& device, AudioFormat format);

    size_t channels() const{ return m_channels; }
    size_t sample_rate() const{ return m_sample_rate; }

private:
    void init(AudioFormat format, AudioSampleFormat stream_format);

private:
    class InternalListener;

    Logger& m_logger;

    size_t m_channels;
    size_t m_sample_rate;
    size_t m_multiplier;

    SpinLock m_lock;

    std::unique_ptr<InternalListener> m_internal_listener;
    std::unique_ptr<AudioSourceReader> m_reader;

    std::unique_ptr<AudioInputFile> m_input_file;
    std::unique_ptr<AudioInputDevice> m_input_device;

    std::set<AudioFloatStreamListener*> m_listeners;
};



}
#endif
