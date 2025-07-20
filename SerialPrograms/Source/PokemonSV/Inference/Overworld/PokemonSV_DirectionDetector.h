/*  Direction Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_DirectionDetector_H
#define PokemonAutomation_PokemonSV_DirectionDetector_H

#include "Common/Cpp/Color.h"
#include "CommonFramework/Notifications/ProgramInfo.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/Tools/VideoStream.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"
#include "CommonTools/VisualDetector.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_ProController.h"

namespace PokemonAutomation{

class VideoOverlaySet;
class VideoOverlay;
class OverlayBoxScope;

namespace NintendoSwitch{
namespace PokemonSV{

static constexpr double PI = 3.14159265358979323846;

class DirectionDetector {
public:
    DirectionDetector(Color color = COLOR_RED, const ImageFloatBox& box = ImageFloatBox(0.815, 0.680, 0.180, 0.310));
    virtual ~DirectionDetector();

    bool detect_north(Logger& logger, const ImageViewRGB32& screen) const;

    // return the coordinates of the N symbol, where the coordinates are measured in absolute pixels (scaled to 1080/height)
    // with respect to the radar ball
    // return 0,0 if unable to locate the N symbol
    std::pair<double, double> locate_north(Logger& logger, const ImageViewRGB32& screen) const;

    // return the direction of the N symbol, in radians, using North-clockwise convention. [0, 2pi)
    // return -1 if unable to locate the N symbol
    double get_current_direction(VideoStream& stream, const ImageViewRGB32& screen) const;

    bool is_minimap_possibly_locked(double current_direction) const;

    bool is_minimap_definitely_locked(VideoStream& stream, ProControllerContext& context, double current_direction) const;

    // given direction in radians (North-clockwise), rotate the camera so N is pointing in the desired direction.
    // mini-map must be unlocked.
    void change_direction(
        const ProgramInfo& info,
        VideoStream& stream,
        ProControllerContext& context,
        double direction
    ) const;

protected:
    Color m_color;
    ImageFloatBox m_minimap_box;
};



}
}
}
#endif
