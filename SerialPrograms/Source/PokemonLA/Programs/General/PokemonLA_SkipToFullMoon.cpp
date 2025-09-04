/*  Skip to Full Moon
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonLA/Inference/Objects/PokemonLA_ArcPhoneDetector.h"
#include "PokemonLA/Inference/PokemonLA_ItemCompatibilityDetector.h"
#include "PokemonLA_SkipToFullMoon.h"


namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{

using namespace Pokemon;


SkipToFullMoon_Descriptor::SkipToFullMoon_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonLA:SkipToFullMoon",
        STRING_POKEMON + " LA", "Skip to Full Moon",
        "ComputerControl/blob/master/Wiki/Programs/PokemonLA/SkipToFullMoon.md",
        "Skip nights until full moon.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}


SkipToFullMoon::SkipToFullMoon()
    : NOTIFICATIONS({
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_OPTION(NOTIFICATIONS);
}


void SkipToFullMoon::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    //  Connect the controller.
    pbf_press_button(context, BUTTON_LCLICK, 5, 5);

    while (true){
        // Open menu
        pbf_press_dpad(context, DPAD_UP, 20, 120);
        context.wait_for_all_requests();

        const auto compatibility = detect_item_compatibility(env.console.video().snapshot());

        if (compatibility == ItemCompatibility::NONE){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Unable to detect item compatibility.",
                env.console
            );
        }

        if (compatibility == ItemCompatibility::COMPATIBLE){
            // Ursaring can evolve, it's full moon now.
            break;
        }

        // Do another time skip:

        // Close menu
        pbf_press_button(context, BUTTON_B, 20, 100);
        // Character turn around to face the tent
        pbf_move_left_joystick(context, 128, 0, 20, 100);
        // Press A to show the "how long do you rest" dialogue
        pbf_press_button(context, BUTTON_A, 10, 100);
        // Press A to show the time menu
        pbf_press_button(context, BUTTON_A, 10, 30);
        // Move the selection to "Until nightfall"
        pbf_press_dpad(context, DPAD_UP, 10, 30);
        pbf_press_dpad(context, DPAD_UP, 10, 50);

        // Press A to sleep to next night
        pbf_press_button(context, BUTTON_A, 20, 50);
        // Sleeping
        pbf_wait(context, 8 * TICKS_PER_SECOND);
        context.wait_for_all_requests();

        const bool stop_on_detected = true;

        // DialogueEllipseDetector dialogue_ellipse_detector(env.console, env.console, std::chrono::milliseconds(200), 
        //     DialogueEllipseDetector::EllipseLocation::TENT, stop_on_detected);
        // int ret = wait_until(
        //     env.console, context, std::chrono::seconds(8),
        //     {{dialogue_ellipse_detector}}
        // );
        // if (ret != 0){
        //     std::cout << "ERROR! Cannot detect the dialogue ellipse" << std::endl;
        // }
        // Press B to clear the "You Pokemon happy and healthy" dialogue.
        // pbf_press_button(context, BUTTON_B, 20, 100);


        ArcPhoneDetector arc_phone_detector(env.console, env.console, std::chrono::milliseconds(100), stop_on_detected);
        run_until<ProControllerContext>(
            env.console, context,
            [](ProControllerContext& local_context){
                // pbf_mash_button(local_context, BUTTON_B, 7 * TICKS_PER_SECOND);
                for(size_t i = 0; i < 15; i++){
                     pbf_press_button(local_context, BUTTON_B, 20, 80);
                }
            },
            {{arc_phone_detector}}
        );        
    }

    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}




}
}
}
