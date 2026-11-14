/*  Pokemon BDSP Tests
 *
 *  From: https://github.com/PokemonAutomation/
 */

#include "CommonFramework/GlobalAutoPaths.h"
#include <array>
#include <cmath>
#include <map>
#include <limits>
#include "Common/Cpp/Filesystem/Filesystem.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "Common/Cpp/Json/JsonArray.h"
#include "Common/Cpp/Color.h"
#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
#include "CommonFramework/Globals.h"
#include "CommonTools/VisualDetectors/ImageMatchDetector.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "Programs/AutoStory/PokemonBDSP_AutoStoryTools.h"
#include "Programs/AutoStory/Utils/PokemonBDSP_AutoStory_Battle.h"
#include "Programs/AutoStory/Utils/PokemonBDSP_OverworldMatchers.h"
#include "PokemonBDSP_Tests.h"
#include "Inference/Battles/PokemonBDSP_BattleMenuDetector.h"
#include "Inference/PokemonBDSP_MenuDetector.h"
#include "Inference/PokemonBDSP_OptionsDetector.h"
#include "Inference/PokemonBDSP_ShopItemNameReader.h"
#include "Inference/PokemonBDSP_TownBannerDetector.h"
#include "Tests/ImageUnitTest.h"

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

class Test_Route215AceAlignment : public ImageUnitTest{
public:
    Test_Route215AceAlignment(const std::string& name, bool located, bool at_target)
        : ImageUnitTest("PokemonBDSP::Route215AceAlignment/" + name,
            {"PokemonBDSP/Route215AceAlignment/" + name})
        , m_located(located)
        , m_at_target(at_target)
    {
        m_memory += MAX_TEST_IMAGE_RGB32_BYTES;
    }

    UnitTestResult run_on_images(Logger&, CancellableScope&, const std::vector<ImageRGB32>& images) const override{
        if (images.empty() || !images[0]){
            return false;
        }
        for (size_t width : {size_t(1920), size_t(1280)}){
            const auto result = match_route215_ace_renewal(images[0].scale_to(width, width * 9 / 16));
            if (result.located != m_located || result.at_target != m_at_target){
                return false;
            }
        }
        return true;
    }

private:
    bool m_located;
    bool m_at_target;
};

class Test_BattleMoveMenu : public ImageUnitTest{
public:
    Test_BattleMoveMenu(const std::string& name, size_t expected,
        std::array<int, 4> expected_pp = {-1, -1, -1, -1})
        : ImageUnitTest("PokemonBDSP::BattleMoveMenu/" + name,
            {"PokemonBDSP/BattleMoveMenu/" + name})
        , m_expected(expected)
        , m_expected_pp(expected_pp)
    {
        m_memory += MAX_TEST_IMAGE_RGB32_BYTES;
    }

    UnitTestResult run_on_images(Logger& logger, CancellableScope&, const std::vector<ImageRGB32>& images) const override{
        if (images.empty() || !images[0]){
            return false;
        }
        for (size_t width : {size_t(1920), size_t(1280), size_t(960)}){
            const ImageRGB32 scaled = images[0].scale_to(width, width * 9 / 16);
            if (detect_battle_move_menu(scaled).move_count != m_expected ||
                read_all_pp(logger, scaled) != m_expected_pp){
                return false;
            }
        }
        return true;
    }

private:
    size_t m_expected;
    std::array<int, 4> m_expected_pp;
};

class Test_PlayerHPRatio : public ImageUnitTest{
public:
    Test_PlayerHPRatio(const std::string& name, double expected,
        int current = -1, int maximum = -1, BattleHPLayout layout = BattleHPLayout::SINGLES, size_t slot = 1)
        : ImageUnitTest("PokemonBDSP::PlayerHPRatio/" + name,
            {"PokemonBDSP/PlayerHPRatio/" + name})
        , m_expected(expected)
        , m_current(current)
        , m_maximum(maximum)
        , m_layout(layout)
        , m_slot(slot)
    {
        m_memory += MAX_TEST_IMAGE_RGB32_BYTES;
    }

