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
#include "Tests/ImageUnitTest.h"
#include "VideoGames/PokemonFRLG/PokemonFRLG_Settings.h"
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
    if (is_solid(target_box_party, { 0.6455696, 0.2835, 0.070886 }, 0.25, 30)
    ){
        return true;
    }
    return false;
}


class Test_PartyMenuDetector : public ImageUnitTest{
public:
    Test_PartyMenuDetector(const std::string& image, bool expected)
        : ImageUnitTest("PokemonFRLG::PartyMenuDetector - " + image, {image})
        , m_expected(expected)
    {}

    virtual UnitTestResult run_on_images(
        Logger&, CancellableScope&,
        const std::vector<ImageRGB32>& images
    ) const override{
        PartyMenuDetector detector(COLOR_RED);
        const ImageRGB32& image = images[0];
        bool result = detector.detect(image);
        if (result != m_expected){
            return std::string("Expected party menu = ") + (m_expected ? "true" : "false") +
                ", got " + (result ? "true" : "false");
        }
        return true;
    }

private:
    bool m_expected;
};

void add_tests_PartyMenuDetector(UnitTestDatabase& database){
    database.add<Test_PartyMenuDetector>("PokemonFRLG/Party/Party_1.png", true);
    database.add<Test_PartyMenuDetector>("PokemonFRLG/Party/Party_2.png", true);
    database.add<Test_PartyMenuDetector>("PokemonFRLG/Party/Party_3.png", true);
    database.add<Test_PartyMenuDetector>("PokemonFRLG/Party/Party_4.png", true);
    database.add<Test_PartyMenuDetector>("PokemonFRLG/Party/Party_5.png", true);
    database.add<Test_PartyMenuDetector>("PokemonFRLG/Party/Party_6.png", true);
    database.add<Test_PartyMenuDetector>("PokemonFRLG/Party/JPN_HeldItem.png", true);
    database.add<Test_PartyMenuDetector>("PokemonFRLG/Party/Party_6_With_Held_Items.png", true);

    // A dialog covers the normal detection region in these images.
    database.add<Test_PartyMenuDetector>("PokemonFRLG/Party/JPN_Party.png", false);
    database.add<Test_PartyMenuDetector>("PokemonFRLG/Party/SPA_HeldItem.png", false);
}


class Test_PartySlotDetector : public ImageUnitTest{
public:
    Test_PartySlotDetector(
        const std::string& image,
        bool expected
    )
        : ImageUnitTest("PokemonFRLG::PartySlotDetector - " + image, {image})
        , m_expected(expected)
    {}

    virtual UnitTestResult run_on_images(
        Logger& logger, CancellableScope& scope,
        const std::vector<ImageRGB32>& images
    ) const override{
        PartySlotWatcher detector(COLOR_RED, PartySlot::SIX);
        const ImageRGB32& image = images[0];
        return detector.detect(image) == m_expected;
    };

private:
    bool m_expected;
};

void add_tests_PartySlotDetector(UnitTestDatabase& database){
    database.add<Test_PartySlotDetector>("PokemonFRLG/PartySlotDetector/elgato-PartySlotDetector_True.png", true);
    database.add<Test_PartySlotDetector>("PokemonFRLG/PartySlotDetector/SirGArilla-PartySlotDetector_True.png", true);
    database.add<Test_PartySlotDetector>("PokemonFRLG/PartySlotDetector/elgato-EmptySlot_False.png", false);
    database.add<Test_PartySlotDetector>("PokemonFRLG/PartySlotDetector/elgato-PartySlotDetector_False.png", false);
}

}
}
}
