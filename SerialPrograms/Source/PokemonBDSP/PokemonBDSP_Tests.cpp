/*  Pokemon BDSP Tests
 *
 *  From: https://github.com/PokemonAutomation/
 */

#include <array>
#include <map>
#include <limits>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
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
        return run_bag_screenshot_tests(logger, RESOURCE_PATH() + "PokemonBDSP/itembagtests");
    }
};

}

bool run_bag_screenshot_tests(Logger& logger, const std::string& folder){
    const std::string prefix = folder + "/";
    QFile file(QString::fromStdString(prefix + "bag_tests.json"));
    if (!file.open(QIODevice::ReadOnly)){
        logger.log("[Bag tests] Cannot open " + prefix + "bag_tests.json", COLOR_RED);
        return false;
    }
    QJsonParseError error;
    const auto doc = QJsonDocument::fromJson(file.readAll(), &error);
    const auto manifest = doc.object();
    const auto template_json = manifest["templates"].toArray();
    const auto cases = manifest["cases"].toArray();
    if (error.error != QJsonParseError::NoError || template_json.isEmpty() || cases.isEmpty()){
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
        const auto entry = value.toObject();
        TestItemTemplate t;
        t.item.label = entry["label"].toString().toStdString();
        t.item.reference_row = entry["row"].toInt();
        const auto name = entry["image"].toString().toStdString();
        if (t.item.label.empty() || t.item.reference_row < 1 || t.item.reference_row > 7 ||
            !QFileInfo::exists(QString::fromStdString(prefix + name))){
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
        const auto test = case_value.toObject();
        const auto filename = test["image"].toString().toStdString();
        const auto expected = test["items"].toArray();
        const int expected_cursor = test["cursor"].toInt(-1);
        if (expected.size() != 7 || expected_cursor < 0 || expected_cursor > 7 ||
            !QFileInfo::exists(QString::fromStdString(prefix + filename))){
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
            check(labels[row] == expected[row].toString().toStdString(),
                filename + ": row " + std::to_string(row + 1) + " = " +
                (labels[row].empty() ? "Empty" : labels[row]) + ", expected " +
                (expected[row].toString().isEmpty() ? "Empty" : expected[row].toString().toStdString()));
        }
        for (const auto& query_value : test["queries"].toArray()){
            const auto query = query_value.toObject();
            const auto wanted = query["item"].toString().toStdString();
            int item_row = 0;
            for (size_t row = 0; row < 7; row++){
                if (labels[row] == wanted){
                    item_row = static_cast<int>(row + 1);
                }
            }
            const auto plan = plan_bag_item_move(cursor, item_row);
            check(action_name(plan.action) == query["action"].toString().toStdString() &&
                plan.presses == static_cast<size_t>(query["presses"].toInt(-1)),
                filename + ": looking for " + wanted + ", item row " + std::to_string(item_row) +
                ", cursor row " + std::to_string(cursor) + ": " + action_name(plan.action) +
                " " + std::to_string(plan.presses) + " (no input sent)");
        }
    }
    // Optional pocket fixtures extend the same offline/dev-mode suite.
    QFile pocket_file(QString::fromStdString(prefix + "pocket_tests.json"));
    if (pocket_file.exists()){
        if (!pocket_file.open(QIODevice::ReadOnly)){
            check(false, "Cannot open pocket test manifest");
        }else{
            QJsonParseError pocket_error;
            const auto pocket_doc = QJsonDocument::fromJson(pocket_file.readAll(), &pocket_error);
            const auto pocket_cases = pocket_doc.object()["cases"].toArray();
            check(pocket_error.error == QJsonParseError::NoError && !pocket_cases.isEmpty(),
                "Pocket test manifest is valid");
            for (const auto& value : pocket_cases){
                const auto entry = value.toObject();
                const auto name = entry["image"].toString().toStdString();
                const int expected_pocket = entry["pocket"].toInt(-1);
                if (expected_pocket < 0 || expected_pocket > 8 || !entry["empty"].isBool() ||
                    !QFileInfo::exists(QString::fromStdString(prefix + name))){
                    check(false, "Invalid or missing pocket case: " + name);
                    continue;
                }
                const auto image = load_image(name);
                for (size_t width : {1920, 1280, 960}){
                    const auto scaled = image->scale_to(width, width * 9 / 16);
                    check(bag_pocket_position(scaled) == expected_pocket &&
                        bag_list_is_empty(scaled) == entry["empty"].toBool(),
                        name + ": pocket " + std::to_string(expected_pocket) +
                        ", empty=" + (entry["empty"].toBool() ? "true" : "false") +
                        " at width " + std::to_string(width));
                }
            }
        }
    }
    check(bag_pocket_position(blank) == 0 && !bag_list_is_empty(blank),
        "Blank screenshot has no pocket and is not an empty Bag");
    check(bag_pocket_position(ImageRGB32()) == 0 && !bag_list_is_empty(ImageRGB32()),
        "Missing screenshot has no pocket and is not an empty Bag");
    logger.log("[Bag tests] " + std::to_string(passed) + " passed, " + std::to_string(failed) + " failed.",
        failed == 0 ? COLOR_GREEN : COLOR_RED);
    return failed == 0;
}

void add_tests(UnitTestDatabase& database){
    database.add<Test_BagScreenshots>();
    add_tests_MenuDetector(database);
}

}
}
}
