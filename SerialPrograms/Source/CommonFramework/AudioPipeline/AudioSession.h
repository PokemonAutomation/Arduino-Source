/*  Audio Session
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *      AudioSession represents a live audio session. It holds onto the audio
 *  input/output sessions which can be asynchronously set at any time.
 *
 *  This class is not responsible for any UI. However, any changes made to this
 *  class will be forwarded to any UI components that are attached to it.
 *
 *  The UI that allows a user to control this class is in AudioSelectorWidget.
 *  The actual display of the audio spectrum is in AudioDisplayWidget.
 *
 */

#ifndef PokemonAutomation_AudioPipeline_AudioSession_H
#define PokemonAutomation_AudioPipeline_AudioSession_H

#include "Common/Cpp/ListenerSet.h"
#include "Common/Cpp/Concurrency/Watchdog.h"
#include "AudioFeed.h"
#include "AudioPassthroughPair.h"
#include "Spectrum/FFTStreamer.h"
#include "Spectrum/AudioSpectrumHolder.h"
#include "AudioOption.h"

namespace PokemonAutomation{

class Logger;


class AudioSession final : public AudioFeed, private FFTListener, private WatchdogCallback{
public:
    struct StateListener{
        virtual void pre_input_change(){}
        virtual void post_input_change(const std::string& file, const AudioDeviceInfo& device, AudioChannelFormat format){}
        virtual void post_output_change(const AudioDeviceInfo& device){}
        virtual void post_volume_change(double volume){}
        virtual void post_display_change(AudioOption::AudioDisplayType display){}
    };
    void add_state_listener(StateListener& listener);
    void remove_state_listener(StateListener& listener);

    void add_stream_listener(AudioFloatStreamListener& listener);
    void remove_stream_listener(AudioFloatStreamListener& listener);

    void add_spectrum_listener(AudioSpectrumHolder::Listener& listener);
    void remove_spectrum_listener(AudioSpectrumHolder::Listener& listener);


public:
    ~AudioSession();
    AudioSession(Logger& logger, AudioOption& option);

    void get(AudioOption& option);
    void set(const AudioOption& option);

    std::pair<std::string, AudioDeviceInfo> input_device() const;
    AudioChannelFormat input_format() const;

    AudioDeviceInfo output_device() const;
    double output_volume() const;

    AudioOption::AudioDisplayType display_type() const;
    AudioSpectrumHolder& spectrums(){ return m_spectrum_holder; }

    void clear_audio_input();
    void set_audio_input(std::string file);
    void set_audio_input(AudioDeviceInfo info);
    void set_format(AudioChannelFormat format);

    void clear_audio_output();
    void set_audio_output(AudioDeviceInfo info);
    void set_volume(double volume);
    void set_display(AudioOption::AudioDisplayType display);


public:
    virtual void reset() override;
    virtual std::vector<AudioSpectrum> spectrums_since(uint64_t starting_seqnum) override;
    virtual std::vector<AudioSpectrum> spectrums_latest(size_t num_last_spectrums) override;
    virtual void add_overlay(uint64_t starting_seqnum, size_t end_seqnum, Color color) override;


private:
    virtual void on_fft(size_t sample_rate, std::shared_ptr<const AlignedVector<float>> fft_output) override;
    virtual void on_watchdog_timeout() override;

    bool sanitize_format();

    void signal_pre_input_change();
    void signal_post_input_change();
    void signal_post_output_change();


private:
    Logger& m_logger;
    AudioOption& m_option;
    AudioSpectrumHolder m_spectrum_holder;
    std::unique_ptr<AudioPassthroughPair> m_devices;

    mutable std::mutex m_lock;

    ListenerSet<StateListener> m_listeners;

};



}
#endif
