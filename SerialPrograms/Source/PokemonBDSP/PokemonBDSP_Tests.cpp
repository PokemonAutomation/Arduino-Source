/*  Pokemon BDSP Tests
 *
 *  From: https://github.com/PokemonAutomation/
 */

#include "CommonFramework/GlobalAutoPaths.h"
#include <array>
#include <map>
#include <limits>
#include "Common/Cpp/Filesystem/Filesystem.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "Common/Cpp/Json/JsonArray.h"
#include "Common/Cpp/Color.h"
#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "Programs/AutoStory/PokemonBDSP_AutoStoryTools.h"
#include "PokemonBDSP_Tests.h"
#include "Inference/PokemonBDSP_MenuDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{
namespace{

std::string action_name(BagMoveAction action){
    switch (action){
    case BagMoveAction::CONFIRM: return "Confirm";
    case BagMoveAction::UP: return "Up";
    case BagMoveAction::DOWN: return "Down";
    case BagMoveAction::PAGE_DOWN: return "Page down";
    case BagMoveAction::REWIND: return "Rewind";
    default: return "Unknown";
    }
}

// A name ROI with no dark text is an empty row, not an unknown item.
bool empty_bag_row(const ImageViewRGB32& image, size_t row){
    const auto text = extract_box_reference(image, ImageFloatBox(0.492, 0.213 + (row - 1) * 0.075, 0.335, 0.052));
    size_t dark = 0;
    for (size_t y = 0; y < text.height(); y++){
        for (size_t x = 0; x < text.width(); x++){
            uint32_t pixel = text.pixel(x, y);
            dark += ((pixel >> 16) & 255) < 100 && ((pixel >> 8) & 255) < 100 && (pixel & 255) < 100;
        }
    }
    return dark < text.width() * text.height() * 0.001;
}

struct TestItemTemplate{
    BagItemTemplate item;
    std::shared_ptr<const ImageRGB32> reference;
};

class Test_BagScreenshots : public UnitTest{
public:
    Test_BagScreenshots() : UnitTest("PokemonBDSP::Bag screenshot tests"){}
    UnitTestResult run(Logger& logger, CancellableScope&) const override{
        return run_bag_screenshot_tests(logger, UNIT_TEST_RESOURCE_PATH() + "PokemonBDSP/itembagtests");
    }
};

}

