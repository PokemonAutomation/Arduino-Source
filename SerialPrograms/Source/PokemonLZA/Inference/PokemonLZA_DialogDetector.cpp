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

//#include <iostream>
//using std::cout;
//using std::endl;

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

    static const ImageMatch::WaterfillTemplateMatcher& instance(){
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
        m_aspect_ratio_upper = 1.2;
        m_area_ratio_lower = 0.83;
        m_area_ratio_upper = 1.1;
    }

    static const ImageMatch::WaterfillTemplateMatcher& instance(){
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

    static const ImageMatch::WaterfillTemplateMatcher& instance(){
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

    static const ImageMatch::WaterfillTemplateMatcher& instance(){
        static DialogTealArrowMatcher matcher;
        return matcher;
    }
};

class DialogBlueArrowMatcher : public ImageMatch::WaterfillTemplateMatcher{
public:
    DialogBlueArrowMatcher()
        : WaterfillTemplateMatcher(
            "PokemonLZA/DialogBox/DialogBoxBlueArrow-Template.png",
            Color(0xff005460), Color(0xff6BBEC9), 50
        )
    {
        m_aspect_ratio_lower = 0.9;
        m_aspect_ratio_upper = 1.1;
        m_area_ratio_lower = 0.8;
        m_area_ratio_upper = 1.1;
    }

    static const ImageMatch::WaterfillTemplateMatcher& instance(){
        static DialogBlueArrowMatcher matcher;
        return matcher;
    }
};


namespace {

const static ImageFloatBox DIALOG_ARROW_BOX{0.727, 0.868, 0.037, 0.086};


// detect the white arrow in blue dialog box and transparent battle dialog box
bool detect_white_arrow(const ImageViewRGB32& screen, PokemonAutomation::ImageFloatBox& found_box){
    const double screen_rel_size = (screen.height() / 1080.0);
    const double screen_rel_size_2 = screen_rel_size * screen_rel_size;

    const double min_area_1080p = 150.0;
    const double rmsd_threshold = 120.0;
    const size_t min_area = size_t(screen_rel_size_2 * min_area_1080p);

    const std::vector<std::pair<uint32_t, uint32_t>> FILTERS = {
        {0xffc0c0c0, 0xffffffff},
        {0xffb0b0b0, 0xffffffff},
        {0xffa0a0a0, 0xffffffff},
        {0xff909090, 0xffffffff},
        {0xff808080, 0xffffffff},
    };

    bool found = match_template_by_waterfill(
        screen.size(),
        extract_box_reference(screen, DIALOG_ARROW_BOX),
        DialogWhiteArrowMatcher::instance(),
        FILTERS,
        {min_area, SIZE_MAX},
        rmsd_threshold,
        [&](Kernels::Waterfill::WaterfillObject& object) -> bool {
            found_box = translate_to_parent(screen, DIALOG_ARROW_BOX, object);
            return true;
        }
    );
    return found;
}

bool detect_blue_arrow(const ImageViewRGB32& screen, PokemonAutomation::ImageFloatBox& found_box){
    double screen_rel_size = (screen.height() / 1080.0);
    double screen_rel_size_2 = screen_rel_size * screen_rel_size;

    double min_area_1080p = 150.0;
    double rmsd_threshold = 120.0;
    size_t min_area = size_t(screen_rel_size_2 * min_area_1080p);

    const std::vector<std::pair<uint32_t, uint32_t>> FILTERS = {
        {0x005460, 0xff7FC0C9},
    };

    bool found = match_template_by_waterfill(
        screen.size(),
        extract_box_reference(screen, DIALOG_ARROW_BOX),
        DialogBlueArrowMatcher::instance(),
        FILTERS,
        {min_area, SIZE_MAX},
        rmsd_threshold,
        [&](Kernels::Waterfill::WaterfillObject& object) -> bool {
            found_box = translate_to_parent(screen, DIALOG_ARROW_BOX, object);
            return true;
        }
    );
    return found;
}

} // end anonymous namespace



FlatWhiteDialogDetector::FlatWhiteDialogDetector(Color color, VideoOverlay* overlay)
    : m_color(color)
    , m_overlay(overlay)
    , m_bottom(0.265, 0.931, 0.465, 0.020)
{}
void FlatWhiteDialogDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_bottom);
    items.add(m_color, DIALOG_ARROW_BOX);
}
bool FlatWhiteDialogDetector::detect(const ImageViewRGB32& screen){
    if (!is_white(extract_box_reference(screen, m_bottom), 500.0, 20.0)){
        m_last_detected_box_scope.reset();
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
        extract_box_reference(screen, DIALOG_ARROW_BOX),
        DialogBlackArrowMatcher::instance(),
        FILTERS,
        {min_area, SIZE_MAX},
        rmsd_threshold,
        [&](Kernels::Waterfill::WaterfillObject& object) -> bool {
            m_last_detected_box = translate_to_parent(screen, DIALOG_ARROW_BOX, object);
            return true;
        }
    );

    if (m_overlay){
        if (found){
            m_last_detected_box_scope.emplace(*m_overlay, m_last_detected_box, COLOR_GREEN);
        }else{
            m_last_detected_box_scope.reset();
        }
    }

    return found;
}



