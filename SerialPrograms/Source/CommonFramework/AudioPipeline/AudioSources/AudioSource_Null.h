/*  Audio Source (Null)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_AudioPipeline_AudioSource_Null_H
#define PokemonAutomation_AudioPipeline_AudioSource_Null_H

#include "CommonFramework/AudioPipeline/AudioSourceDescriptor.h"
#include "CommonFramework/AudioPipeline/IO/AudioSource.h"

namespace PokemonAutomation{


class AudioSourceDescriptor_Null : public AudioSourceDescriptor{
public:
    AudioSourceDescriptor_Null()
        : AudioSourceDescriptor(AudioSourceType::None)
    {}

    virtual bool operator==(const AudioSourceDescriptor& x) const override;
    virtual std::string display_name() const override;

    virtual JsonValue to_json() const override;
    virtual void load_json(const JsonValue& json) override;

    virtual std::unique_ptr<AudioSource> make_AudioSource(
        Logger& logger,
        const AudioInputChoice& format
    ) const override;
};







}
#endif
