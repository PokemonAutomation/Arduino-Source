/*  Selected Region Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/AbstractLogger.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
// #include "CommonFramework/ImageMatch/ImageDiff.h"
// #include "CommonFramework/VideoPipeline/VideoFeed.h"
// #include "CommonFramework/VideoPipeline/VideoOverlay.h"
// #include "CommonFramework/InferenceInfra/VisualInferenceCallback.h"
// #include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "PokemonLA_MapWeatherAndTimeReader.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


Weather detect_weather_on_map(Logger& logger, const ImageViewRGB32& screen){
    const ImageFloatBox box{0.0285, 0.069, 0.025, 0.044};
    
    const ImageViewRGB32 image = extract_box_reference(screen, box);

    image.save("./weather_crop.png");


    return Weather::SUNNY;
}


TimeOfDay detect_time_of_day_on_map(Logger& logger, const ImageViewRGB32& screen){
    return TimeOfDay::MORNING;
}


}
}
}
