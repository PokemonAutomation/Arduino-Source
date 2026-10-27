/*  Audio Source (Device)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Json/JsonValue.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/AudioPipeline/AudioPipelineOptions.h"
#include "CommonFramework/AudioPipeline/Tools/AudioFormatUtils.h"
#include "AudioSource_Device.h"

namespace PokemonAutomation{



bool AudioSourceDescriptor_Device::operator==(const AudioSourceDescriptor& x) const{
    if (typeid(*this) != typeid(x)){
        return false;
    }
    return m_info == static_cast<const AudioSourceDescriptor_Device&>(x).m_info;
}
std::string AudioSourceDescriptor_Device::display_name() const{
    return m_info.display_name();
}

JsonValue AudioSourceDescriptor_Device::to_json() const{
    return m_info.device_name();
}
void AudioSourceDescriptor_Device::load_json(const JsonValue& json){
    const std::string* name = json.to_string();
    if (name != nullptr){
        m_info = AudioDeviceInfo(*name);
    }
}

std::unique_ptr<AudioSource> AudioSourceDescriptor_Device::make_AudioSource(
    Logger& logger,
    const AudioInputChoice& format
) const{
    if (!format){
        return nullptr;
    }
//    cout << "format = " << (int)format << endl;
    return std::make_unique<AudioSource_Device>(logger, m_info, format);
}




AudioSource_Device::AudioSource_Device(
    Logger& logger,
    const AudioDeviceInfo& device,
    const AudioInputChoice& format
){
    if (!format){
        throw InternalProgramError(
            nullptr, PA_CURRENT_FUNCTION, "Invalid audio input choice."
        );
    }

    NativeAudioInfo native_info = device.native_info();
    QAudioFormat native_format = format.physical_format().native_format();

    m_stream_format = format.format().sample_format();
    if (m_stream_format == AudioSampleFormat::INVALID){
        //  Defensive: the choices built from input_choices() always carry
        //  a sample format the device supports.
        set_sample_format_to_float(native_format);
        m_stream_format = AudioSampleFormat::FLOAT32;
    }

    m_input_device = std::make_unique<AudioInputDevice>(
        logger,
        native_info,
        native_format,
        format.is_reinterpreted()
    );
}

void AudioSource_Device::add_raw_listener(RawAudioStreamListener& listener){
    m_input_device->add_listener(listener);
}
void AudioSource_Device::remove_raw_listener(RawAudioStreamListener& listener){
    m_input_device->remove_listener(listener);
}

size_t AudioSource_Device::raw_sample_rate() const{
    return m_input_device->sample_rate();
}
size_t AudioSource_Device::raw_channels() const{
    return m_input_device->channels();
}

float AudioSource_Device::volume_multiplier() const{
    return (float)GlobalSettings::instance().AUDIO_PIPELINE->DEVICE_VOLUME_SCALE;
}







}
