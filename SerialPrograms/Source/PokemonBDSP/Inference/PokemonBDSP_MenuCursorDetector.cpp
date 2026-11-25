/*  Menu Cursor Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <format>
#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "Tests/ImageUnitTest.h"
#include "PokemonBDSP_MenuDetector.h"
#include "PokemonBDSP_MenuCursorDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


std::string menu_cursor_to_string(MenuCursorPosition pos){
    switch (pos){
    case MenuCursorPosition::POKEDEX:
        return "POKEDEX";
    case MenuCursorPosition::POKEMON:
        return "POKEMON";
    case MenuCursorPosition::BAG:
        return "BAG";
    case MenuCursorPosition::CARD:
        return "CARD";
    case MenuCursorPosition::OPTIONS:
        return "OPTIONS";
    case MenuCursorPosition::MAP:
        return "MAP";
    case MenuCursorPosition::CAPSULES:
        return "CAPSULES";
    case MenuCursorPosition::MYSTERYGIFT:
        return "MYSTERYGIFT";
    default:
        return "UNKNOWN";
    }
}


MenuCursorDetector::MenuCursorDetector(Color color)
    : m_color(color)
{
    for (int i = 0; i < 8; i++){
        m_icons[i] = ImageFloatBox(0.180 + 0.1675 * (i % 4), 0.115 + 0.249 * (i / 4), 0.135, 0.195);
    }
}

void MenuCursorDetector::make_overlays(VideoOverlaySet& items) const{
    for (const ImageFloatBox& icon : m_icons){
        items.add(m_color, icon);
        items.add(m_color, ImageFloatBox(icon.x + 0.128, icon.y + 0.050, 0.002, 0.090));
        items.add(m_color, ImageFloatBox(icon.x + 0.040, icon.y + 0.1805, 0.062, 0.003));
    }
}

MenuCursorPosition MenuCursorDetector::detect(const ImageViewRGB32& screen, int* icon_count) const{
    if (icon_count != nullptr){
        *icon_count = 0;
    }
    MenuDetector menu;
    if (!screen || screen.width() < 640 || screen.height() < 360 || !menu.detect(screen)){
        return MenuCursorPosition::UNKNOWN;
    }

    int count = 0;
    int selected = -1;
    bool empty_seen = false;
    for (int i = 0; i < 8; i++){
        const double x = m_icons[i].x;
        const double y = m_icons[i].y;
        const ImageStats interior = image_stats(extract_box_reference(screen,
            ImageFloatBox(x + 0.020, y + 0.025, 0.095, 0.140)));
        if (interior.stddev.sum() < 25){
            empty_seen = true;
            continue;
        }
        // Default layouts have no gaps. Do not infer a layout from an obscured menu.
        if (empty_seen){
            return MenuCursorPosition::UNKNOWN;
        }
        count++;

        // Compare each border with nearby icon fill. Averaging RGB gives equal-weight
        // grayscale brightness without requiring an absolute white or black value.
        // Use right and bottom edges to avoid the NEW tags and most icon artwork.
        const double right = image_stats(extract_box_reference(screen,
            ImageFloatBox(x + 0.128, y + 0.050, 0.002, 0.090))).average.sum();
        const double right_inside = image_stats(extract_box_reference(screen,
            ImageFloatBox(x + 0.115, y + 0.050, 0.002, 0.090))).average.sum();
        const double bottom = image_stats(extract_box_reference(screen,
            ImageFloatBox(x + 0.040, y + 0.1805, 0.062, 0.003))).average.sum();
        const double bottom_inside = image_stats(extract_box_reference(screen,
            ImageFloatBox(x + 0.040, y + 0.1635, 0.062, 0.003))).average.sum();
        if (right - right_inside > 3 * 12 && bottom - bottom_inside > 3 * 12){
            if (selected >= 0){
                return MenuCursorPosition::UNKNOWN;
            }
            selected = i;
        }
    }
    if (selected < 0 || count < 2){
        return MenuCursorPosition::UNKNOWN;
    }

    // Options moves as story features unlock. Mystery Gift follows Options.
    static constexpr MenuCursorPosition LAYOUTS[7][8] = {
        {MenuCursorPosition::BAG, MenuCursorPosition::OPTIONS},
        {MenuCursorPosition::POKEMON, MenuCursorPosition::BAG, MenuCursorPosition::OPTIONS},
        {MenuCursorPosition::POKEDEX, MenuCursorPosition::POKEMON, MenuCursorPosition::BAG, MenuCursorPosition::OPTIONS},
        {MenuCursorPosition::POKEDEX, MenuCursorPosition::POKEMON, MenuCursorPosition::BAG, MenuCursorPosition::CARD,
            MenuCursorPosition::OPTIONS},
        {MenuCursorPosition::POKEDEX, MenuCursorPosition::POKEMON, MenuCursorPosition::BAG, MenuCursorPosition::CARD,
            MenuCursorPosition::MAP, MenuCursorPosition::OPTIONS},
        {MenuCursorPosition::POKEDEX, MenuCursorPosition::POKEMON, MenuCursorPosition::BAG, MenuCursorPosition::CARD,
            MenuCursorPosition::MAP, MenuCursorPosition::CAPSULES, MenuCursorPosition::OPTIONS},
        {MenuCursorPosition::POKEDEX, MenuCursorPosition::POKEMON, MenuCursorPosition::BAG, MenuCursorPosition::CARD,
            MenuCursorPosition::MAP, MenuCursorPosition::CAPSULES, MenuCursorPosition::OPTIONS, MenuCursorPosition::MYSTERYGIFT},
    };
    if (icon_count != nullptr){
        *icon_count = count;
    }
    return LAYOUTS[count - 2][selected];
}

namespace{

class Test_MenuCursorDetector : public ImageUnitTest{
public:
    Test_MenuCursorDetector(const std::string& image, MenuCursorPosition expected, int count)
        : ImageUnitTest("PokemonBDSP::MenuCursorDetector - " + image, {image})
        , m_expected(expected)
        , m_count(count)
    {
        m_memory += MAX_TEST_IMAGE_RGB32_BYTES;
    }

    UnitTestResult run_on_images(Logger&, CancellableScope&, const std::vector<ImageRGB32>& images) const override{
        for (size_t width : {size_t(1920), size_t(1280), size_t(960)}){
            const ImageRGB32 scaled = images[0].scale_to(width, width * 9 / 16);
            MenuCursorDetector detector;
            int count = -1;
            const MenuCursorPosition actual = detector.detect(scaled, &count);
            if (actual != m_expected || count != m_count){
                return std::format("At {}px: expected {} / {} icons, read {} / {} icons.", width,
                    menu_cursor_to_string(m_expected), m_count, menu_cursor_to_string(actual), count);
            }
        }
        return true;
    }

private:
    MenuCursorPosition m_expected;
    int m_count;
};

}

void add_tests_MenuCursorDetector(UnitTestDatabase& database){
    using P = MenuCursorPosition;
    database.add<Test_MenuCursorDetector>("PokemonBDSP/MenuDetector/NewGameBagOptions_True.png", P::BAG, 2);
    database.add<Test_MenuCursorDetector>("PokemonBDSP/MenuDetector/JubilifeTownMapNew_True.png", P::BAG, 6);
    database.add<Test_MenuCursorDetector>("PokemonBDSP/MenuDetector/FloaromaMenuThreeNew_True.png", P::BAG, 7);
    database.add<Test_MenuCursorDetector>("PokemonBDSP/MenuDetector/Options_4.png", P::OPTIONS, 4);
    database.add<Test_MenuCursorDetector>("PokemonBDSP/MenuDetector/Pokemon_6.png", P::POKEMON, 6);
    database.add<Test_MenuCursorDetector>("PokemonBDSP/MenuDetector/Capsules_8.png", P::CAPSULES, 8);
    database.add<Test_MenuCursorDetector>("PokemonBDSP/MenuDetector/Bag_6.png", P::BAG, 6);
    database.add<Test_MenuCursorDetector>("PokemonBDSP/MenuDetector/TrainerCard_6_06.png", P::CARD, 6);
    database.add<Test_MenuCursorDetector>("PokemonBDSP/MenuDetector/TrainerCard_6_08.png", P::CARD, 6);
    database.add<Test_MenuCursorDetector>("PokemonBDSP/MenuDetector/TrainerCard_6_07.png", P::CARD, 6);
    database.add<Test_MenuCursorDetector>("PokemonBDSP/MenuDetector/Bag_6_01.png", P::BAG, 6);
    database.add<Test_MenuCursorDetector>("PokemonBDSP/MenuDetector/Bag_6_02.png", P::BAG, 6);
    database.add<Test_MenuCursorDetector>("PokemonBDSP/MenuDetector/Bag_6_03.png", P::BAG, 6);
    database.add<Test_MenuCursorDetector>("PokemonBDSP/MenuDetector/Bag_6_04.png", P::BAG, 6);
    database.add<Test_MenuCursorDetector>("PokemonBDSP/MenuDetector/Options_6_01.png", P::OPTIONS, 6);
    database.add<Test_MenuCursorDetector>("PokemonBDSP/MenuDetector/Options_6_02.png", P::OPTIONS, 6);
    database.add<Test_MenuCursorDetector>("PokemonBDSP/MenuDetector/Options_6_03.png", P::OPTIONS, 6);
    database.add<Test_MenuCursorDetector>("PokemonBDSP/MenuDetector/Options_6_04.png", P::OPTIONS, 6);
    database.add<Test_MenuCursorDetector>("PokemonBDSP/MenuDetector/Pokedex_6_01.png", P::POKEDEX, 6);
    database.add<Test_MenuCursorDetector>("PokemonBDSP/MenuDetector/Pokedex_6_02.png", P::POKEDEX, 6);
    database.add<Test_MenuCursorDetector>("PokemonBDSP/MenuDetector/Pokedex_6_03.png", P::POKEDEX, 6);
    database.add<Test_MenuCursorDetector>("PokemonBDSP/MenuDetector/Pokemon_6_01.png", P::POKEMON, 6);
    database.add<Test_MenuCursorDetector>("PokemonBDSP/MenuDetector/Pokemon_6_02.png", P::POKEMON, 6);
    database.add<Test_MenuCursorDetector>("PokemonBDSP/MenuDetector/Pokemon_6_03.png", P::POKEMON, 6);
    database.add<Test_MenuCursorDetector>("PokemonBDSP/MenuDetector/Pokemon_6_04.png", P::POKEMON, 6);
    database.add<Test_MenuCursorDetector>("PokemonBDSP/MenuDetector/TownMap_6_01.png", P::MAP, 6);
    database.add<Test_MenuCursorDetector>("PokemonBDSP/MenuDetector/TownMap_6_02.png", P::MAP, 6);
    database.add<Test_MenuCursorDetector>("PokemonBDSP/MenuDetector/TownMap_6_03.png", P::MAP, 6);
    database.add<Test_MenuCursorDetector>("PokemonBDSP/MenuDetector/TownMap_6_04.png", P::MAP, 6);
    database.add<Test_MenuCursorDetector>("PokemonBDSP/MenuDetector/TownMap_6_05.png", P::MAP, 6);
    database.add<Test_MenuCursorDetector>("PokemonBDSP/MenuDetector/TrainerCard_6_01.png", P::CARD, 6);
    database.add<Test_MenuCursorDetector>("PokemonBDSP/MenuDetector/TrainerCard_6_02.png", P::CARD, 6);
    database.add<Test_MenuCursorDetector>("PokemonBDSP/MenuDetector/TrainerCard_6_03.png", P::CARD, 6);
    database.add<Test_MenuCursorDetector>("PokemonBDSP/MenuDetector/TrainerCard_6_04.png", P::CARD, 6);
    database.add<Test_MenuCursorDetector>("PokemonBDSP/MenuDetector/TrainerCard_6_05.png", P::CARD, 6);
    for (const char* image : {"SelectionAnimation.png", "Synthetic_Ambiguous.png", "Synthetic_NoSelection.png",
        "NotMenu_Bag.png", "NotMenu_Battle.png", "Synthetic_Black.png", "Synthetic_White.png"}){
        database.add<Test_MenuCursorDetector>(std::string("PokemonBDSP/MenuDetector/") + image, P::UNKNOWN, 0);
    }
    for (const char* image : {"Home_False.png", "OverworldSandgemMart_False.png", "OverworldOreburghMine_False.png",
        "IntroDialog_False.png", "IntroBarry_False.png", "FloaromaOldManDialog_False.png", "FloaromaCloudShadows_False.png"}){
        database.add<Test_MenuCursorDetector>(std::string("PokemonBDSP/MenuDetector/") + image, P::UNKNOWN, 0);
    }
}

}
}
}
