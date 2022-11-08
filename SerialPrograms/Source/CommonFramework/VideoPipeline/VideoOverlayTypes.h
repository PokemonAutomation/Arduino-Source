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
#include "CommonFramework/ImageTools/ImageBoxes.h"

namespace PokemonAutomation{


struct OverlayBox{
    Color color;
    ImageFloatBox box;
    std::string label;

    OverlayBox(Color p_color, const ImageFloatBox& p_box, std::string p_label)
        : color(p_color)
        , box(p_box)
        , label(std::move(p_label))
    {}
    operator const ImageFloatBox&() const{ return box; }
    operator ImageFloatBox&(){ return box; }
};



// A text as part of the video overlay.
struct OverlayText{
    // Text color.
    Color color;
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

    OverlayText(
        Color color,
        std::string message,
        double x, double y, double font_size
    )
        : color(color)
        , message(std::move(message))
        , x(x), y(y)
        , font_size(font_size)
    {}

};



struct OverlayLogLine{
    Color color;
    std::string message;

    OverlayLogLine(
        Color color,
        std::string message
    )
        : color(color)
        , message(std::move(message))
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
