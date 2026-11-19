/*  Door Entry Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <algorithm>
#include <memory>
#include <sstream>
#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
#include "Kernels/Waterfill/Kernels_Waterfill_Types.h"
#include "CommonTools/ImageMatch/ExactImageMatcher.h"
#include "CommonTools/ImageMatch/WaterfillTemplateMatcher.h"
#include "CommonTools/Images/ImageFilter.h"
#include "CommonTools/Images/WaterfillUtilities.h"
#include "Tests/ImageUnitTest.h"
#include "Tests/TestUtils.h"
#include "PokemonLZA_DoorEntryDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{

class DoorEntryComponentMatcher : public ImageMatch::WaterfillTemplateMatcher{
public:
    DoorEntryComponentMatcher(const char* path)
        : WaterfillTemplateMatcher(path, Color(0xffc0c0c0), COLOR_WHITE, 100)
    {
        // Compare isolated strokes against black to verify gaps
        // Preserve antialiasing on the white edges
        m_matcher = std::make_unique<ImageMatch::ExactImageMatcher>(filter_rgb32_range(
            image_template(), 0xffa0a0a0, 0xffffffff, COLOR_BLACK, false
        ));
    }

    virtual double rmsd_original(
        Resolution resolution, const ImageViewRGB32& image,
        const Kernels::Waterfill::WaterfillObject& object
    ) const override{
        ImageRGB32 component = filter_rgb32_range(
            extract_box_reference(image, object), 0xffa0a0a0, 0xffffffff, COLOR_BLACK, false
        );
        return WaterfillTemplateMatcher::rmsd_precropped(resolution, component, object);
    }
};

class DoorEntryMatcher : public DoorEntryComponentMatcher{
public:
    DoorEntryMatcher()
        : DoorEntryComponentMatcher("PokemonLZA/DoorEntry.png")
        , m_arrow("PokemonLZA/DoorEntryArrow.png")
    {
        m_aspect_ratio_lower = 0.85;
        m_aspect_ratio_upper = 1.15;
        m_area_ratio_lower = 0.75;
        m_area_ratio_upper = 1.25;
    }

    virtual bool check_image(Resolution resolution, const ImageViewRGB32& image) const override{
        return image.width() >= 40 * resolution.width / 3840
            && image.height() >= 32 * resolution.height / 2160
            && image.width() <= 160 * resolution.width / 3840
            && image.height() <= 140 * resolution.height / 2160;
    }

    virtual double rmsd_original(
        Resolution resolution, const ImageViewRGB32& image,
        const Kernels::Waterfill::WaterfillObject& object
    ) const override{
        const double outline_rmsd = DoorEntryComponentMatcher::rmsd_original(resolution, image, object);
        if (outline_rmsd >= 90){
            return outline_rmsd;
        }

        // The template's doorway is 25 x 34; the arrow starts 19 pixels right and 9 pixels down
        // Allow a small margin for scaling/rounding
        const double scale_x = object.width() / 25.0;
        const double scale_y = object.height() / 34.0;
        const size_t x = object.min_x + static_cast<size_t>(16 * scale_x + 0.5);
        const size_t y = object.min_y + static_cast<size_t>(6 * scale_y + 0.5);
        const size_t width = static_cast<size_t>(30 * scale_x + 0.5);
        const size_t height = static_cast<size_t>(24 * scale_y + 0.5);
        if (x + width > image.width() || y + height > image.height()){
            return 99999;
        }
        const size_t min_area = std::max<size_t>(1, static_cast<size_t>(80 * scale_x * scale_y));
        const size_t max_area = static_cast<size_t>(1000 * scale_x * scale_y);
        const bool arrow_found = match_template_by_waterfill(
            resolution, image.sub_image(x, y, width, height), m_arrow,
            {
                {0xffa0a0a0, 0xffffffff},
                {0xffc0c0c0, 0xffffffff},
                {0xffe0e0e0, 0xffffffff},
            },
            {min_area, max_area}, 100,
            [](Kernels::Waterfill::WaterfillObject&){
                return true;
            }
        );
        return arrow_found ? outline_rmsd : 99999;
    }

private:
    DoorEntryComponentMatcher m_arrow;
};

DoorEntryDetector::DoorEntryDetector(Color color, VideoOverlay* overlay, const ImageFloatBox& box)
    : m_color(color)
    , m_overlay(overlay)
    , m_box(box)
{}

void DoorEntryDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_box);
}

bool DoorEntryDetector::detect(const ImageViewRGB32& screen){
    static const DoorEntryMatcher matcher;
    const double scale = screen.height() / 1080.0;
    const size_t min_area = std::max<size_t>(1, static_cast<size_t>(100 * scale * scale));
    const size_t max_area = static_cast<size_t>(3000 * scale * scale);
    const bool found = match_template_by_waterfill(
        screen.size(), extract_box_reference(screen, m_box), matcher,
        {
            {0xffa0a0a0, 0xffffffff},
            {0xffc0c0c0, 0xffffffff},
            {0xffe0e0e0, 0xffffffff},
            // Bright capture-card edge pixels can widen the candidate bounds.
            {0xfff0f0f0, 0xffffffff},
            {0xfff8f8f8, 0xffffffff},
        },
        {min_area, max_area}, 90,
        [&](Kernels::Waterfill::WaterfillObject& object){
            m_last_detected = translate_to_parent(screen, m_box, object);
            return true;
        }
    );
    if (!found){
        reset_state();
    }else if (m_overlay){
        m_last_detected_box.emplace(*m_overlay, m_last_detected, COLOR_GREEN);
    }
    return found;
}

void DoorEntryDetector::reset_state(){
    m_last_detected = {};
    m_last_detected_box.reset();
}

class Test_DoorEntryDetector : public ImageUnitTest{
public:
    Test_DoorEntryDetector(const std::string& image, bool expected, size_t icon_x = 948, size_t icon_y = 323)
        : ImageUnitTest("PokemonLZA::DoorEntryDetector - " + image, {image})
        , m_expected(expected)
        , m_icon_x(icon_x)
        , m_icon_y(icon_y)
    {
        m_memory += MAX_TEST_IMAGE_RGB32_BYTES;
    }

    virtual UnitTestResult run_on_images(
        Logger&, CancellableScope&,
        const std::vector<ImageRGB32>& images
    ) const override{
        for (size_t height : {720, 1080, 2160}){
            ImageRGB32 scaled = images[0].scale_to(height * 16 / 9, height);
            DoorEntryDetector detector(COLOR_RED);
            const bool detected = detector.detect(scaled);
            TEST_RESULT_COMPONENT_EQUAL_STR(detected, m_expected, "door entry at height " + std::to_string(height));
            if (m_expected){
                DoorEntryDetector outside(COLOR_RED, nullptr, {0.0, 0.0, 0.25, 0.5});
                const bool outside_detected = outside.detect(scaled);
                TEST_RESULT_COMPONENT_EQUAL_STR(outside_detected, false, "search region excludes door");

                // Require a sustained match and verify that reset starts a new hold.
                DoorEntryWatcher watcher(COLOR_RED);
                const WallClock start = current_time();
                const bool first = watcher.process_frame(scaled, start);
                const bool held = watcher.process_frame(scaled, start + std::chrono::milliseconds(200));
                TEST_RESULT_COMPONENT_EQUAL_STR(first, false, "initial watcher frame");
                TEST_RESULT_COMPONENT_EQUAL_STR(held, true, "sustained watcher match");
                watcher.reset_state();
                const bool after_reset = watcher.process_frame(scaled, start + std::chrono::milliseconds(300));
                TEST_RESULT_COMPONENT_EQUAL_STR(after_reset, false, "watcher reset");

                // A doorway outline without its separate arrow must not match.
                const ImagePixelBox arrow_box = floatbox_to_pixelbox(
                    scaled, {(m_icon_x + 24.0) / 1920, (m_icon_y + 13.0) / 1080, 24.0 / 1920, 18.0 / 1080}
                );
                ImageRGB32 arrow = extract_box_reference(scaled, arrow_box).copy();
                for (size_t y = 0; y < arrow.height(); y++){
                    for (size_t x = 0; x < arrow.width(); x++){
                        scaled.pixel(arrow_box.min_x + x, arrow_box.min_y + y) = 0xff202020;
                    }
                }
                const bool without_arrow = detector.detect(scaled);
                TEST_RESULT_COMPONENT_EQUAL_STR(without_arrow, false, "entry arrow removed");
                for (size_t y = 0; y < arrow.height(); y++){
                    for (size_t x = 0; x < arrow.width(); x++){
                        scaled.pixel(arrow_box.min_x + x, arrow_box.min_y + y) = arrow.pixel(x, y);
                    }
                }

                // Remove only the entry icon: the quest marker above it must not match.
                const ImagePixelBox icon_box = floatbox_to_pixelbox(
                    scaled, {m_icon_x / 1920.0, m_icon_y / 1080.0, 52.0 / 1920, 42.0 / 1080}
                );
                ImageRGB32 icon = extract_box_reference(scaled, icon_box).copy();
                for (size_t y = 0; y < icon.height(); y++){
                    for (size_t x = 0; x < icon.width(); x++){
                        scaled.pixel(icon_box.min_x + x, icon_box.min_y + y) = 0xff202020;
                    }
                }
                const bool erased = detector.detect(scaled);
                TEST_RESULT_COMPONENT_EQUAL_STR(erased, false, "entry icon removed");

                // The same icon must also match elsewhere in the search region.
                const size_t target_x = scaled.width() * 3 / 5;
                const size_t target_y = scaled.height() / 2;
                for (size_t y = 0; y < icon.height(); y++){
                    for (size_t x = 0; x < icon.width(); x++){
                        scaled.pixel(target_x + x, target_y + y) = icon.pixel(x, y);
                    }
                }
                const bool shifted = detector.detect(scaled);
                TEST_RESULT_COMPONENT_EQUAL_STR(shifted, true, "entry icon moved");
            }
        }
        return true;
    }

private:
    bool m_expected;
    size_t m_icon_x;
    size_t m_icon_y;
};

void add_tests_DoorEntryDetector(UnitTestDatabase& database){
    database.add<Test_DoorEntryDetector>("PokemonLZA/DoorEntryDetector/pokemon_center_entry.png", true);
    database.add<Test_DoorEntryDetector>("PokemonLZA/DoorEntryDetector/hotel_elevator_entry.png", true, 1317, 605);
    database.add<Test_DoorEntryDetector>("PokemonLZA/DoorEntryDetector/hotel_room_alternate_card.png", true, 848, 389);
    database.add<Test_DoorEntryDetector>("PokemonLZA/AutoStory/required_pancham.png", false);
    database.add<Test_DoorEntryDetector>("PokemonLZA/AutoStory/required_cleffa_no_instructions.png", false);
    database.add<Test_DoorEntryDetector>("PokemonLZA/TransparentBattleDialogDetector/mac_jacinthe_False.png", false);
    database.add<Test_DoorEntryDetector>("PokemonLZA/TransparentBattleDialogDetector/mac_jacinthe_overworld_False.png", false);
    database.add<Test_DoorEntryDetector>("PokemonLZA/ButtonDetector/french_fossil_1_ButtonA_True.png", false);
    database.add<Test_DoorEntryDetector>("PokemonLZA/OverworldPartySelectionDetector/mac_zone_10_0_5.png", false);
}

}
}
}
