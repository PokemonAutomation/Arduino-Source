/*  Box Detection
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Containers/FixedLimitVector.tpp"
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
#include "CommonFramework/GlobalAutoPaths.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonTools/Images/SolidColorTest.h"
#include "Tests/TestUtils.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonSV_BoxGenderDetector.h"
#include "PokemonSV_BoxShinyDetector.h"
#include "PokemonSV_BoxDetection.h"

#include <iostream>
using std::cout;
using std::endl;;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

std::string BOX_CURSOR_LOCATION_NAMES(BoxCursorLocation location){
    switch(location){
    case BoxCursorLocation::NONE:
        return "NONE";
    case BoxCursorLocation::PARTY:
        return "PARTY";
    case BoxCursorLocation::BOX_CHANGE:
        return "BOX_CHANGE";
    case BoxCursorLocation::ALL_BOXES:
        return "ALL_BOXES";
    case BoxCursorLocation::SEARCH:
        return "SEARCH";
    case BoxCursorLocation::SLOTS:
        return "SLOTS";
    default:
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Unknown BoxCursorLocation");
    }
}

std::string BOX_LOCATION_STRING(BoxCursorLocation location, uint8_t row, uint8_t col){
    return "(" + BOX_CURSOR_LOCATION_NAMES(location) + " row " + std::to_string(row) + " col " + std::to_string(col) + ")";
}


SomethingInBoxSlotDetector::SomethingInBoxSlotDetector(Color color, bool true_if_exists)
    : m_true_if_exists(true_if_exists)
    , m_color(color)
    , m_right(0.985, 0.010, 0.010, 0.050)
    , m_top(0.660, 0.005, 0.330, 0.006)
    , m_bottom(0.660, 0.064, 0.330, 0.005)
    , m_body(0.670, 0.010, 0.060, 0.050)
{}
void SomethingInBoxSlotDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_right);
    items.add(m_color, m_top);
    items.add(m_color, m_bottom);
    items.add(m_color, m_body);
}
bool SomethingInBoxSlotDetector::detect(const ImageViewRGB32& screen){
    ImageStats right = image_stats(extract_box_reference(screen, m_right));
//    extract_box_reference(screen, m_right).save("test.png");
//    cout << right.average << right.stddev << endl;
    if (!is_solid(right, {0.533473, 0.466527, 0.0}, 0.15, 20)){
//        cout << "asdf" << endl;
        return !m_true_if_exists;
    }
    ImageStats top = image_stats(extract_box_reference(screen, m_top));
//    cout << top.average << top.stddev << endl;
    if (!is_solid(top, {0.533473, 0.466527, 0.0})){
//        cout << "qwer" << endl;
        return !m_true_if_exists;
    }
    ImageStats bottom = image_stats(extract_box_reference(screen, m_bottom));
//     cout << bottom.average << bottom.stddev << endl;
//    extract_box_reference(screen, m_bottom).save("./tmp_bottom.png");
    if (!is_solid(bottom, {0.533473, 0.466527, 0.0}, 0.15, 15.0)){
//        cout << "zxcv" << endl;
        return !m_true_if_exists;
    }
    ImageStats body = image_stats(extract_box_reference(screen, m_body));
//    cout << body.average << body.stddev << endl;
    if (body.stddev.sum() < 100){
        return !m_true_if_exists;
    }

    return m_true_if_exists;
}




BoxSelectDetector::BoxSelectDetector(Color color)
    : m_color(color)
    , m_exists(color, true)
    , m_dialog(color)
    , m_gradient(color, GradientArrowType::RIGHT, {0.20, 0.17, 0.50, 0.10})
{}
void BoxSelectDetector::make_overlays(VideoOverlaySet& items) const{
    m_exists.make_overlays(items);
    m_dialog.make_overlays(items);
    m_gradient.make_overlays(items);
}
bool BoxSelectDetector::exists(const ImageViewRGB32& screen){
    return m_exists.detect(screen);
}
bool BoxSelectDetector::detect(const ImageViewRGB32& screen){
    if (!exists(screen)){
        return false;
    }
    if (!m_dialog.detect(screen)){
        return false;
    }
    if (!m_gradient.detect(screen)){
        return false;
    }
    return true;
}


BoxDetector::BoxDetector(Color color)
    : m_color(color)
    , m_party(color, GradientArrowType::DOWN, {0.140, 0.150, 0.050, 0.700})
    , m_box_change(color, GradientArrowType::DOWN, {0.405, 0.070, 0.050, 0.090})
    , m_all_boxes(color, GradientArrowType::DOWN, {0.295, 0.780, 0.050, 0.090})
    , m_search(color, GradientArrowType::DOWN, {0.510, 0.780, 0.050, 0.090})
    , m_slots(color, GradientArrowType::DOWN, {0.240, 0.160, 0.380, 0.550})
{}
void BoxDetector::make_overlays(VideoOverlaySet& items) const{
    m_party.make_overlays(items);
    m_box_change.make_overlays(items);
    m_all_boxes.make_overlays(items);
    m_search.make_overlays(items);
    m_slots.make_overlays(items);
}
bool BoxDetector::detect(const ImageViewRGB32& screen){
    if (m_party.detect(screen)){
        return true;
    }
    if (m_box_change.detect(screen)){
        return true;
    }
    if (m_all_boxes.detect(screen)){
        return true;
    }
    if (m_search.detect(screen)){
        return true;
    }
    if (m_slots.detect(screen)){
        return true;
    }
    return false;
}
std::pair<BoxCursorLocation, BoxCursorCoordinates> BoxDetector::detect_location(const ImageViewRGB32& screen){
    if (m_box_change.detect(screen)){
        return {BoxCursorLocation::BOX_CHANGE, {0, 0}};
    }
    if (m_all_boxes.detect(screen)){
        return {BoxCursorLocation::ALL_BOXES, {0, 0}};
    }
    if (m_search.detect(screen)){
        return {BoxCursorLocation::SEARCH, {0, 0}};
    }

    ImageFloatBox box;
    if (m_party.detect(box, screen)){
//        cout << box.y << endl;
        int slot = (int)((box.y - 0.175926) / 0.116296 + 0.5);
        if (slot < 0){
            return {BoxCursorLocation::NONE, {0, 0}};
        }
        return {BoxCursorLocation::PARTY, {(uint8_t)slot, 0}};
    }
    if (m_slots.detect(box, screen)){
//        cout << box.x << " " << box.y << endl;
        int x = (int)((box.x - 0.247917) / 0.065625 + 0.5);
        if (x < 0){
            return {BoxCursorLocation::NONE, {0, 0}};
        }
        int y = (int)((box.y - 0.175926) / 0.11713 + 0.5);
        if (x < 0){
            return {BoxCursorLocation::NONE, {0, 0}};
        }
        return {BoxCursorLocation::SLOTS, {(uint8_t)y, (uint8_t)x}};
    }

    return {BoxCursorLocation::NONE, {0, 0}};
}


bool BoxDetector::to_coordinates(int& x, int& y, BoxCursorLocation side, uint8_t row, uint8_t col) const{
    switch (side){
    case BoxCursorLocation::NONE:
        return false;
    case BoxCursorLocation::PARTY:
        x = -1;
        y = row;
        break;
    case BoxCursorLocation::BOX_CHANGE:
        x = 2;
        y = -1;
        break;
    case BoxCursorLocation::ALL_BOXES:
        x = 1;
        y = 5;
        break;
    case BoxCursorLocation::SEARCH:
        x = 4;
        y = 5;
        break;
    case BoxCursorLocation::SLOTS:
        x = col;
        y = row;
        break;
    default:
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Unknown BoxCursorLocation");
    }
    return true;
}
void BoxDetector::move_vertical(ProControllerContext& context, int current, int desired) const{
    int diff = (current - desired + 7) % 7;
//    cout << "diff = " << diff << endl;
    if (diff <= 3){
        for (int c = 0; c < diff; c++){
            pbf_press_dpad(context, DPAD_UP, 160ms, 240ms);
        }
    }else{
        for (int c = diff; c < 7; c++){
            pbf_press_dpad(context, DPAD_DOWN, 160ms, 240ms);
        }
    }
}
void BoxDetector::move_horizontal(ProControllerContext& context, int current, int desired) const{
    int diff = (current - desired + 7) % 7;
    if (diff <= 3){
        for (int c = 0; c < diff; c++){
            pbf_press_dpad(context, DPAD_LEFT, 160ms, 240ms);
        }
    }else{
        for (int c = diff; c < 7; c++){
            pbf_press_dpad(context, DPAD_RIGHT, 160ms, 240ms);
        }
    }
}


void BoxDetector::move_cursor(
    const ProgramInfo& info, VideoStream& stream, ProControllerContext& context,
    BoxCursorLocation side, uint8_t row, uint8_t col
){
    int desired_x = 0, desired_y = 0;
    if (!to_coordinates(desired_x, desired_y, side, row, col)){
        throw InternalProgramError(
            &stream.logger(), PA_CURRENT_FUNCTION,
            "BoxDetector::move_cursor() called with BoxCursorLocation::NONE."
        );
    }
//    cout << "desired_x = " << desired_x << ", desired_y = " << desired_y << endl;

    size_t consecutive_fails = 0;
    WallClock start = current_time();
    while (true){
        if (current_time() - start > std::chrono::seconds(60)){
            dump_image_and_throw_recoverable_exception(
                info, stream, "BoxMoveCursor",
                "Failed to move cursor to desired location after 1 minute."
            );
        }

        context.wait_for_all_requests();
        VideoSnapshot screen = stream.video().snapshot();
        std::pair<BoxCursorLocation, BoxCursorCoordinates> current = this->detect_location(screen);

        int current_x = 0, current_y = 0;
        if (!to_coordinates(current_x, current_y, current.first, current.second.row, current.second.col)){
            consecutive_fails++;
            if (consecutive_fails > 100){
                dump_image_and_throw_recoverable_exception(info, stream, "BoxSystemNotDetected", "move_cursor(): Unable to detect box system.");
            }
            context.wait_for(std::chrono::milliseconds(100));
            continue;
        }
//        cout << "current_x = " << current_x << ", current_y = " << current_y << endl;

        if (current_x == desired_x && current_y == desired_y){
//            cout << "done!" << endl;
            return;
        }

        //  If we're on the party, always move horizontally first.
        if (current_x == -1){
            if (desired_x == -1){
                if (row < current.second.row){
                    for (uint8_t r = row; r < current.second.row; r++){
                        pbf_press_dpad(context, DPAD_UP, 160ms, 240ms);
                    }
                }else{ // row >= current.second.row
                    for (uint8_t r = current.second.row; r < row; r++){
                        pbf_press_dpad(context, DPAD_DOWN, 160ms, 240ms);
                    }
                }
                continue;
            }else if (desired_x < 3){
                pbf_press_dpad(context, DPAD_RIGHT, 160ms, 240ms);
            }else{
                pbf_press_dpad(context, DPAD_LEFT, 160ms, 240ms);
            }
            continue;
        }

        //  Otherwise, always move vertically first.
        if (current_y != desired_y){
            move_vertical(context, current_y, desired_y);
            continue;
        }

        //  Special case for bottom row.
        if (current_y == 5){
            int diff = (current_x - desired_x + 7) % 7;
            if (diff <= 3){
                pbf_press_dpad(context, DPAD_LEFT, 160ms, 240ms);
            }else{
                pbf_press_dpad(context, DPAD_RIGHT, 160ms, 240ms);
            }
            continue;
        }

        //  Now handle horizontals.
        move_horizontal(context, current_x, desired_x);
    }
}

BoxEmptySlotDetector::BoxEmptySlotDetector(BoxCursorLocation side, uint8_t row, uint8_t col, Color color)
    : m_color(color)
{
    if (side == BoxCursorLocation::PARTY){
        m_box = ImageFloatBox(0.142, 0.1165 * row + 0.201, 0.048, 0.082);
    }else if (side == BoxCursorLocation::SLOTS){
        m_box = ImageFloatBox(0.0656 * col + 0.242, 0.1165 * row + 0.201, 0.048, 0.082);
    }else{
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "INVALID BoxCursorLocation for BoxEmptySlotDetector");
    }
}

void BoxEmptySlotDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_box);
}

bool BoxEmptySlotDetector::detect(const ImageViewRGB32& frame){
    const auto stats = image_stats(extract_box_reference(frame, m_box));
    return stats.stddev.sum() < 20.0;
}


BoxEmptySlotWatcher::~BoxEmptySlotWatcher() = default;


BoxEmptyPartyWatcher::BoxEmptyPartyWatcher(Color color) : VisualInferenceCallback("BoxEmptyPartyWatcher"), m_empty_watchers(5){
    for (uint8_t i = 0; i < 5; i++){
        m_empty_watchers.emplace_back(
            BoxCursorLocation::PARTY,
            (uint8_t)(i + 1), (uint8_t)0,
            BoxEmptySlotWatcher::FinderType::CONSISTENT,
            color
        );
    }
}

void BoxEmptyPartyWatcher::make_overlays(VideoOverlaySet& items) const{
    for (int i = 0; i < 5; i++){
        m_empty_watchers[i].make_overlays(items);
    }
}

bool BoxEmptyPartyWatcher::process_frame(const ImageViewRGB32& frame, WallClock timestamp){
    bool all_certain = true;
    for (int i = 0; i < 5; i++){
        // Return true if it is sure that the slot is empty or not
        const bool empty_certain = m_empty_watchers[i].process_frame(frame, timestamp);

        if (!empty_certain){
            all_certain = false;
        }
    }
    return all_certain;
}

uint8_t BoxEmptyPartyWatcher::num_empty_slots_found() const{
    uint8_t num_empty = 0;
    for (int i = 0; i < 5; i++){
        if (m_empty_watchers[i].consistent_result()){
            num_empty++;
        }
    }
    return num_empty;
}

namespace{
    ImageFloatBox BOTTOM_BUTTON_Y_BOX{0.391, 0.939, 0.400, 0.047};
} // anonymous namespace

BoxBottomButtonYDetector::BoxBottomButtonYDetector(Color color)
    : WhiteButtonDetector(color, WhiteButton::ButtonY, BOTTOM_BUTTON_Y_BOX)
{}
BoxBottomButtonBDetector::BoxBottomButtonBDetector(Color color)
    : WhiteButtonDetector(color, WhiteButton::ButtonB, {0.842, 0.939, 0.139, 0.047})
{}


BoxSelectionBoxModeWatcher::BoxSelectionBoxModeWatcher(Color color)
    : VisualInferenceCallback("BoxSelectionBoxModeWatcher")
    , button_y_watcher(color, WhiteButton::ButtonY, BOTTOM_BUTTON_Y_BOX, WhiteButtonWatcher::FinderType::CONSISTENT)
{}

void BoxSelectionBoxModeWatcher::make_overlays(VideoOverlaySet& items) const{
    button_y_watcher.make_overlays(items);
}

bool BoxSelectionBoxModeWatcher::process_frame(const ImageViewRGB32& frame, WallClock timestamp){
    return button_y_watcher.process_frame(frame, timestamp);
}

bool BoxSelectionBoxModeWatcher::in_box_selection_mode() const{
    // If we have button Y detected, then we are NOT in box selection mode
    return !button_y_watcher.consistent_result();
}







class Test_BoxInfoDetector : public UnitTest{
public:
    Test_BoxInfoDetector(
        const std::string& image,
        std::vector<std::string> words
    )
        : UnitTest("PokemonSV::BoxInfoDetector - " + image)
        , m_image(UNIT_TEST_RESOURCE_PATH() + image)
        , m_words(std::move(words))
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        // two words: <shiny or not> <gender (1: male, 2: female, 3: genderless)
        if (m_words.size() < 2){
            std::stringstream ss;
            ss << "Error: not enough number of words in the filename. Found only " << m_words.size() << "." << std::endl;
            return ss.str();
        }

        bool target_shiny = false;
        int target_gender = 0;
        if (parse_bool(m_words[m_words.size()-2], target_shiny) == false){
            std::stringstream ss;
            ss << "Error: word " << m_words[m_words.size()-2] << " is wrong. Must be True or False. " << std::endl;
            return ss.str();
        }
        if (parse_int(m_words[m_words.size()-1], target_gender) == false){
            std::stringstream ss;
            ss << "Error: word " << m_words[m_words.size()-1] << " is wrong. Must be int (1: male, 2: female, 3: genderless). " << std::endl;
            return ss.str();
        }

        SomethingInBoxSlotDetector sth_detector(COLOR_RED);
        ImageRGB32 image(m_image);
        bool sth = sth_detector.detect(image);
        TEST_RESULT_EQUAL_STR(sth, true);

        BoxShinyDetector shiny_detector;
        bool shiny_result = shiny_detector.detect(image);

        TEST_RESULT_EQUAL_STR(shiny_result, target_shiny);

        BoxGenderDetector gender_detector;
        int gender_result = (int)gender_detector.detect(image);
        TEST_RESULT_EQUAL_STR(gender_result, target_gender);

        return true;
    };

private:
    std::string m_image;
    const std::vector<std::string> m_words;
};

class Test_SomethingInBoxSlotDetector : public UnitTest{
public:
    Test_SomethingInBoxSlotDetector(const std::string& image, bool expected)
        : UnitTest("PokemonSV::SomethingInBoxSlotDetector - " + image)
        , m_image(UNIT_TEST_RESOURCE_PATH() + image)
        , m_expected(expected)
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        SomethingInBoxSlotDetector detector(COLOR_RED);
        ImageRGB32 image(m_image);
        return detector.detect(image) == m_expected;
    }

private:
    std::string m_image;
    bool m_expected;
};

class Test_BoxBottomButtonDetector : public UnitTest{
public:
    Test_BoxBottomButtonDetector(const std::string& image, bool target_y, bool target_b)
        : UnitTest("PokemonSV::BoxBottomButtonDetector - " + image)
        , m_image(UNIT_TEST_RESOURCE_PATH() + image)
        , m_target_y(target_y)
        , m_target_b(target_b)
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        ImageRGB32 image(m_image);
        BoxBottomButtonYDetector y_detector;
        TEST_RESULT_COMPONENT_EQUAL_STR(y_detector.detect(image), m_target_y, "button Y");

        BoxBottomButtonBDetector b_detector;
        TEST_RESULT_COMPONENT_EQUAL_STR(b_detector.detect(image), m_target_b, "button B");
        return true;
    }

private:
    std::string m_image;
    bool m_target_y;
    bool m_target_b;
};



void add_tests_BoxInfoDetector(UnitTestDatabase& database){
    database.add<Test_BoxInfoDetector>(
        "PokemonSV/BoxPokemonInfoDetector/Deerling_True_1.png",
        std::vector<std::string>{"Deerling", "True", "1"}
    );
    database.add<Test_BoxInfoDetector>(
        "PokemonSV/BoxPokemonInfoDetector/Dragapult_2_False_2.png",
        std::vector<std::string>{"Dragapult", "2", "False", "2"}
    );
    database.add<Test_BoxInfoDetector>(
        "PokemonSV/BoxPokemonInfoDetector/Dragapult_False_2.png",
        std::vector<std::string>{"Dragapult", "False", "2"}
    );
    database.add<Test_BoxInfoDetector>(
        "PokemonSV/BoxPokemonInfoDetector/Egg_False_3.png",
        std::vector<std::string>{"Egg", "False", "3"}
    );
    database.add<Test_BoxInfoDetector>(
        "PokemonSV/BoxPokemonInfoDetector/Lachonk_Korean_True_2.png",
        std::vector<std::string>{"Lachonk", "Korean", "True", "2"}
    );
    database.add<Test_BoxInfoDetector>(
        "PokemonSV/BoxPokemonInfoDetector/Magnemite_False_3.png",
        std::vector<std::string>{"Magnemite", "False", "3"}
    );
    database.add<Test_BoxInfoDetector>(
        "PokemonSV/BoxPokemonInfoDetector/Salamence_False_2.png",
        std::vector<std::string>{"Salamence", "False", "2"}
    );
    database.add<Test_BoxInfoDetector>(
        "PokemonSV/BoxPokemonInfoDetector/Tauros_False_1.png",
        std::vector<std::string>{"Tauros", "False", "1"}
    );
    database.add<Test_SomethingInBoxSlotDetector>("PokemonSV/SomethingInBoxSlotDetector/Yellow_Background_2_False.png", false);
    database.add<Test_SomethingInBoxSlotDetector>("PokemonSV/SomethingInBoxSlotDetector/Yellow_Background_False.png", false);
    database.add<Test_BoxBottomButtonDetector>("PokemonSV/BoxBottomButtonDetector/Empty_True_True.png", true, true);
    database.add<Test_BoxBottomButtonDetector>("PokemonSV/BoxBottomButtonDetector/Korean_Flamingo_False_True.png", false, true);
    database.add<Test_BoxBottomButtonDetector>("PokemonSV/BoxBottomButtonDetector/Korean_Flamingo_True_True.png", true, true);
    database.add<Test_BoxBottomButtonDetector>("PokemonSV/BoxBottomButtonDetector/Talonflame_box_selection_2_False_True.png", false, true);
    database.add<Test_BoxBottomButtonDetector>("PokemonSV/BoxBottomButtonDetector/Talonflame_box_selection_False_True.png", false, true);
    database.add<Test_BoxBottomButtonDetector>("PokemonSV/BoxBottomButtonDetector/Talonflame_True_True.png", true, true);
    database.add<Test_BoxBottomButtonDetector>("PokemonSV/BoxBottomButtonDetector/Volcarona_False_True.png", false, true);
    database.add<Test_BoxBottomButtonDetector>("PokemonSV/BoxBottomButtonDetector/Volcarona_True_True.png", true, true);
}





}
}
}
