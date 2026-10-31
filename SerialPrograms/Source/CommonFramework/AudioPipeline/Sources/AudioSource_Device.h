/*  Audio Source (Device)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_AudioPipeline_AudioSource_Device_H
#define PokemonAutomation_AudioPipeline_AudioSource_Device_H

#include "CommonFramework/AudioPipeline/AudioSourceDescriptor.h"
#include "CommonFramework/AudioPipeline/AudioSource.h"
#include "CommonFramework/AudioPipeline/Backends/AudioDeviceInfo.h"
#include "CommonFramework/AudioPipeline/IO/AudioInputDevice.h"

namespace PokemonAutomation{


class AudioSourceDescriptor_Device : public AudioSourceDescriptor{
public:
    AudioSourceDescriptor_Device()
        : AudioSourceDescriptor(AudioSourceType::Device)
    {}
    AudioSourceDescriptor_Device(const std::string& device_name)
        : AudioSourceDescriptor(AudioSourceType::Device)
        , m_info(std::move(device_name))
    {}
    AudioSourceDescriptor_Device(AudioDeviceInfo info)
        : AudioSourceDescriptor(AudioSourceType::Device)
        , m_info(std::move(info))
    {}


public:
    const AudioDeviceInfo& device_info() const{
        return m_info;
    }

    virtual bool operator==(const AudioSourceDescriptor& x) const override;
    virtual std::string display_name() const override;

    virtual std::vector<AudioInputChoice> supported_formats() const override;
    virtual bool sanitize_desired_format(Logger& logger, AudioInputChoice& format) const override;

    virtual JsonValue to_json() const override;
    virtual void load_json(const JsonValue& json) override;

    virtual std::unique_ptr<AudioSource> make_AudioSource(
        Logger& logger,
        const AudioInputChoice& format
    ) const override;


private:
    AudioDeviceInfo m_info;
};




class AudioSource_Device : public AudioSource{
public:
    AudioSource_Device(
        Logger& logger,
        const AudioDeviceInfo& device,
        const AudioInputChoice& format
    );

    virtual void add_raw_listener(RawAudioStreamListener& listener) override;
    virtual void remove_raw_listener(RawAudioStreamListener& listener) override;


public:
    virtual AudioSampleFormat raw_sample_format() const override{
        return m_stream_format;
    }
    virtual size_t raw_sample_rate() const override;
    virtual size_t raw_channels() const override;

    virtual float volume_multiplier() const override;


private:
    AudioSampleFormat m_stream_format;
    std::unique_ptr<AudioInputDevice> m_input_device;
};





}
#endif
