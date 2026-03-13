/*  Summary Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonTools/Images/SolidColorTest.h"
#include "CommonTools/Images/ImageFilter.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Images/SolidColorTest.h"
#include "CommonTools/Images/WaterfillUtilities.h"
#include "PokemonFRLG/PokemonFRLG_Settings.h"
#include "PokemonFRLG_SummaryDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{

SummaryDetector::SummaryDetector(Color color)
    : m_page_1_box(0.441235, 0.033869, 0.016889, 0.030754) //white
    , m_page_2_box(0.509681, 0.035206, 0.014223, 0.026742) //blue
    , m_page_3_box(0.577239, 0.039218, 0.012445, 0.021394) //blue
{}
void SummaryDetector::make_overlays(VideoOverlaySet& items) const{
    const BoxOption& GAME_BOX = GameSettings::instance().GAME_BOX;
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_page_1_box));
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_page_2_box));
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_page_3_box));
}
bool SummaryDetector::detect(const ImageViewRGB32& screen){
    ImageViewRGB32 game_screen = extract_box_reference(screen, GameSettings::instance().GAME_BOX);

    ImageViewRGB32 page_1_image = extract_box_reference(game_screen, m_page_1_box);
    ImageViewRGB32 page_2_image = extract_box_reference(game_screen, m_page_2_box);
    ImageViewRGB32 page_3_image = extract_box_reference(game_screen, m_page_3_box);
    if (is_white(page_1_image)
        && is_solid(page_2_image, { 0, 0.3333, 0.6666 }, 0.25, 20)
        && is_solid(page_3_image, { 0, 0.3333, 0.6666 }, 0.25, 20)
    ){
        return true;
    }
    return false;
}

SummaryPage2Detector::SummaryPage2Detector(Color color)
    : m_page_1_box(0.441235, 0.033869, 0.016889, 0.030754) //tan
    , m_page_2_box(0.509681, 0.035206, 0.014223, 0.026742) //white
    , m_page_3_box(0.577239, 0.039218, 0.012445, 0.021394) //blue
{}
void SummaryPage2Detector::make_overlays(VideoOverlaySet& items) const{
    const BoxOption& GAME_BOX = GameSettings::instance().GAME_BOX;
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_page_1_box));
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_page_2_box));
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_page_3_box));
}
bool SummaryPage2Detector::detect(const ImageViewRGB32& screen){
    ImageViewRGB32 game_screen = extract_box_reference(screen, GameSettings::instance().GAME_BOX);

    ImageViewRGB32 page_1_image = extract_box_reference(game_screen, m_page_1_box);
    ImageViewRGB32 page_2_image = extract_box_reference(game_screen, m_page_2_box);
    ImageViewRGB32 page_3_image = extract_box_reference(game_screen, m_page_3_box);
    if (is_solid(page_1_image, { 0.5117, 0.3763, 0.2294 }, 0.25, 20)
        && is_white(page_2_image)
        && is_solid(page_3_image, { 0, 0.3333, 0.6666 }, 0.25, 20)
    ){
        return true;
    }
    return false;
}

SummaryPage3Detector::SummaryPage3Detector(Color color)
    : m_page_1_box(0.441235, 0.033869, 0.016889, 0.030754) //tan
    , m_page_2_box(0.509681, 0.035206, 0.014223, 0.026742) //tan
    , m_page_3_box(0.577239, 0.039218, 0.012445, 0.021394) //white
{}
void SummaryPage3Detector::make_overlays(VideoOverlaySet& items) const{
    const BoxOption& GAME_BOX = GameSettings::instance().GAME_BOX;
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_page_1_box));
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_page_2_box));
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_page_3_box));
}
bool SummaryPage3Detector::detect(const ImageViewRGB32& screen){
    ImageViewRGB32 game_screen = extract_box_reference(screen, GameSettings::instance().GAME_BOX);

    ImageViewRGB32 page_1_image = extract_box_reference(game_screen, m_page_1_box);
    ImageViewRGB32 page_2_image = extract_box_reference(game_screen, m_page_2_box);
    ImageViewRGB32 page_3_image = extract_box_reference(game_screen, m_page_3_box);
    if (is_solid(page_1_image, { 0.5117, 0.3763, 0.2294 }, 0.25, 20)
        && is_solid(page_2_image, { 0.5117, 0.3763, 0.2294 }, 0.25, 20)
        && is_white(page_3_image)
    ){
        return true;
    }
    return false;
}


}
}
}
