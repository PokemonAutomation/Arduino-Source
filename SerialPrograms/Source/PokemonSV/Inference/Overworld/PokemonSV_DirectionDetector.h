/*  Direction Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_DirectionDetector_H
#define PokemonAutomation_PokemonSV_DirectionDetector_H

#include <vector>
#include "Common/Cpp/Color.h"
#include "Common/Cpp/Containers/FixedLimitVector.h"
#include "ClientSource/Connection/BotBase.h"
#include "CommonFramework/Tools/ConsoleHandle.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonFramework/InferenceInfra/VisualInferenceCallback.h"
#include "CommonFramework/Inference/VisualDetector.h"

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

    // return the coordinates of the N symbol, where the coordinates are measured in absolute pixels (scaled to 1080/height)
    // with respect to the radar ball
    // return 0,0 if unable to locate the N symbol
    std::pair<double, double> locate_north(ConsoleHandle& console, const ImageViewRGB32& screen) const;

    // return the direction of the N symbol, in radians, using North-clockwise convention. [0, 2pi)
    // return -1 if unable to locate the N symbol
    double current_direction(ConsoleHandle& console, const ImageViewRGB32& screen) const;

    // given direction in radians (North-clockwise), rotate the camera so N is pointing in the desired direction.
    // mini-map must be unlocked.
    void change_direction(
        ConsoleHandle& console, 
        BotBaseContext& context,
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
