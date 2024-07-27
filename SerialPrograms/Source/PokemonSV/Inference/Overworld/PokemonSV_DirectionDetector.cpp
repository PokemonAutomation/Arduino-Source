/*  Fast Travel Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/GlobalSettingsPanel.h"
#include "Common/Cpp/Containers/FixedLimitVector.tpp"
#include "CommonFramework/ImageMatch/WaterfillTemplateMatcher.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/ImageTools/WaterfillUtilities.h"
#include "CommonFramework/ImageTools/BinaryImage_FilterRgb32.h"
#include "CommonFramework/ImageTools/ImageManip.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonFramework/Logging/Logger.h"
#include "CommonFramework/Tools/DebugDumper.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Kernels/Waterfill/Kernels_Waterfill_Types.h"
#include "PokemonSV_DirectionDetector.h"
#include <cmath>
#include <iostream>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


class DirectionMatcher : public ImageMatch::WaterfillTemplateMatcher{
public:
    
    /* 
    - selects for the white, wing-shaped component of the fast travel icon
    - Use Waterfilltemplate matcher to get your template so you get the correct area ratio
    along with the blue background. You need the blue backgroun so you don't false positive
    against the fly icon in Pokemon centers.

     */
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

void DirectionDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_minimap_box);
}

bool DirectionDetector::detect(const ImageViewRGB32& screen) const{
    return true;
}

std::pair<double, double> DirectionDetector::locate_north(const ImageViewRGB32& screen) const{
    const std::vector<std::pair<uint32_t, uint32_t>> filters = {
        {combine_rgb(0, 100, 0), combine_rgb(50, 230, 80)},

    };

    const double min_object_size = 50.0;
    const double rmsd_threshold = 60.0;

    const double screen_rel_size = (screen.height() / 1080.0);
    const size_t min_size = size_t(screen_rel_size * screen_rel_size * min_object_size);

    std::pair<double, double> north_location(-1.0, -1.0);

    #if 0
        std::vector<ImageFloatBox> candidate_locations = north_candidate_locations(screen);
        // std::vector<ImageRGB32> candidate_images = north_candidate_locations(screen); // vector<imagefloatbox>, filters
        for (ImageFloatBox& box : candidate_locations){
            ImagePixelBox pixel_search_area = floatbox_to_pixelbox(screen.width(), screen.height(), box);
            match_template_by_waterfill(
                extract_box_reference(screen, box),
                DirectionMatcher::instance(),
                filters,
                {min_size, SIZE_MAX},
                rmsd_threshold,
                [&](Kernels::Waterfill::WaterfillObject& object) -> bool {
                    north_location = std::make_pair(
                        (object.center_of_gravity_x() + pixel_search_area.min_x) / (double)screen.width(),
                        (object.center_of_gravity_y() + pixel_search_area.min_y) / (double)screen.height()
                    );
                    return true;
                }
            ); 
            if (north_location.first >= 0.0) {
                break;
            }          
        }     
    #endif   

    #if 0
        std::vector<ImageFloatBox> candidate_locations = north_candidate_locations(screen);
        // std::vector<ImageRGB32> candidate_images = north_candidate_locations(screen); // vector<imagefloatbox>, filters
        for (ImageFloatBox& box : candidate_locations){
            ImagePixelBox pixel_search_area = floatbox_to_pixelbox(screen.width(), screen.height(), box);
            std::vector<ImageRGB32> candidate_image = north_candidate_images(screen, box);
            for (ImageRGB32& candidate : candidate_image){
                match_template_by_waterfill(
                    candidate, 
                    DirectionMatcher::instance(),
                    filters,
                    {min_size, SIZE_MAX},
                    rmsd_threshold,
                    [&](Kernels::Waterfill::WaterfillObject& object) -> bool {
                        north_location = std::make_pair(
                            (object.center_of_gravity_x() + pixel_search_area.min_x) / (double)screen.width(),
                            (object.center_of_gravity_y() + pixel_search_area.min_y) / (double)screen.height()
                        );
                        return true;
                    }
                ); 
                if (north_location.first >= 0.0) {
                    break;
                }                
            }
            if (north_location.first >= 0.0) {
                break;
            }             
        }     
    #endif                

    #if 1
        ImagePixelBox pixel_search_area = floatbox_to_pixelbox(screen.width(), screen.height(), m_minimap_box);
        match_template_by_waterfill(
            extract_box_reference(screen, m_minimap_box), 
            DirectionMatcher::instance(),
            filters,
            {min_size, SIZE_MAX},
            rmsd_threshold,
            [&](Kernels::Waterfill::WaterfillObject& object) -> bool {
                north_location = std::make_pair(
                    (object.center_of_gravity_x() + pixel_search_area.min_x) / (double)screen.width(),
                    (object.center_of_gravity_y() + pixel_search_area.min_y) / (double)screen.height()
                );
                return true;
            }
        );
    #endif


    std::cout << "north location: " << std::to_string(north_location.first) << ", " << std::to_string(north_location.second) << std::endl;

    return north_location;
}



