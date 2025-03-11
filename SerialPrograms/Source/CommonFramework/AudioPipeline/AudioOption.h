/*  Audio Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_AudioPipeline_AudioOption_H
#define PokemonAutomation_AudioPipeline_AudioOption_H

#include "AudioInfo.h"

namespace PokemonAutomation{

class JsonValue;
class AudioSession;
class AudioDisplayWidget;
class AudioSelectorWidget;


// TODO: if needed, can add state of FFT parameters (FFT length, sliding
// window step, etc.) here.
class AudioOption{
    static const std::string JSON_INPUT_FILE;
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

public:
    AudioOption();

    const std::string& input_file() const{ return m_input_file; }
    const AudioDeviceInfo& input_device() const{ return m_input_device; }
    AudioChannelFormat input_format() const{ return m_input_format; }

    const AudioDeviceInfo& output_device() const{ return m_output_device; }
    AudioDisplayType display_type() const{ return m_display_type; }
    double volume() const{ return m_volume; }

//    void set_input_file(std::string&& value){ m_input_file = std::move(value); }
//    void set_input_device(AudioDeviceInfo&& value){ m_input_device = std::move(value); }
//    void set_input_format(AudioChannelFormat value){ m_input_format = value; }
//    void set_output_device(AudioDeviceInfo&& value){ m_output_device = std::move(value); }
//    void set_volume(double value){ m_volume = value; }
//    void set_display_type(AudioDisplayType value){ m_display_type = value; }

public:
    void load_json(const JsonValue& json);
    JsonValue to_json() const;

private:
    friend class AudioSession;
    std::string m_input_file;
    AudioDeviceInfo m_input_device;
    AudioChannelFormat m_input_format = AudioChannelFormat::NONE;
    AudioDeviceInfo m_output_device;
    AudioDisplayType m_display_type = AudioDisplayType::FREQ_BARS;
    double m_volume = 1.0;  //  Volume Range: [0, 1.0]
};





}
#endif
