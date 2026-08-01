/*  Party Menu Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Images/SolidColorTest.h"
#include "CommonTools/Images/ImageFilter.h"
#include "CommonTools/Images/WaterfillUtilities.h"
#include "PokemonRSE_PartyMenuDetector.h"


namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonRSE{


PartyMenuDetector::PartyMenuDetector(Color color)
    : m_dialog_box(0.706471, 0.85002, 0.023937, 0.096045) //Choose a Pokemon - position works for all games
    , m_background_box_e(0.015847, 0.591232, 0.038122, 0.133396) //solid color in the background for emerald, similar to FRLG
    , m_top_bar_rs(0.028666, 0.035895, 0.942495, 0.004067) //both rs have a red bar on top (was this intended for s?)
{}
void PartyMenuDetector::make_overlays(VideoOverlaySet& items) const{
    const BoxOption& GAME_BOX = GameSettings::instance().GAME_BOX;
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_dialog_box));
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_background_box_e));
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_top_bar_rs));
}
bool PartyMenuDetector::detect(const ImageViewRGB32& screen){
    ImageViewRGB32 game_screen = extract_box_reference(screen, GameSettings::instance().GAME_BOX);

    //Dialog is white
    ImageViewRGB32 dialog_image = extract_box_reference(game_screen, m_dialog_box);

    //Menu background is dark yellow/gold-ish
    ImageViewRGB32 background_image = extract_box_reference(game_screen, m_background_box_e);

    //The top bar is red
    ImageViewRGB32 top_bar_image = extract_box_reference(game_screen, m_top_bar_rs);

    if ((is_white(dialog_image) //Ruby and Sapphire, all languages
        && is_solid(top_bar_image, { 0.71282, 0.16923, 0.1179487 }, 0.25, 20)) //139, 33, 23
        ||
        (is_white(dialog_image) //Emerald, all languages
        && is_solid(background_image, { 0.41566, 0.46084, 0.12349 }, 0.25, 20)) //138, 153, 41
    ){
        return true;
    }
    return false;
}



ImageFloatBox PartySlotDetector::party_slot_boxes(PartySlot position){
    //These positions should work for RSE all languages
    switch (position){
    case PartySlot::ONE:
        return ImageFloatBox(0.35185, 0.271083, 0.003546, 0.138731);
    case PartySlot::TWO:
        return ImageFloatBox(0.987, 0.085, 0.003, 0.09966);
    case PartySlot::THREE:
        return ImageFloatBox(0.987, 0.235, 0.003, 0.09966);
    case PartySlot::FOUR:
        return ImageFloatBox(0.987, 0.385, 0.003, 0.09966);
    case PartySlot::FIVE:
        return ImageFloatBox(0.987, 0.535, 0.003, 0.09966);
    case PartySlot::SIX:
        return ImageFloatBox(0.987, 0.685, 0.003, 0.09966);
    default:
        break;
    }
    throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Invalid RSE Party Slot Position");
}
PartySlotDetector::PartySlotDetector(
    Color color,
    const ImageFloatBox& box
)
    : m_color(color)
    , m_party_box(box)
{}
PartySlotDetector::PartySlotDetector(
    Color color, 
    PartySlot position
)
    : m_color(color)
    , m_party_box(party_slot_boxes(position))
{}
void PartySlotDetector::make_overlays(VideoOverlaySet& items) const{
    const BoxOption& GAME_BOX = GameSettings::instance().GAME_BOX;
    items.add(m_color, GAME_BOX.inner_to_outer(m_party_box));
}
bool PartySlotDetector::detect(const ImageViewRGB32& screen){
    ImageViewRGB32 game_screen = extract_box_reference(screen, GameSettings::instance().GAME_BOX);

    ImageViewRGB32 target_box_party = extract_box_reference(game_screen, m_party_box);

    //orange border FD7132 for emerald
    //FD7132 AND EB712D for rs, the background is faintly striped, so it may require a wider range?
    //FD7132 - 253, 113, 50
    if (is_solid(target_box_party, { 0.608173, 0.2716346, 0.1201923 }, 0.25, 20)
    ){
        return true;
    }
    return false;
}



}
}
}
