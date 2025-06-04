/*  Video Overlay Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_VideoOverlayOption_H
#define PokemonAutomation_VideoOverlayOption_H

#include <string>
#include <atomic>

namespace PokemonAutomation{

class JsonValue;


// store data related to the checkerboxes on UI: whether user enabled box overlay,
// text overlay, etc.
class VideoOverlayOption{
    static const std::string JSON_BOXES;
    static const std::string JSON_TEXT;
    static const std::string JSON_IMAGES;
    static const std::string JSON_LOG;
    static const std::string JSON_STATS;

public:
    VideoOverlayOption();

    void load_json(const JsonValue& json);
    JsonValue to_json() const;

public:
    std::atomic<bool> boxes;  // whether user enabled box overlay
    std::atomic<bool> text;   // whether user enabled text overlay
    std::atomic<bool> images; // whether user enabled image overlay
    std::atomic<bool> log;    // whether user enabled log overlay
    std::atomic<bool> stats;  // whether user enabled stats overlay
};



}
#endif
