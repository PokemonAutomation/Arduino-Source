/*  Audio Device Info
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_AudioPipeline_Backends_AudioDeviceInfo_H
#define PokemonAutomation_AudioPipeline_Backends_AudioDeviceInfo_H

#include <string>
#include <vector>
#include "CommonFramework/AudioPipeline/AudioFormat.h"
#include "CommonFramework/AudioPipeline/AudioInputChoice.h"

namespace PokemonAutomation{


class AudioDeviceInfo{
public:
    ~AudioDeviceInfo();
    AudioDeviceInfo(const AudioDeviceInfo& x);
    void operator=(const AudioDeviceInfo& x);

public:
    AudioDeviceInfo();
    explicit AudioDeviceInfo(const std::string& device_name);

    explicit operator bool() const;

    const std::string& display_name() const;
    const std::string& device_name() const;

    //  The device-driven format list (see input_choices()). Populated for
    //  input devices only; empty for output devices (the sink opens in its
    //  native format).
    const std::vector<AudioInputChoice>& supported_choices() const;
    size_t preferred_choice_index() const;

    const NativeAudioInfo& native_info() const;

    bool operator==(const AudioDeviceInfo& info) const;

    static std::vector<AudioDeviceInfo> all_input_devices();
    static std::vector<AudioDeviceInfo> all_output_devices();

private:
    struct Data;
    Pimpl<Data> m_body;
};





}
#endif
