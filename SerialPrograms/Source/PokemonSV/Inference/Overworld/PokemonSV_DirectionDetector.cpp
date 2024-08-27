/*  Direction Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/GlobalSettingsPanel.h"
#include "Common/Cpp/Containers/FixedLimitVector.tpp"
#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/ImageMatch/WaterfillTemplateMatcher.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/ImageTools/WaterfillUtilities.h"
#include "CommonFramework/ImageTools/BinaryImage_FilterRgb32.h"
#include "CommonFramework/ImageTools/ImageManip.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonFramework/Logging/Logger.h"
#include "CommonFramework/Tools/DebugDumper.h"
#include "CommonFramework/Tools/VideoResolutionCheck.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Kernels/Waterfill/Kernels_Waterfill_Types.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_OverworldDetector.h"
#include "PokemonSV_DirectionDetector.h"
#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


class DirectionMatcher : public ImageMatch::WaterfillTemplateMatcher{
public:
    
    DirectionMatcher() : WaterfillTemplateMatcher(
        "PokemonSV/Map/DirectionIcon-Template.png", Color(0, 100, 30), Color(50, 230, 85), 50
    ){
        m_aspect_ratio_lower = 0.8;
        m_aspect_ratio_upper = 1.2;
        m_area_ratio_lower = 0.8;
        m_area_ratio_upper = 1.2;

    }

    static const ImageMatch::WaterfillTemplateMatcher& instance(){
        static DirectionMatcher matcher;
        return matcher;
    }
};


DirectionDetector::~DirectionDetector() = default;

DirectionDetector::DirectionDetector(Color color, const ImageFloatBox& box)
    : m_color(color)
    , m_minimap_box(box)
{}


std::pair<double, double> DirectionDetector::locate_north(ConsoleHandle& console, const ImageViewRGB32& screen) const{
    assert_16_9_720p_min(console, screen);
    const std::vector<std::pair<uint32_t, uint32_t>> filters = {
        {combine_rgb(0, 100, 0), combine_rgb(50, 230, 80)},

    };

    const double min_object_size = 50.0;
    const double rmsd_threshold = 60.0;

    const double screen_rel_size = (screen.height() / 1080.0);
    const size_t min_size = size_t(screen_rel_size * screen_rel_size * min_object_size);

    std::pair<double, double> north_location(0, 0);
    OverworldDetector detector;
    std::pair<double, double> zero_coord = detector.locate_ball(screen, true); // 0.9061, 0.8328
    if (zero_coord.first > 0.91 || zero_coord.first < 0.90 || 
        zero_coord.second > 0.84 || zero_coord.second < 0.82)
    {
        console.log("Unable to locate the overworld radar ball, falling back on hard coded location.", COLOR_ORANGE);
        zero_coord.first = 0.9061;
        zero_coord.second = 0.8328;
        // throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Unable to locate the overworld radar ball.");
    }

    ImagePixelBox pixel_search_area = floatbox_to_pixelbox(screen.width(), screen.height(), m_minimap_box);
    match_template_by_waterfill(
        extract_box_reference(screen, m_minimap_box), 
        DirectionMatcher::instance(),
        filters,
        {min_size, SIZE_MAX},
        rmsd_threshold,
        [&](Kernels::Waterfill::WaterfillObject& object) -> bool {
            
            // relative coord wrt upper left corner
            double x1 = (object.center_of_gravity_x() + pixel_search_area.min_x) / (double)screen.width();
            double y1 = (object.center_of_gravity_y() + pixel_search_area.min_y) / (double)screen.height();
            // relative coord wrt minimap radar
            double x2 = x1 - zero_coord.first;
            double y2 = zero_coord.second - y1; // subtract y from zero_coord since I want north to be positive
            // absolute coord wrt minimap radar, scaled to screen size
            double x3 = x2 * (double)screen.width() / screen_rel_size;
            double y3 = y2 * (double)screen.height() / screen_rel_size;

            double hypotenuse = std::hypot(x3, y3);
            // std::cout << "hypotenuse: " << std::to_string(hypotenuse) << std::endl;
            if (hypotenuse < 135 || hypotenuse > 155){
                return false;
            }
            north_location = std::make_pair(x3, y3);
            return true;
        }
    );

    // std::cout << "north location: " << std::to_string(north_location.first) << ", " << std::to_string(north_location.second) << std::endl;

    return north_location;
}


double DirectionDetector::current_direction(ConsoleHandle& console, const ImageViewRGB32& screen) const{
    std::pair<double, double> north_location = locate_north(console, screen);
    if (north_location.first == 0 && north_location.second == 0){ // unable to locate north
        return -1;
    }
    double direction = std::atan2(north_location.first, north_location.second);  // swap x and y to use north-clockwise convention
    // std::cout << std::to_string(direction) << std::endl;
    direction = (direction < 0) ? (direction + 2 * M_PI) : direction; // change (-pi, pi] to [0, 2pi)
    // std::cout << "current_direction: " << std::to_string(direction) << std::endl;
    return direction;
}

void DirectionDetector::change_direction(
    ConsoleHandle& console, 
    BotBaseContext& context,
    double direction
) const{
    for (size_t i = 0; i < 10; i++){ // 10 attempts to move the direction to the target
        context.wait_for_all_requests();
        VideoSnapshot screen = console.video().snapshot();
        double current = current_direction(console, screen);
        if (current < 0){ 
            return;
        }
        double target = std::fmod(direction, (2 * M_PI));

        // try to find the shortest path around the circle
        double diff = target - current;
        if(diff > M_PI) {
            diff -= (2 * M_PI);
        }
        if(diff <= -M_PI) {
            diff += (2 * M_PI);
        }
        double abs_diff = std::abs(diff);
        console.log("current direction: " +  std::to_string(current));
        console.log("target: " +  std::to_string(target) + ", diff: " + std::to_string(diff));
        if (abs_diff < 0.02){
            // stop the loop when we're close enough to the target
            break;
        }
        uint8_t scale_factor = 80;

        uint16_t push_duration = std::max(uint16_t(std::abs(diff * scale_factor)), uint16_t(3));
        int16_t push_direction = (diff > 0) ? -1 : 1;
        double push_magnitude = (128 / (i + 1)); // push less with each iteration/attempt
        uint8_t push_x = uint8_t(std::max(std::min(int(128 + (push_direction * push_magnitude)), 255), 0));
        console.log("push magnitude: " + std::to_string(push_x) + ", push duration: " +  std::to_string(push_duration));
        pbf_move_right_joystick(context, push_x, 128, push_duration, 100);
    }
    
}




}
}
}
