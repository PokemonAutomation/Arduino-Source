/*  Dialog Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Kernels/Waterfill/Kernels_Waterfill_Types.h"
#include "CommonFramework/ImageTools/ImageStats.h"
//#include "CommonTools/Images/SolidColorTest.h"
#include "PokemonLZA_DialogDetector.h"
#include "CommonTools/Images/SolidColorTest.h"
#include "CommonTools/Images/WaterfillUtilities.h"
#include "CommonTools/ImageMatch/WaterfillTemplateMatcher.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"

// #include <iostream>
// using std::cout;
// using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{


class DialogTitleGreenLineMatcher : public ImageMatch::WaterfillTemplateMatcher{
public:
    // The white background for the template file is of color range [r=240, g=255, b=230] to [255, 255, 255]
    // the green line is of color range [r=180,g=200,b=75] to [190, 214, 110]
    DialogTitleGreenLineMatcher()
        : WaterfillTemplateMatcher(
            "PokemonLZA/DialogBox/DialogBoxTitleGreenLine-Template.png",
            Color(180,200,70), Color(200, 220, 115), 50
        )
    {
        m_aspect_ratio_lower = 0.9;
        m_aspect_ratio_upper = 1.1;
        m_area_ratio_lower = 0.8;
        m_area_ratio_upper = 1.1;
    }

    static const ImageMatch::WaterfillTemplateMatcher& instance() {
        static DialogTitleGreenLineMatcher matcher;
        return matcher;
    }
};

class DialogBlackArrowMatcher : public ImageMatch::WaterfillTemplateMatcher{
public:
    DialogBlackArrowMatcher()
        : WaterfillTemplateMatcher(
            "PokemonLZA/DialogBox/DialogBoxBlackArrow-Template.png",
            Color(0,0,0), Color(90, 90, 90), 50
        )
    {
        m_aspect_ratio_lower = 0.9;
        m_aspect_ratio_upper = 1.1;
        m_area_ratio_lower = 0.85;
        m_area_ratio_upper = 1.1;
    }

    static const ImageMatch::WaterfillTemplateMatcher& instance() {
        static DialogBlackArrowMatcher matcher;
        return matcher;
    }
};

class DialogWhiteArrowMatcher : public ImageMatch::WaterfillTemplateMatcher{
public:
    DialogWhiteArrowMatcher()
        : WaterfillTemplateMatcher(
            "PokemonLZA/DialogBox/DialogBoxWhiteArrow-Template.png",
            Color(0xffc0c0c0), Color(0xffffffff), 50
        )
    {
        m_aspect_ratio_lower = 0.9;
        m_aspect_ratio_upper = 1.1;
        m_area_ratio_lower = 0.85;
        m_area_ratio_upper = 1.1;
    }

    static const ImageMatch::WaterfillTemplateMatcher& instance() {
        static DialogWhiteArrowMatcher matcher;
        return matcher;
    }
};

class DialogTealArrowMatcher : public ImageMatch::WaterfillTemplateMatcher{
public:
    DialogTealArrowMatcher()
        : WaterfillTemplateMatcher(
            "PokemonLZA/DialogBox/DialogBoxTealArrow-Template.png",
            Color(0xff00c080), Color(0xff3fffff), 50
        )
    {
        m_aspect_ratio_lower = 0.9;
        m_aspect_ratio_upper = 1.1;
        m_area_ratio_lower = 0.8;
        m_area_ratio_upper = 1.1;
    }

    static const ImageMatch::WaterfillTemplateMatcher& instance() {
        static DialogTealArrowMatcher matcher;
        return matcher;
    }
};


 
NormalDialogDetector::NormalDialogDetector(Logger& logger, VideoOverlay& overlay, bool stop_on_detected)
    : VisualInferenceCallback("NormalDialogDetector")
    , m_stop_on_detected(stop_on_detected)
    , m_detected(false)
    , m_title_green_line_box(0.224, 0.727, 0.016, 0.056)
    , m_black_arrow_box (0.727, 0.868, 0.037, 0.086)
{}
void NormalDialogDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(COLOR_RED, m_title_green_line_box);
    items.add(COLOR_RED, m_black_arrow_box);
}
bool NormalDialogDetector::process_frame(const ImageViewRGB32& frame, WallClock timestamp){
    const double screen_rel_size = (frame.height() / 1080.0);
    const double screen_rel_size_2 = screen_rel_size * screen_rel_size;

    bool found_green_title_line = false;
    bool found_black_arrow = false;

    // Example green pixels from screenshots:
    // 194,230,70
    // 212,235,127
    // 176,212,62
    const std::vector<std::pair<uint32_t, uint32_t>> green_line_filters = {
        {combine_rgb(160,200,55), combine_rgb(220, 240, 130)}
    };
  
    const double min_green_line_size_1080P = 100.0;
    const double green_line_rmsd_threshold = 50.0;
    const size_t min_green_line_size = size_t(screen_rel_size_2 * min_green_line_size_1080P);
    match_template_by_waterfill(
        frame.size(),
        extract_box_reference(frame, m_title_green_line_box), 
        DialogTitleGreenLineMatcher::instance(),
        green_line_filters,
        {min_green_line_size, SIZE_MAX},
        green_line_rmsd_threshold,
        [&](Kernels::Waterfill::WaterfillObject& object) -> bool {
            found_green_title_line = true;
            return true;
        }
    );

    // Example green pixels from screenshots:
    // [37,34,6] [20,15,55]
    // [39,42,58] [60,56,74] [50,49,63]
    const std::vector<std::pair<uint32_t, uint32_t>> black_arrow_filters = {
        {combine_rgb(0,0,0), combine_rgb(100, 100, 100)}
    };
  
    const double min_black_arrow_size_1080P = 150.0;
    const double black_arrow_rmsd_threshold = 120.0;
    const size_t min_black_arrow_size = size_t(screen_rel_size_2 * min_black_arrow_size_1080P);
    match_template_by_waterfill(
        frame.size(),
        extract_box_reference(frame, m_black_arrow_box), 
        DialogBlackArrowMatcher::instance(),
        black_arrow_filters,
        {min_black_arrow_size, SIZE_MAX},
        black_arrow_rmsd_threshold,
        [&](Kernels::Waterfill::WaterfillObject& object) -> bool {
            found_black_arrow = true;
            return true;
        }
    );

    bool is_dialog_box = found_green_title_line & found_black_arrow;

    if (is_dialog_box){
        m_detected.store(is_dialog_box);
    }

    return is_dialog_box & m_stop_on_detected;


    // size_t hits = 0;

    // const ImageStats title_top = image_stats(extract_box_reference(frame, m_title_top));
    // const ImageStats title_bottom = image_stats(extract_box_reference(frame, m_title_bottom));
    // const ImageStats title_left = image_stats(extract_box_reference(frame, m_title_left));
    // const ImageStats title_right = image_stats(extract_box_reference(frame, m_title_right));

    // ImageStats top_white = image_stats(extract_box_reference(frame, m_top_white));
    // hits += is_white(top_white, 480, 30) ? 1 : 0;

    // ImageStats bottom_white = image_stats(extract_box_reference(frame, m_bottom_white));
    // hits += is_white(bottom_white, 480, 30) ? 1 : 0;

    // ImageStats left_white = image_stats(extract_box_reference(frame, m_left_white));
    // hits += is_white(left_white, 480, 30) ? 1 : 0;

    // ImageStats right_white = image_stats(extract_box_reference(frame, m_right_white));
    // hits += is_white(right_white, 480, 30) ? 1 : 0;

    // bool detected = hits == 5;
    // m_detected.store(detected, std::memory_order_release);

    // return detected && m_stop_on_detected;
}



FlatWhiteDialogDetector::FlatWhiteDialogDetector(Color color, VideoOverlay* overlay)
    : m_color(color)
    , m_overlay(overlay)
    , m_top(0.267838, 0.784156, 0.467618, 0.019531)
    , m_arrow_box(0.727, 0.868, 0.037, 0.086)
{}
void FlatWhiteDialogDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_top);
    items.add(m_color, m_arrow_box);
}
bool FlatWhiteDialogDetector::detect(const ImageViewRGB32& screen){
    if (!is_white(extract_box_reference(screen, m_top), 500.0, 20.0)){
        m_last_detected_box.reset();
        return false;
    }

    double screen_rel_size = (screen.height() / 1080.0);
    double screen_rel_size_2 = screen_rel_size * screen_rel_size;

    double min_area_1080p = 150.0;
    double rmsd_threshold = 120.0;
    size_t min_area = size_t(screen_rel_size_2 * min_area_1080p);

    const std::vector<std::pair<uint32_t, uint32_t>> FILTERS = {
        {combine_rgb(0,0,0), combine_rgb(100, 100, 100)},
    };

    bool found = match_template_by_waterfill(
        screen.size(),
        extract_box_reference(screen, m_arrow_box),
        DialogBlackArrowMatcher::instance(),
        FILTERS,
        {min_area, SIZE_MAX},
        rmsd_threshold,
        [&](Kernels::Waterfill::WaterfillObject& object) -> bool {
            m_last_detected = translate_to_parent(screen, m_arrow_box, object);
            return true;
        }
    );

    if (m_overlay){
        if (found){
            m_last_detected_box.emplace(*m_overlay, m_last_detected, COLOR_GREEN);
        }else{
            m_last_detected_box.reset();
        }
    }

    return found;
}



BlueDialogDetector::BlueDialogDetector(Color color, VideoOverlay* overlay)
    : m_color(color)
    , m_overlay(overlay)
    , m_corner(0.765093, 0.933594, 0.006586, 0.013672)
    , m_arrow_box(0.727, 0.868, 0.037, 0.086)
{}
void BlueDialogDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_corner);
    items.add(m_color, m_arrow_box);
}
bool BlueDialogDetector::detect(const ImageViewRGB32& screen){
    do{
        //  Check 2 shades of blue.
        if (is_solid(
            extract_box_reference(screen, m_corner),
            {0.0869318, 0.255479, 0.65759},
            0.25
        )){
            break;
        }
        if (is_solid(
            extract_box_reference(screen, m_corner),
            {0, 0.134207, 0.865793},
            0.25
        )){
            break;
        }

        m_last_detected_box.reset();
//        cout << "not solid" << endl;
        return false;
    }while (false);

    double screen_rel_size = (screen.height() / 1080.0);
    double screen_rel_size_2 = screen_rel_size * screen_rel_size;

    double min_area_1080p = 150.0;
    double rmsd_threshold = 120.0;
    size_t min_area = size_t(screen_rel_size_2 * min_area_1080p);

    const std::vector<std::pair<uint32_t, uint32_t>> FILTERS = {
        {0xffc0c0c0, 0xffffffff},
        {0xffb0b0b0, 0xffffffff},
        {0xffa0a0a0, 0xffffffff},
        {0xff909090, 0xffffffff},
        {0xff808080, 0xffffffff},
    };

    bool found = match_template_by_waterfill(
        screen.size(),
        extract_box_reference(screen, m_arrow_box),
        DialogWhiteArrowMatcher::instance(),
        FILTERS,
        {min_area, SIZE_MAX},
        rmsd_threshold,
        [&](Kernels::Waterfill::WaterfillObject& object) -> bool {
            m_last_detected = translate_to_parent(screen, m_arrow_box, object);
            return true;
        }
    );

    if (m_overlay){
        if (found){
            m_last_detected_box.emplace(*m_overlay, m_last_detected, COLOR_GREEN);
        }else{
            m_last_detected_box.reset();
        }
    }

    return found;
}



ItemReceiveDetector::ItemReceiveDetector(Color color, VideoOverlay* overlay)
    : m_color(color)
    , m_overlay(overlay)
    , m_arrow_box(0.718648, 0.875728, 0.034896, 0.056311)
{}
void ItemReceiveDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_arrow_box);
}
bool ItemReceiveDetector::detect(const ImageViewRGB32& screen){
    double screen_rel_size = (screen.height() / 1080.0);
    double screen_rel_size_2 = screen_rel_size * screen_rel_size;

    double min_area_1080p = 150.0;
    double rmsd_threshold = 120.0;
    size_t min_area = size_t(screen_rel_size_2 * min_area_1080p);

    const std::vector<std::pair<uint32_t, uint32_t>> FILTERS = {
        {0xffc0c0c0, 0xffffffff},
        {0xffb0b0b0, 0xffffffff},
        {0xffa0a0a0, 0xffffffff},
        {0xff909090, 0xffffffff},
        {0xff808080, 0xffffffff},
    };

    bool found = match_template_by_waterfill(
        screen.size(),
        extract_box_reference(screen, m_arrow_box),
        DialogWhiteArrowMatcher::instance(),
        FILTERS,
        {min_area, SIZE_MAX},
        rmsd_threshold,
        [&](Kernels::Waterfill::WaterfillObject& object) -> bool {
            m_last_detected = translate_to_parent(screen, m_arrow_box, object);
            return true;
        }
    );

    if (m_overlay){
        if (found){
            m_last_detected_box.emplace(*m_overlay, m_last_detected, COLOR_GREEN);
        }else{
            m_last_detected_box.reset();
        }
    }

    return found;
}



TealDialogDetector::TealDialogDetector(Color color, VideoOverlay* overlay)
    : m_color(color)
    , m_overlay(overlay)
    , m_arrow_box(0.727, 0.868, 0.037, 0.086)
{}
void TealDialogDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_arrow_box);
}
bool TealDialogDetector::detect(const ImageViewRGB32& screen){
    double screen_rel_size = (screen.height() / 1080.0);
    double screen_rel_size_2 = screen_rel_size * screen_rel_size;

    double min_area_1080p = 150.0;
    double rmsd_threshold = 120.0;
    size_t min_area = size_t(screen_rel_size_2 * min_area_1080p);

    const std::vector<std::pair<uint32_t, uint32_t>> FILTERS = {
        {0xff00c080, 0xff3fffff},
    };

    bool found = match_template_by_waterfill(
        screen.size(),
        extract_box_reference(screen, m_arrow_box),
        DialogTealArrowMatcher::instance(),
        FILTERS,
        {min_area, SIZE_MAX},
        rmsd_threshold,
        [&](Kernels::Waterfill::WaterfillObject& object) -> bool {
            m_last_detected = translate_to_parent(screen, m_arrow_box, object);
            return true;
        }
    );

    if (m_overlay){
        if (found){
            m_last_detected_box.emplace(*m_overlay, m_last_detected, COLOR_GREEN);
        }else{
            m_last_detected_box.reset();
        }
    }

    return found;
}





}
}
}












