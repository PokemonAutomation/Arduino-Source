/*  Pokemon LA Weather And Time Icons
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/Globals.h"
#include "CommonTools/ImageMatch/ImageCropper.h"
#include "PokemonLA_WeatherAndTimeIcons.h"

// #include <iostream>

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{

std::array<ImageRGB32, NUM_WEATHER> BUILD_WEATHER_ICONS(){
    const std::string image_folder_path = RESOURCE_PATH() + "PokemonLA/Weather/";

    std::array<ImageRGB32, NUM_WEATHER> ret;
    for(size_t i = 0; i < NUM_WEATHER; i++){
        const auto& weather_name = WEATHER_NAMES[i];
        const std::string image_path = image_folder_path + weather_name + ".png";
        // std::cout << image_path << std::endl;
        // Trim the image to remove 0-alpha boundaries.
        ImageRGB32 image(ImageMatch::trim_image_alpha(ImageRGB32(image_path)).copy());
        ret[i] = std::move(image);
    }

    return ret;
}

const std::array<ImageRGB32, NUM_WEATHER>& ALL_WEATHER_ICONS(){
    const static auto icons = BUILD_WEATHER_ICONS();
    return icons;
}


std::array<ImageRGB32, NUM_TIMES_OF_DAY> BUILD_TIME_OF_DAY_ICONS(){
    const std::string image_folder_path = RESOURCE_PATH() + "PokemonLA/Time/";

    std::array<ImageRGB32, NUM_TIMES_OF_DAY> ret;
    for(size_t i = 0; i < NUM_TIMES_OF_DAY; i++){
        // +1 here to skip TimeOfDay::NONE
        const auto& time_name = TIME_OF_DAY_NAMES[i+1];
        const std::string image_path = image_folder_path + time_name + ".png";
        // std::cout << image_path << std::endl;
        // Trim the image to remove 0-alpha boundaries.
        ImageRGB32 image(ImageMatch::trim_image_alpha(ImageRGB32(image_path)).copy());
        ret[i] = std::move(image);
    }

    return ret;
}


const std::array<ImageRGB32, NUM_TIMES_OF_DAY>& ALL_TIME_OF_DAY_ICONS(){
    const static auto icons = BUILD_TIME_OF_DAY_ICONS();
    return icons;
}


}
}
}