    UnitTestResult run_on_images(Logger&, CancellableScope&, const std::vector<ImageRGB32>& images) const override{
        if (images.empty() || !images[0]){
            return false;
        }
        for (size_t width : {size_t(1920), size_t(1280), size_t(960)}){
            const ImageRGB32 scaled = images[0].scale_to(width, width * 9 / 16);
            BattleHPReading reading;
            const double observed = read_player_hp_ratio(scaled, m_layout, m_slot, &reading);
            if (m_current >= 0 && (reading.current != m_current || reading.maximum != m_maximum)){
                return false;
            }
            if (!std::isfinite(observed) || std::abs(observed - m_expected) > 0.02){
                return false;
            }
        }
        return true;
    }

private:
    double m_expected;
    int m_current;
    int m_maximum;
    BattleHPLayout m_layout;
    size_t m_slot;
};

class Test_BagPartyTarget : public ImageUnitTest{
public:
    Test_BagPartyTarget(const std::string& name, int expected)
        : ImageUnitTest("PokemonBDSP::BagPartyTarget/" + name,
            {"PokemonBDSP/BagPartyTarget/" + name})
        , m_expected(expected)
    {
        m_memory += MAX_TEST_IMAGE_RGB32_BYTES;
    }

    UnitTestResult run_on_images(Logger&, CancellableScope&, const std::vector<ImageRGB32>& images) const override{
        if (images.empty() || !images[0]){
            return false;
        }
        for (size_t width : {size_t(1920), size_t(1280), size_t(960)}){
            const ImageRGB32 scaled = images[0].scale_to(width, width * 9 / 16);
            if (bag_cursor_position(scaled, true) != m_expected){
                return false;
            }
        }
        return true;
    }

private:
    int m_expected;
};

class Test_TownBanner : public ImageUnitTest{
public:
    Test_TownBanner(const std::string& name)
        : ImageUnitTest("PokemonBDSP::TownBannerDetector/" + name,
            {"PokemonBDSP/TownBannerDetector/" + name})
        , m_expected(name.ends_with("_True.png"))
    {
        m_memory += MAX_TEST_IMAGE_RGB32_BYTES;
    }

    UnitTestResult run_on_images(Logger&, CancellableScope&, const std::vector<ImageRGB32>& images) const override{
        if (images.empty() || !images[0]){
            return false;
        }
        TownBannerDetector detector;
        for (size_t width : {size_t(1920), size_t(1280), size_t(960)}){
            const ImageRGB32 scaled = images[0].scale_to(width, width * 9 / 16);
            if (detector.detect(scaled) != m_expected){
                return false;
            }
        }
        return true;
    }

private:
    bool m_expected;
};

class Test_OptionsOCR : public ImageUnitTest{
public:
    Test_OptionsOCR(const std::string& name, const std::string& row, const std::string& value,
        Language language = Language::English)
        : ImageUnitTest("PokemonBDSP::OptionsOCR/" + name, {"PokemonBDSP/OptionsOCR/" + name})
        , m_row(row)
        , m_value(value)
        , m_language(language)
    {
        m_memory += MAX_TEST_IMAGE_RGB32_BYTES;
    }
    UnitTestResult run_on_images(Logger& logger, CancellableScope&, const std::vector<ImageRGB32>& images) const override{
        if (images.empty() || !images[0]){
            return false;
        }
        TutorialOptionsDetector screen;
        TutorialOptionPositionDetector position;
        for (size_t width : {size_t(1920), size_t(1280), size_t(960)}){
            const auto scaled = images[0].scale_to(width, width * 9 / 16);
            const double y = position.selected_row(scaled);
            if (m_row.empty()){
                if (screen.detect(scaled) && y >= 0){
                    return false;
                }
                continue;
            }
            if (!screen.detect(scaled) || y < 0 ||
                position.read_text(logger, m_language, scaled, y) != m_row ||
                position.read_text(logger, m_language, scaled, y, m_row) != m_value){
                return false;
            }
        }
        return true;
    }
private:
    std::string m_row;
    std::string m_value;
    Language m_language;
};

