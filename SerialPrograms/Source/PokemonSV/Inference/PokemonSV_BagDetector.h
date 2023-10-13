/*  Bag Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_BagDetector_H
#define PokemonAutomation_PokemonSV_BagDetector_H

#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/Inference/VisualDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{



class BagDetector : public StaticScreenDetector{
public:
    BagDetector(Color color = COLOR_RED);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) const override;

    bool is_shiny(const ImageViewRGB32& screen) const;


protected:
    Color m_color;
    ImageFloatBox m_top_blue_left;
    ImageFloatBox m_top_blue_right;
    ImageFloatBox m_bottom;
};
class BagWatcher : public DetectorToFinder<BagDetector>{
public:
    BagWatcher(FinderType type, Color color = COLOR_RED)
         : DetectorToFinder("PokemonSummaryWatcher", type, std::chrono::milliseconds(250), color)
    {}
};



}
}
}
#endif
