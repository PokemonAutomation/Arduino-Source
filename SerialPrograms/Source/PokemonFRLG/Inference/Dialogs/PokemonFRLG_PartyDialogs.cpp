/*  Party Dialog Detectors
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonTools/Images/SolidColorTest.h"
#include "CommonTools/Images/ImageFilter.h"
#include "PokemonFRLG/PokemonFRLG_Settings.h"
#include "PokemonFRLG_PartyDialogs.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{


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


PartyLevelUpDetector::PartyLevelUpDetector(Color color, PartyLevelUpDialog dialog_type)
    : dialog_type(dialog_type)
    , m_border_top_box(0.619231, 0.023038, 0.362179, 0.001923) // gray (120, 115, 140)
    , m_border_right_box(0.982692, 0.025923, 0.001923, 0.597115)
    , m_dialog_top_box(0.626282, 0.042492, 0.341026, 0.006175)  // white
    , m_dialog_right_box(0.967949, 0.050923, 0.003846, 0.550962)
    , m_plus_box(0.862663, 0.051852, 0.034267, 0.553560)
{}
void PartyLevelUpDetector::make_overlays(VideoOverlaySet& items) const{
    const BoxOption& GAME_BOX = GameSettings::instance().GAME_BOX;
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_border_top_box));
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_border_right_box));
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_dialog_top_box));
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_dialog_right_box));
}
bool PartyLevelUpDetector::detect(const ImageViewRGB32& screen){
    ImageViewRGB32 game_screen = extract_box_reference(screen, GameSettings::instance().GAME_BOX);

    ImageViewRGB32 border_top_image = extract_box_reference(game_screen, m_border_top_box);
    ImageViewRGB32 border_right_image = extract_box_reference(game_screen, m_border_right_box);

    ImageViewRGB32 dialog_top_image = extract_box_reference(game_screen, m_dialog_top_box);
    ImageViewRGB32 dialog_right_image = extract_box_reference(game_screen, m_dialog_right_box);
   
    //Plus box is not solid white on the first screen, but is white on the second one
    ImageViewRGB32 plus_image = extract_box_reference(game_screen, m_plus_box);
    bool good_plus_image = (
        dialog_type == PartyLevelUpDialog::either
        || (dialog_type == PartyLevelUpDialog::plus && !is_white(plus_image))
        || (dialog_type == PartyLevelUpDialog::stats && is_white(plus_image))
    );

    if (is_solid(border_top_image, { 0.320, 0.307, 0.373 }, 0.25, 20)
        && is_solid(border_right_image, { 0.320, 0.307, 0.373 }, 0.25, 20)
        && is_white(dialog_top_image)
        && is_white(dialog_right_image)
        && good_plus_image
    ){
        return true;
    }
    return false;
}

PartyMoveLearnDetector::PartyMoveLearnDetector(Color color)
    : m_border_top_box(0.685096, 0.422836, 0.229327, 0.002163) // gray (120, 115, 140)
    , m_border_right_box(0.916346, 0.426442, 0.001442, 0.245913)
    , m_dialog_top_box(0.699038, 0.445913, 0.200000, 0.006490)  // white
    , m_dialog_right_box(0.899519, 0.454567, 0.004327, 0.194712)
{}
void PartyMoveLearnDetector::make_overlays(VideoOverlaySet& items) const{
    const BoxOption& GAME_BOX = GameSettings::instance().GAME_BOX;
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_border_top_box));
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_border_right_box));
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_dialog_top_box));
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_dialog_right_box));
}
bool PartyMoveLearnDetector::detect(const ImageViewRGB32& screen){
    ImageViewRGB32 game_screen = extract_box_reference(screen, GameSettings::instance().GAME_BOX);

    ImageViewRGB32 border_top_image = extract_box_reference(game_screen, m_border_top_box);
    ImageViewRGB32 border_right_image = extract_box_reference(game_screen, m_border_right_box);

    ImageViewRGB32 dialog_top_image = extract_box_reference(game_screen, m_dialog_top_box);
    ImageViewRGB32 dialog_right_image = extract_box_reference(game_screen, m_dialog_right_box);

    if (is_solid(border_top_image, { 0.320, 0.307, 0.373 }, 0.25, 20)
        && is_solid(border_right_image, { 0.320, 0.307, 0.373 }, 0.25, 20)
        && is_white(dialog_top_image)
        && is_white(dialog_right_image)
    ){
        return true;
    }
    return false;
}


}
}
}
