/*  Main Menu Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_ItemPrinterMenuDetector_H
#define PokemonAutomation_PokemonSV_ItemPrinterMenuDetector_H

#include "CommonTools/VisualDetector.h"
#include "PokemonSV/Inference/PokemonSV_WhiteButtonDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{



class ItemPrinterMenuDetector : public StaticScreenDetector{
public:
    ItemPrinterMenuDetector(Color color);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;


private:
    Color m_color;
    ImageFloatBox m_bottom;
    WhiteButtonWatcher m_buttons;
};
class ItemPrinterMenuWatcher : public DetectorToFinder<ItemPrinterMenuDetector>{
public:
    ItemPrinterMenuWatcher(Color color)
         : DetectorToFinder("ItemPrinterMenuWatcher", std::chrono::milliseconds(250), color)
    {}
};




}
}
}
#endif
