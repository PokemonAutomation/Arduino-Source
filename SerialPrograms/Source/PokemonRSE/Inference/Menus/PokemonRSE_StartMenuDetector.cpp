/*  Start Menu Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Color.h"
#include "Common/Cpp/Exceptions.h"
#include "CommonTools/Images/SolidColorTest.h"
#include "CommonFramework/GlobalAutoPaths.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "PokemonRSE/PokemonRSE_Settings.h"
#include "PokemonRSE_StartMenuDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonRSE{


static double menu_bottom_box_y(StartMenuState state){
    switch (state) {
    case StartMenuState::FULL:
        return 0.94;
    case StartMenuState::PRE_POKENAV:
    case StartMenuState::SAFARI:
        return 0.84;
    case StartMenuState::PRE_POKEDEX:
        return 0.74;
    case StartMenuState::PRE_POKEMON:
        return 0.64;
    default:
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Invalid StartMenuState");
    }
}

StartMenuDetector::StartMenuDetector(Color color, StartMenuState state)
    : m_menu_top_box(0.766756, 0.046978, 0.197701, 0.01334)
    , m_menu_bottom_box(0.766756, menu_bottom_box_y(state), 0.197701, 0.01334)
{}
void StartMenuDetector::make_overlays(VideoOverlaySet& items) const{
    const BoxOption& GAME_BOX = GameSettings::instance().GAME_BOX;
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_menu_top_box));
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_menu_bottom_box));
}
bool StartMenuDetector::detect(const ImageViewRGB32& screen){
    ImageViewRGB32 game_screen = extract_box_reference(screen, GameSettings::instance().GAME_BOX);

    ImageViewRGB32 menu_top_image = extract_box_reference(game_screen, m_menu_top_box);
    ImageViewRGB32 menu_bottom_image = extract_box_reference(game_screen, m_menu_bottom_box);

    return (is_white(menu_top_image)
        && is_white(menu_bottom_image)
    );
}


// Row of each option from the top of the menu, per state; -1 = not shown
// column = (int)StartMenuSlot, row = (int)StartMenuState.
//                       DEX  MON  BAG  NAV  TRN  SAV  OPT  EXT  RET
constexpr int8_t START_MENU_ROWS[5][9] = {
    /* PRE_POKEMON */ {  -1,  -1,   0,  -1,   1,   2,   3,   4,  -1 },
    /* PRE_POKEDEX */ {  -1,   0,   1,  -1,   2,   3,   4,   5,  -1 },
    /* PRE_POKENAV */ {   0,   1,   2,  -1,   3,   4,   5,   6,  -1 },
    /* FULL        */ {   0,   1,   2,   3,   4,   5,   6,   7,  -1 },
    /* SAFARI      */ {   1,   2,   3,  -1,   4,  -1,   5,   6,   0 },
};

static std::vector<SlotCandidate> start_menu_slot_candidates(StartMenuSlot slot, StartMenuState state) {
	int8_t row = START_MENU_ROWS[(int)state][(int)slot];
	if (row == -1) {
		throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Invalid StartMenuSlot for StartMenuState");
	}
    double offset = SELECTION_SLOT_OFFSETS[row];
    return {
        { 0.725, 0.098 + offset, SelectionIndicator::ARROW  },
        { 0.759, 0.098 + offset, SelectionIndicator::ARROW  }, // jpn rse
        { 0.754, 0.106 + offset, SelectionIndicator::BORDER }
    };
}


StartMenuSlotDetector::StartMenuSlotDetector(Color color, StartMenuSlot slot, StartMenuState state)
    : SelectionSlotDetector(color, start_menu_slot_candidates(slot, state))
{}



class Test_StartMenuDetector : public UnitTest {
public:

    Test_StartMenuDetector(
        const std::string& image,
		StartMenuState state,
        bool expected
    )
        : UnitTest("PokemonRSE::StartMenuDetector - " + image)
        , m_image(UNIT_TEST_RESOURCE_PATH() + image)
        , m_state(state)
        , m_expected(expected)
    {
    }

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override {
        StartMenuDetector detector(COLOR_RED, m_state);
        ImageRGB32 image(m_image);
        return detector.detect(image) == m_expected;
    };

private:
    std::string m_image;
	StartMenuState m_state;
    bool m_expected;
};

