/*  Null Video Source
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "VideoSource.h"
#include "VideoSource_Null.h"

namespace PokemonAutomation{


bool VideoSourceDescriptor_Null::operator==(const VideoSourceDescriptor& x) const{
    return typeid(*this) == typeid(x);
}
std::string VideoSourceDescriptor_Null::display_name() const{
    return "(none)";
}

void VideoSourceDescriptor_Null::load_json(const JsonValue& json){

}
JsonValue VideoSourceDescriptor_Null::to_json() const{
    return JsonValue();
}

std::unique_ptr<VideoSource> VideoSourceDescriptor_Null::make_VideoSource(Logger& logger, Resolution resolution) const{
    return nullptr;
}


}
