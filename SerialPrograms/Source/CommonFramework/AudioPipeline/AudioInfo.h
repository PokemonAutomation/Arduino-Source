/*  Audio Device Info
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_AudioPipeline_AudioInfo_H
#define PokemonAutomation_AudioPipeline_AudioInfo_H

#include <string>
#include <vector>
#include <QtConfig>
#include "Common/Cpp/Pimpl.h"

class QString;

#if QT_VERSION_MAJOR == 5
class QAudioDeviceInfo;
using NativeAudioInfo = QAudioDeviceInfo;
#elif QT_VERSION_MAJOR == 6
class QAudioDevice;
using NativeAudioInfo = QAudioDevice;
#else
#error "Unknown Qt version."
#endif



namespace PokemonAutomation{


class AudioInfo{
public:
    ~AudioInfo();

public:
    AudioInfo();
    AudioInfo(const std::string& device_name);

    operator bool() const;

    const std::string& device_name() const;
    const QString& display_name() const;
    const NativeAudioInfo& native_info() const;

    bool operator==(const AudioInfo& info);

    static std::vector<AudioInfo> all_input_devices();
    static std::vector<AudioInfo> all_output_devices();

private:
    struct Data;
    Pimpl<Data> m_body;
};





}
#endif
