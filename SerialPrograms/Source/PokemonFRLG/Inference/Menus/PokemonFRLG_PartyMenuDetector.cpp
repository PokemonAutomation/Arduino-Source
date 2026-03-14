/*  Party Menu Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Images/SolidColorTest.h"
#include "CommonTools/Images/ImageFilter.h"
#include "CommonTools/Images/WaterfillUtilities.h"
#include "PokemonFRLG_PartyMenuDetector.h"


namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{


PartyMenuDetector::PartyMenuDetector(Color color)
    : m_dialog_top_box(0.028, 0.840, 0.705, 0.010)
    , m_page_background_box(0.028, 0.500, 0.010, 0.250)
{}
void PartyMenuDetector::make_overlays(VideoOverlaySet& items) const{
    const BoxOption& GAME_BOX = GameSettings::instance().GAME_BOX;
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_dialog_top_box));
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_page_background_box));
}
bool PartyMenuDetector::detect(const ImageViewRGB32& screen){
    ImageViewRGB32 game_screen = extract_box_reference(screen, GameSettings::instance().GAME_BOX);

    //Dialog is white
    ImageViewRGB32 dialog_top_image = extract_box_reference(game_screen, m_dialog_top_box);

    //Menu background is teal
    ImageViewRGB32 menu_background_image = extract_box_reference(game_screen, m_page_background_box);

    if (is_white(dialog_top_image)
        && is_solid(menu_background_image, { 0.020, 0.424, 0.412 }, 0.25, 20) //5, 108, 105 teal
    ){
        return true;
    }
    return false;
}

PartySelectionDetector::PartySelectionDetector(Color color)
    : m_dialog_right_box(0.955, 0.648, 0.010, 0.303)
    , m_page_background_box(0.028, 0.500, 0.010, 0.250)
{}
void PartySelectionDetector::make_overlays(VideoOverlaySet& items) const{
    const BoxOption& GAME_BOX = GameSettings::instance().GAME_BOX;
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_dialog_right_box));
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_page_background_box));
}
bool PartySelectionDetector::detect(const ImageViewRGB32& screen){
    ImageViewRGB32 game_screen = extract_box_reference(screen, GameSettings::instance().GAME_BOX);

    //Dialog is white
    ImageViewRGB32 dialog_right_image = extract_box_reference(game_screen, m_dialog_right_box);

    //Menu background is teal
    ImageViewRGB32 menu_background_image = extract_box_reference(game_screen, m_page_background_box);

    if (is_white(dialog_right_image)
        && is_solid(menu_background_image, { 0.020, 0.424, 0.412 }, 0.25, 20) //5, 108, 105 teal
    ){
        return true;
    }
    return false;
}


}
}
}