class Test_TrainersSchoolSign : public ImageUnitTest{
public:
    Test_TrainersSchoolSign(const std::string& name, bool expected, double x = 0, double y = 0, bool at_target = false)
        : ImageUnitTest("PokemonBDSP::TrainersSchoolSign/" + name,
            {"PokemonBDSP/TrainersSchoolSign/" + name})
        , m_expected(expected)
        , m_x(x)
        , m_y(y)
        , m_at_target(at_target)
    {
        m_memory += MAX_TEST_IMAGE_RGB32_BYTES;
    }

    UnitTestResult run_on_images(Logger&, CancellableScope&, const std::vector<ImageRGB32>& images) const override{
        if (images.empty() || !images[0]){
            return false;
        }
        for (size_t width : {size_t(1920), size_t(1280), size_t(960)}){
            const auto scaled = images[0].scale_to(width, width * 9 / 16);
            const auto result = match_jubilife_trainers_school_sign(scaled);
            if (result.located != m_expected || result.at_target != m_at_target || result.coarse){
                return false;
            }
            if (m_expected && (result.landmarks.size() != 1 ||
                std::abs(result.landmarks[0].found.x - m_x) > 0.01 ||
                std::abs(result.landmarks[0].found.y - m_y) > 0.01)){
                return false;
            }
        }
        return true;
    }

private:
    bool m_expected;
    double m_x;
    double m_y;
    bool m_at_target;
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
        // Exercise all resource languages, BDSP categories, and shared Ball/Berry names.
        const struct{
            const char* language;
            const char* name;
            const char* slug;
        } item_names[]{
            {"eng", "Berry Juice", "berry-juice"},
            {"jpn", "プラスパワー", "x-attack"},
            {"spa", "Reproductor DS", "ds-sounds"},
            {"fra", "Guérison", "full-restore"},
            {"deu", "X-Verteidigung", "x-defense"},
            {"ita", "Turbinchiave", "works-key"},
            {"kor", "기력의덩어리", "max-revive"},
            {"chi_sim", "特防强化", "x-sp-def"},
            {"chi_tra", "閃耀護符", "shiny-charm"},
            {"eng", "Escape Rope", "escape-rope"},
            {"eng", "Super Repel", "super-repel"},
            {"eng", "Rare Candy", "rare-candy"},
            {"spa", "Carburante", "carbos"},
            {"deu", "Wahlbrille", "choice-specs"},
            {"fra", "Super Repousse", "super-repel"},
            {"jpn", "きんのたま", "nugget"},
            {"eng", "Dusk Stone", "dusk-stone"},
            {"fra", "Hyper Ball", "ultra-ball"},
            {"kor", "자뭉열매", "sitrus-berry"},
            {"eng", "TM01", "tm01"},
            {"eng", "Focus Punch", "tm01"},
            {"jpn", "わざマシン０６", "tm06"},
            {"jpn", "どくどく", "tm06"},
            {"spa", "MT14", "tm14"},
            {"spa", "Ventisca", "tm14"},
            {"fra", "CT35", "tm35"},
            {"fra", "Lance-Flammes", "tm35"},
            {"deu", "TM37", "tm37"},
            {"deu", "Sandsturm", "tm37"},
            {"ita", "MT42", "tm42"},
            {"ita", "Facciata", "tm42"},
            {"kor", "기술머신43", "tm43"},
            {"kor", "볼트체인지", "tm43"},
            {"chi_sim", "招式学习器９８", "tm98"},
            {"chi_sim", "碎岩", "tm98"},
            {"chi_tra", "招式學習器１００", "tm100"},
            {"chi_tra", "攀岩", "tm100"},
        };
        for (const auto& item : item_names){
            const auto result = ShopItemNameReader::instance().match_substring(
                language_code_to_enum(item.language), item.name);
            check(result.results.size() == 1 && result.results.begin()->second.token == item.slug,
                std::string("Item dictionary: ") + item.language + " / " + item.slug);
        }
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
        auto bag_reference = std::make_shared<const ImageRGB32>(RESOURCE_PATH() + "PokemonBDSP/Autostory/Bag_Potion.png");
        ImageMatchDetector bag(bag_reference, ImageFloatBox(0.365, 0.22, 0.055, 0.10), 35.0, true);
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
                check(expected_pocket == 0 ? !bag.detect(scaled) :
                    bag.detect(scaled) && bag_pocket_position(scaled) == expected_pocket &&
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
        const auto ocr_doc = load_json_file(prefix + "ocr_tests.json");
        const auto& ocr_cases = ocr_doc.to_object_throw().get_array_throw("cases");
        const std::string default_language = ocr_doc.to_object_throw().get_string_default("language", "eng");
        check(!ocr_cases.empty(), "Bag OCR manifest is nonempty");
        for (const auto& value : ocr_cases){
            const auto& entry = value.to_object_throw();
            const auto& name = entry.get_string_throw("image");
            const auto& expected = entry.get_array_throw("items");
            if (expected.size() != 7 || !Filesystem::is_regular_file(prefix + name)){
                check(false, "Invalid or missing Bag OCR case: " + name);
                continue;
            }
            const Language language = language_code_to_enum(entry.get_string_default("language", default_language.c_str()));
            const auto image = load_image(name);
            for (size_t width : {size_t(1920), size_t(1280)}){
                const auto scaled = image->scale_to(width, width * 9 / 16);
                if (const auto* cursor = entry.get_value("cursor")){
                    check(bag_cursor_position(scaled) == cursor->to_integer_throw(),
                        name + ": selected item cursor at width " + std::to_string(width));
                }
                for (size_t row = 0; row < 7; row++){
                    auto result = ShopItemNameReader::instance().read_substring(logger,
                        language, extract_box_reference(scaled, bag_list_name_box(row + 1)));
                    result.clear_beyond_log10p(ShopItemNameReader::MAX_LOG10P);
                    const std::string& wanted = expected[row].to_string_throw();
                    const std::string detected = result.results.size() == 1 ?
                        result.results.begin()->second.token : "";
                    bool matches = detected == wanted;
                    if (wanted == "Unknown"){
                        matches = true;
                        for (const auto& candidate : result.results){
                            matches &= candidate.second.token != "escape-rope";
                        }
                    }
                    check(matches, name + ": OCR row " + std::to_string(row + 1) +
                        " at width " + std::to_string(width) + " = " + detected + ", expected " + wanted);
                }
            }
        }
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
    database.add<Test_Route215AceAlignment>("east-approach.png", true, false);
    database.add<Test_Route215AceAlignment>("north-offset.png", true, false);
    database.add<Test_Route215AceAlignment>("vertical-recovery-gap.png", true, false);
    database.add<Test_Route215AceAlignment>("goal-second-attempt.png", true, true);
    database.add<Test_Route215AceAlignment>("westward-recovery.png", true, false);
    database.add<Test_Route215AceAlignment>("goal-third-attempt.png", true, true);
    database.add<Test_Route215AceAlignment>("negative-bridge.png", false, false);
    database.add<Test_Route215AceAlignment>("negative-derek.png", false, false);
    database.add<Test_Route215AceAlignment>("negative-stair-base.png", false, false);
    database.add<Test_Route215AceAlignment>("negative-stairs.png", false, false);
    database.add<Test_Route215AceAlignment>("negative-stair-top.png", false, false);
    database.add<Test_OptionsOCR>("Spanish_20261112_17-40.png", "textspeed", "normal", Language::Spanish);
    database.add<Test_OptionsOCR>("Spanish_20261112_18-20.png", "textspeed", "fast", Language::Spanish);
    database.add<Test_OptionsOCR>("Spanish_20261112_18-50.png", "changewindow", "type1", Language::Spanish);
    database.add<Test_OptionsOCR>("Spanish_20261112_19-05.png", "battleeffects", "on", Language::Spanish);
    database.add<Test_OptionsOCR>("Spanish_20261112_19-45.png", "battleeffects", "off", Language::Spanish);
    database.add<Test_OptionsOCR>("Spanish_20261112_19-90.png", "battlestyle", "switch", Language::Spanish);
    database.add<Test_OptionsOCR>("Spanish_20261112_20-30.png", "battlestyle", "set", Language::Spanish);
    database.add<Test_OptionsOCR>("Spanish_20261112_20-75.png", "sendtoboxes", "manual", Language::Spanish);
    database.add<Test_OptionsOCR>("Spanish_20261112_21-20.png", "sendtoboxes", "automatic", Language::Spanish);
    database.add<Test_OptionsOCR>("Spanish_20261112_21-65.png", "givenicknames", "give", Language::Spanish);
    database.add<Test_OptionsOCR>("Spanish_20261112_22-10.png", "givenicknames", "dontgive", Language::Spanish);
    database.add<Test_OptionsOCR>("Spanish_20261112_22-55.png", "gyroscope", "use", Language::Spanish);
    database.add<Test_OptionsOCR>("Spanish_20261112_22-95.png", "autosave", "on", Language::Spanish);
    database.add<Test_OptionsOCR>("Spanish_20261112_23-40.png", "autosave", "off", Language::Spanish);
    database.add<Test_OptionsOCR>("Spanish_20261112_24-80.png", "shownicknames", "show", Language::Spanish);
    database.add<Test_OptionsOCR>("Spanish_20261112_25-70.png", "backgroundmusic", "volume8", Language::Spanish);
    database.add<Test_OptionsOCR>("Spanish_20261112_26-30.png", "soundeffects", "volume8", Language::Spanish);
    database.add<Test_OptionsOCR>("Spanish_20261112_41-70.png", "autosave", "off", Language::Spanish);
    database.add<Test_OptionsOCR>("Spanish_20261112_27-05.png", "", "", Language::Spanish);
    database.add<Test_OptionsOCR>("Spanish_20261112_31-00.png", "", "", Language::Spanish);
    database.add<Test_OptionsOCR>("Spanish_20261112_48-60.png", "", "", Language::Spanish);
    database.add<Test_OptionsOCR>("Synthetic_textspeed.png", "textspeed", "fast");
    database.add<Test_OptionsOCR>("Synthetic_changewindow.png", "changewindow", "type1");
    database.add<Test_OptionsOCR>("Synthetic_battleeffects.png", "battleeffects", "off");
    database.add<Test_OptionsOCR>("Synthetic_battlestyle.png", "battlestyle", "set");
    database.add<Test_OptionsOCR>("Synthetic_sendtoboxes.png", "sendtoboxes", "automatic");
    database.add<Test_OptionsOCR>("Synthetic_givenicknames.png", "givenicknames", "dontgive");
    database.add<Test_OptionsOCR>("Synthetic_gyroscope.png", "gyroscope", "use");
    database.add<Test_OptionsOCR>("Synthetic_autosave.png", "autosave", "off");
    database.add<Test_OptionsOCR>("Synthetic_shownicknames.png", "shownicknames", "show");
    database.add<Test_OptionsOCR>("Synthetic_backgroundmusic.png", "backgroundmusic", "volume8");
    database.add<Test_OptionsOCR>("Synthetic_soundeffects.png", "soundeffects", "volume8");
    database.add<Test_OptionsOCR>("Synthetic_AutosaveOn.png", "autosave", "on");
    database.add<Test_OptionsOCR>("Synthetic_AmbiguousToggle.png", "autosave", "");
    database.add<Test_OptionsOCR>("Synthetic_MissingValue.png", "autosave", "");
    database.add<Test_OptionsOCR>("Synthetic_DifferentRow.png", "battleeffects", "off");
    database.add<Test_OptionsOCR>("Synthetic_NoSelection.png", "", "");
    database.add<Test_TrainersSchoolSign>("ManualEast_2744.png", true, 614.0 / 1920, 144.0 / 1080, false);
    database.add<Test_TrainersSchoolSign>("ManualCentered_2746.png", true, 886.0 / 1920, 144.0 / 1080, true);
    database.add<Test_TrainersSchoolSign>("ManualFarSouth_2777.png", true, 884.0 / 1920, 68.0 / 1080, false);
    database.add<Test_TrainersSchoolSign>("ManualFarSouth_2780.png", true, 884.0 / 1920, 68.0 / 1080, false);
    database.add<Test_TrainersSchoolSign>("ManualSouthWest_2818.png", true, 998.0 / 1920, 112.0 / 1080, false);
    database.add<Test_TrainersSchoolSign>("ManualSouthWest_2820.png", true, 998.0 / 1920, 112.0 / 1080, false);
    database.add<Test_TrainersSchoolSign>("ManualWest_2857.png", true, 872.0 / 1920, 156.0 / 1080, true);
    database.add<Test_TrainersSchoolSign>("ManualCentered_2859.png", true, 886.0 / 1920, 156.0 / 1080, true);
    database.add<Test_TrainersSchoolSign>("Interior_2748.png", false);
    database.add<Test_TrainersSchoolSign>("Lighting_00.png", true, 886.0 / 1920, 206.0 / 1080, true);
    database.add<Test_TrainersSchoolSign>("Lighting_01.png", true, 886.0 / 1920, 206.0 / 1080, true);
    database.add<Test_TrainersSchoolSign>("Lighting_02.png", true, 886.0 / 1920, 206.0 / 1080, true);
    database.add<Test_TrainersSchoolSign>("Lighting_03.png", true, 886.0 / 1920, 206.0 / 1080, true);
    database.add<Test_TrainersSchoolSign>("Lighting_04.png", true, 886.0 / 1920, 206.0 / 1080, true);
    database.add<Test_TrainersSchoolSign>("Lighting_05.png", true, 886.0 / 1920, 206.0 / 1080, true);
    database.add<Test_TrainersSchoolSign>("Lighting_06.png", true, 886.0 / 1920, 206.0 / 1080, true);
    database.add<Test_TrainersSchoolSign>("Lighting_07.png", true, 886.0 / 1920, 206.0 / 1080, true);
    database.add<Test_TrainersSchoolSign>("Lighting_08.png", true, 886.0 / 1920, 206.0 / 1080, true);
    database.add<Test_TrainersSchoolSign>("Lighting_09.png", true, 886.0 / 1920, 206.0 / 1080, true);
    database.add<Test_TrainersSchoolSign>("Lighting_10.png", true, 886.0 / 1920, 206.0 / 1080, true);
    database.add<Test_TrainersSchoolSign>("Lighting_11.png", true, 886.0 / 1920, 206.0 / 1080, true);
    database.add<Test_TrainersSchoolSign>("Lighting_12.png", true, 886.0 / 1920, 206.0 / 1080, true);
    database.add<Test_TrainersSchoolSign>("Lighting_13.png", true, 886.0 / 1920, 206.0 / 1080, true);
    database.add<Test_TrainersSchoolSign>("Lighting_14.png", true, 886.0 / 1920, 206.0 / 1080, true);
    database.add<Test_TrainersSchoolSign>("Lighting_15.png", true, 886.0 / 1920, 206.0 / 1080, true);
    database.add<Test_TrainersSchoolSign>("Lighting_16.png", true, 886.0 / 1920, 206.0 / 1080, true);
    database.add<Test_TrainersSchoolSign>("Lighting_17.png", true, 886.0 / 1920, 206.0 / 1080, true);
    database.add<Test_TrainersSchoolSign>("Lighting_18.png", true, 886.0 / 1920, 206.0 / 1080, true);
    database.add<Test_TrainersSchoolSign>("Lighting_19.png", true, 886.0 / 1920, 206.0 / 1080, true);
    database.add<Test_TrainersSchoolSign>("Lighting_20.png", true, 886.0 / 1920, 206.0 / 1080, true);
    database.add<Test_TrainersSchoolSign>("Lighting_21.png", true, 886.0 / 1920, 206.0 / 1080, true);
    database.add<Test_TrainersSchoolSign>("Lighting_22.png", true, 886.0 / 1920, 206.0 / 1080, true);
    database.add<Test_TrainersSchoolSign>("Lighting_23.png", true, 886.0 / 1920, 206.0 / 1080, true);
    database.add<Test_TrainersSchoolSign>("Approach_771.png", true, 191.0 / 480, 41.0 / 270);
    database.add<Test_TrainersSchoolSign>("Wall_823.png", true, 192.0 / 480, 52.0 / 270);
    database.add<Test_TrainersSchoolSign>("Wall_930.png", true, 192.0 / 480, 53.0 / 270);
    database.add<Test_TrainersSchoolSign>("Dawn_745.png", false);
    database.add<Test_TrainersSchoolSign>("Save_758.png", false);
    database.add<Test_TrainersSchoolSign>("Street_765.png", false);
    database.add<Test_TrainersSchoolSign>("Bag.png", false);
    database.add<Test_BagScreenshots>();
    database.add<Test_PlayerHPRatio>("Cynthia_3818_NoCommands.png", -1.0);
    database.add<Test_PlayerHPRatio>("Cynthia_3824_NoCommands.png", -1.0);
    database.add<Test_PlayerHPRatio>("Cynthia_3828_NoCommands.png", -1.0);
    database.add<Test_PlayerHPRatio>("Cynthia_3830_FullHP.png", 1.0);
    database.add<Test_PlayerHPRatio>("Cynthia_3831_FullHP.png", 1.0);
    database.add<Test_PlayerHPRatio>("Cynthia_3832_FullHP.png", 1.0);
    database.add<Test_PlayerHPRatio>("Cynthia_3833_FullHP.png", 1.0);
    database.add<Test_PlayerHPRatio>("Cynthia_3925_FullHP.png", 1.0);
    database.add<Test_PlayerHPRatio>("Cynthia_5642_FullHP.png", 1.0);
    database.add<Test_PlayerHPRatio>("Bertha_275.png", 275.0 / 293, 275, 293, BattleHPLayout::SINGLES, 1);
    database.add<Test_PlayerHPRatio>("Bertha_239.png", 239.0 / 293, 239, 293, BattleHPLayout::SINGLES, 1);
    database.add<Test_PlayerHPRatio>("Bertha_113.png", 113.0 / 293, 113, 293, BattleHPLayout::SINGLES, 1);
    database.add<Test_PlayerHPRatio>("Bertha_23.png", 23.0 / 293, 23, 293, BattleHPLayout::SINGLES, 1);
    database.add<Test_PlayerHPRatio>("LeadCommand_Left.png", 214.0 / 293, 214, 293, BattleHPLayout::OWN_PARTNER, 1);
    database.add<Test_PlayerHPRatio>("LeadCommand_Right.png", 49.0 / 49, 49, 49, BattleHPLayout::OWN_PARTNER, 2);
    database.add<Test_PlayerHPRatio>("PartnerCommand_Left.png", 214.0 / 293, 214, 293, BattleHPLayout::OWN_PARTNER, 1);
    database.add<Test_PlayerHPRatio>("PartnerCommand_Right.png", 21.0 / 21, 21, 21, BattleHPLayout::OWN_PARTNER, 2);
    database.add<Test_PlayerHPRatio>("BidoofCommand_Left.png", 210.0 / 287, 210, 287, BattleHPLayout::OWN_PARTNER, 1);
    database.add<Test_PlayerHPRatio>("BidoofCommand_Right.png", 19.0 / 19, 19, 19, BattleHPLayout::OWN_PARTNER, 2);
    database.add<Test_PlayerHPRatio>("NPC_Left.png", 285.0 / 293, 285, 293, BattleHPLayout::NPC_PARTNER, 1);
    database.add<Test_PlayerHPRatio>("NPC_Right.png", 56.0 / 88, 56, 88, BattleHPLayout::NPC_PARTNER, 2);
    database.add<Test_PlayerHPRatio>("NPC_Animation.png", -1.0, -1, -1, BattleHPLayout::NPC_PARTNER, 1);
    database.add<Test_PlayerHPRatio>("MoveMenu.png", -1.0, -1, -1, BattleHPLayout::OWN_PARTNER, 1);
    database.add<Test_PlayerHPRatio>("SingleRejectsRaised.png", -1.0, -1, -1, BattleHPLayout::SINGLES, 1);
    database.add<Test_PlayerHPRatio>("OwnRejectsSingle.png", -1.0, -1, -1, BattleHPLayout::OWN_PARTNER, 1);
    database.add<Test_BagPartyTarget>("Selected_1.png", 1);
    database.add<Test_BagPartyTarget>("ItemList.png", 0);
    database.add<Test_BagPartyTarget>("UseMenu.png", 0);
    database.add<Test_BagPartyTarget>("NoBag.png", 0);
    database.add<Test_BagPartyTarget>("Synthetic_2.png", 2);
    database.add<Test_BagPartyTarget>("Synthetic_6.png", 6);
    database.add<Test_BagPartyTarget>("Synthetic_Ambiguous.png", 0);
    add_tests_MenuDetector(database);
    database.add<Test_BattleMoveMenu>("Wake_MoveList_618-7.png", 4, std::array<int, 4>{7, 15, 10, 15});
    database.add<Test_BattleMoveMenu>("Wake_MoveList_619-1.png", 4, std::array<int, 4>{7, 15, 10, 15});
    database.add<Test_BattleMoveMenu>("Wake_MoveList_1064-9.png", 4, std::array<int, 4>{7, 15, 10, 15});
    database.add<Test_BattleMoveMenu>("Wake_Attack_620-0.png", 0);
    database.add<Test_BattleMoveMenu>("Wake_Attack_623-1.png", 0);
    database.add<Test_BattleMoveMenu>("Wake_Fainted_624-1.png", 0);
    database.add<Test_BattleMoveMenu>("Wake_Attack_1069-2.png", 0);
    database.add<Test_BattleMoveMenu>("Wake_Fainted_1069-5.png", 0);
    database.add<Test_BattleMoveMenu>("Route203_PP24.png", 4, std::array<int, 4>{24, 24, 16, 24});
    database.add<Test_BattleMoveMenu>("Route203_PP23.png", 4, std::array<int, 4>{23, 24, 16, 24});
    database.add<Test_BattleMoveMenu>("Route203_PP22.png", 4, std::array<int, 4>{22, 24, 16, 24});
    database.add<Test_BattleMoveMenu>("Gate_PP18.png", 4, std::array<int, 4>{18, 24, 16, 24});
    database.add<Test_BattleMoveMenu>("DoubleBattle_PP.png", 4, std::array<int, 4>{8, 8, 9, 15});
    for (const char* name : {
        "Floaroma_RepelText_3791-0_True.png",
        "Floaroma_RepelChoice_3791-3_True.png",
        "Floaroma_Clear_3791-6_True.png",
        "Floaroma_BannerEntering_3790-0_False.png",
        "Floaroma_BannerLeaving_3792-0_False.png",
        "Floaroma_NoBanner_3793-0_False.png",
        "Battle_600-0_False.png",
        "Route204_Sign_650-0_False.png",
        "Oreburgh_Exterior_0-0_False.png",
        "Jubilife_Plaza_1034-0_False.png",
        "Floaroma_HouseApproach_678-0_False.png",
        "Floaroma_FirstArrival_666-0_True.png",
        "Floaroma_FirstArrivalNorth_667-0_True.png",
        "Floaroma_BeforeBanner_665-0_False.png",
        "Floaroma_AfterBanner_668-0_False.png",
        "Floaroma_EnteringTown_3790-3_True.png",
    }){
        database.add<Test_TownBanner>(name);
    }
}

}
}
}
