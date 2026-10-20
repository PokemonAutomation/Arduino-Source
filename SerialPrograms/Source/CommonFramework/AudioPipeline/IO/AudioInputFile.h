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
#include "Common/Cpp/StreamConverters.h"
#include "Kernels/AudioPipeline/Kernels_AudioPipeline.h"

class QAudioFormat;

namespace PokemonAutomation{

class Logger;
class AudioStreamToFloat;
class AudioFileLoader;


//
//  Raw audio input from a file (.wav via WavFile, compressed formats via
//  QAudioDecoder).
//
//  Feeds the legacy AudioStreamToFloat reader and, in parallel, emits the raw
//  stream to RawAudioStreamListener children (root of the new pipeline tree).
//  AudioFileLoader only ever emits chunks that are whole multiples of the
//  frame size, so for ordinary formats the raw output is frame-aligned by
//  construction and is passed to the listeners as-is.
//
//  The INTERLEAVE_96000 presets load the file as a physical 1ch x 96000
//  stream that is really two 48 kHz channels (a misreporting capture card),
//  so when "reinterpret_96k_mono_as_stereo" is set the raw output
//  reinterprets that stream as 2ch x 48000 from the start: the byte stream
//  is the same, only the (rate, channels) framing changes, and the
//  MisalignedStreamConverter reassembler aligns the listener output to
//  2-sample stereo-frame boundaries. MONO_96000 is a true 96 kHz mono
//  stream and keeps that shape through the whole pipeline. This
//  reinterpretation is temporary: the format dropdown rewrite will request
//  48 kHz stereo directly, at which point it goes away.
//
class AudioInputFile final
    : public QObject
    , private MisalignedStreamConverter
    , private ObjectStreamListener
{
public:
    ~AudioInputFile();

    AudioInputFile(
        Logger& logger, AudioStreamToFloat& reader,
        const std::string& file, const QAudioFormat& format,
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


protected:
    //  MisalignedStreamConverter: raw pass-through (in and out are the same size).
    virtual void convert(void* out, const void* in, size_t count) override;
    //  ObjectStreamListener: aligned frames out to the raw listeners.
    virtual void on_objects(const void* data, size_t objects) override;


private:
    void on_buffer_ready(const char* data, size_t len);


private:
    AudioStreamToFloat& m_reader;
    std::unique_ptr<AudioFileLoader> m_source;
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
