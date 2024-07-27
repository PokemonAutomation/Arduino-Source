/*  Fast Travel Detector

 Detects the Fast Travel symbol on the screen. 

 Limitations:
 - Does not work if obstructed by the radar beam (while outdoors), 
 or if obstructed by the radar dot

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


class DirectionDetector : public StaticScreenDetector{
public:
    DirectionDetector(Color color = COLOR_RED, const ImageFloatBox& box = ImageFloatBox(0.815, 0.680, 0.180, 0.310));
    virtual ~DirectionDetector();

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) const override;

    // return the coordinates of the N symbol, where the coordinates are measured relative to the original image size
    std::pair<double, double> locate_north(const ImageViewRGB32& screen) const;

    std::vector<ImageFloatBox> north_candidate_locations(const ImageViewRGB32& screen) const;

    std::vector<ImageRGB32> north_candidate_images(const ImageViewRGB32& screen, const ImageFloatBox& box) const;

    // return the direction of the N symbol, in radians, using North-clockwise convention
    double current_direction(const ImageViewRGB32& screen) const;

    // given direction in radians (North-clockwise), change the direction 
    void change_direction(
        ConsoleHandle& console, 
        BotBaseContext& context,
        double direction
    ) const;

protected:
    Color m_color;
    ImageFloatBox m_minimap_box;
};



class DirectionWatcher : public VisualInferenceCallback{
public:
    DirectionWatcher(Color color, VideoOverlay& overlay, const ImageFloatBox& box = ImageFloatBox(0.815, 0.680, 0.180, 0.310));
    virtual ~DirectionWatcher();

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;


protected:
    VideoOverlay& m_overlay;
    DirectionDetector m_detector;
    FixedLimitVector<OverlayBoxScope> m_hits;
};




}
}
}
#endif
