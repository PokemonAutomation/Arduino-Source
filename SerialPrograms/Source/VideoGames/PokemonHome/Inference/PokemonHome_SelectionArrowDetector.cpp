/*  Selection Arrow Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <algorithm>
#include <cmath>
#include "Kernels/Waterfill/Kernels_Waterfill_Types.h"
#include "CommonFramework/GlobalAutoPaths.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/ImageMatch/WaterfillTemplateMatcher.h"
#include "CommonTools/Images/WaterfillUtilities.h"
#include "Tests/ImageUnitTest.h"
#include "PokemonHome_SelectionArrowDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonHome{
namespace{

const ImageFloatBox SELECTION_SEARCH_BOX(0.02, 0.085, 0.47, 0.69);
const ImageFloatBox MODE_ICON_BORDER(0.495, 0.005, 0.010, 0.006);

class SelectionCursorMatcher : public ImageMatch::WaterfillTemplateMatcher{
public:
    SelectionCursorMatcher(const char* path, Color min_color, Color max_color)
        : WaterfillTemplateMatcher(path, min_color, max_color, 100)
    {
        m_aspect_ratio_lower = 0.8;
        m_aspect_ratio_upper = 1.25;
        m_area_ratio_lower = 0.75;
        m_area_ratio_upper = 1.25;
    }
};

struct ArrowStyle{
    SelectionCursorMatcher matcher;
    BoxSelectionMode mode;
    bool down;
    uint32_t min_color;
    uint32_t max_color;
};

const auto& arrow_styles(){
    static const ArrowStyle styles[] = {
        {{"PokemonHome/SelectionArrows/SelectionArrowDown.png", Color(160, 0, 0), Color(255, 192, 192)},
            BoxSelectionMode::Normal, true, 0xffa00000, 0xffffc0c0},
        {{"PokemonHome/SelectionArrows/SelectionArrowRight.png", Color(160, 0, 0), Color(255, 192, 192)},
            BoxSelectionMode::Normal, false, 0xffa00000, 0xffffc0c0},
        {{"PokemonHome/SelectionArrows/SelectionArrowDownBlue.png", Color(0, 100, 160), Color(128, 224, 255)},
            BoxSelectionMode::Relocate, true, 0xff0064a0, 0xff80e0ff},
        {{"PokemonHome/SelectionArrows/SelectionArrowRightBlue.png", Color(0, 100, 160), Color(128, 224, 255)},
            BoxSelectionMode::Relocate, false, 0xff0064a0, 0xff80e0ff},
        {{"PokemonHome/SelectionArrows/SelectionArrowDownGreen.png", Color(0, 160, 0), Color(128, 255, 160)},
            BoxSelectionMode::Multi, true, 0xff00a000, 0xff80ffa0},
        {{"PokemonHome/SelectionArrows/SelectionArrowRightGreen.png", Color(0, 160, 0), Color(128, 255, 160)},
            BoxSelectionMode::Multi, false, 0xff00a000, 0xff80ffa0},
    };
    return styles;
}

// Arrows have a slightly different starting Y compared to the hand.
std::optional<BoxSelection> grid_selection_at(
    const ImageFloatBox& cursor, double first_row_y, BoxSelectionMode mode
){
    constexpr double FIRST_COLUMN_X = 0.075;
    constexpr double COLUMN_SPACING = 0.072;
    constexpr double ROW_SPACING = 0.1055;
    const double center_x = cursor.x + cursor.width / 2;
    const double center_y = cursor.y + cursor.height / 2;
    const int column = (int)std::lround((center_x - FIRST_COLUMN_X) / COLUMN_SPACING);
    const int row = (int)std::lround((center_y - first_row_y) / ROW_SPACING);
    if (column < 0 || column >= 6 || row < 0 || row >= 5){
        return std::nullopt;
    }
    if (std::abs(center_x - (FIRST_COLUMN_X + column * COLUMN_SPACING)) > 0.015
        || std::abs(center_y - (first_row_y + row * ROW_SPACING)) > 0.025){
        return std::nullopt;
    }
    return BoxSelection{mode, BoxSelectionLocation::Grid, (size_t)row, (size_t)column};
}

std::optional<BoxSelection> selection_at(const ImageFloatBox& arrow, const ArrowStyle& style){
    // Right arrows are used for buttons outside the grid.
    if (style.down){
        constexpr double FIRST_COLUMN_Y = 0.16;
        return grid_selection_at(arrow, FIRST_COLUMN_Y, style.mode);
    }

    const double center_x = arrow.x + arrow.width / 2;
    const double center_y = arrow.y + arrow.height / 2;
    if (std::abs(center_y - 0.123) < 0.025 && std::abs(center_x - 0.12) < 0.025){
        return BoxSelection{style.mode, BoxSelectionLocation::BoxName};
    }
    if (std::abs(center_y - 0.738) < 0.025){
        if (std::abs(center_x - 0.18) < 0.025){
            return BoxSelection{style.mode, BoxSelectionLocation::BoxSpaces};
        }
        if (std::abs(center_x - 0.332) < 0.025){
            return BoxSelection{style.mode, BoxSelectionLocation::Newest30};
        }
    }
    return std::nullopt;
}

std::optional<BoxSelectionMode> hand_selection_mode(const ImageViewRGB32& screen){
    // The hand is white in every mode. The active toolbar icon has a colored border.
    const FloatPixel color = image_average(extract_box_reference(screen, MODE_ICON_BORDER));
    if (color.r > 170 && color.g < 120 && color.b < 120){
        return BoxSelectionMode::Normal;
    }
    if (color.r < 125 && color.g > 90 && color.b > 170){
        return BoxSelectionMode::Relocate;
    }
    if (color.r < 125 && color.g > 145 && color.b < 140){
        return BoxSelectionMode::Multi;
    }
    return std::nullopt;
}

std::optional<BoxSelection> detect_hand(
    const ImageViewRGB32& screen, const ImageViewRGB32& region, double screen_rel_size
){
    const auto mode = hand_selection_mode(screen);
    if (!mode){
        return std::nullopt;
    }

    constexpr double FIRST_COLUMN_Y = 0.174;
    constexpr uint32_t MIN_WHITE = 0xffdcdcdc;
    static const SelectionCursorMatcher matcher(
        "PokemonHome/SelectionArrows/SelectionHand.png", Color(MIN_WHITE), COLOR_WHITE
    );
    const double area_scale = screen_rel_size * screen_rel_size;
    const size_t min_area = std::max<size_t>(1, size_t(area_scale * 700));
    const size_t max_area = std::max(min_area, size_t(area_scale * 2200));
    double best_rmsd = 40;
    std::optional<BoxSelection> best;
    match_template_by_waterfill(
        screen.size(), region, matcher, {{MIN_WHITE, 0xffffffff}}, {min_area, max_area}, best_rmsd,
        [&](Kernels::Waterfill::WaterfillObject& object){
            const ImageFloatBox hand = translate_to_parent(screen, SELECTION_SEARCH_BOX, object);
            if (hand.width < 0.017 || hand.width > 0.029 || hand.height < 0.034 || hand.height > 0.057){
                return false;
            }
            const auto selection = grid_selection_at(hand, FIRST_COLUMN_Y, *mode);
            if (!selection){
                return false;
            }

            // Verify black pixels at the wrist. Ensuring we don't detect a blob of white pixels.
            const ImageFloatBox wrist_outline(
                hand.x + hand.width * 0.4, hand.y - hand.height * 0.065,
                hand.width * 0.2, hand.height * 0.04
            );
            if (image_average(extract_box_reference(screen, wrist_outline)).sum() > 300){
                return false;
            }
            const double rmsd = matcher.rmsd_original(screen.size(), region, object);
            if (rmsd < best_rmsd){
                best_rmsd = rmsd;
                best = selection;
            }
            return false;
        }
    );
    return best;
}

std::string selection_string(const std::optional<BoxSelection>& selection){
    if (!selection){
        return "none";
    }
    std::string result;
    switch (selection->mode){
    case BoxSelectionMode::Normal:
        result = "Normal ";
        break;
    case BoxSelectionMode::Relocate:
        result = "Relocate ";
        break;
    case BoxSelectionMode::Multi:
        result = "Multi ";
        break;
    }
    switch (selection->location){
    case BoxSelectionLocation::Grid:
        return result + "Grid(" + std::to_string(selection->row) + ", " + std::to_string(selection->column) + ")";
    case BoxSelectionLocation::BoxName:
        return result + "BoxName";
    case BoxSelectionLocation::BoxSpaces:
        return result + "BoxSpaces";
    case BoxSelectionLocation::Newest30:
        return result + "Newest30";
    }
    return result;
}

}

void SelectionArrowDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, SELECTION_SEARCH_BOX);
    items.add(m_color, MODE_ICON_BORDER);
}

std::optional<BoxSelection> SelectionArrowDetector::detect(const ImageViewRGB32& screen) const{
    const ImageViewRGB32 region = extract_box_reference(screen, SELECTION_SEARCH_BOX);
    const double screen_rel_size = screen.height() / 1080.0;
    if (auto hand = detect_hand(screen, region, screen_rel_size)){
        return hand;
    }

    const size_t min_area = std::max<size_t>(1, size_t(screen_rel_size * screen_rel_size * 500));
    double best_rmsd = 80;
    std::optional<BoxSelection> best;
    for (const ArrowStyle& style : arrow_styles()){
        match_template_by_waterfill(
            screen.size(), region, style.matcher,
            {{style.min_color, style.max_color}}, {min_area, SIZE_MAX}, best_rmsd,
            [&](Kernels::Waterfill::WaterfillObject& object){
                const ImageFloatBox arrow = translate_to_parent(screen, SELECTION_SEARCH_BOX, object);
                if (arrow.width < 0.01 || arrow.width > 0.035 || arrow.height < 0.015 || arrow.height > 0.065){
                    return false;
                }
                const auto selection = selection_at(arrow, style);
                if (!selection){
                    return false;
                }
                const double rmsd = style.matcher.rmsd_original(screen.size(), region, object);
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

SelectionArrowWatcher::SelectionArrowWatcher(Color color, BoxSelectionMode mode, BoxSelectionLocation location)
    : SelectionArrowDetector(color)
    , VisualInferenceCallback("SelectionArrowWatcher")
    , m_mode(mode)
    , m_location(location)
{}

bool SelectionArrowWatcher::process_frame(const ImageViewRGB32& frame, WallClock timestamp){
    m_last_detected = detect(frame);
    if (m_last_detected && m_last_detected->mode == m_mode && m_last_detected->location == m_location){
        return true;
    }
    m_last_detected.reset();
    return false;
}

class Test_SelectionArrowDetector : public UnitTest{
public:
    Test_SelectionArrowDetector(const std::string& image, std::optional<BoxSelection> expected)
        : UnitTest("PokemonHome::SelectionArrowDetector - " + image)
        , m_image(UNIT_TEST_RESOURCE_PATH() + image)
        , m_expected(expected)
    {}

    virtual UnitTestResult run( Logger& logger, CancellableScope& scope) const override{
        ImageRGB32 image(m_image);
        const auto result = SelectionArrowDetector().detect(image);
        if (result.has_value() != m_expected.has_value()
            || (result && (result->mode != m_expected->mode || result->location != m_expected->location
                || result->row != m_expected->row || result->column != m_expected->column))){
            return "Expected: " + selection_string(m_expected) + ", received: " + selection_string(result);
        }
        return true;
    }

private:
    std::string m_image;
    std::optional<BoxSelection> m_expected;
};

void add_tests_SelectionArrowDetector(UnitTestDatabase& database){
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/BoxView-1.png", BoxSelection{BoxSelectionMode::Normal, BoxSelectionLocation::Grid, 0, 3});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionArrow_Normal_Grid_Row0_Column0_02.png", BoxSelection{BoxSelectionMode::Normal, BoxSelectionLocation::Grid, 0, 0});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionArrow_Normal_Grid_Row0_Column0_01.png", BoxSelection{BoxSelectionMode::Normal, BoxSelectionLocation::Grid, 0, 0});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionArrow_Multi_BoxSpaces_01.png", BoxSelection{BoxSelectionMode::Multi, BoxSelectionLocation::BoxSpaces, 0, 0});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionArrow_Multi_BoxSpaces_02.png", BoxSelection{BoxSelectionMode::Multi, BoxSelectionLocation::BoxSpaces, 0, 0});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionArrow_Multi_BoxSpaces_03.png", BoxSelection{BoxSelectionMode::Multi, BoxSelectionLocation::BoxSpaces, 0, 0});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionArrow_Multi_BoxSpaces_04.png", BoxSelection{BoxSelectionMode::Multi, BoxSelectionLocation::BoxSpaces, 0, 0});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionArrow_Multi_BoxSpaces_05.png", BoxSelection{BoxSelectionMode::Multi, BoxSelectionLocation::BoxSpaces, 0, 0});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionArrow_Multi_BoxSpaces_06.png", BoxSelection{BoxSelectionMode::Multi, BoxSelectionLocation::BoxSpaces, 0, 0});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionArrow_Multi_BoxSpaces_07.png", BoxSelection{BoxSelectionMode::Multi, BoxSelectionLocation::BoxSpaces, 0, 0});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionArrow_Multi_BoxSpaces_08.png", BoxSelection{BoxSelectionMode::Multi, BoxSelectionLocation::BoxSpaces, 0, 0});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionArrow_Multi_BoxSpaces_09.png", BoxSelection{BoxSelectionMode::Multi, BoxSelectionLocation::BoxSpaces, 0, 0});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionArrow_Normal_BoxSpaces_01.png", BoxSelection{BoxSelectionMode::Normal, BoxSelectionLocation::BoxSpaces, 0, 0});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionArrow_Normal_BoxSpaces_02.png", BoxSelection{BoxSelectionMode::Normal, BoxSelectionLocation::BoxSpaces, 0, 0});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionArrow_Normal_BoxSpaces_03.png", BoxSelection{BoxSelectionMode::Normal, BoxSelectionLocation::BoxSpaces, 0, 0});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionArrow_Normal_BoxSpaces_04.png", BoxSelection{BoxSelectionMode::Normal, BoxSelectionLocation::BoxSpaces, 0, 0});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionArrow_Normal_BoxSpaces_05.png", BoxSelection{BoxSelectionMode::Normal, BoxSelectionLocation::BoxSpaces, 0, 0});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionArrow_Normal_BoxSpaces_06.png", BoxSelection{BoxSelectionMode::Normal, BoxSelectionLocation::BoxSpaces, 0, 0});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionArrow_Normal_BoxSpaces_07.png", BoxSelection{BoxSelectionMode::Normal, BoxSelectionLocation::BoxSpaces, 0, 0});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionArrow_Normal_BoxSpaces_08.png", BoxSelection{BoxSelectionMode::Normal, BoxSelectionLocation::BoxSpaces, 0, 0});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionArrow_Normal_BoxSpaces_09.png", BoxSelection{BoxSelectionMode::Normal, BoxSelectionLocation::BoxSpaces, 0, 0});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionArrow_Normal_BoxSpaces_10.png", BoxSelection{BoxSelectionMode::Normal, BoxSelectionLocation::BoxSpaces, 0, 0});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionArrow_Normal_Newest30_01.png", BoxSelection{BoxSelectionMode::Normal, BoxSelectionLocation::Newest30, 0, 0});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionArrow_Normal_Newest30_02.png", BoxSelection{BoxSelectionMode::Normal, BoxSelectionLocation::Newest30, 0, 0});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionArrow_Normal_Newest30_03.png", BoxSelection{BoxSelectionMode::Normal, BoxSelectionLocation::Newest30, 0, 0});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionArrow_Relocate_BoxSpaces_01.png", BoxSelection{BoxSelectionMode::Relocate, BoxSelectionLocation::BoxSpaces, 0, 0});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionArrow_Relocate_BoxSpaces_02.png", BoxSelection{BoxSelectionMode::Relocate, BoxSelectionLocation::BoxSpaces, 0, 0});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionArrow_Relocate_BoxSpaces_03.png", BoxSelection{BoxSelectionMode::Relocate, BoxSelectionLocation::BoxSpaces, 0, 0});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionArrow_Relocate_BoxSpaces_04.png", BoxSelection{BoxSelectionMode::Relocate, BoxSelectionLocation::BoxSpaces, 0, 0});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionArrow_Relocate_BoxSpaces_05.png", BoxSelection{BoxSelectionMode::Relocate, BoxSelectionLocation::BoxSpaces, 0, 0});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionArrow_Relocate_BoxSpaces_06.png", BoxSelection{BoxSelectionMode::Relocate, BoxSelectionLocation::BoxSpaces, 0, 0});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionArrow_Relocate_BoxSpaces_07.png", BoxSelection{BoxSelectionMode::Relocate, BoxSelectionLocation::BoxSpaces, 0, 0});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionArrow_Relocate_BoxSpaces_08.png", BoxSelection{BoxSelectionMode::Relocate, BoxSelectionLocation::BoxSpaces, 0, 0});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionArrow_Relocate_BoxSpaces_09.png", BoxSelection{BoxSelectionMode::Relocate, BoxSelectionLocation::BoxSpaces, 0, 0});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionArrow_Relocate_BoxSpaces_10.png", BoxSelection{BoxSelectionMode::Relocate, BoxSelectionLocation::BoxSpaces, 0, 0});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionArrow_Relocate_BoxSpaces_11.png", BoxSelection{BoxSelectionMode::Relocate, BoxSelectionLocation::BoxSpaces, 0, 0});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionArrow_Relocate_BoxSpaces_12.png", BoxSelection{BoxSelectionMode::Relocate, BoxSelectionLocation::BoxSpaces, 0, 0});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionArrow_Relocate_BoxSpaces_13.png", BoxSelection{BoxSelectionMode::Relocate, BoxSelectionLocation::BoxSpaces, 0, 0});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionArrow_Relocate_BoxSpaces_14.png", BoxSelection{BoxSelectionMode::Relocate, BoxSelectionLocation::BoxSpaces, 0, 0});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionArrow_Relocate_BoxSpaces_15.png", BoxSelection{BoxSelectionMode::Relocate, BoxSelectionLocation::BoxSpaces, 0, 0});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionArrow_Relocate_BoxSpaces_16.png", BoxSelection{BoxSelectionMode::Relocate, BoxSelectionLocation::BoxSpaces, 0, 0});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionArrow_Relocate_Newest30_01.png", BoxSelection{BoxSelectionMode::Relocate, BoxSelectionLocation::Newest30, 0, 0});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionArrow_Relocate_Newest30_02.png", BoxSelection{BoxSelectionMode::Relocate, BoxSelectionLocation::Newest30, 0, 0});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionArrow_Relocate_BoxName_01.png", BoxSelection{BoxSelectionMode::Relocate, BoxSelectionLocation::BoxName, 0, 0});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionArrow_Relocate_BoxName_02.png", BoxSelection{BoxSelectionMode::Relocate, BoxSelectionLocation::BoxName, 0, 0});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionArrow_Relocate_BoxName_03.png", BoxSelection{BoxSelectionMode::Relocate, BoxSelectionLocation::BoxName, 0, 0});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionArrow_Relocate_BoxName_04.png", BoxSelection{BoxSelectionMode::Relocate, BoxSelectionLocation::BoxName, 0, 0});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionArrow_Relocate_BoxName_05.png", BoxSelection{BoxSelectionMode::Relocate, BoxSelectionLocation::BoxName, 0, 0});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionArrow_Relocate_Grid_Row2_Column0_01.png", BoxSelection{BoxSelectionMode::Relocate, BoxSelectionLocation::Grid, 2, 0});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionArrow_Relocate_Grid_Row2_Column0_02.png", BoxSelection{BoxSelectionMode::Relocate, BoxSelectionLocation::Grid, 2, 0});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionArrow_Relocate_Grid_Row2_Column5_01.png", BoxSelection{BoxSelectionMode::Relocate, BoxSelectionLocation::Grid, 2, 5});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionArrow_Relocate_Grid_Row2_Column5_02.png", BoxSelection{BoxSelectionMode::Relocate, BoxSelectionLocation::Grid, 2, 5});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionArrow_Multi_Grid_Row0_Column0_01.png", BoxSelection{BoxSelectionMode::Multi, BoxSelectionLocation::Grid, 0, 0});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionArrow_Multi_Grid_Row0_Column0_02.png", BoxSelection{BoxSelectionMode::Multi, BoxSelectionLocation::Grid, 0, 0});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionArrow_Multi_Grid_Row3_Column3_01.png", BoxSelection{BoxSelectionMode::Multi, BoxSelectionLocation::Grid, 3, 3});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionArrow_Multi_Grid_Row3_Column3_02.png", BoxSelection{BoxSelectionMode::Multi, BoxSelectionLocation::Grid, 3, 3});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionArrow_Multi_Grid_Row2_Column0_01.png", BoxSelection{BoxSelectionMode::Multi, BoxSelectionLocation::Grid, 2, 0});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionArrow_Normal_Grid_Row3_Column0_01.png", BoxSelection{BoxSelectionMode::Normal, BoxSelectionLocation::Grid, 3, 0});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionArrow_Normal_BoxSpaces_11.png", BoxSelection{BoxSelectionMode::Normal, BoxSelectionLocation::BoxSpaces, 0, 0});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionArrow_Normal_BoxName_01.png", BoxSelection{BoxSelectionMode::Normal, BoxSelectionLocation::BoxName, 0, 0});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionArrow_Normal_BoxName_02.png", BoxSelection{BoxSelectionMode::Normal, BoxSelectionLocation::BoxName, 0, 0});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionHand_Normal_Grid_Row1_Column5_01.png", BoxSelection{BoxSelectionMode::Normal, BoxSelectionLocation::Grid, 1, 5});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionHand_Normal_Grid_Row4_Column0_01.png", BoxSelection{BoxSelectionMode::Normal, BoxSelectionLocation::Grid, 4, 0});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionHand_Normal_Grid_Row0_Column5_01.png", BoxSelection{BoxSelectionMode::Normal, BoxSelectionLocation::Grid, 0, 5});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionHand_Relocate_Grid_Row0_Column4_01.png", BoxSelection{BoxSelectionMode::Relocate, BoxSelectionLocation::Grid, 0, 4});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionHand_Relocate_Grid_Row0_Column0_01.png", BoxSelection{BoxSelectionMode::Relocate, BoxSelectionLocation::Grid, 0, 0});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionHand_Multi_Grid_Row3_Column4_01.png", BoxSelection{BoxSelectionMode::Multi, BoxSelectionLocation::Grid, 3, 4});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionHand_Multi_Grid_Row4_Column5_01.png", BoxSelection{BoxSelectionMode::Multi, BoxSelectionLocation::Grid, 4, 5});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionArrow_None_01.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionHand_Normal_Grid_Row1_Column3_03.png", BoxSelection{BoxSelectionMode::Normal, BoxSelectionLocation::Grid, 1, 3});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionHand_Relocate_Grid_Row4_Column0_02.png", BoxSelection{BoxSelectionMode::Relocate, BoxSelectionLocation::Grid, 4, 0});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionHand_Multi_Grid_Row2_Column2_02.png", BoxSelection{BoxSelectionMode::Multi, BoxSelectionLocation::Grid, 2, 2});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionArrow_Normal_BoxName_03.png", BoxSelection{BoxSelectionMode::Normal, BoxSelectionLocation::BoxName, 0, 0});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionArrow_Relocate_BoxName_06.png", BoxSelection{BoxSelectionMode::Relocate, BoxSelectionLocation::BoxName, 0, 0});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionHand_Normal_Grid_Row1_Column5_02.png", BoxSelection{BoxSelectionMode::Normal, BoxSelectionLocation::Grid, 1, 5});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionHand_Normal_Grid_Row1_Column3_01.png", BoxSelection{BoxSelectionMode::Normal, BoxSelectionLocation::Grid, 1, 3});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionHand_Normal_Grid_Row4_Column1_01.png", BoxSelection{BoxSelectionMode::Normal, BoxSelectionLocation::Grid, 4, 1});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionHand_Normal_Grid_Row4_Column4_01.png", BoxSelection{BoxSelectionMode::Normal, BoxSelectionLocation::Grid, 4, 4});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionHand_Normal_Grid_Row3_Column5_01.png", BoxSelection{BoxSelectionMode::Normal, BoxSelectionLocation::Grid, 3, 5});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionArrow_Normal_Grid_Row0_Column4_01.png", BoxSelection{BoxSelectionMode::Normal, BoxSelectionLocation::Grid, 0, 4});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionArrow_Relocate_Grid_Row0_Column4_01.png", BoxSelection{BoxSelectionMode::Relocate, BoxSelectionLocation::Grid, 0, 4});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionHand_Relocate_Grid_Row0_Column1_01.png", BoxSelection{BoxSelectionMode::Relocate, BoxSelectionLocation::Grid, 0, 1});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionHand_Relocate_Grid_Row2_Column0_01.png", BoxSelection{BoxSelectionMode::Relocate, BoxSelectionLocation::Grid, 2, 0});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionHand_Relocate_Grid_Row2_Column4_01.png", BoxSelection{BoxSelectionMode::Relocate, BoxSelectionLocation::Grid, 2, 4});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionHand_Relocate_Grid_Row2_Column4_02.png", BoxSelection{BoxSelectionMode::Relocate, BoxSelectionLocation::Grid, 2, 4});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionHand_Relocate_Grid_Row4_Column3_01.png", BoxSelection{BoxSelectionMode::Relocate, BoxSelectionLocation::Grid, 4, 3});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionHand_Relocate_Grid_Row4_Column0_01.png", BoxSelection{BoxSelectionMode::Relocate, BoxSelectionLocation::Grid, 4, 0});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionHand_Relocate_Grid_Row1_Column0_01.png", BoxSelection{BoxSelectionMode::Relocate, BoxSelectionLocation::Grid, 1, 0});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionHand_Relocate_Grid_Row0_Column2_01.png", BoxSelection{BoxSelectionMode::Relocate, BoxSelectionLocation::Grid, 0, 2});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionArrow_Relocate_Grid_Row0_Column3_01.png", BoxSelection{BoxSelectionMode::Relocate, BoxSelectionLocation::Grid, 0, 3});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionArrow_Relocate_Grid_Row0_Column4_02.png", BoxSelection{BoxSelectionMode::Relocate, BoxSelectionLocation::Grid, 0, 4});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionArrow_Multi_Grid_Row0_Column4_01.png", BoxSelection{BoxSelectionMode::Multi, BoxSelectionLocation::Grid, 0, 4});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionArrow_Multi_Grid_Row3_Column2_01.png", BoxSelection{BoxSelectionMode::Multi, BoxSelectionLocation::Grid, 3, 2});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionHand_Multi_Grid_Row3_Column5_01.png", BoxSelection{BoxSelectionMode::Multi, BoxSelectionLocation::Grid, 3, 5});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionHand_Multi_Grid_Row2_Column5_01.png", BoxSelection{BoxSelectionMode::Multi, BoxSelectionLocation::Grid, 2, 5});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionHand_Multi_Grid_Row1_Column5_01.png", BoxSelection{BoxSelectionMode::Multi, BoxSelectionLocation::Grid, 1, 5});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionHand_Multi_Grid_Row0_Column4_01.png", BoxSelection{BoxSelectionMode::Multi, BoxSelectionLocation::Grid, 0, 4});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionHand_Multi_Grid_Row0_Column2_01.png", BoxSelection{BoxSelectionMode::Multi, BoxSelectionLocation::Grid, 0, 2});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionArrow_Multi_BoxSpaces_10.png", BoxSelection{BoxSelectionMode::Multi, BoxSelectionLocation::BoxSpaces, 0, 0});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionHand_Multi_Grid_Row4_Column2_01.png", BoxSelection{BoxSelectionMode::Multi, BoxSelectionLocation::Grid, 4, 2});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionHand_Multi_Grid_Row4_Column3_01.png", BoxSelection{BoxSelectionMode::Multi, BoxSelectionLocation::Grid, 4, 3});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionArrow_Multi_Newest30_01.png", BoxSelection{BoxSelectionMode::Multi, BoxSelectionLocation::Newest30, 0, 0});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionHand_Multi_Grid_Row4_Column5_02.png", BoxSelection{BoxSelectionMode::Multi, BoxSelectionLocation::Grid, 4, 5});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionHand_Multi_Grid_Row0_Column5_01.png", BoxSelection{BoxSelectionMode::Multi, BoxSelectionLocation::Grid, 0, 5});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionHand_Multi_Grid_Row0_Column5_02.png", BoxSelection{BoxSelectionMode::Multi, BoxSelectionLocation::Grid, 0, 5});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionHand_Multi_Grid_Row0_Column1_01.png", BoxSelection{BoxSelectionMode::Multi, BoxSelectionLocation::Grid, 0, 1});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionHand_Multi_Grid_Row1_Column2_01.png", BoxSelection{BoxSelectionMode::Multi, BoxSelectionLocation::Grid, 1, 2});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionArrow_Multi_Grid_Row1_Column2_01.png", BoxSelection{BoxSelectionMode::Multi, BoxSelectionLocation::Grid, 1, 2});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionArrow_Multi_Grid_Row1_Column2_02.png", BoxSelection{BoxSelectionMode::Multi, BoxSelectionLocation::Grid, 1, 2});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionArrow_Multi_Grid_Row1_Column2_03.png", BoxSelection{BoxSelectionMode::Multi, BoxSelectionLocation::Grid, 1, 2});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionArrow_Multi_Grid_Row2_Column2_01.png", BoxSelection{BoxSelectionMode::Multi, BoxSelectionLocation::Grid, 2, 2});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionHand_Multi_Grid_Row2_Column4_01.png", BoxSelection{BoxSelectionMode::Multi, BoxSelectionLocation::Grid, 2, 4});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionHand_Multi_Grid_Row2_Column4_02.png", BoxSelection{BoxSelectionMode::Multi, BoxSelectionLocation::Grid, 2, 4});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionHand_Multi_Grid_Row4_Column5_03.png", BoxSelection{BoxSelectionMode::Multi, BoxSelectionLocation::Grid, 4, 5});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionHand_Multi_Grid_Row4_Column4_01.png", BoxSelection{BoxSelectionMode::Multi, BoxSelectionLocation::Grid, 4, 4});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionHand_Multi_Grid_Row0_Column1_02.png", BoxSelection{BoxSelectionMode::Multi, BoxSelectionLocation::Grid, 0, 1});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionHand_Multi_Grid_Row1_Column1_01.png", BoxSelection{BoxSelectionMode::Multi, BoxSelectionLocation::Grid, 1, 1});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionArrow_Multi_Grid_Row1_Column2_04.png", BoxSelection{BoxSelectionMode::Multi, BoxSelectionLocation::Grid, 1, 2});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionArrow_Multi_Grid_Row2_Column3_01.png", BoxSelection{BoxSelectionMode::Multi, BoxSelectionLocation::Grid, 2, 3});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionArrow_Multi_Grid_Row2_Column3_02.png", BoxSelection{BoxSelectionMode::Multi, BoxSelectionLocation::Grid, 2, 3});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionHand_Multi_Grid_Row2_Column4_03.png", BoxSelection{BoxSelectionMode::Multi, BoxSelectionLocation::Grid, 2, 4});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionHand_Multi_Grid_Row2_Column4_04.png", BoxSelection{BoxSelectionMode::Multi, BoxSelectionLocation::Grid, 2, 4});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionHand_Multi_Grid_Row1_Column3_01.png", BoxSelection{BoxSelectionMode::Multi, BoxSelectionLocation::Grid, 1, 3});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionHand_Multi_Grid_Row2_Column2_01.png", BoxSelection{BoxSelectionMode::Multi, BoxSelectionLocation::Grid, 2, 2});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionHand_Multi_Grid_Row4_Column2_02.png", BoxSelection{BoxSelectionMode::Multi, BoxSelectionLocation::Grid, 4, 2});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionArrow_Multi_Grid_Row2_Column2_02.png", BoxSelection{BoxSelectionMode::Multi, BoxSelectionLocation::Grid, 2, 2});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionArrow_Normal_Grid_Row1_Column5_01.png", BoxSelection{BoxSelectionMode::Normal, BoxSelectionLocation::Grid, 1, 5});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionHand_Normal_Grid_Row0_Column5_02.png", BoxSelection{BoxSelectionMode::Normal, BoxSelectionLocation::Grid, 0, 5});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionHand_Normal_Grid_Row0_Column5_03.png", BoxSelection{BoxSelectionMode::Normal, BoxSelectionLocation::Grid, 0, 5});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionHand_Normal_Grid_Row3_Column1_01.png", BoxSelection{BoxSelectionMode::Normal, BoxSelectionLocation::Grid, 3, 1});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionHand_Normal_Grid_Row3_Column2_01.png", BoxSelection{BoxSelectionMode::Normal, BoxSelectionLocation::Grid, 3, 2});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionHand_Normal_Grid_Row1_Column3_02.png", BoxSelection{BoxSelectionMode::Normal, BoxSelectionLocation::Grid, 1, 3});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionArrow_Normal_Grid_Row2_Column1_01.png", BoxSelection{BoxSelectionMode::Normal, BoxSelectionLocation::Grid, 2, 1});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionHand_Relocate_Grid_Row2_Column1_01.png", BoxSelection{BoxSelectionMode::Relocate, BoxSelectionLocation::Grid, 2, 1});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionHand_Relocate_Grid_Row3_Column5_01.png", BoxSelection{BoxSelectionMode::Relocate, BoxSelectionLocation::Grid, 3, 5});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionHand_Relocate_Grid_Row4_Column4_01.png", BoxSelection{BoxSelectionMode::Relocate, BoxSelectionLocation::Grid, 4, 4});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionHand_Relocate_Grid_Row3_Column4_01.png", BoxSelection{BoxSelectionMode::Relocate, BoxSelectionLocation::Grid, 3, 4});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionHand_Relocate_Grid_Row2_Column3_01.png", BoxSelection{BoxSelectionMode::Relocate, BoxSelectionLocation::Grid, 2, 3});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionHand_Relocate_Grid_Row1_Column3_01.png", BoxSelection{BoxSelectionMode::Relocate, BoxSelectionLocation::Grid, 1, 3});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionHand_Relocate_Grid_Row0_Column0_02.png", BoxSelection{BoxSelectionMode::Relocate, BoxSelectionLocation::Grid, 0, 0});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionHand_Relocate_Grid_Row0_Column0_03.png", BoxSelection{BoxSelectionMode::Relocate, BoxSelectionLocation::Grid, 0, 0});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionArrow_Relocate_Grid_Row4_Column0_01.png", BoxSelection{BoxSelectionMode::Relocate, BoxSelectionLocation::Grid, 4, 0});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionHand_Relocate_Grid_Row4_Column0_03.png", BoxSelection{BoxSelectionMode::Relocate, BoxSelectionLocation::Grid, 4, 0});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionHand_Relocate_Grid_Row4_Column0_04.png", BoxSelection{BoxSelectionMode::Relocate, BoxSelectionLocation::Grid, 4, 0});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionHand_Relocate_Grid_Row4_Column1_01.png", BoxSelection{BoxSelectionMode::Relocate, BoxSelectionLocation::Grid, 4, 1});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionHand_Relocate_Grid_Row2_Column1_02.png", BoxSelection{BoxSelectionMode::Relocate, BoxSelectionLocation::Grid, 2, 1});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionHand_Relocate_Grid_Row2_Column1_03.png", BoxSelection{BoxSelectionMode::Relocate, BoxSelectionLocation::Grid, 2, 1});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionHand_Relocate_Grid_Row1_Column2_01.png", BoxSelection{BoxSelectionMode::Relocate, BoxSelectionLocation::Grid, 1, 2});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionArrow_Relocate_Grid_Row2_Column2_01.png", BoxSelection{BoxSelectionMode::Relocate, BoxSelectionLocation::Grid, 2, 2});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionHand_Multi_Grid_Row2_Column4_05.png", BoxSelection{BoxSelectionMode::Multi, BoxSelectionLocation::Grid, 2, 4});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionHand_Multi_Grid_Row1_Column4_01.png", BoxSelection{BoxSelectionMode::Multi, BoxSelectionLocation::Grid, 1, 4});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionHand_Multi_Grid_Row1_Column4_02.png", BoxSelection{BoxSelectionMode::Multi, BoxSelectionLocation::Grid, 1, 4});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionHand_Multi_Grid_Row1_Column4_03.png", BoxSelection{BoxSelectionMode::Multi, BoxSelectionLocation::Grid, 1, 4});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionArrow_Multi_Grid_Row2_Column4_01.png", BoxSelection{BoxSelectionMode::Multi, BoxSelectionLocation::Grid, 2, 4});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionArrow_Multi_Grid_Row2_Column5_01.png", BoxSelection{BoxSelectionMode::Multi, BoxSelectionLocation::Grid, 2, 5});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionHand_Normal_Grid_Row2_Column5_01.png", BoxSelection{BoxSelectionMode::Normal, BoxSelectionLocation::Grid, 2, 5});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionHand_Normal_Grid_Row3_Column1_02.png", BoxSelection{BoxSelectionMode::Normal, BoxSelectionLocation::Grid, 3, 1});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionHand_Normal_Grid_Row4_Column2_01.png", BoxSelection{BoxSelectionMode::Normal, BoxSelectionLocation::Grid, 4, 2});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionHand_Normal_Grid_Row4_Column2_02.png", BoxSelection{BoxSelectionMode::Normal, BoxSelectionLocation::Grid, 4, 2});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionHand_Normal_Grid_Row1_Column2_01.png", BoxSelection{BoxSelectionMode::Normal, BoxSelectionLocation::Grid, 1, 2});
    database.add<Test_SelectionArrowDetector>("PokemonHome/BoxView/SelectionArrow_Normal_Grid_Row3_Column2_01.png", BoxSelection{BoxSelectionMode::Normal, BoxSelectionLocation::Grid, 3, 2});
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/annihilape_Regular.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/bidoof_Regular.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/bulbasaur_Regular.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/bulbasuar_Shiny_Go.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/bulbasuar_Shiny_Lza.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/capskid_Regular.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/castform_Regular.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/cyclizar_Regular.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/dudunsparce_Regular.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/dudunsparce_Regular_Sv.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/enamorus_Shiny.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/gimmighoul_Regular.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/glimmet_Regular.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/gogoat_Regular.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/greatTusk_Shiny.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/hatterne_Regular.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/houndstone_Regular.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/ironBunde_Regular.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/ironBundle_Regular_Sv.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/ironJugulis_Regular.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/ironThorns_Regular.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/kilowattrel_Regular.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/kingler_Shiny.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/komala_Regular.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/krabby_Shiny.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/machamp_Regular.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/pancham_Shiny.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/rapidash_Regular.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/rellor_Regular.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/riolu_Regular.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/rowlet_ShinyAlpha.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/scovillain_Regular.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/slitherWing_Shiny.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/squirtle_Shiny.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/tapuLele_Shiny.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/tatsugiri_Regular.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/teddiursa_Regular.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/terapagos_regular.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/vulpix_Regular.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/vulpix_Shiny.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/wartortle_Regular.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/wurmple_Regular.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/GBA-Origin-ElgatoHD60Pro-NV12.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/GBA-Origin-ElgatoHD60Pro-YUYV.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/Farfetchd-fast-ball.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/Machamp-gigaton-ball.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/Machoke-gigaton-ball.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/Machop-gigaton-ball.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/Magnemite-gigaton-ball.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/Magneton-gigaton-ball.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/Slowpoke-fast-ball.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/absol_Regular_none_1792424917318.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/arrokuda_Regular_none_1792424917770.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/azelf_Regular_none_1792424918264.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/bisharp_Regular_none_1792424918722.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/budew_Regular_none_1792424919162.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/buneary_Regular_none_1792424919615.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/bunnelby_Regular_none_1792424920096.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/buzzwole_Regular_none_1792424920562.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/calyrex_Regular_none_1792424921051.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/centiskorch_Regular_none_1792424921522.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/charjabug_Regular_none_1792424921973.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/charmander_Regular_none_1792424922423.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/cherubi_Regular_none_1792424922878.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/cottonee_Regular_none_1792424923397.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/cottonee_Regular_none_1792424923843.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/cramorant_Regular_none_1792424924277.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/cubone_Regular_none_1792424924758.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/delibird_Regular_none_1792424925212.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/dracovish_Regular_none_1792424925673.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/dreepy_Regular_none_1792424926124.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/dusclops_Regular_none_1792424926621.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/electrike_Regular_none_1792424927083.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/electrike_Regular_none_1792424927528.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/eternatus_Regular_Shiny_none_1792424927959.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/eternatus_Regular_none_1792424928413.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/ferroseed_Regular_none_1792424928886.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/fletchling_Regular_none_1792424929346.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/gastly_Regular_none_1792424929814.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/gible_Regular_none_1792424930272.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/gible_Regular_none_1792424930703.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/golurk_Regular_none_1792424931148.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/golurk_Regular_none_1792424931632.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/goodra_Regular_none_1792424932092.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/grookey_Regular_none_1792424932537.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/gurdurr_Regular_none_1792424932986.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/gyarados_Regular_none_1792424933448.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/hattrem_Regular_none_1792424933893.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/hattrem_Regular_none_1792424934353.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/haunter_Regular_none_1792424934828.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/honedge_Regular_none_1792424935285.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/jellicent_Regular_none_1792424935736.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/keldeo_Regular_Shiny_none_1792424936178.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/linoone_Regular_none_1792424936621.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/litten_Regular_none_1792424937059.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/litten_Regular_none_1792424937516.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/machoke_Regular_none_1792424937960.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/machoke_Regular_none_1792424938413.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/machop_Regular_none_1792424938862.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/magearna_Regular_none_1792424939321.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/magikarp_Regular_none_1792424939790.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/melmetal_Regular_none_1792424940261.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/melmetal_Regular_none_1792424940711.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/meowth_Regular_none_1792424941158.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/meowth_Regular_none_1792424941611.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/metapod_Regular_none_1792424942051.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/milcery_Regular_none_1792424942492.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/mime-jr_Regular_none_1792424942964.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/mudkip_Regular_none_1792424943412.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/nidoking_Regular_none_1792424943853.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/nidoran-f_Regular_none_1792424944297.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/nidoran-m_Regular_none_1792424944756.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/ninjask_Regular_none_1792424945199.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/noibat_Regular_none_1792424945647.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/orbeetle_Regular_none_1792424946083.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/pangoro_Regular_none_1792424946529.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/pikachu_Regular_none_1792424946978.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/poliwhirl_Regular_none_1792424947416.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/pumpkaboo_Regular_none_1792424947882.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/purrloin_Regular_none_1792424948321.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/ralts_Regular_none_1792424948769.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/rapidash_Regular_none_1792424949229.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/remoraid_Regular_none_1792424949680.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/rhydon_Regular_none_1792424950146.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/rookidee_Regular_none_1792424950592.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/rotom_Regular_none_1792424951021.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/sandaconda_Regular_none_1792424951459.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/sandslash_Regular_none_1792424951934.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/scorbunny_Regular_none_1792424952390.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/seismitoad_Regular_none_1792424952847.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/sharpedo_Regular_none_1792424953289.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/shellos_Regular_none_1792424953738.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/shiinotic_Regular_none_1792424954189.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/sizzlipede_Regular_none_1792424954668.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/sizzlipede_Regular_none_1792424955123.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/sizzlipede_Regular_none_1792424955582.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/skwovet_Regular_none_1792424956033.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/slowpoke_Regular_none_1792424956466.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/snover_Regular_none_1792424956935.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/sobble_Regular_none_1792424957377.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/stunfisk_Regular_none_1792424957817.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/stunfisk_Regular_none_1792424958276.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/tapu-bulu_Regular_none_1792424958716.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/tapu-lele_Regular_none_1792424959172.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/timburr_Regular_none_1792424959640.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/timburr_Regular_none_1792424960105.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/timburr_Regular_none_1792424960552.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/timburr_Regular_none_1792424960992.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/timburr_Regular_none_1792424961449.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/toxapex_Regular_none_1792424961906.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/toxtricity_Regular_none_1792424962375.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/tranquill_Regular_none_1792424962829.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/tympole_Regular_none_1792424963266.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/vanillish_Regular_none_1792424963705.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/vanillish_Regular_none_1792424964155.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/vulpix_Regular_none_1792424964627.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/vulpix_Regular_none_1792424965074.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/wishiwashi_Regular_none_1792424965509.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/wobbuffet_Regular_none_1792424965947.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/wynaut_Regular_none_1792424966405.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/zarude_Regular_none_1792424966858.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/zarude_Regular_none_1792424967363.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/zigzagoon_Regular_none_1792424967861.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/zigzagoon_Regular_none_1792424968351.png", std::nullopt);
}

}
}
}