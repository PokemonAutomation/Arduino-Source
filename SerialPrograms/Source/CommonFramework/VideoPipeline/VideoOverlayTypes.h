/*  Video Overlay Types
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_VideoOverlayTypes_H
#define PokemonAutomation_VideoOverlayTypes_H

#include <string>
#include "Common/Cpp/Color.h"
#include "Common/Cpp/Containers/Pimpl.h"

namespace PokemonAutomation{


// A text as part of the video overlay.
struct OverlayText{
    // Text content.
    // Note overlay cannot handle newline character "\n".
    std::string message;
    // x coordinate of the text start, range: 0.0-1.0.
    double x;
    // y coordinate of the text start, range: 0.0-1.0.
    double y;
    // Font point size. This value is relative to the video overlay widget height. So you can
    // set it without considering overlay widget resolution.
    // Value of 4.0 gives a large, comfortable font size while not too large to clutter the screen.
    double font_size;
    // Text color.
    Color color;

public:
    OverlayText(const OverlayText& other) = default;
    OverlayText(OverlayText&& other) = default;

    OverlayText(std::string message,
        double x,
        double y,
        double font_size = 1.0,
        Color color = COLOR_BLUE
    )
        : message(std::move(message))
        , x(x)
        , y(y)
        , font_size(font_size)
        , color(color)
    {}

};




struct OverlayStatSnapshot{
    std::string text = "---";
    Color color = COLOR_WHITE;
};

class OverlayStat{
public:
    OverlayStat(const OverlayStat&) = delete;
    void operator=(const OverlayStat&) = delete;

public:
    virtual ~OverlayStat() = default;
    OverlayStat() = default;

    virtual OverlayStatSnapshot get_current() const = 0;
};






}
#endif
