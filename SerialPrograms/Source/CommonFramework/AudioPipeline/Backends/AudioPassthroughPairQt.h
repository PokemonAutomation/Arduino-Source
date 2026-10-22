/*  Audio Passthrough Pair (Qt)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_AudioPipeline_AudioPassthroughPairQt_H
#define PokemonAutomation_AudioPipeline_AudioPassthroughPairQt_H

#include <memory>
#include <map>
//#include <set>
#include <QObject>
#include "Common/Cpp/ListenerSet.h"
#include "Common/Cpp/LifetimeSanitizer.h"
#include "Common/Cpp/Concurrency/SpinLock.h"
#include "CommonFramework/AudioPipeline/AudioPassthroughPair.h"

namespace PokemonAutomation{

class Logger;
class AudioSource;
class AudioOutputDevice;
class WorkingAudioToFFT;

namespace Kernels{
namespace AudioPipeline{
class RawAudio2WorkingAudio;
class WorkingAudio2RawAudio;
class WorkingAudio2WorkingAudio;
class WorkingAudioStreamListener;
}
}


class AudioPassthroughPairQt final : public QObject, public AudioPassthroughPair{
public:
    virtual void add_listener(AudioFloatStreamListener& listener) override;
    virtual void remove_listener(AudioFloatStreamListener& listener) override;

    virtual void add_listener(FFTListener& listener) override;
    virtual void remove_listener(FFTListener& listener) override;

    virtual void add_listener(const AudioFormat& format, Kernels::AudioPipeline::WorkingAudioStreamListener& listener) override;
    virtual void remove_listener(Kernels::AudioPipeline::WorkingAudioStreamListener& listener) override;


public:
    virtual ~AudioPassthroughPairQt();
    AudioPassthroughPairQt(Logger& logger);

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
    class SampleListener;
    class InternalFFTListener;
    class InternalWorkingListener;

    //  One entry per attached working-stream listener: the requested format
    //  plus the conversion node (source shape -> requested shape) and the
    //  forwarder that carries the requested shape as its nominal one. The
    //  node attaches to m_source_decoder; the forwarder attaches to the node.
    //  All of these are only touched while holding m_lock.
    struct WorkingListenerEntry{
        AudioFormat m_format;
        std::unique_ptr<Kernels::AudioPipeline::WorkingAudio2WorkingAudio> m_node;
        std::unique_ptr<InternalWorkingListener> m_forwarder;
    };

    void init_audio_sink();

    //  New pipeline tree. Both endpoints have switched over: the FFT
    //  branch feeds m_fft_node (m_fft_listener forwards its FFTs to the
    //  pair's FFTListener set), and the sink branch's encoder pushes raw
    //  bytes to m_sink_device, the open output device.
    //  All of these are only touched while holding m_lock; the producing
    //  thread only ever sees them through the node listener sets, where
    //  ListenerSet::remove blocks on in-flight pushes so rebuilds never
    //  race a live push.
    void teardown_tree();
    void teardown_sink_branch();
    void build_tree(float input_volume);
    void rebuild_sink_branch();

    void build_working_entry(WorkingListenerEntry& entry, Kernels::AudioPipeline::WorkingAudioStreamListener& listener);
    void detach_working_entry(WorkingListenerEntry& entry);


private:
    Logger& m_logger;

    mutable SpinLock m_lock;

    //  The order of these parameters is important due to destruction order.
    //  Objects lower on this list attach to and hold references to those
    //  higher on the list.

    AudioChannelFormat m_input_format;
    std::unique_ptr<AudioSource> m_reader;
    std::unique_ptr<SampleListener> m_sample_listener;      //  Attaches to "m_reader".

    //  New pipeline tree. The decoder attaches to m_reader's raw stream and
    //  the branch heads attach to the decoder (sink head before FFT head).
    //  Declared after m_reader so the nodes die before the reader does.
    //  (For the INTERLEAVE_96000 presets, m_reader emits the two 48 kHz
    //  channels reinterpreted from the physical 1ch x 96000 stream;
    //  MONO_96000 keeps the physical shape.)
    std::unique_ptr<Kernels::AudioPipeline::RawAudio2WorkingAudio> m_source_decoder;
    std::unique_ptr<Kernels::AudioPipeline::WorkingAudio2WorkingAudio> m_sink_conv;  //  Source shape -> sink shape (mix + resample).
    std::unique_ptr<Kernels::AudioPipeline::WorkingAudio2RawAudio> m_sink_encoder;
    std::unique_ptr<Kernels::AudioPipeline::WorkingAudio2WorkingAudio> m_fft_conv;   //  Source shape -> 48 kHz mono (mix + resample).
    std::unique_ptr<WorkingAudioToFFT> m_fft_node;                              //  The tree's FFT endpoint.

    //  Per-listener working-stream conversion nodes. The entries survive
    //  tree rebuilds (the listeners stay attached across input changes);
    //  their nodes are rebuilt for each new source shape in build_tree()
    //  and detached in teardown_tree().
    std::map<Kernels::AudioPipeline::WorkingAudioStreamListener*, WorkingListenerEntry> m_working_entries;

    AudioDeviceInfo m_output_device;
    float m_file_input_multiplier = 1.0;
    float m_device_input_multiplier = 1.0;
    double m_output_volume = 1.0;
    std::unique_ptr<AudioOutputDevice> m_sink_device;   //  Attaches to m_sink_encoder.


    std::unique_ptr<InternalFFTListener> m_fft_listener;    //  Attaches to m_fft_node.

    ListenerSet<AudioFloatStreamListener> m_stream_listeners;
    ListenerSet<FFTListener> m_fft_listeners;

    LifetimeSanitizer m_sanitizer;
};



}
#endif
