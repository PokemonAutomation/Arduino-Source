/*  Audio Input File
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_AudioPipeline_AudioInputFile_H
#define PokemonAutomation_AudioPipeline_AudioInputFile_H

#include <memory>
#include <string>
#include <QObject>
#include "Common/Cpp/ListenerSet.h"
#include "Common/Cpp/LifetimeSanitizer.h"
#include "Kernels/AudioPipeline/Kernels_AudioPipeline.h"

class QAudioFormat;

namespace PokemonAutomation{

class Logger;
class AudioStreamToFloat;
class AudioFileLoader;


//  Probes a .wav file's native audio format (sample rate, channel count,
//  sample type) by reading its header. Returns false if the file cannot be
//  opened or is not a supported WAV (PCM 8/16/32-bit or IEEE-float).
bool probe_wav_native_format(const std::string& filename, QAudioFormat& out_format);


//
//  Raw audio input from a file (.wav via WavFile, compressed formats via
//  QAudioDecoder).
//
//  Feeds the legacy AudioStreamToFloat reader and, in parallel, emits the raw
//  stream to RawAudioStreamListener children (root of the new pipeline tree).
//  AudioFileLoader only ever emits chunks that are whole multiples of the
//  frame size, so the raw output is frame-aligned by construction and is
//  passed to the listeners as-is.
//
class AudioInputFile final : public QObject{
public:
    ~AudioInputFile();

    AudioInputFile(
        Logger& logger, AudioStreamToFloat& reader,
        const std::string& file, const QAudioFormat& format
    );

    Kernels::AudioPipeline::SampleFormat sample_type() const{
        return m_sample_type;
    }
    size_t sample_rate() const{
        return m_sample_rate;
    }
    size_t channels() const{
        return m_channels;
    }

    void add_listener(Kernels::AudioPipeline::RawAudioStreamListener& listener);
    void remove_listener(Kernels::AudioPipeline::RawAudioStreamListener& listener);


private:
    void on_buffer_ready(const char* data, size_t len);


private:
    AudioStreamToFloat& m_reader;
    std::unique_ptr<AudioFileLoader> m_source;
    Kernels::AudioPipeline::SampleFormat m_sample_type;
    size_t m_sample_rate;
    size_t m_channels;
    size_t m_frame_bytes;
    ListenerSet<Kernels::AudioPipeline::RawAudioStreamListener> m_listeners;
    LifetimeSanitizer m_sanitizer;
};

}
#endif
