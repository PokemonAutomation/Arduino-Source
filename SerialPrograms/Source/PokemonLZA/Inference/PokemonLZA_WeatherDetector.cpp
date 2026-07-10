#include "CommonFramework/Globals.h"
#include "CommonFramework/ImageTools/ImageDiff.h"
#include "CommonTools/Images/WaterfillUtilities.h"
#include "CommonTools/ImageMatch/WaterfillTemplateMatcher.h"
#include "PokemonLZA_WeatherDetector.h"
#include <algorithm>
#include <stdexcept>
#include <utility>
#include <vector>

namespace PokemonAutomation {
namespace NintendoSwitch {
namespace PokemonLZA {

namespace{
const ImageFloatBox WEATHER_ICON_ROI(0.880000, 0.010000, 0.035800, 0.068000);

struct SupplementalTemplateCheck{
    const char* path;
    ImageFloatBox box;
    Color color;
    double rmsd_threshold;
};

const std::vector<SupplementalTemplateCheck>& supplemental_template_checks(WeatherIconType type){
    static const std::vector<SupplementalTemplateCheck> NONE = {};

    static const std::vector<SupplementalTemplateCheck> RAIN = {
        {"PokemonLZA/Weather/rain_cloud.png", ImageFloatBox(0.8865, 0.0265, 0.0210, 0.0270), COLOR_GREEN, 90.0},
        {"PokemonLZA/Weather/rain_drop.png",  ImageFloatBox(0.8885, 0.0525, 0.0050, 0.0150), COLOR_BLUE,  90.0},
    };
    static const std::vector<SupplementalTemplateCheck> CLOUDY = {
        {"PokemonLZA/Weather/cloudy_cloud.png", ImageFloatBox(0.8895, 0.0430, 0.0210, 0.0265), COLOR_GREEN, 90.0},
        {"PokemonLZA/Weather/cloudy_drop.png",  ImageFloatBox(0.8915, 0.0255, 0.0050, 0.0100), COLOR_BLUE,  90.0},
    };
    static const std::vector<SupplementalTemplateCheck> RAINBOW = {
        {"PokemonLZA/Weather/rainbow_cloud.png", ImageFloatBox(0.8840, 0.0465, 0.0140, 0.0165), COLOR_GREEN, 90.0},
        {"PokemonLZA/Weather/rainbow_arch.png",  ImageFloatBox(0.8930, 0.0420, 0.0120, 0.0100), COLOR_BLUE,  90.0},
    };
    static const std::vector<SupplementalTemplateCheck> FOGGY = {
        {"PokemonLZA/Weather/foggy_tray_1.png", ImageFloatBox(0.8893, 0.0487, 0.0218, 0.0080), COLOR_GREEN, 90.0},
        {"PokemonLZA/Weather/foggy_tray_2.png", ImageFloatBox(0.8880, 0.0555, 0.0225, 0.0080), COLOR_BLUE,  90.0},
    };

    switch (type){
    case WeatherIconType::Rain:
        return RAIN;
    case WeatherIconType::Cloudy:
        return CLOUDY;
    case WeatherIconType::Rainbow:
        return RAINBOW;
    case WeatherIconType::Foggy:
        return FOGGY;
    default:
        return NONE;
    }
}

class WeatherFullMatcher : public ImageMatch::WaterfillTemplateMatcher{
public:
    WeatherFullMatcher(const char* path, double max_rmsd)
        : WaterfillTemplateMatcher(path, Color(0xff707070), Color(0xffffffff), 50)
        , m_max_rmsd(max_rmsd)
    {
        m_aspect_ratio_lower = 0.60;
        m_aspect_ratio_upper = 1.40;
        m_area_ratio_lower = 0.55;
        m_area_ratio_upper = 1.45;
    }

