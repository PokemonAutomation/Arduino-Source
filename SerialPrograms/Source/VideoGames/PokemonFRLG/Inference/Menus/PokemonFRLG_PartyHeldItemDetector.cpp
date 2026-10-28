/*  Party Held Item Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
#include "CommonFramework/GlobalAutoPaths.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonTools/ImageMatch/WaterfillTemplateMatcher.h"
#include "CommonTools/Images/ImageFilter.h"
#include "CommonTools/Images/WaterfillUtilities.h"
#include "VideoGames/PokemonFRLG/PokemonFRLG_Settings.h"
#include "PokemonFRLG_PartyHeldItemDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{


class PartyHeldItemMatcher : public ImageMatch::WaterfillTemplateMatcher{
public:
    PartyHeldItemMatcher(const char* path)
        : WaterfillTemplateMatcher(
            path,
            Color(210, 210, 100),
            Color(255, 255, 190),
            20
        )
    {
        m_aspect_ratio_lower = 0.5;
        m_aspect_ratio_upper = 2.0;
        m_area_ratio_lower = 0.2;
        m_area_ratio_upper = 4.0;
    }

    static const PartyHeldItemMatcher& matcher(){
        static PartyHeldItemMatcher matcher("PokemonFRLG/PartyHeldItem.png");
        return matcher;
    }
};

ImageFloatBox PartyHeldItemDetector::box_for_slot(PartySlot slot, Language language){
    const double x_offset = language == Language::Japanese ? 0.03 : 0;
    switch (slot){
    case PartySlot::ONE:
        return ImageFloatBox(0.069 + x_offset, 0.285, 0.025, 0.050);
    case PartySlot::TWO:
        return ImageFloatBox(0.432 + x_offset, 0.150, 0.025, 0.050);
    case PartySlot::THREE:
        return ImageFloatBox(0.432 + x_offset, 0.3, 0.025, 0.050);
    case PartySlot::FOUR:
        return ImageFloatBox(0.432 + x_offset, 0.45, 0.025, 0.050);
    case PartySlot::FIVE:
        return ImageFloatBox(0.432 + x_offset, 0.6, 0.025, 0.050);
    case PartySlot::SIX:
        return ImageFloatBox(0.432 + x_offset, 0.725, 0.025, 0.050);
    default:
        break;
    }
    throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Invalid FRLG Party Held Item Slot");
}

PartyHeldItemDetector::PartyHeldItemDetector(
    Color color,
    VideoOverlay* overlay,
    PartySlot slot,
    Language language
)
    : m_color(color)
    , m_overlay(overlay)
    , m_box(box_for_slot(slot, language))
{}
PartyHeldItemDetector::PartyHeldItemDetector(
    Color color,
    VideoOverlay* overlay,
    const ImageFloatBox& box
)
    : m_color(color)
    , m_overlay(overlay)
    , m_box(box)
{}
void PartyHeldItemDetector::make_overlays(VideoOverlaySet& items) const{
    const BoxOption& GAME_BOX = GameSettings::instance().GAME_BOX;
    items.add(m_color, GAME_BOX.inner_to_outer(m_box));
}
bool PartyHeldItemDetector::detect(const ImageViewRGB32& screen){
    const BoxOption& GAME_BOX = GameSettings::instance().GAME_BOX;
    ImageViewRGB32 game_screen = extract_box_reference(screen, GAME_BOX);

    double screen_rel_size = (game_screen.height() / 1080.0);
    double screen_rel_size_2 = screen_rel_size * screen_rel_size;

    double min_area_1080p = 100;
    double rmsd_threshold = 220;
    size_t min_area = size_t(screen_rel_size_2 * min_area_1080p);

    const std::vector<std::pair<uint32_t, uint32_t>> FILTERS = {
        {0xffd2d264, 0xffffffbe}
    };

    ImageViewRGB32 item_box = extract_box_reference(game_screen, m_box);
    // The icon has a red band as well as a yellow body. Pokemon sprites can match the yellow alone.
    size_t red_pixels;
    filter_rgb32_range(red_pixels, item_box, 0xffc83c1e, 0xffffa08c, COLOR_BLACK, true);

    bool found = red_pixels >= screen_rel_size_2 * 50 && match_template_by_waterfill(
        game_screen.size(),
        item_box,
        PartyHeldItemMatcher::matcher(),
        FILTERS,
        {min_area, SIZE_MAX},
        rmsd_threshold,
        [&](Kernels::Waterfill::WaterfillObject& object) -> bool {
            m_last_detected = translate_to_parent(game_screen, m_box, object);
            return true;
        }
    );

    if (m_overlay){
        if (found){
            m_last_detected_box.emplace(*m_overlay, GAME_BOX.inner_to_outer(m_last_detected), COLOR_GREEN);
        }else{
            m_last_detected_box.reset();
        }
    }

    return found;
}


class Test_PartyHeldItemDetector : public UnitTest{
public:
    Test_PartyHeldItemDetector(
        const std::string& image, PartySlot slot, bool expected, Language language = Language::English
    )
        : UnitTest("PokemonFRLG::PartyHeldItemDetector - " + image + " - slot " + std::to_string(static_cast<unsigned>(slot) + 1))
        , m_image(UNIT_TEST_RESOURCE_PATH() + image)
        , m_slot(slot)
        , m_expected(expected)
        , m_language(language)
    {}

    virtual UnitTestResult run(Logger&, CancellableScope&) const override{
        PartyHeldItemDetector detector(COLOR_RED, nullptr, m_slot, m_language);
        ImageRGB32 image(m_image);
        bool result = detector.detect(image);
        if (result != m_expected){
            return std::string("Expected held item = ") + (m_expected ? "true" : "false") +
                ", got " + (result ? "true" : "false");
        }
        return true;
    }

private:
    std::string m_image;
    PartySlot m_slot;
    bool m_expected;
    Language m_language;
};

void add_tests_PartyHeldItemDetector(UnitTestDatabase& database){
    for (unsigned c = 0; c < 6; c++){
        PartySlot slot = static_cast<PartySlot>(c);
        database.add<Test_PartyHeldItemDetector>("PokemonFRLG/Party/Party_1.png", slot, false);
        database.add<Test_PartyHeldItemDetector>("PokemonFRLG/Party/Party_2.png", slot, false);
        database.add<Test_PartyHeldItemDetector>("PokemonFRLG/Party/Party_3.png", slot, false);
        database.add<Test_PartyHeldItemDetector>("PokemonFRLG/Party/Party_4.png", slot, false);
        database.add<Test_PartyHeldItemDetector>("PokemonFRLG/Party/Party_5.png", slot, false);
        database.add<Test_PartyHeldItemDetector>("PokemonFRLG/Party/Party_6.png", slot, false);
        database.add<Test_PartyHeldItemDetector>("PokemonFRLG/Party/JPN_Party.png", slot, false, Language::Japanese);
        database.add<Test_PartyHeldItemDetector>("PokemonFRLG/Party/Party_6_With_Held_Items.png", slot, c == 0 || c == 3);
        database.add<Test_PartyHeldItemDetector>("PokemonFRLG/Party/SPA_HeldItem.png", slot, c == 0, Language::Spanish);
    }
    database.add<Test_PartyHeldItemDetector>("PokemonFRLG/Party/JPN_HeldItem.png", PartySlot::ONE, true, Language::Japanese);
}

}
}
}
