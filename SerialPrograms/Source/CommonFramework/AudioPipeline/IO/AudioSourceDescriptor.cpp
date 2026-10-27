/*  Audio Source Descriptor
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <memory>
#include "Common/Cpp/Logging/AbstractLogger.h"
#include "Common/Qt/StringToolsQt.h"
#include "CommonFramework/AudioPipeline/Tools/AudioFormatUtils.h"
#include <QAudioFormat>
#include "AudioSource.h"
#include "AudioSourceDescriptor.h"
#include "AudioInputFile.h"
#include "AudioInputDevice.h"

namespace PokemonAutomation{



std::unique_ptr<AudioSource> AudioSourceDescriptor_File::make_AudioSource(
    Logger& logger
) const{
    if (m_path.empty()){
        return nullptr;
    }
    QAudioFormat native_format;
    if (has_extension(m_path, "wav") && probe_wav_native_format(m_path, native_format)){
        //  Native WAV: the stream is the file's own format (rate, channels,
        //  sample type). The tree's decoder / resampler / mixer convert it
        //  to the sink's format; no 96k reinterpretation for files.
    }else{
        //  Compressed formats: probe the decoder's native output format
        //  and stream the file at its own rate / channels / sample type,
        //  like the native WAV path (the tree's decoder / resampler /
        //  mixer convert it to the sink's format). If the probe fails
        //  (no decode backend, unreadable file), fall back to the legacy
        //  48k stereo float target; the loader logs the decode failure as
        //  before. An unreadable .wav falls through here too.
        if (!probe_decoded_native_format(m_path, native_format)){
            //  The channel count must be set before the channel config:
            //  QAudioFormat::setChannelCount() resets the config.
            native_format.setChannelCount(2);
            native_format.setChannelConfig(QAudioFormat::ChannelConfig::ChannelConfigStereo);
            native_format.setSampleRate(48000);
            set_sample_format_to_float(native_format);
        }
    }
    return std::make_unique<AudioInputFile>(logger, m_path, native_format);
}



std::unique_ptr<AudioSource> AudioSourceDescriptor_Device::make_AudioSource(
    Logger& logger
) const{
    if (!m_choice){
        return nullptr;
    }
    NativeAudioInfo native_info = m_device.native_info();

    //  The device is opened in the choice's physical format: for the
    //  reinterpreted capture-card choices this is the physical 1ch x
    //  96000 stream; for native choices it is the choice itself.
    QAudioFormat native_format = m_choice.physical_format().native_format();
    if (m_choice.format().sample_format() == AudioSampleFormat::INVALID){
        //  Defensive: the choices built from input_choices() always carry
        //  a sample format the device supports.
        set_sample_format_to_float(native_format);
    }
    return std::make_unique<AudioInputDevice>(
        logger,
        native_info,
        native_format,
        m_choice.is_reinterpreted()
    );
}



}
