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


ImageFloatBox PartySlotDetector::party_slot_boxes(PartySlot position){
    switch (position){
    case PartySlot::ONE:
        return ImageFloatBox(0.035000, 0.376617, 0.003280, 0.070000);
    case PartySlot::TWO:
        return ImageFloatBox(0.985000, 0.080000, 0.003280, 0.099663);
    case PartySlot::THREE:
        return ImageFloatBox(0.985000, 0.230000, 0.003280, 0.099663);
    case PartySlot::FOUR:
        return ImageFloatBox(0.985000, 0.380000, 0.003280, 0.099663);
    case PartySlot::FIVE:
        return ImageFloatBox(0.985000, 0.530000, 0.003280, 0.099663);
    case PartySlot::SIX:
        return ImageFloatBox(0.985000, 0.675000, 0.003280, 0.099663);
    default:
        break;
    }
    throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Invalid FRLG Party Slot Position");
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

    //orange FF701C border. light/dark blues in the selected box are close to each other.
    if (is_solid(target_box_party, { 0.6455696, 0.2835, 0.070886 }, 0.25, 20)
    ){
        return true;
    }
    return false;
}



}
}
}
