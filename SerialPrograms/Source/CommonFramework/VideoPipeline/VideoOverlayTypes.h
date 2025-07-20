/*  Video Overlay Types
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_VideoOverlayTypes_H
#define PokemonAutomation_VideoOverlayTypes_H

#include <string>
#include "Common/Cpp/Color.h"
#include "Common/Cpp/Time.h"
#include "Common/Cpp/Containers/Pimpl.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"

namespace PokemonAutomation{

// A bounding box as part of the video overlay
// Check CommonFramework/VideoPipeline/VideoOverlay.h to see how it's used.
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
// Check CommonFramework/VideoPipeline/VideoOverlay.h to see how it's used.
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

// An image as part of the video overlay.
// Allow transparency to create mask overlay.
// Check CommonFramework/VideoPipeline/VideoOverlay.h to see how it's used.
// e.g. OverlayImage(image, {x=0.5, y=0.5, width=0.5, height=0.5}) will render
// an overlay image on the lower right quadrant of the view.
// Note: for efficiency, the image stored is just a pointer. Be careful not to
// modify the image data async or release it before the rendering code calls it.
struct OverlayImage{
    // Image view. The image data must live longer than the OverlayImage.
    // Its alpha channel will be used during rendering.
    ImageViewRGB32 image;
    // starting x coordinate of the image in the video window, range: 0.0-1.0.
    // starting y coordinate of the image in the video window, range: 0.0-1.0.
    // relative width of the image in the video window, range: 0.0-1.0
    // relative height of the image in the video window, range: 0.0-1.0
    ImageFloatBox box;
    
    OverlayImage(
        ImageViewRGB32 image, const ImageFloatBox& box
    )
        : image(image)
        , box(box)
    {}
};


// A log line to show as part of overlay.
// Check CommonFramework/VideoPipeline/VideoOverlay.h to see how it's used.
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
    std::string text;
    Color color = COLOR_WHITE;
};

class OverlayStat{
public:
    OverlayStat(const OverlayStat&) = delete;
    void operator=(const OverlayStat&) = delete;

public:
    virtual ~OverlayStat() = default;
    OverlayStat() = default;

    virtual OverlayStatSnapshot get_current() = 0;
};


class OverlayStatUtilizationPrinter{
public:
    OverlayStatUtilizationPrinter(double max_utilization = 1.0);
    OverlayStatSnapshot get_snapshot(const std::string& label, double utilization);

private:
    double m_max_utilization;
    WallClock m_last_active;
};





}
#endif
