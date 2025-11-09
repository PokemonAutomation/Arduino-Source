/*  Box Info Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "Kernels/Waterfill/Kernels_Waterfill_Types.h"
#include "CommonTools/ImageMatch/WaterfillTemplateMatcher.h"
#include "CommonTools/Images/WaterfillUtilities.h"
#include "PokemonLZA_BoxInfoDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{


class BoxShinySymbolMatcher : public ImageMatch::WaterfillTemplateMatcher{
public:
    BoxShinySymbolMatcher(const char* path)
        : WaterfillTemplateMatcher(
            path,
            Color(0xff000000), Color(0xffb4b4b4), 40
        )
    {
        m_aspect_ratio_lower = 0.9;
        m_aspect_ratio_upper = 1.1;
        m_area_ratio_lower = 0.85;
        m_area_ratio_upper = 1.3;
    }

    static const BoxShinySymbolMatcher& matcher(){
        const static BoxShinySymbolMatcher matcher("PokemonLZA/BoxShiny-Template.png");
        return matcher;
    }
};

BoxShinyDetector::BoxShinyDetector(Color color, VideoOverlay* overlay)
    : m_color(color)
    , m_box{0.809, 0.109, 0.095, 0.039}
    , m_overlay(overlay)
{}

void BoxShinyDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_box);
}

bool BoxShinyDetector::detect(const ImageViewRGB32& screen){
    double screen_rel_size = (screen.height() / 1080.0);
    double screen_rel_size_2 = screen_rel_size * screen_rel_size;

    double min_area_1080p = 100.0;
//    double rmsd_threshold = 120.0;
    size_t min_area = size_t(screen_rel_size_2 * min_area_1080p);

    const std::vector<std::pair<uint32_t, uint32_t>> FILTERS = {
        {0xff000000, 0xff3c3c3c}, // 60
        {0xff000000, 0xff646464}, // 100
        {0xff000000, 0xff8c8c8c}, // 140
        {0xff000000, 0xffb4b4b4}, // 180
    };

    const double max_rsmd = 100.0;
    bool found = match_template_by_waterfill(
        screen.size(),
        extract_box_reference(screen, m_box),
        BoxShinySymbolMatcher::matcher(),
        FILTERS,
        {min_area, SIZE_MAX},
        max_rsmd,
        [&](Kernels::Waterfill::WaterfillObject& object) -> bool {
//            cout << "width = " << object.width() << ", height = " << object.height() << endl;
            m_last_detected = translate_to_parent(screen, m_box, object);
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


class BoxAlphaSymbolMatcher : public ImageMatch::WaterfillTemplateMatcher{
public:
    BoxAlphaSymbolMatcher(const char* path)
        : WaterfillTemplateMatcher(
            path,
            Color(combine_rgb(200, 50, 50)), Color(combine_rgb(255, 100, 100)), 200
        )
    {
        m_aspect_ratio_lower = 0.9;
        m_aspect_ratio_upper = 1.1;
        m_area_ratio_lower = 0.85;
        m_area_ratio_upper = 1.1;
    }

    static const BoxAlphaSymbolMatcher& matcher(){
        const static BoxAlphaSymbolMatcher matcher("PokemonLZA/AlphaSymbol-Template.png");
        return matcher;
    }
};

BoxAlphaDetector::BoxAlphaDetector(Color color, VideoOverlay* overlay)
    : m_color(color)
    , m_box{0.820, 0.107, 0.085, 0.042}
    , m_overlay(overlay)
{}

void BoxAlphaDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_box);
}

bool BoxAlphaDetector::detect(const ImageViewRGB32& screen){
    double screen_rel_size = (screen.height() / 1080.0);
    double screen_rel_size_2 = screen_rel_size * screen_rel_size;

    double min_area_1080p = 300.0;
    size_t min_area = size_t(screen_rel_size_2 * min_area_1080p);

    const std::vector<std::pair<uint32_t, uint32_t>> FILTERS = {
        {combine_rgb(200, 50, 50), combine_rgb(255, 150, 150)},
        {combine_rgb(200, 50, 50), combine_rgb(255, 100, 100)},
        {combine_rgb(150, 0, 0), combine_rgb(255, 100, 100)},
        {combine_rgb(100, 0, 0), combine_rgb(255, 100, 100)},
    };

    const double max_rsmd = 100.0;
    bool found = match_template_by_waterfill(
        screen.size(),
        extract_box_reference(screen, m_box),
        BoxAlphaSymbolMatcher::matcher(),
        FILTERS,
        {min_area, SIZE_MAX},
        max_rsmd,
        [&](Kernels::Waterfill::WaterfillObject& object) -> bool {
//            cout << "width = " << object.width() << ", height = " << object.height() << endl;
            m_last_detected = translate_to_parent(screen, m_box, object);
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

BoxPageInfoWatcher::BoxPageInfoWatcher(VideoOverlay* overlay)
: VisualInferenceCallback("BoxPageInfoWatcher")
, m_shiny_watcher(COLOR_BLACK, overlay)
, m_alpha_watcher(COLOR_RED, overlay)
{}

void BoxPageInfoWatcher::make_overlays(VideoOverlaySet& items) const{
    m_shiny_watcher.make_overlays(items);
    m_alpha_watcher.make_overlays(items);
}

bool BoxPageInfoWatcher::process_frame(const ImageViewRGB32& frame, WallClock timestamp){
    bool shiny_determined = m_shiny_watcher.process_frame(frame, timestamp);
    bool alpha_determined = m_alpha_watcher.process_frame(frame, timestamp);
    return shiny_determined && alpha_determined;
}

std::string BoxPageInfoWatcher::info_str() const{
    const bool is_shiny = m_shiny_watcher.consistent_result();
    const bool is_alpha = m_alpha_watcher.consistent_result();

    if (is_shiny && is_alpha){
        return "Shiny Alpha";
    } else if (is_shiny){
        return "Shiny";
    } else if (is_alpha){
        return "Alpha";
    }
    return "Regular";
}



}
}
}
