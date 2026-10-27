/*  Audio Source Descriptor
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_AudioPipeline_AudioSourceDescriptor_H
#define PokemonAutomation_AudioPipeline_AudioSourceDescriptor_H

#include <memory>
#include "AudioInfo.h"

namespace PokemonAutomation{

class Logger;
class JsonValue;
class AudioSource;



enum class AudioSourceType{
    None,
    File,
    Device,
};


class AudioSourceDescriptor{
public:
    const AudioSourceType type;

    AudioSourceDescriptor(AudioSourceType p_type)
        : type(p_type)
    {}
    virtual ~AudioSourceDescriptor() = default;

public:
    virtual bool should_reload() const{ return false; }
    virtual bool operator==(const AudioSourceDescriptor& x) const = 0;
    virtual std::string display_name() const = 0;

public:
    //  After user selects a new video source on the UI, call this function to
    //  do some optional additional stuff, e.g. open a file selection dialog
    //  box to continue user interaction.
    virtual void run_post_select(){};
    virtual JsonValue to_json() const = 0;
    virtual void load_json(const JsonValue& json) = 0;

public:
    virtual std::unique_ptr<AudioSource> make_AudioSource(
        Logger& logger,
        const AudioInputChoice& format
    ) const = 0;
};









}
#endif