void add_tests_StartMenuDetector(UnitTestDatabase & database) {
    // Switch 1/2

    // Original GBA resolution; {0.0, 0.0, 1.0, 1.0}
    database.add<Test_StartMenuDetector>("PokemonRSE/StartMenuDetector/GBA-Ger-Emerald-Full-Pokedex_true.png", StartMenuState::FULL, true);
    database.add<Test_StartMenuDetector>("PokemonRSE/StartMenuDetector/GBA-Ger-Emerald-PrePokenav-Exit_true.png", StartMenuState::PRE_POKENAV, true);
    database.add<Test_StartMenuDetector>("PokemonRSE/StartMenuDetector/GBA-Ger-Emerald-PrePokedex-Option_true.png", StartMenuState::PRE_POKEDEX, true);
    database.add<Test_StartMenuDetector>("PokemonRSE/StartMenuDetector/GBA-Ger-Emerald-PrePokemon-Bag_true.png", StartMenuState::PRE_POKEMON, true);

    database.add<Test_StartMenuDetector>("PokemonRSE/StartMenuDetector/GBA-Ger-Ruby-Full-Exit_true.png", StartMenuState::FULL, true);
    database.add<Test_StartMenuDetector>("PokemonRSE/StartMenuDetector/GBA-Ger-Ruby-PrePokenav-Bag_true.png", StartMenuState::PRE_POKENAV, true);
    database.add<Test_StartMenuDetector>("PokemonRSE/StartMenuDetector/GBA-Ger-Ruby-PrePokedex-Exit_true.png", StartMenuState::PRE_POKEDEX, true);
    database.add<Test_StartMenuDetector>("PokemonRSE/StartMenuDetector/GBA-Ger-Ruby-PrePokemon-Bag_true.png", StartMenuState::PRE_POKEMON, true);

    // Game Boy Player; {0.064217, 0.107667, 0.86413, 0.787928}
    database.add<Test_StartMenuDetector>("PokemonRSE/StartMenuDetector/GBP-Ger-Emerald-Full-Pokemon_true.png", StartMenuState::FULL, true);
    database.add<Test_StartMenuDetector>("PokemonRSE/StartMenuDetector/GBP-Ger-Ruby-Full-Pokemon_true.png", StartMenuState::FULL, true);
}


class Test_StartMenuSlotDetector : public UnitTest {
public:

    Test_StartMenuSlotDetector(
        const std::string& image,
        StartMenuSlot slot,
        StartMenuState state,
        bool expected
    )
        : UnitTest("PokemonRSE::StartMenuSlotDetector - " + std::to_string((int)slot) + " - " + image)
        , m_image(UNIT_TEST_RESOURCE_PATH() + image)
        , m_slot(slot)
        , m_state(state)
        , m_expected(expected)
    {
    }

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override {
        StartMenuSlotDetector detector(COLOR_RED, m_slot, m_state);
        ImageRGB32 image(m_image);
        return detector.detect(image) == m_expected;
    };

private:
    std::string m_image;
    StartMenuSlot m_slot;
    StartMenuState m_state;
    bool m_expected;
};

void add_tests_StartMenuSlotDetector(UnitTestDatabase & database) {
    // Switch 1/2

    // Original GBA resolution; {0.0, 0.0, 1.0, 1.0}
    database.add<Test_StartMenuSlotDetector>("PokemonRSE/StartMenuDetector/GBA-Ger-Emerald-Full-Pokedex_true.png", StartMenuSlot::POKEDEX, StartMenuState::FULL, true);
    database.add<Test_StartMenuSlotDetector>("PokemonRSE/StartMenuDetector/GBA-Ger-Emerald-PrePokenav-Exit_true.png", StartMenuSlot::EXIT, StartMenuState::PRE_POKENAV, true);
    database.add<Test_StartMenuSlotDetector>("PokemonRSE/StartMenuDetector/GBA-Ger-Emerald-PrePokedex-Option_true.png", StartMenuSlot::OPTION, StartMenuState::PRE_POKEDEX, true);
    database.add<Test_StartMenuSlotDetector>("PokemonRSE/StartMenuDetector/GBA-Ger-Emerald-PrePokemon-Bag_true.png", StartMenuSlot::BAG, StartMenuState::PRE_POKEMON, true);

    database.add<Test_StartMenuSlotDetector>("PokemonRSE/StartMenuDetector/GBA-Ger-Ruby-Full-Exit_true.png", StartMenuSlot::EXIT, StartMenuState::FULL, true);
    database.add<Test_StartMenuSlotDetector>("PokemonRSE/StartMenuDetector/GBA-Ger-Ruby-PrePokenav-Bag_true.png", StartMenuSlot::BAG, StartMenuState::PRE_POKENAV, true);
    database.add<Test_StartMenuSlotDetector>("PokemonRSE/StartMenuDetector/GBA-Ger-Ruby-PrePokedex-Exit_true.png", StartMenuSlot::EXIT, StartMenuState::PRE_POKEDEX, true);
    database.add<Test_StartMenuSlotDetector>("PokemonRSE/StartMenuDetector/GBA-Ger-Ruby-PrePokemon-Bag_true.png", StartMenuSlot::BAG, StartMenuState::PRE_POKEMON, true);

    // Game Boy Player; {0.064217, 0.107667, 0.86413, 0.787928}
    database.add<Test_StartMenuSlotDetector>("PokemonRSE/StartMenuDetector/GBP-Ger-Emerald-Full-Pokemon_true.png", StartMenuSlot::POKEMON, StartMenuState::FULL, true);
    database.add<Test_StartMenuSlotDetector>("PokemonRSE/StartMenuDetector/GBP-Ger-Ruby-Full-Pokemon_true.png", StartMenuSlot::POKEMON, StartMenuState::FULL, true);

}





}
}
}
