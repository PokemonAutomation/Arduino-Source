/*  Box Selection Mode Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <algorithm>
#include <cmath>
#include "CommonFramework/GlobalAutoPaths.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonTools/ImageMatch/WaterfillTemplateMatcher.h"
#include "CommonTools/Images/WaterfillUtilities.h"
#include "Kernels/Waterfill/Kernels_Waterfill_Types.h"
#include "VideoGames/PokemonFRLG/PokemonFRLG_Settings.h"
#include "PokemonFRLG_BoxSelectionModeDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{
namespace{

const ImageFloatBox HAND_SEARCH_BOX(0.33, 0.0, 0.67, 0.95);

class SelectionHandMatcher : public ImageMatch::WaterfillTemplateMatcher{
public:
    SelectionHandMatcher(const char* path, Color min_color, Color max_color)
        : WaterfillTemplateMatcher(path, min_color, max_color, 100)
    {
        m_area_ratio_lower = 0.7;
        m_area_ratio_upper = 1.3;
    }
};

struct HandStyle{
    SelectionHandMatcher matcher;
    BoxSelectionMode mode;
    bool inverted;
    uint32_t min_color;
    uint32_t max_color;
    bool closed = false;
};

const auto& hand_styles(){
    static const HandStyle styles[] = {
        {{"PokemonFRLG/Box/SelectionHand_White.png", Color(180, 180, 180), Color(255, 255, 255)},
            BoxSelectionMode::Normal, false, 0xffb4b4b4, 0xffffffff},
        {{"PokemonFRLG/Box/SelectionHand_White2.png", Color(180, 180, 180), Color(255, 255, 255)},
            BoxSelectionMode::Normal, false, 0xffb4b4b4, 0xffffffff},
        {{"PokemonFRLG/Box/SelectionHand_White_Inverted.png", Color(180, 180, 180), Color(255, 255, 255)},
            BoxSelectionMode::Normal, true, 0xffb4b4b4, 0xffffffff},
        {{"PokemonFRLG/Box/SelectionHand_Gold.png", Color(180, 120, 0), Color(255, 255, 150)},
            BoxSelectionMode::Relocate, false, 0xffb47800, 0xffffff96},
        {{"PokemonFRLG/Box/SelectionHand_Gold2.png", Color(180, 120, 0), Color(255, 255, 150)},
            BoxSelectionMode::Relocate, false, 0xffb47800, 0xffffff96},
        {{"PokemonFRLG/Box/SelectionHand_Gold_Inverted.png", Color(180, 120, 0), Color(255, 255, 150)},
            BoxSelectionMode::Relocate, true, 0xffb47800, 0xffffff96},
        {{"PokemonFRLG/Box/SelectionHand_Closed_White.png", Color(180, 180, 180), Color(255, 255, 255)},
            BoxSelectionMode::Normal, false, 0xffb4b4b4, 0xffffffff, true},
        {{"PokemonFRLG/Box/SelectionHand_Closed_Gold.png", Color(180, 120, 0), Color(255, 255, 150)},
            BoxSelectionMode::Relocate, false, 0xffb47800, 0xffffff96, true},
    };
    return styles;
}

std::optional<BoxSelection> selection_at(const ImageFloatBox& hand, const HandStyle& style){
    constexpr double MAX_TOP_BUTTON_HAND_CENTER_Y = 0.15;
    constexpr double BOX_TILE_GRID_HAND_CENTER_Y_SPLIT = 0.12;
    constexpr double PARTY_CLOSE_BUTTON_HAND_CENTER_X_SPLIT = 0.62;
    constexpr double FIRST_GRID_HAND_CENTER_X = 0.425;
    constexpr double FIRST_GRID_HAND_CENTER_Y = 0.19;
    constexpr double GRID_COLUMN_SPACING = 24.0 / 240;
    constexpr double GRID_ROW_SPACING = 24.0 / 160;
    constexpr int GRID_COLUMNS = 6;
    constexpr int GRID_ROWS = 5;

    const double center_x = hand.x + hand.width / 2;
    const double center_y = hand.y + hand.height / 2;
    // Closed hands keep their shape when carrying a Pokemon onto a top button.
    if (style.inverted || (style.closed && center_y < BOX_TILE_GRID_HAND_CENTER_Y_SPLIT)){
        if (center_y > MAX_TOP_BUTTON_HAND_CENTER_Y){
            return std::nullopt;
        }
        return BoxSelection{style.mode, center_x < PARTY_CLOSE_BUTTON_HAND_CENTER_X_SPLIT
            ? BoxSelectionLocation::OpenPartyButton : BoxSelectionLocation::CloseBoxButton, style.closed};
    }
    if (center_y < BOX_TILE_GRID_HAND_CENTER_Y_SPLIT){
        return BoxSelection{style.mode, BoxSelectionLocation::BoxTile, style.closed};
    }

    const int column = (int)std::lround((center_x - FIRST_GRID_HAND_CENTER_X) / GRID_COLUMN_SPACING);
    const int row = (int)std::lround((center_y - FIRST_GRID_HAND_CENTER_Y) / GRID_ROW_SPACING);
    if (column < 0 || column >= GRID_COLUMNS || row < 0 || row >= GRID_ROWS){
        return std::nullopt;
    }
    return BoxSelection{style.mode, BoxSelectionLocation::Grid, style.closed, (size_t)row, (size_t)column};
}

std::string selection_string(const BoxSelection& selection){
    std::string result = selection.mode == BoxSelectionMode::Normal ? "Normal " : "Relocate ";
    result += selection.closed ? "Closed " : "Open ";
    switch (selection.location){
    case BoxSelectionLocation::Grid:
        return result + "Grid(" + std::to_string(selection.row) + ", " + std::to_string(selection.column) + ")";
    case BoxSelectionLocation::BoxTile:
        return result + "BoxTile";
    case BoxSelectionLocation::OpenPartyButton:
        return result + "OpenPartyButton";
    case BoxSelectionLocation::CloseBoxButton:
        return result + "CloseBoxButton";
    }
    return result;
}

}

void BoxSelectionModeDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, GameSettings::instance().GAME_BOX.inner_to_outer(HAND_SEARCH_BOX));
}

std::optional<BoxSelection> BoxSelectionModeDetector::detect(const ImageViewRGB32& screen) const{
    const ImageViewRGB32 game_screen = extract_box_reference(screen, GameSettings::instance().GAME_BOX);
    const ImageViewRGB32 region = extract_box_reference(game_screen, HAND_SEARCH_BOX);
    const double screen_rel_size = game_screen.height() / 1080.0;
    const double screen_rel_size_2 = screen_rel_size * screen_rel_size;
    constexpr double min_area_1080p = 540;
    const size_t min_area = std::max<size_t>(1, size_t(screen_rel_size_2 * min_area_1080p));
    double best_rmsd = 50;
    std::optional<BoxSelection> best;
    for (const HandStyle& style : hand_styles()){
        match_template_by_waterfill(
            game_screen.size(), region, style.matcher,
            {{style.min_color, style.max_color}}, {min_area, SIZE_MAX}, best_rmsd,
            [&](Kernels::Waterfill::WaterfillObject& object){
                const ImageFloatBox hand = translate_to_parent(game_screen, HAND_SEARCH_BOX, object);
                if (hand.width < 0.03 || hand.width > 0.085 || hand.height < 0.06 || hand.height > 0.15){
                    return false;
                }
                const auto selection = selection_at(hand, style);
                if (!selection){
                    return false;
                }
                const double rmsd = style.matcher.rmsd_original(game_screen.size(), region, object);
                if (rmsd < best_rmsd){
                    best_rmsd = rmsd;
                    best = selection;
                }
                return false;
            }
        );
    }
    return best;
}

BoxSelectionModeWatcher::BoxSelectionModeWatcher(
    Color color, BoxSelectionMode mode, BoxSelectionLocation location, bool closed
)
    : BoxSelectionModeDetector(color)
    , VisualInferenceCallback("BoxSelectionModeWatcher")
    , m_mode(mode)
    , m_location(location)
    , m_closed(closed)
{}

bool BoxSelectionModeWatcher::process_frame(const ImageViewRGB32& frame, WallClock timestamp){
    m_last_detected = detect(frame);
    if (m_last_detected && m_last_detected->mode == m_mode
        && m_last_detected->location == m_location && m_last_detected->closed == m_closed){
        return true;
    }
    m_last_detected.reset();
    return false;
}

class Test_BoxSelectionModeDetector : public UnitTest{
public:
    Test_BoxSelectionModeDetector(
        const std::string& image, BoxSelectionMode mode, BoxSelectionLocation location,
        size_t row = 0, size_t column = 0, bool closed = false
    )
        : UnitTest("PokemonFRLG::BoxSelectionModeDetector - " + image)
        , m_image(UNIT_TEST_RESOURCE_PATH() + image)
        , m_expected{mode, location, closed, row, column}
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        ImageRGB32 image(m_image);
        const auto result = BoxSelectionModeDetector().detect(image);
        if (!result){
            return "Expected: " + selection_string(m_expected) + ", received no hand.";
        }
        if (result->mode != m_expected.mode || result->location != m_expected.location
            || result->closed != m_expected.closed
            || result->row != m_expected.row || result->column != m_expected.column){
            return "Expected: " + selection_string(m_expected) + ", received: " + selection_string(*result);
        }
        return true;
    }

private:
    std::string m_image;
    BoxSelection m_expected;
};

void add_tests_BoxSelectionModeDetector(UnitTestDatabase& database){
    using Mode = BoxSelectionMode;
    using Location = BoxSelectionLocation;
    database.add<Test_BoxSelectionModeDetector>("PokemonFRLG/Box/Arcanine_M_100_White.png", Mode::Normal, Location::Grid, 0, 4);
    database.add<Test_BoxSelectionModeDetector>("PokemonFRLG/Box/BoxHand_Closed_Gold.png", Mode::Relocate, Location::Grid, 0, 2, true);
    database.add<Test_BoxSelectionModeDetector>("PokemonFRLG/Box/BoxHand_Closed_White.png", Mode::Normal, Location::OpenPartyButton, 0, 0, true);
    database.add<Test_BoxSelectionModeDetector>("PokemonFRLG/Box/BoxHand_Gold_HD60Pro_NV12.png", Mode::Relocate, Location::Grid, 0, 0);
    database.add<Test_BoxSelectionModeDetector>("PokemonFRLG/Box/BoxHand_Gold_HD60Pro_YUYV.png", Mode::Relocate, Location::Grid, 0, 0);
    database.add<Test_BoxSelectionModeDetector>("PokemonFRLG/Box/BoxHand_White_HD60Pro_NV12.png", Mode::Normal, Location::Grid, 0, 0);
    database.add<Test_BoxSelectionModeDetector>("PokemonFRLG/Box/BoxHand_White_HD60Pro_YUYV.png", Mode::Normal, Location::Grid, 0, 0);
    database.add<Test_BoxSelectionModeDetector>("PokemonFRLG/Box/BoxSelection_Gold.png", Mode::Relocate, Location::BoxTile);
    database.add<Test_BoxSelectionModeDetector>("PokemonFRLG/Box/BoxSelection_Gold2.png", Mode::Relocate, Location::Grid, 2, 4);
    database.add<Test_BoxSelectionModeDetector>("PokemonFRLG/Box/BoxSelection_White.png", Mode::Normal, Location::BoxTile);
    database.add<Test_BoxSelectionModeDetector>("PokemonFRLG/Box/BoxSelection_White2.png", Mode::Normal, Location::Grid, 2, 4);
    database.add<Test_BoxSelectionModeDetector>("PokemonFRLG/Box/Caterpie_F_4_White.png", Mode::Normal, Location::Grid, 0, 5);
    database.add<Test_BoxSelectionModeDetector>("PokemonFRLG/Box/CloseSelection_Gold.png", Mode::Relocate, Location::CloseBoxButton);
    database.add<Test_BoxSelectionModeDetector>("PokemonFRLG/Box/CloseSelection_White.png", Mode::Normal, Location::CloseBoxButton);
    database.add<Test_BoxSelectionModeDetector>("PokemonFRLG/Box/Diglett_F_20_White.png", Mode::Normal, Location::Grid, 0, 1);
    database.add<Test_BoxSelectionModeDetector>("PokemonFRLG/Box/Doduo-NV12.png", Mode::Normal, Location::Grid, 1, 2);
    database.add<Test_BoxSelectionModeDetector>("PokemonFRLG/Box/Doduo-YUYV.png", Mode::Normal, Location::Grid, 1, 2);
    database.add<Test_BoxSelectionModeDetector>("PokemonFRLG/Box/Dragonite_F_100_Whtie.png", Mode::Normal, Location::Grid, 2, 5);
    database.add<Test_BoxSelectionModeDetector>("PokemonFRLG/Box/Nidokinq_M_23_White.png", Mode::Normal, Location::Grid, 1, 1);
    database.add<Test_BoxSelectionModeDetector>("PokemonFRLG/Box/nidoran-f-mjpeg.png", Mode::Normal, Location::Grid, 2, 0);
    database.add<Test_BoxSelectionModeDetector>("PokemonFRLG/Box/nidoran-f-NV12.png", Mode::Normal, Location::Grid, 2, 0);
    database.add<Test_BoxSelectionModeDetector>("PokemonFRLG/Box/nidoran-f-YUYV.png", Mode::Normal, Location::Grid, 2, 0);
    database.add<Test_BoxSelectionModeDetector>("PokemonFRLG/Box/nidoran-m-mjpeg.png", Mode::Normal, Location::Grid, 2, 2);
    database.add<Test_BoxSelectionModeDetector>("PokemonFRLG/Box/nidoran-m-NV12.png", Mode::Normal, Location::Grid, 2, 2);
    database.add<Test_BoxSelectionModeDetector>("PokemonFRLG/Box/nidoran-m-YUYV.png", Mode::Normal, Location::Grid, 2, 2);
    database.add<Test_BoxSelectionModeDetector>("PokemonFRLG/Box/PartySelection_Gold.png", Mode::Relocate, Location::OpenPartyButton);
    database.add<Test_BoxSelectionModeDetector>("PokemonFRLG/Box/PartySelection_White.png", Mode::Normal, Location::OpenPartyButton);
    database.add<Test_BoxSelectionModeDetector>("PokemonFRLG/Box/Rhydon-NV12.png", Mode::Normal, Location::Grid, 3, 3);
    database.add<Test_BoxSelectionModeDetector>("PokemonFRLG/Box/Rhydon-YUYV.png", Mode::Normal, Location::Grid, 3, 3);
}

}
}
}
