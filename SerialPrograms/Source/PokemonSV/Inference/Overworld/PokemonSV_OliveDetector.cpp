/*  Olive Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Kernels/Waterfill/Kernels_Waterfill_Types.h"
#include "CommonFramework/ImageTools/ImageDiff.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Images/ImageFilter.h"
#include "CommonTools/Images/WaterfillUtilities.h"
#include "CommonTools/ImageMatch/WaterfillTemplateMatcher.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_DirectionDetector.h"
#include "PokemonSV/Programs/AutoStory/PokemonSV_OliveActionFailedException.h"
#include "PokemonSV_OliveDetector.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


class OliveMatcher : public ImageMatch::WaterfillTemplateMatcher{
public:
    OliveMatcher() : WaterfillTemplateMatcher(
        "PokemonSV/Olive.png", Color(0,0,0), Color(255, 255, 255), 5
    ){
        m_aspect_ratio_lower = 0.35;
        m_aspect_ratio_upper = 3;
        m_area_ratio_lower = 0.5;
        m_area_ratio_upper = 1.5;
    }

    static const ImageMatch::WaterfillTemplateMatcher& instance(){
        static OliveMatcher matcher;
        return matcher;
    }
};


OliveDetector::OliveDetector(VideoStream& stream, Color color)
    : m_overlays(stream.overlay())
    , m_color(color)
{}
void OliveDetector::make_overlays(VideoOverlaySet& items) const{
}


std::pair<double, double> OliveDetector::olive_location(VideoStream& stream, ProControllerContext& context, ImageFloatBox box){
    context.wait_for_all_requests();
    ImageFloatBox location = get_olive_floatbox(stream, context, 30, box);
    double x = location.x + (location.width / 2);
    double y = location.y + (location.height / 2);

    return std::make_pair(x, y);
}

std::pair<double, double> box_center(ImageFloatBox& box){
    double x = box.x + (box.width / 2);
    double y = box.y + (box.height / 2);

    return std::make_pair(x, y);
}

ImageFloatBox OliveDetector::get_olive_floatbox(const ImageViewRGB32& screen, ProControllerContext& context, uint8_t rgb_gap, ImageFloatBox box){
    const std::vector<std::pair<uint32_t, uint32_t>> filters = {
        {combine_rgb(0, 10, 0), combine_rgb(255, 255, 255)},
    };

    ImageRGB32 green_only = filter_green(screen, Color(0xff000000), rgb_gap);

    const double min_object_size = 1000;
    const double rmsd_threshold = 100;

    const double screen_rel_size = (screen.height() / 1080.0);
    const size_t min_size = size_t(screen_rel_size * screen_rel_size * min_object_size);

    size_t largest_size = 0;
    ImageFloatBox largest_green(0, 0, 0, 0);
    ImageViewRGB32 cropped = extract_box_reference(green_only, box);
    ImagePixelBox pixel_search_area = floatbox_to_pixelbox(screen.width(), screen.height(), box);
    match_template_by_waterfill(
        cropped, 
        OliveMatcher::instance(),
        filters,
        {min_size, SIZE_MAX},
        rmsd_threshold,
        [&](Kernels::Waterfill::WaterfillObject& object) -> bool {
            size_t object_area = object.area;
            if (object_area > largest_size){
                largest_size = object_area;
                // cout << largest_size << endl;
                ImagePixelBox found_box(
                    object.min_x + pixel_search_area.min_x, object.min_y + pixel_search_area.min_y,
                    object.max_x + pixel_search_area.min_x, object.max_y + pixel_search_area.min_y);
                largest_green = pixelbox_to_floatbox(screen.width(), screen.height(), found_box);
            }

            return false;
        }
    );

    m_overlays.clear();
    m_overlays.add(m_color, largest_green);
    
    return largest_green;
}

ImageFloatBox OliveDetector::get_olive_floatbox(VideoStream& stream, ProControllerContext& context, uint8_t rgb_gap, ImageFloatBox box){
    size_t MAX_ATTEMPTS = 2;
    for (size_t i = 0; i < MAX_ATTEMPTS; i++){
        context.wait_for_all_requests();
        auto snapshot = stream.video().snapshot();
        const ImageViewRGB32& screen = snapshot;
        ImageFloatBox olive_box = get_olive_floatbox(screen, context, rgb_gap, box);
        if (olive_box.x == 0 && olive_box.y == 0){
            // Olive not detected. try again. may have been obscured by the player's head/hat
            context.wait_for(Milliseconds(2000));
            continue;
        }
        return olive_box;        
    }

    // dump_snapshot(stream);
    throw_and_log<OliveActionFailedException>(
        stream.logger(), ErrorReport::SEND_ERROR_REPORT,
        "get_olive_floatbox(): Olive not detected.",
        stream,
        OliveFail::NO_OLIVE_DETECTED
    );
}

ImageFloatBox OliveDetector::align_to_olive(
    const ProgramInfo& info, 
    VideoStream& stream,
    ProControllerContext& context, 
    double direction_facing, 
    uint8_t rgb_gap,
    ImageFloatBox area_to_check
){
    size_t MAX_ATTEMPTS = 10;
    // size_t olive_unchanged_count = 0;
    ImageFloatBox olive_box;
    DirectionDetector direction;
    uint16_t scale_factor = 130;
    int16_t prev_push_direction = 0;
    for (size_t i = 0; i < MAX_ATTEMPTS; i++){
        direction.change_direction(info, stream, context, direction_facing);
        pbf_move_left_joystick(context, 128, 0, 5, 20);
    
        olive_box = get_olive_floatbox(stream, context, rgb_gap, area_to_check);

        std::pair<double, double> olive = box_center(olive_box);
        double olive_x = olive.first;
        double olive_y = olive.second;
        double olive_area = olive_box.width * olive_box.height;

        double diff_from_center = olive_x - 0.5;
        stream.log("olive_x: " +  std::to_string(olive_x) + ", olive_y: " +  std::to_string(olive_y) + ", olive_area: " +  std::to_string(olive_area));
        if (std::abs(diff_from_center) < 0.01){
            return olive_box;
        }


        int16_t push_direction = (diff_from_center > 0) ? 1 : -1;
        if (olive_y > 0.50 && scale_factor > 100){
            scale_factor = 100;
        }
        if (olive_box.height > 0.35 && scale_factor > 50){
            scale_factor = 50;
        }
        if (push_direction * -1 == prev_push_direction){  
            // if you overshot the olive, and are now pushing in the opposite direction
            // then reduce the push_duration.
            scale_factor = int16_t(scale_factor * 0.5);
        }
        
        // cout << "olive_height" << olive_box.height << endl;
        // if (olive_box.height > 0.4){
        //     scale_factor = 50;
        // }


        uint16_t push_duration = std::max(uint16_t((std::abs(diff_from_center) + 0.02) * scale_factor / (olive_y)), uint16_t(15));
        double push_magnitude = 128; // std::max(double(128 / (i + 1)), double(20)); // push less with each iteration/attempt
        uint8_t push_x = uint8_t(std::max(std::min(int(128 + (push_direction * push_magnitude)), 255), 0));
        stream.log("scale_factor: " + std::to_string(scale_factor));
        stream.log("push x: " + std::to_string(push_x) + ", push duration: " +  std::to_string(push_duration));
        // pbf_wait(context, 100);
        uint16_t wait_ticks = 50;
        if (std::abs(diff_from_center) < 0.05){
            wait_ticks = 100;
        }
        pbf_move_left_joystick(context, push_x, 128, push_duration, wait_ticks);
        prev_push_direction = push_direction;
        

        // // check if we're making progress towards centering on the olive.
        // ImageFloatBox olive_box_2 = get_olive_floatbox(console, context, rgb_gap, area_to_check);
        // double box_1_area = olive_box.width * olive_box.height;
        // double box_2_area = olive_box_2.width * olive_box_2.height;
        // double area_diff = std::abs(box_1_area - box_2_area);
        // double x_diff = std::abs(olive_box.x - olive_box_2.x);
        // double y_diff = std::abs(olive_box.y - olive_box_2.y);

        // if (area_diff < 0.02 && x_diff < 0.01 && y_diff < 0.01){ 
        //     olive_unchanged_count++;
        //     // not moving closer to the olive. either wall/fence is in the way, or we are right next the olive.
        //     console.log("Can't align to Olive. Try moving backwards and try again.");
        //     pbf_move_left_joystick(context, 128, 255, 75, 100);  // walk backwards
        //     if (olive_unchanged_count == 2){
        //         throw OliveActionFailedException(
        //             console, ErrorReport::SEND_ERROR_REPORT,
        //             "align_to_olive(): Failed to align to olive.",
        //             true,
        //             OliveFail::FAILED_ALIGN_TO_OLIVE
        //         );                    
        //     }
        // }else{
        //     olive_unchanged_count = 0;
        // }
    }
    return olive_box;

    // don't throw an exception, since sometimes the program has trouble detecting the olive's exact location with the white logo on the olive.
    // so we rely on maxing out the attempts to move on.
    // throw OliveActionFailedException(
    //     console, ErrorReport::SEND_ERROR_REPORT,
    //     "align_to_olive(): Failed to align to olive.",
    //     true,
    //     OliveFail::FAILED_ALIGN_TO_OLIVE
    // );  
}


// todo: detect and handle case where olive is stuck.
// todo: detect and handle case where olive is slightly to the left, and so we need to move on to the next phase
uint16_t OliveDetector::push_olive_forward(
    const ProgramInfo& info, 
    VideoStream& stream,
    ProControllerContext& context, 
    double direction_facing, 
    uint16_t total_forward_distance,
    uint16_t push_olive,
    uint8_t rgb_gap,
    ImageFloatBox area_to_check
){
    stream.log("push_olive_forward(): Total distance: " + std::to_string(total_forward_distance));
    // uint16_t initial_push_olive = push_olive;
    uint16_t ticks_walked = 0;
    size_t MAX_ATTEMPTS = 10;
    for (size_t i = 0; i < MAX_ATTEMPTS; i++){
        align_to_olive(info, stream, context, direction_facing, rgb_gap, area_to_check);
        ticks_walked += walk_up_to_olive(info, stream, context, direction_facing, rgb_gap, area_to_check);
        

        if (ticks_walked >= total_forward_distance){
            stream.log("Distance walked: " + std::to_string(ticks_walked) + "/" + std::to_string(total_forward_distance));
            return ticks_walked;
        }

        align_to_olive(info, stream, context, direction_facing, rgb_gap, area_to_check);
        // check location of olive before and after push
        // if olive is approximately in the same location, then the olive is stuck. try moving backward and running forward again.
        ImageFloatBox olive_box_1 = get_olive_floatbox(stream, context, rgb_gap, area_to_check);
        for (size_t j = 0; j < 3; j++){
            stream.log("Distance walked: " + std::to_string(ticks_walked) + "/" + std::to_string(total_forward_distance));
            stream.log("Push the olive.");
            pbf_move_left_joystick(context, 128, 0, push_olive, 7 * TICKS_PER_SECOND);
            
            ticks_walked += push_olive;
            ImageFloatBox olive_box_2 = get_olive_floatbox(stream, context, rgb_gap, area_to_check);
            double box_1_area = olive_box_1.width * olive_box_1.height;
            double box_2_area = olive_box_2.width * olive_box_2.height;
            double area_diff = std::abs(box_1_area - box_2_area);
            double x_diff = std::abs(olive_box_1.x - olive_box_2.x);
            double y_diff = std::abs(olive_box_1.y - olive_box_2.y);

            if (area_diff < 0.02 && x_diff < 0.05 && y_diff < 0.05){
                stream.log("Olive is stuck? Move backwards and try pushing again.");
                stream.log("Olive 1: area: " + std::to_string(box_1_area) + ", x: " + std::to_string(olive_box_1.x) + ", y: " + std::to_string(olive_box_1.y));
                stream.log("Olive 2: area: " + std::to_string(box_2_area) + ", x: " + std::to_string(olive_box_2.x) + ", y: " + std::to_string(olive_box_2.y));
                pbf_move_left_joystick(context, 128, 255, 75, 100);  // walk backwards
                ticks_walked -= push_olive;
                push_olive = 200; // run forward more on the next push

                if (j == 2){
                    throw_and_log<OliveActionFailedException>(
                        stream.logger(), ErrorReport::SEND_ERROR_REPORT,
                        "push_olive_forward(): Olive stuck.",
                        stream,
                        OliveFail::OLIVE_STUCK
                    );                    
                }
            }else{
                break;
            }
        }
        
        if (ticks_walked > total_forward_distance){
            stream.log("Distance walked: " + std::to_string(ticks_walked) + "/" + std::to_string(total_forward_distance));
            return ticks_walked;
        }

        stream.log("Distance walked: " + std::to_string(ticks_walked) + "/" + std::to_string(total_forward_distance));
        
    }

    throw_and_log<OliveActionFailedException>(
        stream.logger(), ErrorReport::SEND_ERROR_REPORT,
        "push_olive_forward(): Something went wrong. Failed to walk the Olive forward as expected.",
        stream,
        OliveFail::FAILED_PUSH_OLIVE_TOTAL_DISTANCE
    );

}

uint16_t OliveDetector::walk_up_to_olive(
    const ProgramInfo& info, 
    VideoStream& stream,
    ProControllerContext& context, 
    double direction_facing, 
    uint8_t rgb_gap,
    ImageFloatBox area_to_check
){
    uint16_t ticks_walked = 0;
    size_t MAX_ATTEMPTS = 20;
    for (size_t i = 0; i < MAX_ATTEMPTS; i++){
        ImageFloatBox olive_box = get_olive_floatbox(stream, context, rgb_gap, area_to_check);
        std::pair<double, double> olive = box_center(olive_box);
        // double olive_x = olive.first;
        double olive_y = olive.second;

        uint16_t scale_factor = 2000;
        uint16_t push_duration = std::max(uint16_t((0.57 - olive_y)*(0.57 - olive_y) * scale_factor), uint16_t(20));
        stream.log("walk_up_to_olive(): olive_y: " + std::to_string(olive_y));
        if (olive_y > 0.515){
            return ticks_walked;
        }        
        stream.log("push duration: " +  std::to_string(push_duration));
        // when push durations are low, the player moves less than expected
        // once above 45, you walk about as much as expected
        double walking_factor = 1;
        if (push_duration <= 20){
            walking_factor = 0.40;
        }else if (push_duration <= 25){
            walking_factor = 0.53;
        }else if (push_duration <= 30){
            walking_factor = 0.69;
        }else if (push_duration <= 35){
            walking_factor = 0.83;
        }else if (push_duration <= 40){
            walking_factor = 0.91;
        }
        ticks_walked += uint16_t(push_duration * walking_factor);

        uint16_t wait_ticks = 50;
        if (olive_y > 0.4){
            wait_ticks = 100;
        }        
        pbf_move_left_joystick(context, 128, 0, push_duration, wait_ticks);
    }    

    throw_and_log<OliveActionFailedException>(
        stream.logger(), ErrorReport::SEND_ERROR_REPORT,
        "walk_up_to_olive(): Something went wrong. Failed to walk up to the Olive",
        stream,
        OliveFail::FAILED_WALK_TO_OLIVE
    );

}



}
}
}
