/*  Party Empty Slot Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonTools/Images/SolidColorTest.h"
#include "Tests/ImageUnitTest.h"
#include "VideoGames/PokemonFRLG/PokemonFRLG_Settings.h"
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


class Test_PartyEmptySlotDetector : public ImageUnitTest{
public:
    Test_PartyEmptySlotDetector(const std::string& image, PartySlot slot, bool expected)
        : ImageUnitTest("PokemonFRLG::PartyEmptySlotDetector - " + image + " - slot " + std::to_string(static_cast<unsigned>(slot) + 1), {image})
        , m_slot(slot)
        , m_expected(expected)
    {}

    virtual UnitTestResult run_on_images(
        Logger&, CancellableScope&,
        const std::vector<ImageRGB32>& images
    ) const override{
        PartyEmptySlotDetector detector(COLOR_RED, m_slot);
        const ImageRGB32& image = images[0];
        bool result = detector.detect(image);
        if (result != m_expected){
            return std::string("Expected empty slot = ") + (m_expected ? "true" : "false") +
                ", got " + (result ? "true" : "false");
        }
        return true;
    }

private:
    PartySlot m_slot;
    bool m_expected;
};

void add_tests_PartyEmptySlotDetector(UnitTestDatabase& database){
    // Slot 1 is always occupied and is not supported by PartyEmptySlotDetector.
    for (unsigned c = 1; c < 6; c++){
        PartySlot slot = static_cast<PartySlot>(c);
        for (unsigned occupied = 1; occupied <= 6; occupied++){
            database.add<Test_PartyEmptySlotDetector>(
                "PokemonFRLG/Party/Party_" + std::to_string(occupied) + ".png", slot, c >= occupied
            );
        }
        database.add<Test_PartyEmptySlotDetector>("PokemonFRLG/Party/JPN_HeldItem.png", slot, false);
        database.add<Test_PartyEmptySlotDetector>("PokemonFRLG/Party/JPN_Party.png", slot, c >= 4);
        database.add<Test_PartyEmptySlotDetector>("PokemonFRLG/Party/Party_6_With_Held_Items.png", slot, false);
        database.add<Test_PartyEmptySlotDetector>("PokemonFRLG/Party/SPA_HeldItem.png", slot, false);
    }
}

}
}
}