/*  Audio Input Device Selector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_AudioPipeline_AudioSelector_H
#define PokemonAutomation_AudioPipeline_AudioSelector_H

#include "CommonFramework/Logging/LoggerQt.h"
#include "AudioInfo.h"

class QWidget;

namespace PokemonAutomation{

class JsonValue2;
class AudioDisplayWidget;
class AudioSelectorWidget;

// Handles the state of audio: the audio source.
// Call make_ui() to generate the UI friend class AudioSelectorWidget,
// which directly modifies AudioSelector's internal state.
// This separates state from UI.
// TODO: if needed, can add state of FFT parameters (FFT length, sliding
// window step, etc.) here.
class AudioSelector{
    static const std::string JSON_INPUT_DEVICE;
    static const std::string JSON_INPUT_FORMAT;
    static const std::string JSON_OUTPUT_DEVICE;
    static const std::string JSON_AUDIO_VIS;
    static const std::string JSON_AUDIO_VOLUME;

public:
    enum class AudioDisplayType{
        NO_DISPLAY,
        FREQ_BARS,
        SPECTROGRAM
    };
    static AudioDisplayType stringToAudioDisplayType(const std::string& value);
    static std::string audioDisplayTypeToString(AudioDisplayType type);

    AudioSelector();
    AudioSelector(const JsonValue2& json);

    void load_json(const JsonValue2& json);
    JsonValue2 to_json() const;

    AudioSelectorWidget* make_ui(QWidget& parent, LoggerQt& logger, AudioDisplayWidget& holder);

private:
    friend class AudioSelectorWidget;
    AudioDeviceInfo m_inputDevice;
    AudioFormat m_inputFormat = AudioFormat::NONE;
    AudioDeviceInfo m_outputDevice;
    AudioDisplayType m_audioDisplayType = AudioDisplayType::NO_DISPLAY;
    int m_volume = 100; // volume range: [0, 100]
};





}
#endif
