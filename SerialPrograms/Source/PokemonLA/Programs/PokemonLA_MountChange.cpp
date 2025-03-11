/*  Mount Change
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonLA_MountChange.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


bool get_mount_coordinates(size_t& index, MountState mount){
    switch (mount){
    case MountState::WYRDEER_OFF:
    case MountState::BASCULEGION_OFF:
        index = 0;
        return false;
    case MountState::WYRDEER_ON:
    case MountState::BASCULEGION_ON:
        index = 0;
        return true;
    case MountState::URSALUNA_OFF:
        index = 1;
        return false;
    case MountState::URSALUNA_ON:
        index = 1;
        return true;
    case MountState::SNEASLER_OFF:
        index = 2;
        return false;
    case MountState::SNEASLER_ON:
        index = 2;
        return true;
    case MountState::BRAVIARY_OFF:
        index = 3;
        return false;
    case MountState::BRAVIARY_ON:
        index = 3;
        return true;
    default:
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Invalid MountState = " + std::to_string((size_t)mount));
    }
}

void change_mount(VideoStream& stream, ProControllerContext& context, MountState mount){
    size_t desired_index;
    bool desired_on = get_mount_coordinates(desired_index, mount);

    MountDetector mount_detector;
    for (size_t c = 0; c < 20; c++){
        context.wait_for_all_requests();

        MountState current = mount_detector.detect(stream.video().snapshot());
        if (mount == current){
            return;
        }
        if (current == MountState::NOTHING){
            pbf_press_dpad(context, DPAD_RIGHT, 20, 50);
            continue;
        }

        size_t current_index;
        bool current_on = get_mount_coordinates(current_index, current);

        if (!desired_on && current_on){
            pbf_press_button(context, BUTTON_PLUS, 20, 105);
        }

        size_t index_diff = (4 + current_index - desired_index) % 4;
        switch (index_diff){
        case 0:
            break;
        case 1:
            pbf_press_dpad(context, DPAD_RIGHT, 20, 50);
            break;
        case 2:
            pbf_press_dpad(context, DPAD_RIGHT, 20, 50);
            pbf_press_dpad(context, DPAD_RIGHT, 20, 50);
            break;
        case 3:
            pbf_press_dpad(context, DPAD_LEFT, 20, 50);
            break;
        }

        if (desired_on && !current_on){
            if (desired_index == 3){
                pbf_press_button(context, BUTTON_PLUS, 20, 230);
            }else{
                pbf_press_button(context, BUTTON_PLUS, 20, 105);
            }
        }
    }

    OperationFailedException::fire(
        ErrorReport::SEND_ERROR_REPORT,
        std::string("Unable to find ") + MOUNT_STATE_STRINGS[(size_t)mount] + " after 10 attempts.",
        stream
    );
}

void dismount(VideoStream& stream, ProControllerContext& context){
    MountDetector mount_detector;
    for (size_t c = 0; c < 10; c++){
        context.wait_for_all_requests();

        MountState current = mount_detector.detect(stream.video().snapshot());
        switch (current){
        case MountState::WYRDEER_OFF:
        case MountState::URSALUNA_OFF:
        case MountState::BASCULEGION_OFF:
        case MountState::SNEASLER_OFF:
        case MountState::BRAVIARY_OFF:
            // Already unmount
            return;
        default:
            break;
        }

        if (current == MountState::NOTHING){
            pbf_press_dpad(context, DPAD_RIGHT, 20, 50);
            continue;
        }

        // We are mounted. Press + to unmount
        pbf_press_button(context, BUTTON_PLUS, 20, 105);
    }

    OperationFailedException::fire(
        ErrorReport::SEND_ERROR_REPORT,
        "Unable to dismount after 10 attempts.",
        stream
    );
}


}
}
}
