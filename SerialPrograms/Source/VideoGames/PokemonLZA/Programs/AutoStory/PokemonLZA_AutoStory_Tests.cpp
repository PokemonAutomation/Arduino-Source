/*  Auto Story Tests
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <cmath>
#include <sstream>
#include "Common/Cpp/Json/JsonObject.h"
#include "Common/Cpp/Options/BatchOption.h"
#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
#include "Tests/ImageUnitTest.h"
#include "Tests/TestUtils.h"
#include "VideoGames/PokemonLZA/Inference/Map/PokemonLZA_TutorialMapDetector.h"
#include "VideoGames/PokemonLZA/Inference/PokemonLZA_BoothDetector.h"
#include "VideoGames/PokemonLZA/Inference/PokemonLZA_EventMarkerDetector.h"
#include "VideoGames/PokemonLZA/Inference/PokemonLZA_SelectionArrowDetector.h"
#include "VideoGames/PokemonLZA/Inference/PokemonLZA_TutorialWindowDetector.h"
#include "PokemonLZA_AutoStory.h"
#include "PokemonLZA_AutoStoryTools.h"
#include "PokemonLZA_AutoStory_Tests.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{

class Test_AutoStoryOptions : public UnitTest{
public:
    Test_AutoStoryOptions()
        : UnitTest("PokemonLZA::AutoStoryOptions")
    {}

    virtual UnitTestResult run(Logger&, CancellableScope&) const override{
        AutoStory program;
        StringSelectOption* segment = nullptr;
        StringSelectOption* start = nullptr;
        StringSelectOption* end = nullptr;
        BooleanCheckBoxOption* advanced = nullptr;
        IntegerEnumDropdownOption* section = nullptr;
        StaticTextOption* start_description = nullptr;
        StaticTextOption* end_description = nullptr;
        for (ConfigOption* option : dynamic_cast<BatchOption&>(program.options()).options()){
            if (auto* select = dynamic_cast<StringSelectOption*>(option)){
                if (select->label() == "<b>Start Segment:</b>"){
                    segment = select;
                }else if (select->label() == "<b>Start Checkpoint:</b>"){
                    start = select;
                }else if (select->label() == "<b>End Checkpoint:</b>"){
                    end = select;
                }
            }else if (auto* toggle = dynamic_cast<BooleanCheckBoxOption*>(option)){
                if (toggle->label().find("<b>Advanced mode:") == 0){
                    advanced = toggle;
                }
            }else if (auto* dropdown = dynamic_cast<IntegerEnumDropdownOption*>(option)){
                if (dropdown->label() == "<b>Story Section:</b>"){
                    section = dropdown;
                }
            }else if (auto* text = dynamic_cast<StaticTextOption*>(option)){
                if (text->text().find("Start:") == 0){
                    start_description = text;
                }else if (text->text().find("End:") == 0){
                    end_description = text;
                }
            }
        }
        if (!segment || !start || !end || !advanced || !section || !start_description || !end_description){
            return "Missing story-selection controls or descriptions.";
        }
        TEST_RESULT_COMPONENT_EQUAL_STR(section->database().all_values().size(), size_t(1), "one story section");
        TEST_RESULT_COMPONENT_EQUAL_STR(segment->database().size(), size_t(1), "one starting segment");
        TEST_RESULT_COMPONENT_EQUAL_STR(start->database().size(), size_t(5), "five starting checkpoints");
        TEST_RESULT_COMPONENT_EQUAL_STR(end->database().size(), size_t(5), "five ending checkpoints");
        TEST_RESULT_COMPONENT_EQUAL_STR(start->slug(), std::string("0"), "default start checkpoint");
        TEST_RESULT_COMPONENT_EQUAL_STR(end->slug(), std::string("4"), "default end checkpoint");
        if (segment->visibility() != ConfigOptionState::ENABLED ||
            start->visibility() != ConfigOptionState::HIDDEN || end->visibility() != ConfigOptionState::HIDDEN){
            return "Normal mode should display only the segment selector.";
        }
        if (!program.check_validity().empty()){
            return "Default selection should be valid.";
        }
        *advanced = true;
        if (segment->visibility() != ConfigOptionState::HIDDEN ||
            start->visibility() != ConfigOptionState::ENABLED || end->visibility() != ConfigOptionState::ENABLED){
            return "Advanced mode should display the checkpoint selectors.";
        }
        start->set_by_slug("3");
        end->set_by_slug("3");
        TEST_RESULT_COMPONENT_EQUAL_STR(start_description->text(),
            "Start: " + ALL_AUTO_STORY_CHECKPOINT_LIST()[3]->start_text(), "selected checkpoint start description");
        TEST_RESULT_COMPONENT_EQUAL_STR(end_description->text(),
            "End: " + ALL_AUTO_STORY_CHECKPOINT_LIST()[3]->end_text(), "selected checkpoint end description");
        if (!program.check_validity().empty()){
            return "A single-checkpoint range should be valid.";
        }
        const JsonValue saved = program.to_json();
        AutoStory restored;
        restored.load_json(saved);
        const JsonValue restored_settings = restored.to_json();
        const JsonObject& json = restored_settings.to_object_throw();
        TEST_RESULT_COMPONENT_EQUAL_STR(json.get_boolean_throw("ENABLE_ADVANCED_MODE"), true, "persist advanced mode");
        TEST_RESULT_COMPONENT_EQUAL_STR(json.get_string_throw("STORY_SECTION"), std::string("tutorial"), "persist section");
        TEST_RESULT_COMPONENT_EQUAL_STR(json.get_string_throw("START_SEGMENT"), std::string("0"), "persist segment");
        TEST_RESULT_COMPONENT_EQUAL_STR(json.get_string_throw("START_CHECKPOINT"), std::string("3"), "persist starting checkpoint");
        TEST_RESULT_COMPONENT_EQUAL_STR(json.get_string_throw("END_CHECKPOINT"), std::string("3"), "persist ending checkpoint");
        end->set_by_slug("2");
        if (program.check_validity().empty()){
            return "A reversed checkpoint range must be rejected.";
        }
        *advanced = false;
        if (!program.check_validity().empty()){
            return "Hidden checkpoint selections must not affect normal mode.";
        }
        TEST_RESULT_COMPONENT_EQUAL_STR(start_description->text(), ALL_AUTO_STORY_SEGMENT_LIST()[0]->start_text(),
            "normal mode restores the segment description");
        return true;
    }
};

class Test_AutoStoryEventMarker : public ImageUnitTest{
public:
    Test_AutoStoryEventMarker(const std::string& image, bool expected, double expected_center = 0)
        : ImageUnitTest("PokemonLZA::AutoStoryEventMarker - " + image, {image})
        , m_expected(expected)
        , m_expected_center(expected_center)
    {
        m_memory += 5 * MAX_TEST_IMAGE_RGB32_BYTES;
    }

    virtual UnitTestResult run_on_images(
        Logger&, CancellableScope&,
        const std::vector<ImageRGB32>& images
    ) const override{
        const auto fill_box = [](ImageRGB32& image, const ImageFloatBox& box, uint32_t color){
            const ImagePixelBox pixels = floatbox_to_pixelbox(image, box);
            for (size_t y = pixels.min_y; y < pixels.max_y; y++){
                for (size_t x = pixels.min_x; x < pixels.max_x; x++){
                    image.pixel(x, y) = color;
                }
            }
        };
        for (size_t height : {720, 1080, 2160}){
            ImageRGB32 scaled = images[0].scale_to(height * 16 / 9, height);
            EventMarkerWatcher watcher;
            const bool detected = watcher.detect(scaled);
            TEST_RESULT_COMPONENT_EQUAL_STR(detected, m_expected, "event marker at height " + std::to_string(height));
            if (!m_expected){
                continue;
            }
            const ImageFloatBox box = watcher.marker_box();
            const double center = box.x + box.width / 2;
            const bool correct_location = center > m_expected_center - 0.01 && center < m_expected_center + 0.01;
            TEST_RESULT_COMPONENT_EQUAL_STR(correct_location, true, "world marker coordinates");
            const WallClock start = current_time();
            const bool first = watcher.process_frame(scaled, start);
            const bool held = watcher.process_frame(scaled, start + Milliseconds(200));
            TEST_RESULT_COMPONENT_EQUAL_STR(first, false, "initial event marker frame");
            TEST_RESULT_COMPONENT_EQUAL_STR(held, true, "sustained event marker");

            ImageRGB32 missing_symbol = scaled.copy();
            fill_box(missing_symbol, {box.x + box.width * 0.3, box.y + box.height * 0.15,
                box.width * 0.4, box.height * 0.73}, 0xffffc800);
            const bool without_symbol = watcher.detect(missing_symbol);
            TEST_RESULT_COMPONENT_EQUAL_STR(without_symbol, false, "yellow badge without exclamation mark");

            const ImageFloatBox full_marker{box.x - 0.008, box.y - 0.015, box.width + 0.025, box.height + 0.025};
            ImageRGB32 marker = extract_box_reference(scaled, full_marker).copy();
            ImageRGB32 removed = scaled.copy();
            fill_box(removed, full_marker, 0xff283850);
            const bool only_minimap = watcher.detect(removed);
            TEST_RESULT_COMPONENT_EQUAL_STR(only_minimap, false, "world marker removed with minimap unchanged");
            watcher.reset_state();
            watcher.process_frame(scaled, start);
            watcher.process_frame(removed, start + Milliseconds(100));
            const bool interrupted = watcher.process_frame(scaled, start + Milliseconds(200));
            TEST_RESULT_COMPONENT_EQUAL_STR(interrupted, false, "interrupted marker detection");

            // Reposition the same marker to prove coordinates follow the icon
            const size_t target_x = scaled.width() * 3 / 4;
            const size_t target_y = scaled.height() / 2;
            for (size_t y = 0; y < marker.height(); y++){
                for (size_t x = 0; x < marker.width(); x++){
                    removed.pixel(target_x + x, target_y + y) = marker.pixel(x, y);
                }
            }
            const bool relocated = watcher.detect(removed);
            TEST_RESULT_COMPONENT_EQUAL_STR(relocated, true, "relocated event marker");
            const bool moved_right = watcher.marker_box().x > 0.74;
            TEST_RESULT_COMPONENT_EQUAL_STR(moved_right, true, "relocated marker coordinates");
        }
        return true;
    }

private:
    bool m_expected;
    double m_expected_center;
};

class Test_AutoStoryEventMarkerTransition : public ImageUnitTest{
public:
    Test_AutoStoryEventMarkerTransition(
        const std::string& flagged_image = "PokemonLZA/AutoStory/wild_zone_1_no_booth.png"
    )
        : ImageUnitTest("PokemonLZA::AutoStoryEventMarkerTransition - " + flagged_image, {
            "PokemonLZA/AutoStory/wild_zone_1_event_no_flag.png",
            "PokemonLZA/AutoStory/wild_zone_1_event_clipped.png",
            flagged_image,
        })
    {
        m_memory += 4 * MAX_TEST_IMAGE_RGB32_BYTES;
    }

    virtual UnitTestResult run_on_images(
        Logger&, CancellableScope&,
        const std::vector<ImageRGB32>& images
    ) const override{
        for (size_t height : {720, 1080, 2160}){
            ImageRGB32 flagless = images[0].scale_to(height * 16 / 9, height);
            ImageRGB32 clipped = images[1].scale_to(height * 16 / 9, height);
            ImageRGB32 flagged = images[2].scale_to(height * 16 / 9, height);
            const WallClock start = current_time();
            EventMarkerWatcher watcher;
            const bool first = watcher.process_frame(flagless, start);
            const bool initial = watcher.process_frame(flagless, start + Milliseconds(200));
            const bool lost = watcher.process_frame(clipped, start + Milliseconds(250));
            const bool reacquiring = watcher.process_frame(flagged, start + Milliseconds(300));
            const bool recovered = watcher.process_frame(flagged, start + Milliseconds(500));
            TEST_RESULT_COMPONENT_EQUAL_STR(first, false, "initial flagless frame");
            TEST_RESULT_COMPONENT_EQUAL_STR(initial, true, "flagless marker acquired");
            TEST_RESULT_COMPONENT_EQUAL_STR(lost, false, "clipped transition frame");
            TEST_RESULT_COMPONENT_EQUAL_STR(reacquiring, false, "reacquisition hold restarts");
            TEST_RESULT_COMPONENT_EQUAL_STR(recovered, true, "flagged marker recovered");

            const ImageFloatBox box = watcher.marker_box();
            const ImageFloatBox full_marker{box.x - 0.008, box.y - 0.015, box.width + 0.025, box.height + 0.025};
            const ImagePixelBox pixels = floatbox_to_pixelbox(flagged, full_marker);
            ImageRGB32 icon = extract_box_reference(flagged, pixels).copy();
            ImageRGB32 centered = flagged.copy();
            for (size_t y = pixels.min_y; y < pixels.max_y; y++){
                for (size_t x = pixels.min_x; x < pixels.max_x; x++){
                    centered.pixel(x, y) = 0xff283850;
                }
            }
            const ptrdiff_t shift = std::lround((0.5 - box.x - box.width / 2) * flagged.width());
            const size_t target_x = size_t(ptrdiff_t(pixels.min_x) + shift);
            for (size_t y = 0; y < icon.height(); y++){
                for (size_t x = 0; x < icon.width(); x++){
                    centered.pixel(target_x + x, pixels.min_y + y) = icon.pixel(x, y);
                }
            }
            CenteredEventMarkerWatcher centered_watcher;
            const bool off_center = centered_watcher.process_frame(flagged, start);
            const bool still_off_center = centered_watcher.process_frame(flagged, start + Milliseconds(500));
            TEST_RESULT_COMPONENT_EQUAL_STR(off_center, false, "off-center flagged marker");
            TEST_RESULT_COMPONENT_EQUAL_STR(still_off_center, false, "off-center presence is insufficient");
            const bool centering = centered_watcher.process_frame(centered, start + Milliseconds(600));
            const bool too_brief = centered_watcher.process_frame(centered, start + Milliseconds(800));
            TEST_RESULT_COMPONENT_EQUAL_STR(centering, false, "first centered frame");
            TEST_RESULT_COMPONENT_EQUAL_STR(too_brief, false, "brief alignment must not start walking");
            const bool interrupted = centered_watcher.process_frame(clipped, start + Milliseconds(900));
            TEST_RESULT_COMPONENT_EQUAL_STR(interrupted, false, "graphic transition interrupts alignment");
            const bool restarting = centered_watcher.process_frame(centered, start + Milliseconds(1000));
            const bool not_yet = centered_watcher.process_frame(centered, start + Milliseconds(1399));
            const bool stable = centered_watcher.process_frame(centered, start + Milliseconds(1400));
            TEST_RESULT_COMPONENT_EQUAL_STR(restarting, false, "alignment hold restarts after loss");
            TEST_RESULT_COMPONENT_EQUAL_STR(not_yet, false, "alignment held for less than 400 ms");
            TEST_RESULT_COMPONENT_EQUAL_STR(stable, true, "alignment held for 400 ms");
        }
        return true;
    }
};

class Test_AutoStoryBooth : public ImageUnitTest{
public:
    Test_AutoStoryBooth(const std::string& image, bool expected)
        : ImageUnitTest("PokemonLZA::AutoStoryBooth - " + image, {image})
        , m_expected(expected)
    {
        m_memory += 6 * MAX_TEST_IMAGE_RGB32_BYTES;
    }

    virtual UnitTestResult run_on_images(
        Logger&, CancellableScope&,
        const std::vector<ImageRGB32>& images
    ) const override{
        const auto fill_box = [](ImageRGB32& image, const ImageFloatBox& box, uint32_t color){
            const ImagePixelBox pixels = floatbox_to_pixelbox(image, box);
            for (size_t y = pixels.min_y; y < pixels.max_y; y++){
                for (size_t x = pixels.min_x; x < pixels.max_x; x++){
                    image.pixel(x, y) = color;
                }
            }
        };
        for (size_t height : {720, 1080, 2160}){
            ImageRGB32 scaled = images[0].scale_to(height * 16 / 9, height);
            BoothWatcher watcher;
            const bool detected = watcher.detect(scaled);
            TEST_RESULT_COMPONENT_EQUAL_STR(detected, m_expected, "booth at height " + std::to_string(height));
            if (!m_expected){
                continue;
            }
            const ImageFloatBox roof = watcher.roof_box();
            const bool correct_roof = roof.x > 0.60 && roof.x < 0.62 && roof.y > 0.28 && roof.y < 0.31;
            TEST_RESULT_COMPONENT_EQUAL_STR(correct_roof, true, "cyan roof location");
            const WallClock start = current_time();
            const bool first = watcher.process_frame(scaled, start);
            const bool held = watcher.process_frame(scaled, start + Milliseconds(250));
            TEST_RESULT_COMPONENT_EQUAL_STR(first, false, "initial booth frame");
            TEST_RESULT_COMPONENT_EQUAL_STR(held, true, "sustained booth");

            ImageRGB32 removed_strip = scaled.copy();
            fill_box(removed_strip, {0.595, 0.28, 0.14, 0.025}, 0xff806040);
            const bool without_strip = watcher.detect(removed_strip);
            TEST_RESULT_COMPONENT_EQUAL_STR(without_strip, false, "roof strip removed");
            watcher.reset_state();
            watcher.process_frame(scaled, start);
            watcher.process_frame(removed_strip, start + Milliseconds(100));
            const bool interrupted = watcher.process_frame(scaled, start + Milliseconds(250));
            TEST_RESULT_COMPONENT_EQUAL_STR(interrupted, false, "interrupted booth detection");

            ImageRGB32 removed_wall = scaled.copy();
            fill_box(removed_wall, {0.60, 0.30, 0.13, 0.13}, 0xffffffff);
            const bool without_wall = watcher.detect(removed_wall);
            TEST_RESULT_COMPONENT_EQUAL_STR(without_wall, false, "cyan strip without dark wall");

            const ImageFloatBox booth_box{0.595, 0.255, 0.14, 0.17};
            ImageRGB32 booth = extract_box_reference(scaled, booth_box).copy();
            ImageRGB32 moved = scaled.copy();
            fill_box(moved, booth_box, 0xff806040);
            const size_t target_x = scaled.width() / 3;
            const size_t target_y = scaled.height() / 3;
            for (size_t y = 0; y < booth.height(); y++){
                for (size_t x = 0; x < booth.width(); x++){
                    moved.pixel(target_x + x, target_y + y) = booth.pixel(x, y);
                }
            }
            const bool relocated = watcher.detect(moved);
            TEST_RESULT_COMPONENT_EQUAL_STR(relocated, true, "booth at another screen position");
            const bool moved_left = watcher.roof_box().x < 0.4;
            TEST_RESULT_COMPONENT_EQUAL_STR(moved_left, true, "relocated roof coordinates");

            ImageRGB32 ambiguous = scaled.copy();
            for (size_t y = 0; y < booth.height(); y++){
                for (size_t x = 0; x < booth.width(); x++){
                    ambiguous.pixel(target_x + x, target_y + y) = booth.pixel(x, y);
                }
            }
            const bool multiple = watcher.detect(ambiguous);
            TEST_RESULT_COMPONENT_EQUAL_STR(multiple, false, "ambiguous booth candidates");
        }
        return true;
    }

private:
    bool m_expected;
};

class Test_AutoStoryBeginArrow : public ImageUnitTest{
public:
    Test_AutoStoryBeginArrow(const std::string& image)
        : ImageUnitTest("PokemonLZA::AutoStoryBeginArrow - " + image, {image})
    {
        m_memory += MAX_TEST_IMAGE_RGB32_BYTES;
    }

    virtual UnitTestResult run_on_images(
        Logger&, CancellableScope&,
        const std::vector<ImageRGB32>& images
    ) const override{
        for (size_t height : {720, 1080, 2160}){
            ImageRGB32 scaled = images[0].scale_to(height * 16 / 9, height);
            const ImageFloatBox box{0.05, 0.73, 0.045, 0.12};
            SelectionArrowWatcher watcher(COLOR_GREEN, nullptr, SelectionArrowType::RIGHT, box, Milliseconds(100));
            const bool detected = watcher.detect(scaled);
            TEST_RESULT_COMPONENT_EQUAL_STR(detected, true, "Begin arrow at height " + std::to_string(height));
            const WallClock start = current_time();
            const bool first = watcher.process_frame(scaled, start);
            const bool held = watcher.process_frame(scaled, start + Milliseconds(100));
            TEST_RESULT_COMPONENT_EQUAL_STR(first, false, "initial Begin arrow frame");
            TEST_RESULT_COMPONENT_EQUAL_STR(held, true, "sustained Begin arrow");

            const ImagePixelBox pixels = floatbox_to_pixelbox(scaled, box);
            ImageRGB32 arrow = extract_box_reference(scaled, pixels).copy();
            for (size_t y = pixels.min_y; y < pixels.max_y; y++){
                for (size_t x = pixels.min_x; x < pixels.max_x; x++){
                    scaled.pixel(x, y) = 0xff102030;
                }
            }
            const bool removed = watcher.detect(scaled);
            TEST_RESULT_COMPONENT_EQUAL_STR(removed, false, "Begin arrow removed");

            // A highlight on the options above Begin must not start the game
            const size_t target_y = scaled.height() * 53 / 100;
            for (size_t y = 0; y < arrow.height(); y++){
                for (size_t x = 0; x < arrow.width(); x++){
                    scaled.pixel(pixels.min_x + x, target_y + y) = arrow.pixel(x, y);
                }
            }
            const bool other_option = watcher.detect(scaled);
            TEST_RESULT_COMPONENT_EQUAL_STR(other_option, false, "arrow above Begin");
        }
        return true;
    }
};

class Test_AutoStoryTrainerBattle : public ImageUnitTest{
public:
    Test_AutoStoryTrainerBattle(const std::string& image, bool expected, bool check_second_slot = false)
        : ImageUnitTest("PokemonLZA::AutoStoryTrainerBattle - " + image, {image})
        , m_expected(expected)
        , m_check_second_slot(check_second_slot)
    {
        m_memory += MAX_TEST_IMAGE_RGB32_BYTES;
    }

    virtual UnitTestResult run_on_images(
        Logger&, CancellableScope&,
        const std::vector<ImageRGB32>& images
    ) const override{
        auto watchers = make_trainer_battle_watchers(Milliseconds(0));
        for (size_t height : {720, 1080, 2160}){
            ImageRGB32 scaled = images[0].scale_to(height * 16 / 9, height);
            bool detected = false;
            for (auto& watcher : watchers){
                detected |= watcher.detect(scaled);
            }
            TEST_RESULT_COMPONENT_EQUAL_STR(detected, m_expected, "trainer battle at height " + std::to_string(height));
            if (m_check_second_slot){
                // The second opponent must be detectable independently of slot 0
                const bool second_slot = watchers[1].detect(scaled);
                TEST_RESULT_COMPONENT_EQUAL_STR(second_slot, true, "second opponent at height " + std::to_string(height));
            }
        }
        return true;
    }

private:
    bool m_expected;
    bool m_check_second_slot;
};

class Test_AutoStoryTutorialPage : public ImageUnitTest{
public:
    Test_AutoStoryTutorialPage(const std::string& image, bool expected)
        : ImageUnitTest("PokemonLZA::AutoStoryTutorialPage - " + image, {image})
        , m_expected(expected)
    {
        // The watcher keeps an inverted frame while the resized test is alive
        m_memory += 2 * MAX_TEST_IMAGE_RGB32_BYTES;
    }

    virtual UnitTestResult run_on_images(
        Logger&, CancellableScope&,
        const std::vector<ImageRGB32>& images
    ) const override{
        for (size_t height : {720, 1080, 2160}){
            ImageRGB32 scaled = images[0].scale_to(height * 16 / 9, height);
            TutorialPageWatcher watcher(COLOR_CYAN);
            const bool detected = watcher.detect(scaled);
            TEST_RESULT_COMPONENT_EQUAL_STR(detected, m_expected, "tutorial at height " + std::to_string(height));
            if (m_expected){
                const WallClock start = current_time();
                const bool first = watcher.process_frame(scaled, start);
                const bool held = watcher.process_frame(scaled, start + Milliseconds(100));
                TEST_RESULT_COMPONENT_EQUAL_STR(first, false, "initial tutorial frame");
                TEST_RESULT_COMPONENT_EQUAL_STR(held, true, "sustained tutorial match");

                // Leave the left arrow and page dots in place, removing only Next
                const ImagePixelBox source = floatbox_to_pixelbox(
                    scaled, {998.0 / 1920, 957.0 / 1080, 35.0 / 1920, 34.0 / 1080}
                );
                ImageRGB32 pager = extract_box_reference(scaled, source).copy();
                for (size_t y = 0; y < pager.height(); y++){
                    for (size_t x = 0; x < pager.width(); x++){
                        scaled.pixel(source.min_x + x, source.min_y + y) = 0xffffffff;
                    }
                }
                const bool without_next = watcher.detect(scaled);
                TEST_RESULT_COMPONENT_EQUAL_STR(without_next, false, "right arrow removed");

                // Longer pages may push the pager farther right and down
                const size_t target_x = scaled.width() * 13 / 20;
                const size_t target_y = scaled.height() * 93 / 100;
                for (size_t y = 0; y < pager.height(); y++){
                    for (size_t x = 0; x < pager.width(); x++){
                        scaled.pixel(target_x + x, target_y + y) = pager.pixel(x, y);
                    }
                }
                const bool moved = watcher.detect(scaled);
                TEST_RESULT_COMPONENT_EQUAL_STR(moved, true, "pager moved right and down");

                // Accept the standard light button colors as well
                for (size_t y = 0; y < pager.height(); y++){
                    for (size_t x = 0; x < pager.width(); x++){
                        scaled.pixel(target_x + x, target_y + y) ^= 0x00ffffff;
                    }
                }
                const bool light = watcher.detect(scaled);
                TEST_RESULT_COMPONENT_EQUAL_STR(light, true, "light tutorial pager");
            }
        }
        return true;
    }

private:
    bool m_expected;
};

class Test_AutoStoryTutorialWindow : public ImageUnitTest{
public:
    Test_AutoStoryTutorialWindow(const std::string& image, bool expected)
        : ImageUnitTest("PokemonLZA::AutoStoryTutorialWindow - " + image, {image})
        , m_expected(expected)
    {
        m_memory += 2 * MAX_TEST_IMAGE_RGB32_BYTES;
    }

    virtual UnitTestResult run_on_images(
        Logger&, CancellableScope&,
        const std::vector<ImageRGB32>& images
    ) const override{
        for (size_t height : {720, 1080, 2160}){
            ImageRGB32 scaled = images[0].scale_to(height * 16 / 9, height);
            TutorialWindowWatcher watcher;
            const bool detected = watcher.detect(scaled);
            TEST_RESULT_COMPONENT_EQUAL_STR(detected, m_expected, "tutorial window at height " + std::to_string(height));
            if (m_expected){
                const WallClock start = current_time();
                const bool first = watcher.process_frame(scaled, start);
                const bool held = watcher.process_frame(scaled, start + Milliseconds(100));
                TEST_RESULT_COMPONENT_EQUAL_STR(first, false, "initial tutorial window frame");
                TEST_RESULT_COMPONENT_EQUAL_STR(held, true, "sustained tutorial window");

                // Reject a missing book, a solid green badge, or a missing frame
                for (const ImageFloatBox& box : {
                    ImageFloatBox{523.0 / 1920, 104.0 / 1080, 33.0 / 1920, 39.0 / 1080},
                    ImageFloatBox{0.29, 0.165, 0.42, 0.012},
                    ImageFloatBox{0.265, 0.18, 0.018, 0.68},
                }){
                    for (uint32_t color : {0xff102030, 0xffb8f52e}){
                        ImageRGB32 missing_component = scaled.copy();
                        const ImagePixelBox pixels = floatbox_to_pixelbox(missing_component, box);
                        for (size_t y = pixels.min_y; y < pixels.max_y; y++){
                            for (size_t x = pixels.min_x; x < pixels.max_x; x++){
                                missing_component.pixel(x, y) = color;
                            }
                        }
                        const bool missing = watcher.detect(missing_component);
                        TEST_RESULT_COMPONENT_EQUAL_STR(missing, false, "tutorial window component removed");
                        watcher.reset_state();
                        watcher.process_frame(scaled, start);
                        watcher.process_frame(missing_component, start + Milliseconds(50));
                        const bool interrupted = watcher.process_frame(scaled, start + Milliseconds(100));
                        TEST_RESULT_COMPONENT_EQUAL_STR(interrupted, false, "interrupted tutorial window match");
                    }
                }

                // Illustrations, text, and bottom arrows are not required
                const ImagePixelBox content = floatbox_to_pixelbox(scaled, {0.29, 0.185, 0.42, 0.735});
                for (size_t y = content.min_y; y < content.max_y; y++){
                    for (size_t x = content.min_x; x < content.max_x; x++){
                        scaled.pixel(x, y) = 0xffffffff;
                    }
                }
                const bool without_content = watcher.detect(scaled);
                TEST_RESULT_COMPONENT_EQUAL_STR(without_content, true, "tutorial content replaced");
            }
        }
        return true;
    }

private:
    bool m_expected;
};

class Test_AutoStoryDialogOptions : public ImageUnitTest{
public:
    Test_AutoStoryDialogOptions(const std::string& image, bool expected)
        : ImageUnitTest("PokemonLZA::AutoStoryDialogOptions - " + image, {image})
        , m_expected(expected)
    {
        m_memory += MAX_TEST_IMAGE_RGB32_BYTES;
    }

    virtual UnitTestResult run_on_images(
        Logger&, CancellableScope&,
        const std::vector<ImageRGB32>& images
    ) const override{
        for (size_t height : {720, 1080, 2160}){
            ImageRGB32 scaled = images[0].scale_to(height * 16 / 9, height);
            SelectionArrowWatcher watcher(
                COLOR_GREEN, nullptr, SelectionArrowType::RIGHT,
                {0.5, 0.2, 0.45, 0.6}, Milliseconds(100)
            );
            const bool detected = watcher.detect(scaled);
            TEST_RESULT_COMPONENT_EQUAL_STR(detected, m_expected, "dialog option at height " + std::to_string(height));
            if (m_expected){
                const WallClock start = current_time();
                const bool first = watcher.process_frame(scaled, start);
                const bool held = watcher.process_frame(scaled, start + Milliseconds(100));
                TEST_RESULT_COMPONENT_EQUAL_STR(first, false, "initial option frame");
                TEST_RESULT_COMPONENT_EQUAL_STR(held, true, "sustained option match");

                const ImagePixelBox source = floatbox_to_pixelbox(
                    scaled, {1390.0 / 1920, 650.0 / 1080, 55.0 / 1920, 72.0 / 1080}
                );
                ImageRGB32 arrow = extract_box_reference(scaled, source).copy();
                for (size_t y = 0; y < arrow.height(); y++){
                    for (size_t x = 0; x < arrow.width(); x++){
                        scaled.pixel(source.min_x + x, source.min_y + y) = 0xff102030;
                    }
                }
                const bool removed = watcher.detect(scaled);
                TEST_RESULT_COMPONENT_EQUAL_STR(removed, false, "selection arrow removed");

                // More choices or longer labels can move the highlighted row
                const size_t target_x = scaled.width() * 13 / 20;
                const size_t target_y = scaled.height() * 3 / 10;
                for (size_t y = 0; y < arrow.height(); y++){
                    for (size_t x = 0; x < arrow.width(); x++){
                        scaled.pixel(target_x + x, target_y + y) = arrow.pixel(x, y);
                    }
                }
                const bool moved = watcher.detect(scaled);
                TEST_RESULT_COMPONENT_EQUAL_STR(moved, true, "selection arrow moved");
            }
        }
        return true;
    }

private:
    bool m_expected;
};

class Test_AutoStoryTutorialMap : public ImageUnitTest{
public:
    Test_AutoStoryTutorialMap(const std::string& image, bool expected)
        : ImageUnitTest("PokemonLZA::AutoStoryTutorialMap - " + image, {image})
        , m_expected(expected)
    {
        m_memory += 2 * MAX_TEST_IMAGE_RGB32_BYTES;
    }

    virtual UnitTestResult run_on_images(
        Logger&, CancellableScope&,
        const std::vector<ImageRGB32>& images
    ) const override{
        for (size_t height : {720, 1080, 2160}){
            ImageRGB32 scaled = images[0].scale_to(height * 16 / 9, height);
            TutorialMapWatcher watcher;
            const bool detected = watcher.detect(scaled);
            TEST_RESULT_COMPONENT_EQUAL_STR(detected, m_expected, "tutorial map at height " + std::to_string(height));
            if (m_expected){
                const WallClock start = current_time();
                const bool first = watcher.process_frame(scaled, start);
                const bool held = watcher.process_frame(scaled, start + Milliseconds(100));
                TEST_RESULT_COMPONENT_EQUAL_STR(first, false, "initial map frame");
                TEST_RESULT_COMPONENT_EQUAL_STR(held, true, "sustained map match");

                // One prompt alone must not be treated as a map
                for (const ImageFloatBox& box : {
                    ImageFloatBox{0.005, 0.15, 0.025, 0.11},
                    ImageFloatBox{0.9, 0.94, 0.04, 0.04},
                }){
                    ImageRGB32 missing_prompt = scaled.copy();
                    const ImagePixelBox pixels = floatbox_to_pixelbox(missing_prompt, box);
                    for (size_t y = pixels.min_y; y < pixels.max_y; y++){
                        for (size_t x = pixels.min_x; x < pixels.max_x; x++){
                            missing_prompt.pixel(x, y) = 0xff102030;
                        }
                    }
                    const bool missing = watcher.detect(missing_prompt);
                    TEST_RESULT_COMPONENT_EQUAL_STR(missing, false, "map prompt removed");
                    watcher.reset_state();
                    watcher.process_frame(scaled, start);
                    watcher.process_frame(missing_prompt, start + Milliseconds(50));
                    const bool interrupted = watcher.process_frame(scaled, start + Milliseconds(100));
                    TEST_RESULT_COMPONENT_EQUAL_STR(interrupted, false, "interrupted map match");
                }
            }
        }
        return true;
    }

private:
    bool m_expected;
};

void add_tests_AutoStory(UnitTestDatabase& database){
    database.add<Test_AutoStoryOptions>();
    database.add<Test_AutoStoryEventMarkerTransition>();
    database.add<Test_AutoStoryEventMarkerTransition>("PokemonLZA/AutoStory/wild_zone_1_event_high_062357.png");
    database.add<Test_AutoStoryEventMarkerTransition>("PokemonLZA/AutoStory/wild_zone_1_event_high_062932.png");
    database.add<Test_AutoStoryEventMarker>("PokemonLZA/AutoStory/wild_zone_1_event_high_062357.png", true, 0.476);
    database.add<Test_AutoStoryEventMarker>("PokemonLZA/AutoStory/wild_zone_1_event_high_062932.png", true, 0.529);
    database.add<Test_AutoStoryEventMarker>("PokemonLZA/AutoStory/wild_zone_1_event_clipped.png", false);
    database.add<Test_AutoStoryEventMarker>("PokemonLZA/AutoStory/wild_zone_1_event_no_flag.png", true, 0.055);
    database.add<Test_AutoStoryEventMarker>("PokemonLZA/AutoStory/wild_zone_1_no_booth.png", true, 0.525);
    database.add<Test_AutoStoryEventMarker>("PokemonLZA/AutoStory/wild_zone_1_booth.png", true, 0.05);
    database.add<Test_AutoStoryEventMarker>("PokemonLZA/AutoStory/required_pancham.png", false);
    database.add<Test_AutoStoryEventMarker>("PokemonLZA/AutoStory/required_cleffa_no_instructions.png", false);
    database.add<Test_AutoStoryEventMarker>("PokemonLZA/AutoStory/type_matchup_tutorial.png", false);
    database.add<Test_AutoStoryEventMarker>("PokemonLZA/AutoStory/rival_dialog_options.png", false);
    database.add<Test_AutoStoryEventMarker>("PokemonLZA/DoorEntryDetector/hotel_room_alternate_card.png", false);

    database.add<Test_AutoStoryBooth>("PokemonLZA/AutoStory/wild_zone_1_booth.png", true);
    database.add<Test_AutoStoryBooth>("PokemonLZA/AutoStory/wild_zone_1_no_booth.png", false);
    database.add<Test_AutoStoryBooth>("PokemonLZA/AutoStory/required_pancham.png", false);
    database.add<Test_AutoStoryBooth>("PokemonLZA/AutoStory/required_cleffa_no_instructions.png", false);
    database.add<Test_AutoStoryBooth>("PokemonLZA/AutoStory/type_matchup_tutorial.png", false);
    database.add<Test_AutoStoryBooth>("PokemonLZA/AutoStory/rival_dialog_options.png", false);
    database.add<Test_AutoStoryBooth>("PokemonLZA/DoorEntryDetector/hotel_room_alternate_card.png", false);

    database.add<Test_AutoStoryBeginArrow>("PokemonLZA/AutoStory/profile_confirmation.png");
    database.add<Test_AutoStoryTrainerBattle>("PokemonLZA/AutoStory/required_pancham.png", true);
    database.add<Test_AutoStoryTrainerBattle>("PokemonLZA/AutoStory/required_cleffa_no_instructions.png", true, true);
    database.add<Test_AutoStoryTrainerBattle>("PokemonLZA/TransparentBattleDialogDetector/mac_jacinthe_False.png", false);
    database.add<Test_AutoStoryTrainerBattle>("PokemonLZA/TransparentBattleDialogDetector/mac_jacinthe_True.png", false);
    database.add<Test_AutoStoryTrainerBattle>("PokemonLZA/TransparentBattleDialogDetector/mac_jacinthe_overworld_False.png", false);
    database.add<Test_AutoStoryTrainerBattle>("PokemonLZA/OverworldPartySelectionDetector/mac_zone_10_0_5.png", false);
    database.add<Test_AutoStoryTrainerBattle>("PokemonLZA/OverworldPartySelectionDetector/mac_zone_10_6_2.png", false);

    database.add<Test_AutoStoryTutorialPage>("PokemonLZA/AutoStory/type_matchup_tutorial.png", true);
    database.add<Test_AutoStoryTutorialPage>("PokemonLZA/AutoStory/main_mission_tutorial.png", false);
    database.add<Test_AutoStoryTutorialPage>("PokemonLZA/AutoStory/required_pancham.png", false);
    database.add<Test_AutoStoryTutorialPage>("PokemonLZA/AutoStory/required_cleffa_no_instructions.png", false);
    database.add<Test_AutoStoryTutorialPage>("PokemonLZA/MapDetector/hyperspace_2_True.png", false);
    database.add<Test_AutoStoryTutorialPage>("PokemonLZA/MapDetector/hyperspace_True.png", false);
    database.add<Test_AutoStoryTutorialPage>("PokemonLZA/TransparentBattleDialogDetector/mac_jacinthe_False.png", false);
    database.add<Test_AutoStoryTutorialPage>("PokemonLZA/TransparentBattleDialogDetector/mac_jacinthe_overworld_False.png", false);

    database.add<Test_AutoStoryTutorialWindow>("PokemonLZA/AutoStory/type_matchup_tutorial.png", true);
    database.add<Test_AutoStoryTutorialWindow>("PokemonLZA/AutoStory/main_mission_tutorial.png", true);
    database.add<Test_AutoStoryTutorialWindow>("PokemonLZA/AutoStory/required_pancham.png", false);
    database.add<Test_AutoStoryTutorialWindow>("PokemonLZA/AutoStory/required_cleffa_no_instructions.png", false);
    database.add<Test_AutoStoryTutorialWindow>("PokemonLZA/AutoStory/rival_dialog_options.png", false);
    database.add<Test_AutoStoryTutorialWindow>("PokemonLZA/AutoStory/tutorial_map.png", false);
    database.add<Test_AutoStoryTutorialWindow>("PokemonLZA/MapDetector/hyperspace_True.png", false);
    database.add<Test_AutoStoryTutorialWindow>("PokemonLZA/TransparentBattleDialogDetector/mac_jacinthe_True.png", false);
    database.add<Test_AutoStoryTutorialWindow>("PokemonLZA/TransparentBattleDialogDetector/mac_jacinthe_overworld_False.png", false);

    database.add<Test_AutoStoryDialogOptions>("PokemonLZA/AutoStory/rival_dialog_options.png", true);
    database.add<Test_AutoStoryDialogOptions>("PokemonLZA/AutoStory/type_matchup_tutorial.png", false);
    database.add<Test_AutoStoryDialogOptions>("PokemonLZA/AutoStory/required_pancham.png", false);
    database.add<Test_AutoStoryDialogOptions>("PokemonLZA/AutoStory/required_cleffa_no_instructions.png", false);
    database.add<Test_AutoStoryDialogOptions>("PokemonLZA/MapDetector/hyperspace_2_True.png", false);
    database.add<Test_AutoStoryDialogOptions>("PokemonLZA/MapDetector/hyperspace_True.png", false);
    database.add<Test_AutoStoryDialogOptions>("PokemonLZA/TransparentBattleDialogDetector/mac_jacinthe_False.png", false);
    database.add<Test_AutoStoryDialogOptions>("PokemonLZA/TransparentBattleDialogDetector/mac_jacinthe_overworld_False.png", false);

    database.add<Test_AutoStoryTutorialMap>("PokemonLZA/AutoStory/tutorial_map.png", true);
    database.add<Test_AutoStoryTutorialMap>("PokemonLZA/MapDetector/hyperspace_True.png", true);
    database.add<Test_AutoStoryTutorialMap>("PokemonLZA/MapDetector/hyperspace_2_True.png", true);
    database.add<Test_AutoStoryTutorialMap>("PokemonLZA/AutoStory/rival_dialog_options.png", false);
    database.add<Test_AutoStoryTutorialMap>("PokemonLZA/AutoStory/type_matchup_tutorial.png", false);
    database.add<Test_AutoStoryTutorialMap>("PokemonLZA/AutoStory/required_pancham.png", false);
    database.add<Test_AutoStoryTutorialMap>("PokemonLZA/AutoStory/required_cleffa_no_instructions.png", false);
    database.add<Test_AutoStoryTutorialMap>("PokemonLZA/TransparentBattleDialogDetector/mac_jacinthe_True.png", false);
    database.add<Test_AutoStoryTutorialMap>("PokemonLZA/TransparentBattleDialogDetector/mac_jacinthe_overworld_False.png", false);
}

}
}
}
