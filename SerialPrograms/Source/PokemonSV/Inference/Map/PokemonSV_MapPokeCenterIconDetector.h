/*  Map PokeCenter Icon Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_MapPokeCenterIconDetector_H
#define PokemonAutomation_PokemonSV_MapPokeCenterIconDetector_H

#include <vector>
#include "Common/Cpp/Color.h"
#include "Common/Cpp/Containers/FixedLimitVector.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"
#include "CommonTools/VisualDetector.h"

namespace PokemonAutomation{

class VideoOverlaySet;
class VideoOverlay;
class OverlayBoxScope;

namespace NintendoSwitch{
namespace PokemonSV{

// Detect one or more pokecenter icons on map
class MapPokeCenterIconDetector : public StaticScreenDetector{
public:
    MapPokeCenterIconDetector(Color color, const ImageFloatBox& box);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

    std::vector<ImageFloatBox> detect_all(const ImageViewRGB32& screen) const;

protected:
    Color m_color;
    ImageFloatBox m_box;
};


// Watch for at least one pokecenter icon appearing on map
class MapPokeCenterIconWatcher : public VisualInferenceCallback{
public:
    ~MapPokeCenterIconWatcher();
    MapPokeCenterIconWatcher(Color color, VideoOverlay& overlay, const ImageFloatBox& box);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;

    const std::vector<ImageFloatBox>& found_locations() const { return m_hits; }


protected:
    VideoOverlay& m_overlay;
    MapPokeCenterIconDetector m_detector;
    std::vector<ImageFloatBox> m_hits;
    FixedLimitVector<OverlayBoxScope> m_hit_boxes;
    
};




}
}
}
#endif