bool run_bag_screenshot_tests(Logger& logger, const std::string& folder){
    try{
        const std::string prefix = folder + "/";
        const auto doc = load_json_file(prefix + "bag_tests.json");
        const auto& manifest = doc.to_object_throw();
        const auto& template_json = manifest.get_array_throw("templates");
        const auto& cases = manifest.get_array_throw("cases");
        if (template_json.empty() || cases.empty()){
            logger.log("[Bag tests] Invalid or empty test manifest.", COLOR_RED);
            return false;
        }
        std::map<std::string, std::shared_ptr<const ImageRGB32>> images;
        auto load_image = [&](const std::string& name){
            auto it = images.find(name);
            if (it == images.end()){
                it = images.emplace(name, std::make_shared<const ImageRGB32>(prefix + name)).first;
            }
            return it->second;
        };
        std::vector<TestItemTemplate> templates;
        for (const auto& value : template_json){
            const auto& entry = value.to_object_throw();
            TestItemTemplate t;
            t.item.label = entry.get_string_throw("label");
            const int64_t reference_row = entry.get_integer_throw("row");
            t.item.reference_row = reference_row >= 1 && reference_row <= 7 ? static_cast<int>(reference_row) : 0;
            const auto name = entry.get_string_throw("image");
            if (t.item.label.empty() || t.item.reference_row < 1 || t.item.reference_row > 7 ||
                !Filesystem::is_regular_file(prefix + name)){
                logger.log("[Bag tests] Invalid or missing item reference: " + name, COLOR_RED);
                return false;
            }
            t.reference = load_image(name);
            templates.emplace_back(std::move(t));
        }

        size_t passed = 0;
        size_t failed = 0;
        auto check = [&](bool result, const std::string& detail){
            (result ? passed : failed)++;
            logger.log(std::string(result ? "[PASS] " : "[FAIL] ") + detail, result ? COLOR_GREEN : COLOR_RED);
        };
        // All 64 routes, including wraparound and Right for opposite pockets.
        for (int current = 1; current <= 8; current++){
            for (int target = 1; target <= 8; target++){
                const auto plan = plan_bag_pocket_move(current, target);
                const int delta = target - current;
                const int distance = std::abs(delta);
                const size_t expected = static_cast<size_t>(std::min(distance, 8 - distance));
                const auto direction = distance == 0 ? BagPocketAction::CONFIRM :
                    distance == 4 || (delta > 0 && distance < 4) || (delta < 0 && distance > 4)
                    ? BagPocketAction::RIGHT : BagPocketAction::LEFT;
                check(plan.presses == expected && plan.action == direction,
                    "Pocket route " + std::to_string(current) + " to " + std::to_string(target));
            }
        }
        check(plan_bag_pocket_move(4, 5).action == BagPocketAction::RIGHT &&
            plan_bag_pocket_move(4, 5).presses == 1, "Dropped press: recalculate from pocket 4 to 5");
        check(plan_bag_pocket_move(0, 5).action == BagPocketAction::UNKNOWN &&
            plan_bag_pocket_move(1, 9).action == BagPocketAction::UNKNOWN,
            "Unknown or invalid pocket gives no movement");
        // Independent expected values, including paging and unknown-cursor handling.
        const std::array<size_t, 7> pages{13, 11, 10, 9, 8, 7, 1};
        for (int cursor = 1; cursor <= 7; cursor++){
            const auto plan = plan_bag_item_move(cursor, 0);
            check(plan.presses == pages[cursor - 1] && plan.action ==
                (cursor == 7 ? BagMoveAction::REWIND : BagMoveAction::PAGE_DOWN),
                "Absent item, cursor " + std::to_string(cursor) + ": " + action_name(plan.action) +
                " " + std::to_string(plan.presses));
        }
        check(plan_bag_item_move(0, 3).action == BagMoveAction::UNKNOWN, "Unknown cursor gives no movement");
        check(plan_bag_item_move(2, 8).action == BagMoveAction::UNKNOWN, "Invalid item row gives no movement");
        check(bag_cursor_position(ImageRGB32()) == 0, "Missing image gives unknown cursor");

        // Exact expected displacements for every cursor/target combination.
        const int displacement[7][7] = {
            { 0,  1,  2,  3,  4,  5,  6},
            {-1,  0,  1,  2,  3,  4,  5},
            {-2, -1,  0,  1,  2,  3,  4},
            {-3, -2, -1,  0,  1,  2,  3},
            {-4, -3, -2, -1,  0,  1,  2},
            {-5, -4, -3, -2, -1,  0,  1},
            {-6, -5, -4, -3, -2, -1,  0}
        };
        for (int cursor = 1; cursor <= 7; cursor++){
            for (int target = 1; target <= 7; target++){
                const int delta = displacement[cursor - 1][target - 1];
                const auto plan = plan_bag_item_move(cursor, target);
                check(plan.action == (delta == 0 ? BagMoveAction::CONFIRM :
                    delta < 0 ? BagMoveAction::UP : BagMoveAction::DOWN) &&
                    plan.presses == static_cast<size_t>(delta < 0 ? -delta : delta),
                    "Cursor " + std::to_string(cursor) + " to item " + std::to_string(target) +
                    ": " + action_name(plan.action) + " " + std::to_string(plan.presses));
            }
        }
        ImageRGB32 blank(1920, 1080);
        for (size_t y = 0; y < blank.height(); y++){
            for (size_t x = 0; x < blank.width(); x++){
                blank.pixel(x, y) = 0xffffffff;
            }
        }
        check(bag_cursor_position(blank) == 0, "Blank screenshot has no cursor");
        for (const auto& t : templates){
            const auto matches = match_bag_item_rows(blank, *t.reference, t.item);
            bool found = false;
            for (const auto& m : matches){
                found |= m.matched;
            }
            check(!found, "Blank screenshot does not match " + t.item.label);
        }

        for (const auto& case_value : cases){
            const auto& test = case_value.to_object_throw();
            const auto filename = test.get_string_throw("image");
            const auto& expected = test.get_array_throw("items");
            const int64_t expected_cursor = test.get_integer_throw("cursor");
            if (expected.size() != 7 || expected_cursor < 0 || expected_cursor > 7 ||
                !Filesystem::is_regular_file(prefix + filename)){
                check(false, "Invalid or missing screenshot case: " + filename);
                continue;
            }
            const auto image = load_image(filename);
            const int cursor = bag_cursor_position(*image);
            check(cursor == expected_cursor, filename + ": cursor " + std::to_string(cursor) +
                ", expected " + std::to_string(expected_cursor));
            // Scale the same screen to the minimum supported capture resolution.
            const auto small = image->scale_to(1280, 720);
            check(bag_cursor_position(small) == expected_cursor, filename + ": cursor at 720p");

            std::array<std::string, 7> labels;
            std::array<double, 7> best;
            std::array<double, 7> second;
            best.fill(std::numeric_limits<double>::infinity());
            second = best;
            for (const auto& t : templates){
                const auto matches = match_bag_item_rows(*image, *t.reference, t.item);
                for (size_t row = 0; row < 7; row++){
                    const auto& m = matches[row];
                    if (!m.matched){
                        continue;
                    }
                    const double score = m.icon_rmsd + m.name_rmsd;
                    if (score < best[row]){
                        second[row] = best[row];
                        best[row] = score;
                        labels[row] = t.item.label;
                    }else if (score < second[row]){
                        second[row] = score;
                    }
                }
            }
            for (size_t row = 0; row < 7; row++){
                if (labels[row].empty()){
                    labels[row] = empty_bag_row(*image, row + 1) ? "" : "Unknown";
                }else if (second[row] - best[row] < 2.0){
                    labels[row] = "Unknown";
                }
                check(labels[row] == expected[row].to_string_throw(),
                    filename + ": row " + std::to_string(row + 1) + " = " +
                    (labels[row].empty() ? "Empty" : labels[row]) + ", expected " +
                    (expected[row].to_string_throw().empty() ? "Empty" : expected[row].to_string_throw()));
            }
            for (const auto& query_value : test.get_array_throw("queries")){
                const auto& query = query_value.to_object_throw();
                const auto wanted = query.get_string_throw("item");
                int item_row = 0;
                for (size_t row = 0; row < 7; row++){
                    if (labels[row] == wanted){
                        item_row = static_cast<int>(row + 1);
                    }
                }
                const auto plan = plan_bag_item_move(cursor, item_row);
                check(action_name(plan.action) == query.get_string_throw("action") &&
                    plan.presses == static_cast<size_t>(query.get_integer_throw("presses")),
                    filename + ": looking for " + wanted + ", item row " + std::to_string(item_row) +
                    ", cursor row " + std::to_string(cursor) + ": " + action_name(plan.action) +
                    " " + std::to_string(plan.presses) + " (no input sent)");
            }
        }
        // Pocket fixtures are part of this suite and must be present.
        const auto pocket_doc = load_json_file(prefix + "pocket_tests.json");
        const auto& pocket_cases = pocket_doc.to_object_throw().get_array_throw("cases");
        check(!pocket_cases.empty(), "Pocket test manifest is nonempty");
        for (const auto& value : pocket_cases){
            const auto& entry = value.to_object_throw();
            const auto& name = entry.get_string_throw("image");
            const int64_t expected_pocket = entry.get_integer_throw("pocket");
            const bool expected_empty = entry.get_boolean_throw("empty");
            if (expected_pocket < 0 || expected_pocket > 8 ||
                !Filesystem::is_regular_file(prefix + name)){
                check(false, "Invalid or missing pocket case: " + name);
                continue;
            }
            const auto image = load_image(name);
            for (size_t width : {size_t(1920), size_t(1280), size_t(960)}){
                const auto scaled = image->scale_to(width, width * 9 / 16);
                check(bag_pocket_position(scaled) == expected_pocket &&
                    bag_list_is_empty(scaled) == expected_empty,
                    name + ": pocket " + std::to_string(expected_pocket) +
                    ", empty=" + (expected_empty ? "true" : "false") +
                    " at width " + std::to_string(width));
            }
        }
        check(bag_pocket_position(blank) == 0 && !bag_list_is_empty(blank),
            "Blank screenshot has no pocket and is not an empty Bag");
        check(bag_pocket_position(ImageRGB32()) == 0 && !bag_list_is_empty(ImageRGB32()),
            "Missing screenshot has no pocket and is not an empty Bag");
        logger.log("[Bag tests] " + std::to_string(passed) + " passed, " + std::to_string(failed) + " failed.",
            failed == 0 ? COLOR_GREEN : COLOR_RED);
        return failed == 0;
    }catch (const ParseException& error){
        logger.log("[Bag tests] " + error.to_str(), COLOR_RED);
        return false;
    }catch (const FileException& error){
        logger.log("[Bag tests] " + error.to_str(), COLOR_RED);
        return false;
    }catch (const std::exception& error){
        logger.log("[Bag tests] " + std::string(error.what()), COLOR_RED);
        return false;
    }
}

void add_tests(UnitTestDatabase& database){
    database.add<Test_BagScreenshots>();
    add_tests_MenuDetector(database);
}

}
}
}
