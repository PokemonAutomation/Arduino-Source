/*  Fast Travel Detector

 Detects the Fast Travel symbol on the screen. 

 Limitations:
 - Does not work if obstructed by the radar beam (while outdoors), or if obstructed by the radar dot
 - Less reliable in areas where the map is blue. e.g. Blueberry academy.
 
 Ideas for further improvements:
 - Can try a two step waterfill+template match technique. Use waterfill algorithm to
 get all blue portions of the screen. From these sub-image candidates, run a second
 waterfill to select only white portions. Then compare to the template. Can use the same
 template, but needs to be cropped down slightly. Make sure the template still has a blue
 background. Need to be aware of false positives with the pokemon center, since it also
 has the same wing symbol.

 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_FastTravelDetector_H
#define PokemonAutomation_PokemonSV_FastTravelDetector_H

#include <vector>
#include "Common/Cpp/Color.h"
#include "Common/Cpp/Containers/FixedLimitVector.h"
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

// The area on the screen with the minimap
extern ImageFloatBox MINIMAP_AREA;

class FastTravelDetector : public StaticScreenDetector{
public:
    FastTravelDetector(Color color, const ImageFloatBox& box);
    virtual ~FastTravelDetector();

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) const override;

    std::vector<ImageFloatBox> detect_all(const ImageViewRGB32& screen) const;

protected:
    Color m_color;
    ImageFloatBox m_box;
};



class FastTravelWatcher : public VisualInferenceCallback{
public:
    FastTravelWatcher(Color color, VideoOverlay& overlay, const ImageFloatBox& box);
    virtual ~FastTravelWatcher();

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;


protected:
    VideoOverlay& m_overlay;
    FastTravelDetector m_detector;
    FixedLimitVector<OverlayBoxScope> m_hits;
};




}
}
}
#endif
