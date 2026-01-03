/*  Box Info Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Images/ImageFilter.h"
#include "CommonTools/OCR/OCR_NumberReader.h"
#include "Kernels/Waterfill/Kernels_Waterfill_Types.h"
#include "CommonTools/ImageMatch/WaterfillTemplateMatcher.h"
#include "CommonTools/Images/WaterfillUtilities.h"
#include "PokemonLZA/Resources/PokemonLZA_AvailablePokemon.h"
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

BoxDexNumberDetector::BoxDexNumberDetector(Logger& logger) : m_logger(logger), m_dex_number_box{0.510, 0.203, 0.039, 0.031}, m_dex_type_box{0.472, 0.204, 0.018, 0.030}{}

void BoxDexNumberDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(COLOR_GRAY, m_dex_number_box);
    items.add(COLOR_GRAY, m_dex_type_box);
}

bool BoxDexNumberDetector::detect(const ImageViewRGB32& screen){
    const size_t max_dex_number = std::max(LUMIOSE_DEX_SLUGS().size(), HYPERSPACE_DEX_SLUGS().size());

    const int dex_number = [&](){
        const ImageViewRGB32 dex_image_crop = extract_box_reference(screen, m_dex_number_box);
        return OCR::read_number(m_logger, dex_image_crop);

#if 0
        const bool text_inside_range = true;
        const bool prioritize_numeric_only_results = true;
        const size_t width_max = SIZE_MAX;
        // To accomodate the dex number "No. xxx" for all language, we have to make the dex number crop to cover the "dot" character
        // for some languages. We have to use `min_digit_area` to filter out the dot when doing OCR.
        // The min digit area computation is that any dot with size smaller than image_crop.height()/5 is filtered out when OCR.
        const size_t min_digit_area = dex_image_crop.height()*dex_image_crop.height() / 25;
        return OCR::read_number_waterfill_multifilter(m_logger, dex_image_crop,
            {
                {0x0, 0xff808080},
                {0x0, 0xff909090},
                {0x0, 0xffA0A0A0},
            },
            text_inside_range, prioritize_numeric_only_results, width_max, min_digit_area
        );
#endif
    }();
    if (dex_number <= 0 || dex_number > static_cast<int>(max_dex_number)) {
        m_dex_number = 0;
        m_dex_number_when_error = dex_number;
        return false;
    }
    m_dex_number = static_cast<uint16_t>(dex_number);
    m_dex_number_when_error = 0;

    // Replacing white background with zero-alpha color so that they won't be counted in
    // the following image_stats()
    // The white background is defined as the color between 0xffa0a0a0 and 0xffffffff.
    const bool replace_color_within_range = true;
    ImageRGB32 region = filter_rgb32_range(
        extract_box_reference(screen, m_dex_type_box),
        0xffa0a0a0, 0xffffffff, Color(0), replace_color_within_range
    );

    ImageStats stats = image_stats(region);
    m_dex_type_color_ratio = stats.count / ((double)region.width() * region.height());
    m_dex_type = (m_dex_type_color_ratio > 0.2) ? DexType::LUMIOSE : DexType::HYPERSPACE;

    return true;
}

}
}
}