    static const WeatherFullMatcher& clear(){
        static const WeatherFullMatcher matcher("PokemonLZA/Weather/clear_full.png", 100.0);
        return matcher;
    }
    static const WeatherFullMatcher& sunny(){
        static const WeatherFullMatcher matcher("PokemonLZA/Weather/sunny_full.png", 100.0);
        return matcher;
    }
    static const WeatherFullMatcher& rain(){
        static const WeatherFullMatcher matcher("PokemonLZA/Weather/rain_full.png", 100.0);
        return matcher;
    }
    static const WeatherFullMatcher& cloudy(){
        static const WeatherFullMatcher matcher("PokemonLZA/Weather/cloudy_full.png", 100.0);
        return matcher;
    }
    static const WeatherFullMatcher& foggy(){
        static const WeatherFullMatcher matcher("PokemonLZA/Weather/foggy_full.png", 100.0);
        return matcher;
    }
    static const WeatherFullMatcher& rainbow(){
        static const WeatherFullMatcher matcher("PokemonLZA/Weather/rainbow_full.png", 100.0);
        return matcher;
    }

    double m_max_rmsd;
};

const WeatherFullMatcher& weather_full_matcher(WeatherIconType type){
    switch (type){
    case WeatherIconType::Clear:
        return WeatherFullMatcher::clear();
    case WeatherIconType::Sunny:
        return WeatherFullMatcher::sunny();
    case WeatherIconType::Rain:
        return WeatherFullMatcher::rain();
    case WeatherIconType::Cloudy:
        return WeatherFullMatcher::cloudy();
    case WeatherIconType::Foggy:
        return WeatherFullMatcher::foggy();
    case WeatherIconType::Rainbow:
        return WeatherFullMatcher::rainbow();
    default:
        throw std::runtime_error("No weather full matcher for requested WeatherIconType");
    }
}
}

//-----------------------------------------------------
//  Detector
//-----------------------------------------------------

WeatherIconDetector::WeatherIconDetector(WeatherIconType type, VideoOverlay* overlay)
{
    m_type = type;
    if (overlay){
        m_overlay1.emplace(*overlay, WEATHER_ICON_ROI, COLOR_RED);
    }
}

void WeatherIconDetector::make_overlays(VideoOverlaySet& items) const {
    items.add(COLOR_RED, WEATHER_ICON_ROI);
    for (const auto& check : supplemental_template_checks(m_type)){
        items.add(check.color, check.box);
    }
}

bool WeatherIconDetector::detect(const ImageViewRGB32& screen){
    const WeatherFullMatcher& matcher = weather_full_matcher(m_type);

    const double scale = screen.height() / 1080.0;
    const size_t min_area = (size_t)(scale * scale * 120.0);

    static const std::vector<std::pair<uint32_t, uint32_t>> FILTERS = {
        {0xff707070, 0xffffffff},
    };

    const bool full_match = match_template_by_waterfill(
        screen.size(),
        extract_box_reference(screen, WEATHER_ICON_ROI),
        matcher,
        FILTERS,
        {min_area, SIZE_MAX},
        matcher.m_max_rmsd,
        [](Kernels::Waterfill::WaterfillObject& object) -> bool {
            (void)object;
            return true;
        }
    );

    if (!full_match){
        return false;
    }

    for (const auto& check : supplemental_template_checks(m_type)){
        ImageViewRGB32 candidate = extract_box_reference(screen, check.box);
        ImageRGB32 templ(RESOURCE_PATH() + check.path);
        if (templ.width() == 0 || templ.height() == 0){
            return false;
        }

        auto compute_rmsd = [&](const ImageViewRGB32& image) -> double{
            return image.width() == templ.width() && image.height() == templ.height()
                ? ImageMatch::pixel_RMSD(image, templ)
                : ImageMatch::pixel_RMSD(image, templ.scale_to(image.width(), image.height()));
        };

        double rmsd = compute_rmsd(candidate);

        if (screen.height() < 1080){
            const int search_radius = std::min(candidate.width(), candidate.height()) <= 12 ? 2 : 1;
            for (int dy = -search_radius; dy <= search_radius; dy++){
                for (int dx = -search_radius; dx <= search_radius; dx++){
                    if (dx == 0 && dy == 0){
                        continue;
                    }
                    ImageViewRGB32 shifted = extract_box_reference(screen, check.box, dx, dy);
                    if (shifted.width() != candidate.width() || shifted.height() != candidate.height()){
                        continue;
                    }
                    rmsd = std::min(rmsd, compute_rmsd(shifted));
                }
            }
        }

        if (rmsd >= check.rmsd_threshold){
            return false;
        }
    }

    return true;
}


}
}
}
