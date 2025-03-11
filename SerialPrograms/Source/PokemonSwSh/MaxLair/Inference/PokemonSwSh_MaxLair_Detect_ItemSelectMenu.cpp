/*  Max Lair Detect Item Select Menu
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Images/SolidColorTest.h"
#include "PokemonSwSh_MaxLair_Detect_ItemSelectMenu.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


ItemSelectDetector::ItemSelectDetector(bool stop_no_detect)
    : VisualInferenceCallback("ItemSelectDetector")
    , m_stop_on_no_detect(stop_no_detect)
    , m_bottom_main(0.100, 0.970, 0.600, 0.020)
    , m_bottom_right(0.920, 0.970, 0.070, 0.020)
    , m_blue(0.600, 0.020, 0.200, 0.060)
{}
void ItemSelectDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(COLOR_BLUE, m_bottom_main);
    items.add(COLOR_BLUE, m_bottom_right);
    items.add(COLOR_BLUE, m_blue);
}
bool ItemSelectDetector::detect(const ImageViewRGB32& screen) const{
    ImageStats bottom_main = image_stats(extract_box_reference(screen, m_bottom_main));
//    cout << bottom_main.average << ", " << bottom_main.stddev << endl;
    if (!is_black(bottom_main)){
        return false;
    }
    ImageStats bottom_right = image_stats(extract_box_reference(screen, m_bottom_right));
//    cout << bottom_right.average << ", " << bottom_right.stddev << endl;
    if (bottom_right.stddev.sum() < 30){
        return false;
    }
    ImageStats blue = image_stats(extract_box_reference(screen, m_blue));
//    cout << blue.average << ", " << blue.stddev << endl;
    if (!is_solid(blue, {0.0286572, 0.40799, 0.563353})){
        return false;
    }
    return true;
}
bool ItemSelectDetector::process_frame(const ImageViewRGB32& frame, WallClock timestamp){
    return m_stop_on_no_detect
        ? !detect(frame)
        : detect(frame);
}


}
}
}
}
