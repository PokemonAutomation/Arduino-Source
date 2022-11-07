/*  Video Overlay Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_VideoOverlayOption_H
#define PokemonAutomation_VideoOverlayOption_H

#include <string>
#include <atomic>

namespace PokemonAutomation{

class JsonValue;


class VideoOverlayOption{
    static const std::string JSON_BOXES;
    static const std::string JSON_TEXT;
    static const std::string JSON_LOG;
    static const std::string JSON_STATS;

public:
    VideoOverlayOption();

    void load_json(const JsonValue& json);
    JsonValue to_json() const;

public:
    std::atomic<bool> boxes;
    std::atomic<bool> text;
    std::atomic<bool> log;
    std::atomic<bool> stats;
};



}
#endif
