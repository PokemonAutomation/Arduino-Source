/*  Tutorial Window Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/GlobalAutoPaths.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/ImageMatch/ExactImageMatcher.h"
#include "CommonTools/Images/SolidColorTest.h"
#include "PokemonLZA_TutorialWindowDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{

TutorialWindowDetector::TutorialWindowDetector(Color color)
    : m_color(color)
    , m_book(523.0 / 1920, 104.0 / 1080, 33.0 / 1920, 39.0 / 1080)
    , m_top_border(0.29, 0.165, 0.42, 0.012)
    , m_left_border(0.265, 0.18, 0.018, 0.68)
{}

void TutorialWindowDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_book);
    items.add(m_color, m_top_border);
    items.add(m_color, m_left_border);
}

bool TutorialWindowDetector::detect(const ImageViewRGB32& screen){
    if (!is_white(extract_box_reference(screen, m_top_border), 600, 20)
        || !is_white(extract_box_reference(screen, m_left_border), 600, 20)){
        return false;
    }
    // Only the fixed book badge and its green backing are in the template
    // Tutorial text, illustrations, and the game behind the window can vary
    static const ImageMatch::ExactImageMatcher book(RESOURCE_PATH() + "PokemonLZA/TutorialBook.png");
    return book.rmsd(extract_box_reference(screen, m_book)) <= 45;
}

}
}
}
