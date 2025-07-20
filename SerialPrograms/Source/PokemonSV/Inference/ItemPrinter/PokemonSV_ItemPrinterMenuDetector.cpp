/*  Main Menu Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonTools/Images/SolidColorTest.h"
#include "PokemonSV_ItemPrinterMenuDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


ItemPrinterMenuDetector::ItemPrinterMenuDetector(Color color)
    : m_color(color)
    , m_bottom(0.05, 0.945, 0.50, 0.04)
    , m_buttons(color, WhiteButton::ButtonX, {0.63, 0.93, 0.17, 0.06})
{}

void ItemPrinterMenuDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_bottom);
    m_buttons.make_overlays(items);
}
bool ItemPrinterMenuDetector::detect(const ImageViewRGB32& screen){
    ImageViewRGB32 bottom = extract_box_reference(screen, m_bottom);
    ImageStats bottom_stats = image_stats(bottom);
    if (!is_solid(bottom_stats, {0, 0.318898, 0.681102})){
        return false;
    }
    if (!m_buttons.detect(screen)){
        return false;
    }
    return true;
}


}
}
}
