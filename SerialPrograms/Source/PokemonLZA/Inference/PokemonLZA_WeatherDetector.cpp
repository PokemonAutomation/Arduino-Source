/*  Weather Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
#include "CommonFramework/GlobalAutoPaths.h"
#include "CommonFramework/ImageTools/ImageDiff.h"
#include "CommonTools/Images/WaterfillUtilities.h"
#include "CommonTools/ImageMatch/WaterfillTemplateMatcher.h"
#include "Tests/TestUtils.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "PokemonLZA_WeatherDetector.h"
#include <array>
#include <algorithm>
#include <cctype>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{

namespace{

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
        {"PokemonLZA/Weather/cloudy_cloud.png", ImageFloatBox(0.8895, 0.0430, 0.0210, 0.0265), COLOR_GREEN, 100.0},
        {"PokemonLZA/Weather/cloudy_drop.png",  ImageFloatBox(0.8915, 0.0255, 0.0050, 0.0100), COLOR_BLUE,  100.0},
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

const char* weather_full_template_path(WeatherIconType type){
    switch (type){
    case WeatherIconType::Clear:   return "PokemonLZA/Weather/clear_full.png";
    case WeatherIconType::Sunny:   return "PokemonLZA/Weather/sunny_full.png";
    case WeatherIconType::Rain:    return "PokemonLZA/Weather/rain_full.png";
    case WeatherIconType::Cloudy:  return "PokemonLZA/Weather/cloudy_full.png";
    case WeatherIconType::Foggy:   return "PokemonLZA/Weather/foggy_full.png";
    case WeatherIconType::Rainbow: return "PokemonLZA/Weather/rainbow_full.png";
    default:                       return nullptr;
    }
}

const ImageRGB32& weather_full_template_image(WeatherIconType type){
    static const ImageRGB32 CLEAR(RESOURCE_PATH() + std::string(weather_full_template_path(WeatherIconType::Clear)));
    static const ImageRGB32 SUNNY(RESOURCE_PATH() + std::string(weather_full_template_path(WeatherIconType::Sunny)));
    static const ImageRGB32 RAIN(RESOURCE_PATH() + std::string(weather_full_template_path(WeatherIconType::Rain)));
    static const ImageRGB32 CLOUDY(RESOURCE_PATH() + std::string(weather_full_template_path(WeatherIconType::Cloudy)));
    static const ImageRGB32 FOGGY(RESOURCE_PATH() + std::string(weather_full_template_path(WeatherIconType::Foggy)));
    static const ImageRGB32 RAINBOW(RESOURCE_PATH() + std::string(weather_full_template_path(WeatherIconType::Rainbow)));

    switch (type){
    case WeatherIconType::Clear:   return CLEAR;
    case WeatherIconType::Sunny:   return SUNNY;
    case WeatherIconType::Rain:    return RAIN;
    case WeatherIconType::Cloudy:  return CLOUDY;
    case WeatherIconType::Foggy:   return FOGGY;
    case WeatherIconType::Rainbow: return RAINBOW;
    default:                       return CLEAR;
    }
}

double full_template_rmsd(const ImageViewRGB32& image, WeatherIconType type){
    const ImageRGB32& templ = weather_full_template_image(type);
    if (templ.width() == 0 || templ.height() == 0 || image.width() == 0 || image.height() == 0){
        return std::numeric_limits<double>::infinity();
    }
    return image.width() == templ.width() && image.height() == templ.height()
        ? ImageMatch::pixel_RMSD(image, templ)
        : ImageMatch::pixel_RMSD(image, templ.scale_to(image.width(), image.height()));
}
}

const std::array<WeatherIconType, 6>& weather_test_types(){
    static const std::array<WeatherIconType, 6> TYPES = {
        WeatherIconType::Clear,
        WeatherIconType::Sunny,
        WeatherIconType::Rain,
        WeatherIconType::Cloudy,
        WeatherIconType::Foggy,
        WeatherIconType::Rainbow,
    };
    return TYPES;
}

std::string weather_name(WeatherIconType type){
    switch (type){
    case WeatherIconType::Clear:   return "Clear";
    case WeatherIconType::Sunny:   return "Sunny";
    case WeatherIconType::Rain:    return "Rain";
    case WeatherIconType::Cloudy:  return "Cloudy";
    case WeatherIconType::Foggy:   return "Foggy";
    case WeatherIconType::Rainbow: return "Rainbow";
    default:                       return "Unknown";
    }
}

std::string weather_detection_status_string(const std::vector<std::pair<WeatherIconType, bool>>& statuses){
    std::ostringstream ss;
    ss << "Detection map: ";
    for (size_t c = 0; c < statuses.size(); c++){
        if (c > 0){
            ss << ", ";
        }
        ss << weather_name(statuses[c].first) << "=" << (statuses[c].second ? "true" : "false");
    }
    return ss.str();
}

WeatherIconType weather_from_filename(const std::string& image){
    std::string lower = image;
    std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c){ return (char)std::tolower(c); });

    if (lower.find("rainbow") != std::string::npos){
        return WeatherIconType::Rainbow;
    }
    if (lower.find("sunny") != std::string::npos){
        return WeatherIconType::Sunny;
    }
    if (lower.find("cloud") != std::string::npos){
        return WeatherIconType::Cloudy;
    }
    if (lower.find("clear") != std::string::npos){
        return WeatherIconType::Clear;
    }
    if (lower.find("fog") != std::string::npos){
        return WeatherIconType::Foggy;
    }
    if (lower.find("rain") != std::string::npos){
        return WeatherIconType::Rain;
    }
    return WeatherIconType::Unknown;
}

bool expected_result_from_filename(const std::string& image){
    std::string lower = image;
    std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c){ return (char)std::tolower(c); });

    if (lower.find("_false") != std::string::npos){
        return false;
    }
    return true;
}

std::string weather_list_string(const std::vector<WeatherIconType>& weathers){
    std::ostringstream ss;
    for (size_t c = 0; c < weathers.size(); c++){
        if (c > 0){
            ss << ", ";
        }
        ss << weather_name(weathers[c]);
    }
    return ss.str();
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
        static const WeatherFullMatcher matcher("PokemonLZA/Weather/clear_full.png", 115.0);
        return matcher;
    }
    static const WeatherFullMatcher& sunny(){
        static const WeatherFullMatcher matcher("PokemonLZA/Weather/sunny_full.png", 120.0);
        return matcher;
    }
    static const WeatherFullMatcher& rain(){
        static const WeatherFullMatcher matcher("PokemonLZA/Weather/rain_full.png", 100.0);
        return matcher;
    }
    static const WeatherFullMatcher& cloudy(){
        static const WeatherFullMatcher matcher("PokemonLZA/Weather/cloudy_full.png", 110.0);
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
//-----------------------------------------------------
//  Detector
//-----------------------------------------------------

WeatherIconDetector::WeatherIconDetector(WeatherIconType type, VideoOverlay* overlay)
    : m_box(0.880000, 0.010000, 0.035800, 0.068000)
{
    m_type = type;
    if (overlay){
        m_overlay1.emplace(*overlay, m_box, COLOR_RED);
    }
}

void WeatherIconDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(COLOR_RED, m_box);
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

    ImageViewRGB32 cropped = extract_box_reference(screen, m_box);
    const bool full_match = match_template_by_waterfill(
        screen.size(),
        cropped,
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

    if (m_type == WeatherIconType::Clear || m_type == WeatherIconType::Sunny || m_type == WeatherIconType::Cloudy){
        const double clear_rmsd = full_template_rmsd(cropped, WeatherIconType::Clear);
        const double sunny_rmsd = full_template_rmsd(cropped, WeatherIconType::Sunny);
        const double cloudy_rmsd = full_template_rmsd(cropped, WeatherIconType::Cloudy);

        switch (m_type){
        case WeatherIconType::Clear:
            if (clear_rmsd + 4.0 >= sunny_rmsd || clear_rmsd > cloudy_rmsd + 5.0){
                return false;
            }
            break;
        case WeatherIconType::Sunny:
            if (sunny_rmsd > clear_rmsd + 12.0 || sunny_rmsd > cloudy_rmsd + 6.0){
                return false;
            }
            break;
        case WeatherIconType::Cloudy:
            if (cloudy_rmsd > sunny_rmsd || cloudy_rmsd > clear_rmsd + 5.0){
                return false;
            }
            break;
        default:
            break;
        }
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



class Test_WeatherIconDetector : public UnitTest{
public:
    Test_WeatherIconDetector(
        const std::string& image,
        WeatherIconType expected_weather,
        bool expected_result
    )
        : UnitTest("PokemonPLZA::WeatherIconDetector - " + image + " [" + weather_name(expected_weather) + "]")
        , m_image(UNIT_TEST_RESOURCE_PATH() + image)
        , m_expected_weather(expected_weather)
        , m_expected_result(expected_result)
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        (void)scope;

        DummyVideoOverlay overlay;
        ImageRGB32 image(m_image);
        if (image.width() == 0 || image.height() == 0){
            return "Failed to load test image: " + m_image;
        }

        std::vector<WeatherIconType> detected_weathers;
        std::vector<std::pair<WeatherIconType, bool>> statuses;
        for (WeatherIconType type : weather_test_types()){
            WeatherIconDetector detector(type, &overlay);
            const bool matched = detector.detect(image);
            statuses.emplace_back(type, matched);
            if (matched){
                detected_weathers.push_back(type);
            }
        }
        const std::string status_string = weather_detection_status_string(statuses);
        logger.log(status_string, COLOR_BLUE);

        if (!m_expected_result){
            bool expected_found = false;
            for (WeatherIconType weather : detected_weathers){
                if (weather == m_expected_weather){
                    expected_found = true;
                    break;
                }
            }
            TEST_RESULT_COMPONENT_EQUAL_STR(expected_found, false, "expected weather present");
            if (expected_found){
                return "Expected weather should be absent, but " + weather_name(m_expected_weather) + " was detected. " + status_string;
            }
            return !expected_found;
        }

        TEST_RESULT_COMPONENT_EQUAL_STR(detected_weathers.size(), (size_t)1, "num detected weather types");

        if (detected_weathers.empty()){
            if (m_expected_weather != WeatherIconType::Unknown){
                return "Expected " + weather_name(m_expected_weather) + ", but detected no weather. " + status_string;
            }
            return "Detected no weather. " + status_string;
        }

        if (detected_weathers.size() > 1){
            return "Detected multiple weather types: " + weather_list_string(detected_weathers) + ". " + status_string;
        }

        const WeatherIconType detected = detected_weathers[0];

        if (m_expected_weather != WeatherIconType::Unknown){
            if (detected != m_expected_weather){
                return "Expected " + weather_name(m_expected_weather) + ", but detected " + weather_name(detected) + ". " + status_string;
            }
            TEST_RESULT_COMPONENT_EQUAL_STR(weather_name(detected), weather_name(m_expected_weather), "weather type");
            return true;
        }

        return true;
    }

private:
    std::string m_image;
    WeatherIconType m_expected_weather;
    bool m_expected_result;
};



void add_tests_WeatherDetector(UnitTestDatabase& database){
    auto add = [&](const char* filename){
        const std::string image = "PokemonLZA/WeatherDetector/" + std::string(filename);
        database.add<Test_WeatherIconDetector>(
            image,
            weather_from_filename(filename),
            expected_result_from_filename(filename)
        );
    };

    add("clear_1_True.png");
    add("clear_2_True.png");
    add("clear_dark_1_True.png");
    add("clear_dark_2_True.png");
    add("clear_dark_map_True.png");
    add("clear_overview_True.png");
    add("clear_wild_zone_1_2_True.png");
    add("clear_wild_zone_20_1_True.png");
    add("cloudy_1_True.png");
    add("cloudy_hyperspace_rogue_mega_arena_True.png");
    add("cloudy_zoomed_1_True.png");
    add("cloudy_zoomed_2_True.png");
    add("fog_1_True.png");
    add("fog_wild_zone_1_True.png");
    add("fog_wild_zone_20_True.png");
    add("french_clear_place_centrale_True.png");
    add("french_clear_wild_zone_20_2_True.png");
    add("french_cloudy_wild_zone_20_True.png");
    add("french_rainbow_wild_zone_20_True.png");
    add("french_rain_wild_zone_20_True.png");
    add("french_sunny_wild_zone_20_1_True.png");
    add("japanese_clear_1_True.jpg");
    add("japanese_clear_2_True.jpg");
    add("japanese_sunny_True.png");
    add("japanese_sunny_wild_zone_17_True.png");
    add("rainbow_1_True.png");
    add("rainbow_True.png");
    add("rain_1_True.png");
    add("rain_hyperspace_wild_zone_True.png");
    add("rain_wild_zone_1_True.png");
    add("rain_wild_zone_6_True.png");
    add("sunny_3_True.png");
    add("sunny_wild_zone_1_True.png");
    add("sunny_wild_zone_8_True.png");
    add("sunny_zoomed_1_True.jpg");
    add("sunny_zoomed_2_True.jpg");
}


}
}
}