BlueDialogDetector::BlueDialogDetector(Color color, VideoOverlay* overlay)
    : m_color(color)
    , m_overlay(overlay)
    , m_corner(0.765093, 0.933594, 0.006586, 0.013672)
{}
void BlueDialogDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_corner);
    items.add(m_color, DIALOG_ARROW_BOX);
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

        m_last_detected_box_scope.reset();
//        cout << "not solid" << endl;
        return false;
    }while (false);

    const bool found = detect_white_arrow(screen, m_last_detected_box);

    if (m_overlay){
        if (found){
            m_last_detected_box_scope.emplace(*m_overlay, m_last_detected_box, COLOR_GREEN);
        }else{
            m_last_detected_box_scope.reset();
        }
    }

    return found;
}



ItemReceiveDetector::ItemReceiveDetector(Color color, VideoOverlay* overlay)
    : m_color(color)
    , m_overlay(overlay)
    , m_top(0.309013, 0.719466, 0.418455, 0.015267)
    , m_arrow_box(0.718648, 0.875728, 0.034896, 0.056311)
{}
void ItemReceiveDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_top);
    items.add(m_color, m_arrow_box);
}
bool ItemReceiveDetector::detect(const ImageViewRGB32& screen){
    ImageStats top = image_stats(extract_box_reference(screen, m_top));
//    cout << top.average << top.stddev << endl;
    if (top.average.sum() > 250){
        return false;
    }

    do{
        //  We will probably need to add more color ratios here later.
        if (is_solid(top, {0.153583, 0.245751, 0.600666}, 0.25, 40)){
            break;
        }
        if (is_solid(top, {0.00228537, 0.070115, 0.9276}, 0.25, 40)){
            break;
        }

        m_last_detected_box_scope.reset();
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
            m_last_detected_box = translate_to_parent(screen, m_arrow_box, object);
            return true;
        }
    );

    if (m_overlay){
        if (found){
            m_last_detected_box_scope.emplace(*m_overlay, m_last_detected_box, COLOR_GREEN);
        }else{
            m_last_detected_box_scope.reset();
        }
    }

    return found;
}



TealDialogDetector::TealDialogDetector(Color color, VideoOverlay* overlay)
    : m_color(color)
    , m_overlay(overlay)
{}
void TealDialogDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, DIALOG_ARROW_BOX);
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
        extract_box_reference(screen, DIALOG_ARROW_BOX),
        DialogTealArrowMatcher::instance(),
        FILTERS,
        {min_area, SIZE_MAX},
        rmsd_threshold,
        [&](Kernels::Waterfill::WaterfillObject& object) -> bool {
            m_last_detected_box = translate_to_parent(screen, DIALOG_ARROW_BOX, object);
            return true;
        }
    );

    if (m_overlay){
        if (found){
            m_last_detected_box_scope.emplace(*m_overlay, m_last_detected_box, COLOR_GREEN);
        }else{
            m_last_detected_box_scope.reset();
        }
    }

    return found;
}



TransparentBattleDialogDetector::TransparentBattleDialogDetector(Color color, VideoOverlay* overlay)
    : m_color(color)
    , m_overlay(overlay)
{}
void TransparentBattleDialogDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, DIALOG_ARROW_BOX);
}
bool TransparentBattleDialogDetector::detect(const ImageViewRGB32& screen){
    const bool found = detect_white_arrow(screen, m_last_detected_box);

    if (m_overlay){
        if (found){
            m_last_detected_box_scope.emplace(*m_overlay, m_last_detected_box, COLOR_GREEN);
        }else{
            m_last_detected_box_scope.reset();
        }
    }

    return found;
}


LightBlueDialogDetector::LightBlueDialogDetector(Color color, VideoOverlay* overlay)
    : m_color(color)
    , m_overlay(overlay)
    , m_corner(0.765093, 0.933594, 0.006586, 0.013672)
{}
void LightBlueDialogDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_corner);
    items.add(m_color, DIALOG_ARROW_BOX);
}
bool LightBlueDialogDetector::detect(const ImageViewRGB32& screen){
    do{
        if (is_solid(
            extract_box_reference(screen, m_corner),
            {0.108317, 0.462282, 0.429400},
            0.25
        )){
            break;
        }
        m_last_detected_box_scope.reset();
//        cout << "not solid" << endl;
        return false;
    }while (false);

    const bool found = detect_blue_arrow(screen, m_last_detected_box);

    if (m_overlay){
        if (found){
            m_last_detected_box_scope.emplace(*m_overlay, m_last_detected_box, COLOR_GREEN);
        }else{
            m_last_detected_box_scope.reset();
        }
    }

    return found;
}



}
}
}












