/*  PC Detection
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <iostream>
#include "Common/Cpp/Exceptions.h"
#include "Kernels/Waterfill/Kernels_Waterfill_Types.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/Tools/GlobalThreadPools.h"
#include "CommonTools/ImageMatch/WaterfillTemplateMatcher.h"
#include "CommonTools/Images/WaterfillUtilities.h"
#include "CommonTools/Images/SolidColorTest.h"
#include "CommonTools/OCR/OCR_NumberReader.h"
#include "CommonTools/OCR/OCR_RawOCR.h"
#include "PokemonPokopia_PCDetection.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonPokopia{



class InfoIconMatcher : public ImageMatch::WaterfillTemplateMatcher{
public:
    InfoIconMatcher(const char* path)
        : WaterfillTemplateMatcher(
            path,
            Color(210, 120, 0), Color(255, 190, 100), 700
        )
    {
        m_aspect_ratio_lower = 0.9;
        m_aspect_ratio_upper = 1.1;
        m_area_ratio_lower = 0.85;
        m_area_ratio_upper = 1.1;
    }

    static const InfoIconMatcher& matcher(){
        static InfoIconMatcher matcher("PokemonPokopia/InfoIcon.png");
        return matcher;
    }
};

InfoIconDetector::InfoIconDetector(
    Color color,
    VideoOverlay* overlay,
    const ImageFloatBox& box
)
    : m_color(color)
    , m_overlay(overlay)
    , m_arrow_box(box)
{}
void InfoIconDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_arrow_box);
}
bool InfoIconDetector::detect(const ImageViewRGB32& screen){
    double screen_rel_size = (screen.height() / 1080.0);
    double screen_rel_size_2 = screen_rel_size * screen_rel_size;

    double min_area_1080p = 700;
    double rmsd_threshold = 80;
    size_t min_area = size_t(screen_rel_size_2 * min_area_1080p);

    const std::vector<std::pair<uint32_t, uint32_t>> FILTERS = {
        {0xffd26e00, 0xffffbe64} // RGB(210, 120, 0), RGB(255, 190, 100)
    };

    bool found = match_template_by_waterfill(
        screen.size(),
        extract_box_reference(screen, m_arrow_box),
        InfoIconMatcher::matcher(),
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

class StampMatcher : public ImageMatch::WaterfillTemplateMatcher{
public:
    // image template matcher for stamps
    // - min_width: candidate image min width if video stream is 1080p
    // - min_height: candidate image min height if video stream is 1080p
    StampMatcher(Stamp stamp, size_t min_width, size_t min_height, double max_rmsd);
    static const StampMatcher& mew(){
        static StampMatcher matcher(Stamp::MEW, 40, 40, 70);
        return matcher;
    }

    virtual bool check_image(Resolution input_resolution, const ImageViewRGB32& image) const override{
        size_t min_width = m_min_width * input_resolution.width / 1920;
        size_t min_height = m_min_height * input_resolution.height / 1080;

        if (PreloadSettings::debug().IMAGE_TEMPLATE_MATCHING){
            const double stddev_sum = image_stats(image).stddev.sum();
            std::cout << "???? ButtonMatcher::check_image() ???? min size " << min_width << " x " << min_height
                 << " got " << image.width() << " x " << image.height() << " stddev_sum: " << stddev_sum << std::endl;
        }
        return image.width() >= min_width && image.height() >= min_height;
    };

    size_t m_min_width;
    size_t m_min_height;
    double m_max_rmsd;
};


const char* template_path(Stamp stamp){
    switch (stamp){
    case Stamp::MEW:
        return "PokemonPokopia/MewStamp.png";
    default:
        throw InternalProgramError(
            nullptr, PA_CURRENT_FUNCTION, "No corresponding template path for Stamp"
        );
    }
}

// Using same filter for template and detect for now
std::pair<uint32_t, uint32_t> filter(Stamp stamp){
    switch (stamp){
    case Stamp::MEW:
        return {0xff5a50aa, 0xffd2beff}; // RGB(90, 80, 170), RGB(210, 190, 255)
    default:
        throw InternalProgramError(
            nullptr, PA_CURRENT_FUNCTION, "No corresponding filter for Stamp"
        );
    }
}

const StampMatcher& get_stamp_matcher(Stamp stamp){
    switch (stamp){
    case Stamp::MEW:
        return StampMatcher::mew();
    default:
        throw InternalProgramError(
            nullptr, PA_CURRENT_FUNCTION, "No corresponding StampMatcher for Stamp"
        );
    }
}

StampMatcher::StampMatcher(Stamp stamp, size_t min_width, size_t min_height, double max_rmsd)
    : WaterfillTemplateMatcher(template_path(stamp), Color(filter(stamp).first), Color(filter(stamp).second), 100)
    , m_min_width(min_width)
    , m_min_height(min_height)
    , m_max_rmsd(max_rmsd)
{}



StampDetector::StampDetector(
    Color color,
    Stamp stamp,
    const ImageFloatBox& box,
    VideoOverlay* overlay
)
    : m_stamp(stamp)
    , m_color(color)
    , m_matcher(get_stamp_matcher(stamp))
    , m_box(box)
    , m_overlay(overlay)
{

}
void StampDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_box);
}
bool StampDetector::detect(const ImageViewRGB32& screen){

    double screen_rel_size = (screen.height() / 1080.0);
    double screen_rel_size_2 = screen_rel_size * screen_rel_size;

    double min_area_1080p = 800;
    size_t min_area = size_t(screen_rel_size_2 * min_area_1080p);

    bool found = match_template_by_waterfill(
        screen.size(),
        extract_box_reference(screen, m_box),
        m_matcher,
        {filter(m_stamp)},
        {min_area, SIZE_MAX},
        m_matcher.m_max_rmsd,
        [&](Kernels::Waterfill::WaterfillObject& object) -> bool {
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

class RecipeIconMatcher : public ImageMatch::WaterfillTemplateMatcher{
public:
    // Match the yellow negative space for the recipe icon
    RecipeIconMatcher(const char* path)
        : WaterfillTemplateMatcher(
            path,
            Color(210, 180, 0), Color(255, 250, 160), 2100
        )
    {
        m_aspect_ratio_lower = 0.9;
        m_aspect_ratio_upper = 1.1;
        m_area_ratio_lower = 0.85;
        m_area_ratio_upper = 1.1;
    }

    static const RecipeIconMatcher& single_matcher(){
        static RecipeIconMatcher matcher("PokemonPokopia/Recipe.png");
        return matcher;
    }
    static const RecipeIconMatcher& double_matcher(){
        static RecipeIconMatcher matcher("PokemonPokopia/DoubleRecipe.png");
        return matcher;
    }
    static const RecipeIconMatcher& triple_matcher(){
        static RecipeIconMatcher matcher("PokemonPokopia/TripleRecipe.png");
        return matcher;
    }
    static const RecipeIconMatcher& quad_matcher(){
        static RecipeIconMatcher matcher("PokemonPokopia/QuadRecipe.png");
        return matcher;
    }
};

RecipeIconDetector::RecipeIconDetector(
    Color color,
    VideoOverlay* overlay,
    const ImageFloatBox& box
)
    : m_color(color)
    , m_overlay(overlay)
    , m_arrow_box(box)
{}
void RecipeIconDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_arrow_box);
}
bool RecipeIconDetector::detect(const ImageViewRGB32& screen){
    double screen_rel_size = (screen.height() / 1080.0);
    double screen_rel_size_2 = screen_rel_size * screen_rel_size;

    double min_area_1080p = 500;
    double rmsd_threshold = 95;
    size_t min_area = size_t(screen_rel_size_2 * min_area_1080p);

    const std::vector<std::pair<uint32_t, uint32_t>> FILTERS = {
        {0xffd2b400, 0xfffffaa0} // RGB(210, 180, 0), RGB(255, 250, 160)
    };

    const std::vector<std::pair<const RecipeIconMatcher&, RecipeType>> matchers = {
        {RecipeIconMatcher::single_matcher(), RecipeType::SINGLE},
        {RecipeIconMatcher::double_matcher(), RecipeType::DOUBLE},
        {RecipeIconMatcher::triple_matcher(), RecipeType::TRIPLE},
        {RecipeIconMatcher::quad_matcher(), RecipeType::QUAD},
    };

    bool found = false;
    m_recipe_type = RecipeType::NOT_RECIPE;

    for (const auto& matcher_info : matchers) {
        found = match_template_by_waterfill(
            screen.size(),
            extract_box_reference(screen, m_arrow_box),
            matcher_info.first,
            FILTERS,
            {min_area, SIZE_MAX},
            rmsd_threshold,
            [&](Kernels::Waterfill::WaterfillObject& object) -> bool {
                m_last_detected = translate_to_parent(screen, m_arrow_box, object);
                return true;
            }
        );
        if (found) {
            m_recipe_type = matcher_info.second;
            break;
        }
    }

    if (m_overlay){
        if (found){
            m_last_detected_box.emplace(*m_overlay, m_last_detected, COLOR_GREEN);
        }else{
            m_last_detected_box.reset();
        }
    }

    return found;
}

CoinCountDetector::CoinCountDetector(Logger& logger)
: m_logger(logger)
// location in the shop menu
, m_coin_count_box{0.900000, 0.053000, 0.075000, 0.035000}
{}

void CoinCountDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(COLOR_WHITE, m_coin_count_box);
}

bool CoinCountDetector::detect(const ImageViewRGB32& screen){
    const ImageViewRGB32 coin_image_crop = extract_box_reference(screen, m_coin_count_box);

    const bool text_inside_range = true;
    const bool prioritize_numeric_only_results = true;
    const size_t width_max = SIZE_MAX;
    // The coin crop includes the "," in coin numbers like "1,000".
    // We have to use `min_digit_area` to filter out "," when doing OCR.
    // The min digit area computation is that any dot with size smaller than coin_image_crop.height()/5 is filtered out when OCR.
    const size_t min_digit_area = coin_image_crop.height()*coin_image_crop.height() / 25;
    m_coin_count = 0;
    const std::vector<std::pair<uint32_t, uint32_t>> filters = {
        {0xff808080, 0xffffffff},
        {0xffa0a0a0, 0xffffffff},
        {0xffc0c0c0, 0xffffffff},
        {0xffe0e0e0, 0xffffffff},
        {0xfff0f0f0, 0xffffffff},
    };
    int number = OCR::read_number_waterfill_multifilter(
        m_logger,
        GlobalThreadPools::computation_realtime(),
        coin_image_crop, filters,
        text_inside_range, prioritize_numeric_only_results, width_max, min_digit_area
    );
    if (number <= 0 || number > 999999){
        return false;
    }
    m_coin_count = static_cast<uint32_t>(number);
    return true;
}

CoinCountWatcher::CoinCountWatcher(Logger& logger)
: CoinCountDetector(logger), VisualInferenceCallback("CoinCountWatcher")
{}

void CoinCountWatcher::make_overlays(VideoOverlaySet& items) const{
    CoinCountDetector::make_overlays(items);
}

bool CoinCountWatcher::process_frame(const ImageViewRGB32& frame, WallClock timestamp){
    return detect(frame);
}

class CoinIconMatcher : public ImageMatch::WaterfillTemplateMatcher{
public:
    CoinIconMatcher()
        : WaterfillTemplateMatcher(
            "PokemonPokopia/Coin.png",
            Color(210, 150, 30), Color(255, 240, 140), 900
        )
    {
        m_aspect_ratio_lower = 0.9;
        m_aspect_ratio_upper = 1.1;
        m_area_ratio_lower = 0.85;
        m_area_ratio_upper = 1.1;
    }
};

CoinIconDetector::CoinIconDetector(
    Color color,
    VideoOverlay* overlay,
    const ImageFloatBox& box
)
    : m_color(color)
    , m_overlay(overlay)
    , m_arrow_box(box)
{}
void CoinIconDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_arrow_box);
}
bool CoinIconDetector::detect(const ImageViewRGB32& screen){
    double screen_rel_size = (screen.height() / 1080.0);
    double screen_rel_size_2 = screen_rel_size * screen_rel_size;

    double min_area_1080p = 200;
    double rmsd_threshold = 100;
    size_t min_area = size_t(screen_rel_size_2 * min_area_1080p);

    const std::vector<std::pair<uint32_t, uint32_t>> FILTERS = {
        {0xffd2961e, 0xfffff08c} // RGB(210, 150, 30), RGB(255, 240, 140)
    };

    bool found = match_template_by_waterfill(
        screen.size(),
        extract_box_reference(screen, m_arrow_box),
        CoinIconMatcher(),
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
