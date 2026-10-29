/*  Audio Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_AudioPipeline_AudioOption_H
#define PokemonAutomation_AudioPipeline_AudioOption_H

#include <memory>
#include <map>
#include "AudioInfo.h"
#include "AudioSourceDescriptor.h"

namespace PokemonAutomation{

class JsonValue;
class AudioSession;
class AudioDisplayWidget;
class AudioSelectorWidget;


// TODO: if needed, can add state of FFT parameters (FFT length, sliding
// window step, etc.) here.
class AudioOption{
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

    std::shared_ptr<AudioSourceDescriptor> input_source() const{ return m_input_source; }
    AudioInputChoice input_choice() const{ return m_input_choice; }

    //  Use a cache to help create a VideoSoruceDescriptor.
    //  The cache remembers the last used descriptor for each interface type. That way when
    //  the user switches back-and-forth between two interfaces, it will reload the previous
    //  one and return faster.
    //  If not found in cache, it will create a new one according to the type.
    void set_input(std::shared_ptr<AudioSourceDescriptor> descriptor);
    std::shared_ptr<AudioSourceDescriptor> get_input_from_cache(AudioSourceType type);

    const AudioDeviceInfo& output_device() const{ return m_output_device; }
    AudioDisplayType display_type() const{ return m_display_type; }
    double volume() const{ return m_volume; }


public:
    void load_json(const JsonValue& json);
    JsonValue to_json() const;


private:
    friend class AudioSession;

    std::shared_ptr<AudioSourceDescriptor> m_input_source;
    std::map<AudioSourceType, std::shared_ptr<AudioSourceDescriptor>> m_input_cache;

    AudioInputChoice m_input_choice;
    AudioDeviceInfo m_output_device;
    AudioDisplayType m_display_type = AudioDisplayType::FREQ_BARS;
    double m_volume = 1.0;  //  Volume Range: [0, 1.0]
};





}
#endif
