/*  Box Detection
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/ImageTools/SolidColorTest.h"
#include "PokemonSV_BoxDetection.h"

#include <iostream>
using std::cout;
using std::endl;;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


SomethingInBoxSlotDetector::SomethingInBoxSlotDetector(bool true_if_exists, Color color)
    : m_true_if_exists(true_if_exists)
    , m_color(color)
    , m_exists(0.985, 0.010, 0.010, 0.050)
{}
void SomethingInBoxSlotDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_exists);
}
bool SomethingInBoxSlotDetector::detect(const ImageViewRGB32& screen) const{
    ImageStats stats = image_stats(extract_box_reference(screen, m_exists));
    bool exists = is_solid(stats, {0.533473, 0.466527, 0.0});
//    cout << "exists = " << exists << endl;
    return exists == m_true_if_exists;
}




BoxSelectDetector::BoxSelectDetector(Color color)
    : m_color(color)
    , m_exists(true, color)
    , m_dialog(color)
    , m_gradient(GradientArrowType::RIGHT, {0.20, 0.17, 0.50, 0.10}, color)
{}
void BoxSelectDetector::make_overlays(VideoOverlaySet& items) const{
    m_exists.make_overlays(items);
    m_dialog.make_overlays(items);
    m_gradient.make_overlays(items);
}
bool BoxSelectDetector::exists(const ImageViewRGB32& screen) const{
    return m_exists.detect(screen);
}
bool BoxSelectDetector::detect(const ImageViewRGB32& screen) const{
    if (!exists(screen)){
        return false;
    }
    if (!m_dialog.detect(screen)){
        return false;
    }
    if (!m_gradient.detect(screen)){
        return false;
    }
    return true;
}



}
}
}
