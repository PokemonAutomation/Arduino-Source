/*  Fast Travel Detector

 Detects the Fast Travel symbol on the screen. 

 Limitations:
 - Does not work if obstructed by the radar beam (while outdoors), 
 or if obstructed by the radar dot

 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_FastTravelDetector_H
#define PokemonAutomation_PokemonSV_FastTravelDetector_H

#include <vector>
#include "Common/Cpp/Color.h"
#include "Common/Cpp/Containers/FixedLimitVector.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"
#include "CommonTools/VisualDetector.h"

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
    virtual bool detect(const ImageViewRGB32& screen) override;

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
