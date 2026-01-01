/*  YOLO Navigation
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <cmath>
#include "Common/Cpp/PrettyPrint.h"
#include "Common/Cpp/Color.h"
#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Exceptions/UnexpectedBattleException.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "ML/Inference/ML_YOLOv5Detector.h"
#include "ML/Models/ML_YOLOv5Model.h"
#include "PokemonSV/Programs/AutoStory/PokemonSV_AutoStoryTools.h"
#include "ML_YOLONavigation.h"

using namespace std::chrono_literals;

namespace PokemonAutomation{
namespace NintendoSwitch{

ImageFloatBox get_highest_confident_yolo_box(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    VideoOverlaySet& overlays,
    ML::YOLOv5Detector& yolo_detector,
    const std::string& target_label
){
    context.wait_for_all_requests();
    overlays.clear();
    const std::vector<ML::YOLOv5Session::DetectionBox>& detected_boxes = yolo_detector.detected_boxes();
    auto snapshot = env.console.video().snapshot();
    yolo_detector.detect(snapshot);

    ImageFloatBox target_box{-1, -1, -1, -1};
    double best_score = 0;
    for (const ML::YOLOv5Session::DetectionBox& detected_box : detected_boxes){
        ImageFloatBox box = detected_box.box;
        const std::string& label = yolo_detector.session()->label_name(detected_box.label_idx);
        double score = detected_box.score;
        const std::string label_score = label + ": " + tostr_fixed(score, 2);
        if (target_label == label){
            overlays.add(COLOR_RED, box, label_score);

            if (score > best_score){
                target_box = box;
                best_score = score;
            }
        }else{
            overlays.add(COLOR_BLUE, box, label);
        }

    }

    env.console.log(std::string(target_label) + ": {" + std::to_string(target_box.x) + ", " + std::to_string(target_box.y) + ", " + std::to_string(target_box.width) + ", " + std::to_string(target_box.height) + "}");
    env.console.log("center-y: " + std::to_string(target_box.y + target_box.height/2) + "   center-x: " + std::to_string(target_box.x + target_box.width/2));

    return target_box;
}



void move_camera_yolo(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    CameraAxis axis,
    ML::YOLOv5Detector& yolo_detector,
    const std::string& target_label,
    double target_line,
    std::function<void()>&& recovery_action
){
    VideoOverlaySet overlays(env.console.overlay());
    bool seen_object = false;
    size_t max_attempts = 20;
    size_t not_detected_count = 0;
    size_t max_not_detected = 5;
    bool reached_target_line = false;
    bool exceed_max_not_detected = false;
    for (size_t i = 0; i < max_attempts; i++){
    try{
        PokemonSV::do_action_and_monitor_for_battles_early(env.program_info(), env.console, context,
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            context.wait_for_all_requests();
            ImageFloatBox target_box = get_highest_confident_yolo_box(env, context, overlays, yolo_detector, target_label);

            bool not_found_target = target_box.x == -1;
            if (not_found_target){
                not_detected_count++;
                if (not_detected_count > max_not_detected){
                    exceed_max_not_detected = true;
                    return;      // when too many failed attempts, just assume we're too close to the target to detect it.
                }
                context.wait_for(1000ms); // if we can't see the object, it might just be temporarily obscured. wait one second and reattempt.
                return;
            }else{
                not_detected_count = 0;
                seen_object = true;
            }
       
            
            double diff;
            switch(axis){
            case CameraAxis::X:{
                double object_x_pos = target_box.x + target_box.width/2;
                diff =  target_line - object_x_pos;
                break;
            }
            case CameraAxis::Y:{
                double object_y_pos = target_box.y + target_box.height/2;
                diff =  object_y_pos - target_line;
                break;
            }
            default:
                throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "move_camera_yolo: Unknown CameraAxis enum.");  
            }
            env.console.log("target_line: " + std::to_string(target_line));
            env.console.log("diff: " + std::to_string(diff));
            if (std::abs(diff) < 0.01){
                reached_target_line = true;
                return;    // close enough to target_line. stop.
            }

            
            double duration_scale_factor;
            double push_magnitude_scale_factor;
            switch(axis){
            case CameraAxis::X:
                duration_scale_factor = 2000 / std::sqrt(std::abs(diff));
                if (std::abs(diff) < 0.05){
                    duration_scale_factor /= 2;
                }
                push_magnitude_scale_factor = 0.46875 / std::sqrt(std::abs(diff));
                break;
            case CameraAxis::Y:
                duration_scale_factor = 400 / std::sqrt(std::abs(diff));
                if (std::abs(diff) < 0.1){
                    duration_scale_factor *= 0.5;
                }
                push_magnitude_scale_factor = 0.46875 / std::sqrt(std::abs(diff));
                break;
            
            default:
                throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "move_camera_yolo: Unknown CameraAxis enum.");  
            }

            Milliseconds push_duration = std::max(
                Milliseconds((int64_t)std::abs(diff * duration_scale_factor)),
                64ms
            );
            int16_t push_direction = (diff > 0) ? -1 : 1;
            double push_magnitude = std::max(std::abs(diff * push_magnitude_scale_factor), 0.117); 
            double axis_push = std::max(std::min(push_direction * push_magnitude, +1.0), -1.0);

            // env.console.log("object_x: {" + std::to_string(target_box.x) + ", " + std::to_string(target_box.y) + ", " + std::to_string(target_box.width) + ", " + std::to_string(target_box.height) + "}");
            // env.console.log("object_x_pos: " + std::to_string(object_x_pos));
            env.console.log("axis push: " + std::to_string(axis_push) + ", push duration: " +  std::to_string(push_duration.count()) + " ms");
            switch(axis){
            case CameraAxis::X:{
                pbf_move_right_joystick(context, {axis_push, 0}, push_duration, 0ms);
                break;
            }
            case CameraAxis::Y:{
                pbf_move_right_joystick(context, {0, axis_push}, push_duration, 0ms);
                break;
            }
            default:
                throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "move_camera_yolo: Unknown CameraAxis enum.");  
            }
        });

        if(reached_target_line || exceed_max_not_detected){
            break;
        }

    }catch (UnexpectedBattleException&){
        overlays.clear();
        recovery_action();
        // run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
    }
    }

    if (!seen_object){
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "move_camera_yolo(): Never detected the yolo object.",
            env.console
        );
    }
}


} // namespace NintendoSwitch
} // namespace PokemonAutomation
