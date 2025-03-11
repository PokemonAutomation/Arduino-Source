/*  Time of Day Change
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonLA/Inference/Objects/PokemonLA_DialogueYellowArrowDetector.h"
#include "PokemonLA_TimeOfDayChange.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


void change_time_of_day_at_tent(
    VideoStream& stream, ProControllerContext& context,
    TimeOfDay target_time,
    Camp camp
){
    stream.overlay().add_log("Change time to " + std::string(TIME_OF_DAY_NAMES[int(target_time)]), COLOR_WHITE);
    // Move to the tent
    switch (camp)
    {
    case Camp::FIELDLANDS_FIELDLANDS:
        pbf_move_left_joystick(context, 105, 0, 220, 20);
        break;
    
    case Camp::FIELDLANDS_HEIGHTS:
        pbf_move_left_joystick(context, 95, 0, 250, 20);
        break;
    
    case Camp::MIRELANDS_MIRELANDS:
        pbf_move_left_joystick(context, 70, 0, 180, 20);
        break;
    
    case Camp::MIRELANDS_BOGBOUND:
        pbf_move_left_joystick(context, 70, 0, 170, 20);
        break;
    
    case Camp::COASTLANDS_BEACHSIDE:
        pbf_move_left_joystick(context, 100, 0, 130, 20);
        break;
    
    case Camp::COASTLANDS_COASTLANDS:
        pbf_move_left_joystick(context, 75, 0, 160, 20);
        break;
    
    case Camp::HIGHLANDS_HIGHLANDS:
        pbf_move_left_joystick(context, 95, 0, 190, 20);
        break;
    
    case Camp::HIGHLANDS_MOUNTAIN:
        pbf_move_left_joystick(context, 60, 0, 190, 20);
        break;
    
    case Camp::HIGHLANDS_SUMMIT:
        pbf_move_left_joystick(context, 100, 0, 220, 20);
        break;
    
    case Camp::ICELANDS_SNOWFIELDS:
        pbf_move_left_joystick(context, 80, 0, 150, 20);
        break;

    case Camp::ICELANDS_ICEPEAK:
        pbf_move_left_joystick(context, 110, 0, 220, 20);
        break;
    }

    // Press A to interact with tent
    pbf_press_button(context, BUTTON_A, 30, 30);
    context.wait_for_all_requests();

    const bool stop_on_detected = true;
    DialogueYellowArrowDetector yellow_arrow_detector(stream.logger(), stream.overlay(), stop_on_detected);

    context.wait_for_all_requests();
    // Wait for the dialog box to show up
    int ret = wait_until(
        stream, context, std::chrono::seconds(5), {{yellow_arrow_detector}}
    );
    if (ret < 0){
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "Did not interact with a tent.",
            stream
        );
    }

    // Press A to clear the dialog box, and show the time menu
    // pbf_wait(context, 40);
    pbf_press_button(context, BUTTON_A, 30, 80);
    stream.log("Change time of day to " + std::string(TIME_OF_DAY_NAMES[int(target_time)]));

    // Move down the menu to find the target time

    int num_movements = (int)target_time;
    DpadPosition dpad_dir = DPAD_DOWN;
    if (target_time == TimeOfDay::MIDNIGHT){
        num_movements = 2;
        dpad_dir = DPAD_UP;
    }
    for(int i = 0; i < num_movements; i++){
        pbf_press_dpad(context, dpad_dir, 30, 70);
    }

    // Press A to start resting
    pbf_press_button(context, BUTTON_A, 30, 100);
    context.wait_for_all_requests();

    // Wait for the dialog box to show up
    ret = wait_until(
        stream, context, std::chrono::seconds(30), {{yellow_arrow_detector}}
    );
    if (ret < 0){
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "Failed to stand up after resting in a tent.",
            stream
        );
    }

    // Press A again to clear the dialog box
    pbf_press_button(context, BUTTON_A, 30, 100);

    context.wait_for_all_requests();
}



}
}
}
