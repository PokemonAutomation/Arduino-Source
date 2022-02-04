/*  Audio Input Device Selector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_AudioSelector_H
#define PokemonAutomation_AudioSelector_H

#include "CommonFramework/Logging/Logger.h"
#include "AudioInfo.h"

class QJsonValue;
class QWidget;

namespace PokemonAutomation{

class AudioDisplayWidget;


class AudioSelectorWidget;

// Handles the state of audio: the audio source.
// Call make_ui() to generate the UI friend class AudioSelectorWidget,
// which directly modifies AudioSelector's internal state.
// This separates state from UI.
// TODO: if needed, can add state of FFT parameters (FFT length, sliding
// window step, etc.) here.
class AudioSelector{
    static const QString JSON_INPUT_DEVICE;
    static const QString JSON_OUTPUT_DEVICE;
    static const QString JSON_AUDIO_VIS;

public:
    enum class AudioDisplayType{
        NO_DISPLAY,
        FREQ_BARS,
        SPECTROGRAM
    };
    static AudioDisplayType stringToAudioDisplayType(const std::string& value);
    static std::string audioDisplayTypeToString(AudioDisplayType type);

    AudioSelector();
    AudioSelector(const QJsonValue& json);

    void load_json(const QJsonValue& json);
    QJsonValue to_json() const;

    AudioSelectorWidget* make_ui(QWidget& parent, Logger& logger, AudioDisplayWidget& holder);

private:
    friend class AudioSelectorWidget;
    AudioInfo m_inputDevice, m_outputDevice;
    AudioDisplayType m_audioDisplayType = AudioDisplayType::NO_DISPLAY;
};





}
#endif