std::vector<ImageFloatBox> DirectionDetector::north_candidate_locations(const ImageViewRGB32& screen) const{
    const std::vector<std::pair<uint32_t, uint32_t>> filters = {
        // {combine_rgb(0, 180, 100), combine_rgb(50, 255, 150)},
        {combine_rgb(0, 180, 80), combine_rgb(85, 255, 175)},

    };

    const double min_object_size = 150.0;

    const double screen_rel_size = (screen.height() / 1080.0);
    const size_t min_size = size_t(screen_rel_size * screen_rel_size * min_object_size);

    std::vector<ImageFloatBox> candidate_locations = match_filters_by_waterfill(
                                                    screen,
                                                    m_minimap_box,
                                                    filters,
                                                    {min_size, SIZE_MAX}
                                                );    
    return candidate_locations;                                                
}

// use the color filters to select for the bright-green portion in the overworld compass
// then from these candidates, run a binary filter to keep the bright-green portion, and 
// replace everything else with dark green.
// since the N was originally dark green, this binary filter essentially removes all of the
// extraneous pixels outside of the bright-green portion of the overworld compass.
std::vector<ImageRGB32> DirectionDetector::north_candidate_images(const ImageViewRGB32& screen, const ImageFloatBox& box) const{
    const std::vector<std::pair<uint32_t, uint32_t>> filters = {
        // {combine_rgb(0, 180, 100), combine_rgb(50, 255, 150)},
        {combine_rgb(0, 180, 80), combine_rgb(85, 255, 175)},

    };

    std::vector<ImageRGB32> candidate_images;
    const ImageViewRGB32 &candidate_image = pad_image(extract_box_reference(screen, box), 10, 0xffffffff);
    auto matrices = compress_rgb32_to_binary_range(candidate_image, filters);
    for(PokemonAutomation::PackedBinaryMatrix &matrix : matrices){
        ImageRGB32 binaryImage = pad_image(extract_box_reference(screen, box), 10, 0xffffffff);
        filter_by_mask(matrix, binaryImage, Color(10, 190, 70), true);
        if (PreloadSettings::debug().IMAGE_TEMPLATE_MATCHING){
            dump_debug_image(
                global_logger_command_line(), 
                "CommonFramework/WaterfillTemplateMatcher", 
                "binary_image_candidate", 
                binaryImage);                
        }
        candidate_images.emplace_back(std::move(binaryImage));
    }
                                                  
    return candidate_images;  

}

double DirectionDetector::current_direction(const ImageViewRGB32& screen) const{
    std::pair<double, double> north_location = locate_north(screen);
    std::pair<double, double> zero_coord(0.906250, 0.830);
    double x_coord = north_location.first - zero_coord.first;
    double y_coord = zero_coord.second - north_location.second; // subtract north_location from zero_coord since I want north to be positive
    // std::cout << std::to_string(x_coord) << ", " << std::to_string(y_coord) << std::endl;
    double direction = std::atan2(x_coord, y_coord);  // swap x and y to use north-clockwise convention
    // std::cout << std::to_string(direction) << std::endl;
    direction = (direction < 0) ? (direction + 2 * M_PI) : direction; // change (-pi, pi] to [0, 2pi)
    std::cout << "current_direction: " << std::to_string(direction) << std::endl;
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
        double current = current_direction(screen);
        double target = std::fmod(direction, (2 * M_PI));

        double diff = target - current;
        if(diff > M_PI) {
            diff -= (2 * M_PI);
        }
        if(diff <= -M_PI) {
            diff += (2 * M_PI);
        }
        double abs_diff = std::abs(diff);

        std::cout << "current: " << std::to_string(current) << ", target: ";
        std::cout << std::to_string(target) << ", diff: " << std::to_string(diff) << std::endl;
        if (abs_diff < 0.02){
            // stop the loop when we're close enough to the target
            break;
        }
        uint8_t scale_factor = 70;
        // if (abs_diff <= 0.5){
        //     scale_factor = 115;
        // }else if (abs_diff <= 1){
        //     scale_factor = 100;
        // }else if (abs_diff <= 1.5){
        //     scale_factor = 85;
        // }else if (abs_diff <= 2){
        //     scale_factor = 75;
        // }else if (abs_diff <= 2.5){
        //     scale_factor = 70;
        // }else if (abs_diff <= 3.0){
        //     scale_factor = 75;
        // }else{
        //     scale_factor = 85;
        // }

        uint16_t push_duration = (int16_t)std::abs(diff * scale_factor);
        uint8_t push_direction = (diff > 0) ? 0 : 255;
        std::cout << "push_duration: " << std::to_string(push_duration);
        std::cout << ", scale_factor: " << std::to_string(scale_factor) << std::endl;
        pbf_move_right_joystick(context, push_direction, 128, push_duration, 100);
    }
    


}

DirectionWatcher::~DirectionWatcher() = default;

DirectionWatcher::DirectionWatcher(Color color, VideoOverlay& overlay, const ImageFloatBox& box)
    : VisualInferenceCallback("DirectionWatcher")
    , m_overlay(overlay)
    , m_detector(color, box)
{}

void DirectionWatcher::make_overlays(VideoOverlaySet& items) const{
    m_detector.make_overlays(items);
}

bool DirectionWatcher::process_frame(const ImageViewRGB32& screen, WallClock timestamp){
    return true;
}








}
}
}
