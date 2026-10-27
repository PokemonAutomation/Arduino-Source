/*  Audio Source (Null)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Json/JsonValue.h"
#include "AudioSource_Null.h"

namespace PokemonAutomation{


bool AudioSourceDescriptor_Null::operator==(const AudioSourceDescriptor& x) const{
    return typeid(*this) == typeid(x);
}
std::string AudioSourceDescriptor_Null::display_name() const{
    return "(none)";
}

JsonValue AudioSourceDescriptor_Null::to_json() const{
    return JsonValue();
}
void AudioSourceDescriptor_Null::load_json(const JsonValue& json){

}

std::unique_ptr<AudioSource> AudioSourceDescriptor_Null::make_AudioSource(
    Logger& logger,
    const AudioInputChoice& format
) const{
    return nullptr;
}







}
