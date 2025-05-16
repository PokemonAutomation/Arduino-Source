/*  Object Name Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonHome_HomeApplicationDetector_H
#define PokemonAutomation_PokemonHome_HomeApplicationDetector_H

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
namespace PokemonHome{


class HomeApplicationDetector : public StaticScreenDetector{
public:
    HomeApplicationDetector(Color color);
    virtual ~HomeApplicationDetector();
    
    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) const override;
    
    std::vector<ImageFloatBox> detect_all(const ImageViewRGB32& screen) const;
    
protected:
    Color m_color;
    ImageFloatBox m_box;
};



class HomeApplicationWatcher : public VisualInferenceCallback{
public:
    HomeApplicationWatcher(Color color);
    virtual ~HomeApplicationWatcher();
    
    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;
    
    
protected:
    HomeApplicationDetector m_detector;
    FixedLimitVector<OverlayBoxScope> m_hits;
};




}
}
}
#endif
