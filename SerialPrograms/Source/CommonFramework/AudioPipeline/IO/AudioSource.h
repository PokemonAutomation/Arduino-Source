/*  Audio Source
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *      AudioSource represents an audio input stream. Once constructed, it
 *  emits the raw frame-aligned stream to its RawAudioStreamListener
 *  children (root of the pipeline tree).
 *
 */

#ifndef PokemonAutomation_AudioPipeline_AudioSource_H
#define PokemonAutomation_AudioPipeline_AudioSource_H

#include <memory>
#include "Common/Cpp/LifetimeSanitizer.h"
#include "CommonFramework/AudioPipeline/AudioInfo.h"

namespace PokemonAutomation{

class Logger;
class AudioInputFile;
class AudioInputDevice;

namespace Kernels{namespace AudioPipeline{class RawAudioStreamListener;}}


class AudioSource{
public:
    //  Raw stream (pre-conversion) access. The raw stream is the source's
    //  native frame-aligned output, before the pipeline tree applies any
    //  conversion (resample / mix / sample-type / volume).
    AudioSampleFormat raw_sample_format() const{ return m_stream_format; }
    size_t raw_sample_rate() const;
    size_t raw_channels() const;
    void add_raw_listener(Kernels::AudioPipeline::RawAudioStreamListener& listener);
    void remove_raw_listener(Kernels::AudioPipeline::RawAudioStreamListener& listener);

public:
    ~AudioSource();

    //  Read from an audio file. (i.e. .wav or .mp3) The stream is the
    //  file's native format (rate, channels, sample type); the tree's
    //  decoder / resampler / mixer convert it to the sink's format.
    AudioSource(
        Logger& logger,
        const std::string& file
    );

    //  Read from an audio input device. (i.e. capture card) The device is
    //  opened in the choice's physical format; the raw stream is the
    //  choice's logical format (the 96 kHz reinterpretation for
    //  misreporting capture cards, when the choice carries it).
    AudioSource(
        Logger& logger,
        const AudioDeviceInfo& device,
        const AudioInputChoice& choice
    );

private:
    AudioSampleFormat m_stream_format;

    std::unique_ptr<AudioInputFile> m_input_file;
    std::unique_ptr<AudioInputDevice> m_input_device;

    LifetimeSanitizer m_sanitizer;
};



}
#endif
