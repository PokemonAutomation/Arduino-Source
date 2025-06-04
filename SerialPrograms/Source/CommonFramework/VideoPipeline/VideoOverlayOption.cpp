/*  Video Overlay Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Json/JsonObject.h"
#include "VideoOverlayOption.h"

namespace PokemonAutomation{



const std::string VideoOverlayOption::JSON_BOXES = "Boxes";
const std::string VideoOverlayOption::JSON_TEXT = "Text";
const std::string VideoOverlayOption::JSON_IMAGES = "Images";
const std::string VideoOverlayOption::JSON_LOG = "Log";
const std::string VideoOverlayOption::JSON_STATS = "Stats";


VideoOverlayOption::VideoOverlayOption()
    : boxes(true)
    , text(true)
    , log(false)
    , stats(true)
{}


void VideoOverlayOption::load_json(const JsonValue& json){
    const JsonObject* obj = json.to_object();
    if (obj == nullptr){
        return;
    }
    bool tmp;
    if (obj->read_boolean(tmp, JSON_BOXES)){
        boxes.store(tmp, std::memory_order_relaxed);
    }
    if (obj->read_boolean(tmp, JSON_TEXT)){
        text.store(tmp, std::memory_order_relaxed);
    }
    if (obj->read_boolean(tmp, JSON_IMAGES)){
        images.store(tmp, std::memory_order_relaxed);
    } else{
        // by default the image overlay is on
        images.store(true, std::memory_order_relaxed);
    }
    if (obj->read_boolean(tmp, JSON_LOG)){
        log.store(tmp, std::memory_order_relaxed);
    }
    if (obj->read_boolean(tmp, JSON_STATS)){
        stats.store(tmp, std::memory_order_relaxed);
    }
}
JsonValue VideoOverlayOption::to_json() const{
    JsonObject root;
    root[JSON_BOXES]  = boxes.load(std::memory_order_relaxed);
    root[JSON_TEXT]   = text.load(std::memory_order_relaxed);
    root[JSON_IMAGES] = images.load(std::memory_order_relaxed);
    root[JSON_LOG]    = log.load(std::memory_order_relaxed);
    root[JSON_STATS]  = stats.load(std::memory_order_relaxed);
    return root;
}




}
