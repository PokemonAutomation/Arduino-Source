/*  Audio Thread Controller
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "AudioDisplayWidget.h"
#include "AudioThreadController.h"

namespace PokemonAutomation{



AudioThreadController::AudioThreadController(
    Logger& logger,
    AudioDisplayWidget* parent,
    const AudioDeviceInfo& inputInfo,
    AudioChannelFormat inputFormat,
    const std::string& inputAbsoluteFilepath,
    const AudioDeviceInfo& outputInfo,
    float outputVolume
)
    : m_parent(*parent)
    , m_devices(logger)
{
    if (!inputAbsoluteFilepath.empty()){
        m_devices.set_audio_source(inputAbsoluteFilepath);
    }else{
        m_devices.set_audio_source(inputInfo, inputFormat);
    }

    m_devices.set_audio_sink(outputInfo, outputVolume);

    m_devices.add_listener(*this);
}
AudioThreadController::~AudioThreadController(){
    m_devices.remove_listener(*this);
}

void AudioThreadController::on_fft(size_t sample_rate, std::shared_ptr<AlignedVector<float>> fft_output){
    m_parent.spectrum().push_spectrum(sample_rate, std::move(fft_output));
}




}
