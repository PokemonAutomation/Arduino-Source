/*  Post-Catch Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Images/SolidColorTest.h"
#include "PokemonSV_PostCatchDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{



AddToPartyDetector::AddToPartyDetector(Color color)
    : m_color(color)
    , m_top_left(0.35, 0.12, 0.10, 0.05)
    , m_top_right(0.55, 0.12, 0.10, 0.05)
    , m_bottom_left(0.15, 0.83, 0.08, 0.05)
    , m_bottom_right(0.77, 0.83, 0.08, 0.05)
    , m_dialog(color)
{}
void AddToPartyDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_top_left);
    items.add(m_color, m_top_right);
    items.add(m_color, m_bottom_left);
    items.add(m_color, m_bottom_right);
    m_dialog.make_overlays(items);
}
bool AddToPartyDetector::detect(const ImageViewRGB32& screen){
    ImageStats top_left = image_stats(extract_box_reference(screen, m_top_left));
//    cout << top_left.average << top_left.stddev << endl;
    if (!is_white(top_left)){
        return false;
    }

    ImageStats top_right = image_stats(extract_box_reference(screen, m_top_right));
//    cout << top_right.average << top_right.stddev << endl;
    if (!is_white(top_right)){
        return false;
    }

    ImageStats bottom_left = image_stats(extract_box_reference(screen, m_bottom_left));
//    cout << top_left.average << top_left.stddev << endl;
    if (!is_white(bottom_left)){
        return false;
    }

    ImageStats bottom_right = image_stats(extract_box_reference(screen, m_bottom_right));
//    cout << top_right.average << top_right.stddev << endl;
    if (!is_white(bottom_right)){
        return false;
    }

    return m_dialog.detect(screen);
}







}
}
}
