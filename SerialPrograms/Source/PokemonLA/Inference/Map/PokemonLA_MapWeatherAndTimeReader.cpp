/*  Selected Region Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <iostream>
#include <sstream>
#include "Common/Cpp/AbstractLogger.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonTools/Images/ImageFilter.h"
#include "CommonTools/ImageMatch/ImageCropper.h"
#include "CommonTools/ImageMatch/SilhouetteDictionaryMatcher.h"
#include "PokemonLA/Resources/PokemonLA_WeatherAndTimeIcons.h"
#include "PokemonLA_MapWeatherAndTimeReader.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


ImageMatch::SilhouetteDictionaryMatcher BUILD_WEATHER_ICON_MATCHER(){
    ImageMatch::SilhouetteDictionaryMatcher matcher;
    for (size_t i = 0; i < NUM_WEATHER; i++){
        matcher.add(WEATHER_NAMES[i], ALL_WEATHER_ICONS()[i]);
    }
    return matcher;
}

const ImageMatch::SilhouetteDictionaryMatcher& WEATHER_ICON_MATCHER(){
    const static auto matcher = BUILD_WEATHER_ICON_MATCHER();
    return matcher;
}


ImageMatch::SilhouetteDictionaryMatcher BUILD_TIME_OF_DAY_ICON_MATCHER(){
    ImageMatch::SilhouetteDictionaryMatcher matcher;
    for (size_t i = 0; i < NUM_TIMES_OF_DAY; i++){
        // +1 here to skip the name of TimeOfDay::NONE
        matcher.add(TIME_OF_DAY_NAMES[i+1], ALL_TIME_OF_DAY_ICONS()[i]);
    }
    return matcher;
}

const ImageMatch::SilhouetteDictionaryMatcher& TIME_OF_DAY_ICON_MATCHER(){
    const static auto matcher = BUILD_TIME_OF_DAY_ICON_MATCHER();
    return matcher;
}




Weather detect_weather_on_map(Logger& logger, const ImageViewRGB32& screen){
    const ImageFloatBox box{0.0285, 0.069, 0.025, 0.044};
    
    // Get a loose crop of the weather icon
    ImageViewRGB32 cropped_image = extract_box_reference(screen, box);

    // image.save("./weather_crop.png");
    // Get a tight crop:
    const ImagePixelBox tight_box = ImageMatch::enclosing_rectangle_with_pixel_filter(
        cropped_image,
        // The filter is a lambda function that returns true on white weather icon pixels.
        [](Color pixel){
            return (uint32_t)pixel.red() + pixel.green() + pixel.blue() > 600;
        }
    );
    cropped_image = extract_box_reference(cropped_image, tight_box);

    // Replace the background dark pixels (outside the range from 0xffa0a0a0 to 0xffffffff) with
    // 0-alpha black pixels.
    const bool replace_range = false;
    ImageRGB32 icon_image = filter_rgb32_range(
        cropped_image,
        0xffa0a0a0, 0xffffffff, Color(0), replace_range
    );

    const auto match_result = WEATHER_ICON_MATCHER().match(icon_image, 100);
    std::ostringstream os;
    os << "Weather icon match result: ";
    for(auto result : match_result.results){
        os << result.second << "(" << result.first << ") ";
    }
    logger.log(os.str(), COLOR_BLUE);

    return get_weather(match_result.results.begin()->second);
}


TimeOfDay detect_time_of_day_on_map(Logger& logger, const ImageViewRGB32& screen){
    const ImageFloatBox box{0.910, 0.070, 0.034, 0.041};
    
    // Get a loose crop of the time of day icon
    ImageViewRGB32 cropped_image = extract_box_reference(screen, box);

    // Get a tight crop:
    const ImagePixelBox tight_box = ImageMatch::enclosing_rectangle_with_pixel_filter(
        cropped_image,
        // The filter is a lambda function that returns true on white time of day icon pixels.
        [](Color pixel){
            return (uint32_t)pixel.red() + pixel.green() + pixel.blue() > 600;
        }
    );
    cropped_image = extract_box_reference(cropped_image, tight_box);

    // Replace the background dark pixels (outside the range from 0xffa0a0a0 to 0xffffffff) with
    // 0-alpha black pixels.
    const bool replace_range = false;
    ImageRGB32 icon_image = filter_rgb32_range(
        cropped_image,
        0xffa0a0a0, 0xffffffff, Color(0), replace_range
    );

    const auto match_result = TIME_OF_DAY_ICON_MATCHER().match(icon_image, 100);

    std::ostringstream os;
    os << "Time of day icon match result: ";
    for(auto result : match_result.results){
        os << result.second << "(" << result.first << ") ";
    }
    logger.log(os.str(), COLOR_BLUE);

    return get_time_of_day(match_result.results.begin()->second);
}


}
}
}
