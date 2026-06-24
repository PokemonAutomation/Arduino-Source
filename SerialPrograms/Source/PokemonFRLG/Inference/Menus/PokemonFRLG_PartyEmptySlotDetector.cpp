/*  Party Empty Slot Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "CommonTools/Images/SolidColorTest.h"
#include "PokemonFRLG/PokemonFRLG_Settings.h"
#include "PokemonFRLG_PartyEmptySlotDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{

ImageFloatBox PartyEmptySlotDetector::box_for_slot(PartySlot slot){
    switch (slot){
    case PartySlot::ONE:
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Invalid FRLG Party Empty Slot. Slot 1 should always be occupied.");
    case PartySlot::TWO:
        return ImageFloatBox(0.709, 0.078, 0.01, 0.018);
    case PartySlot::THREE:
        return ImageFloatBox(0.709, 0.228, 0.01, 0.018);
    case PartySlot::FOUR:
        return ImageFloatBox(0.709, 0.378, 0.01, 0.018);
    case PartySlot::FIVE:
        return ImageFloatBox(0.709, 0.527, 0.01, 0.018);
    case PartySlot::SIX:
        return ImageFloatBox(0.709, 0.677, 0.01, 0.018);
    default:
        break;
    }
    throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Invalid FRLG Party Empty Slot.");
}

PartyEmptySlotDetector::PartyEmptySlotDetector(
    Color color,
    PartySlot slot
)
    : m_color(color)
    , m_box(box_for_slot(slot))
{}
void PartyEmptySlotDetector::make_overlays(VideoOverlaySet& items) const{
    const BoxOption& GAME_BOX = GameSettings::instance().GAME_BOX;
    items.add(m_color, GAME_BOX.inner_to_outer(m_box));
}
bool PartyEmptySlotDetector::detect(const ImageViewRGB32& screen){
    const BoxOption& GAME_BOX = GameSettings::instance().GAME_BOX;
    ImageViewRGB32 game_screen = extract_box_reference(screen, GAME_BOX);
    const auto stats = image_stats(extract_box_reference(game_screen, m_box));
    return stats.stddev.sum() > 20.0;
}
}
}
}