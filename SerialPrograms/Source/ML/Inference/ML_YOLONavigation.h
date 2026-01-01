/*  YOLO Navigation
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *  Reusable navigation functions using YOLO object detection for game world navigation.
 *  These functions provide camera alignment, player movement, and approach behaviors
 *  based on detected objects in the game environment.
 */

#ifndef PokemonAutomation_ML_YOLONavigation_H
#define PokemonAutomation_ML_YOLONavigation_H

#include <string>
#include <functional>
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
    namespace ML{
        class YOLOv5Detector;
    }
namespace NintendoSwitch{

enum class CameraAxis{
    X,
    Y,
};

// Placeholder for navigation functions to be migrated from PokemonSV AutoStoryTools
// Functions will include:
// - move_camera_yolo(): Align camera to object position
// - move_player_to_realign_via_yolo(): Move player sideways to align with object
// - move_forward_until_yolo_object_above_min_size(): Approach object until desired size
// - move_player_until_yolo_object_detected(): Search for object by moving player
// - move_camera_until_yolo_object_detected(): Search for object by panning camera

// Get the box of the target object with the highest confidence score.
// If multiple objects have the same label, choose the one with the highest score.
// Returns ImageFloatBox{-1, -1, -1, -1} if target object not found.
ImageFloatBox get_highest_confident_yolo_box(
    NintendoSwitch::SingleSwitchProgramEnvironment& env,
    NintendoSwitch::ProControllerContext& context,
    VideoOverlaySet& overlays,
    ML::YOLOv5Detector& yolo_detector,
    const std::string& target_label
);


// move the camera along `axis` until the target object is aligned with target_line
// if caught in battle, run recovery_action
// throw exception if never detected yolo object
void move_camera_yolo(
    NintendoSwitch::SingleSwitchProgramEnvironment& env,
    NintendoSwitch::ProControllerContext& context,
    CameraAxis axis,
    ML::YOLOv5Detector& yolo_detector,
    const std::string& target_label,
    double target_line,
    std::function<void()>&& recovery_action
);


}
}
#endif
