/*  Activate Menu Glitch (1.1.2)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "CommonFramework/Inference/BlackScreenDetector.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonBDSP/PokemonBDSP_Settings.h"
#include "PokemonBDSP/Inference/PokemonBDSP_DialogDetector.h"
#include "PokemonBDSP/Inference/PokemonBDSP_MapDetector.h"
#include "PokemonBDSP_ActivateMenuGlitch-1.1.2.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{
    using namespace Pokemon;


ActivateMenuGlitch112_Descriptor::ActivateMenuGlitch112_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonBDSP:ActivateMenuGlitch112",
        STRING_POKEMON + " BDSP", "Activate Menu Glitch (1.1.2)",
        "ComputerControl/blob/master/Wiki/Programs/PokemonBDSP/ActivateMenuGlitch-Poketch.md",
        "Activate the menu glitch using the Pok\u00e9tch. "
        "<font color=\"red\">(This requires game versions 1.1.0 - 1.1.2. The glitch it relies on was patched in v1.1.3.)</font>",
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}

ActivateMenuGlitch112::ActivateMenuGlitch112()
    : FLY_A_TO_X_DELAY(
        "<b>Fly Menu A-to-X Delay:</b><br>The delay between the A and X presses to overlap the menu with the fly option.<br>"
        "(German players may need to increase this to 90.)",
        LockMode::LOCK_WHILE_RUNNING,
        TICKS_PER_SECOND,
        20, "50"
    )
{
    PA_ADD_OPTION(FLY_A_TO_X_DELAY);
}



void trigger_menu(ConsoleHandle& console, BotBaseContext& context){
    context.wait_for_all_requests();
    MapWatcher detector;
    int ret = run_until(
        console, context,
        [](BotBaseContext& context){
            for (size_t i = 0; i < 12; i++){
                for (size_t c = 0; c < 42; c++){
                    pbf_controller_state(context, BUTTON_ZL, DPAD_NONE, 128, 128, 128, 128, 1);
                    pbf_controller_state(context, BUTTON_R | BUTTON_ZL, DPAD_NONE, 128, 128, 128, 128, 5);
                    pbf_wait(context, 3);
                }
                pbf_wait(context, 125);
                pbf_press_button(context, BUTTON_R, 20, 105);
            }
        },
        {{detector}}
    );
    if (ret < 0){
        OperationFailedException::fire(
            console, ErrorReport::SEND_ERROR_REPORT,
            "Map not detected after 60 seconds."
        );
    }
    console.log("Detected map!", COLOR_BLUE);

    context.wait_for(std::chrono::milliseconds(500));
    ShortDialogDetector dialog;
    while (dialog.detect(console.video().snapshot())){
        console.log("Overshot mashing. Backing out.", COLOR_ORANGE);
        pbf_press_button(context, BUTTON_B, 20, 105);
        context.wait_for_all_requests();
    }
}
void trigger_map_overlap(ConsoleHandle& console, BotBaseContext& context){
    for (size_t c = 0; c < 10; c++){
        trigger_menu(console, context);

        pbf_press_dpad(context, DPAD_UP, 50, 0);
        context.wait_for_all_requests();
        BlackScreenWatcher detector;
        int ret = wait_until(
            console, context, std::chrono::seconds(4),
            {{detector}}
        );
        if (ret >= 0){
            console.log("Overlap detected! Entered " + STRING_POKEMON + " center.", COLOR_BLUE);
            return;
        }
        console.log("Failed to activate map overlap.", COLOR_ORANGE);
        pbf_mash_button(context, BUTTON_B, 3 * TICKS_PER_SECOND);
        pbf_press_button(context, BUTTON_R, 20, 230);
    }
    OperationFailedException::fire(
        console, ErrorReport::SEND_ERROR_REPORT,
        "Failed to trigger map overlap after 10 attempts."
    );
}



void ActivateMenuGlitch112::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    ConsoleHandle& console = env.console;

    trigger_map_overlap(console, context);
    pbf_wait(context, 3 * TICKS_PER_SECOND);

    //  Move to escalator.
    pbf_press_dpad(context, DPAD_UP, 20, 125);
    pbf_press_dpad(context, DPAD_UP, 20, 125);
    pbf_move_left_joystick(context, 255, 128, 250, 5 * TICKS_PER_SECOND);

    //  Re-enter escalator.
    pbf_press_dpad(context, DPAD_RIGHT, 125, 6 * TICKS_PER_SECOND);

    //  Leave Pokemon center.
    pbf_press_dpad(context, DPAD_LEFT, 20, 105);
    pbf_press_dpad(context, DPAD_LEFT, 20, 105);
    pbf_press_dpad(context, DPAD_LEFT, 20, 105);
    {
        context.wait_for_all_requests();
        BlackScreenWatcher detector;
        int ret = run_until(
            console, context,
            [](BotBaseContext& context){
                for (size_t c = 0; c < 5; c++){
                    pbf_press_dpad(context, DPAD_LEFT, 20, 105);
                    pbf_press_dpad(context, DPAD_DOWN, 20, 105);
                }
            },
            {{detector}}
        );
        if (ret < 0){
            OperationFailedException::fire(
                console, ErrorReport::SEND_ERROR_REPORT,
                "Unable to leave " + STRING_POKEMON + " center."
            );
        }
        console.log("Leaving " + STRING_POKEMON + " center detected!", COLOR_BLUE);
    }
    pbf_move_left_joystick(context, 128, 255, 125, 4 * TICKS_PER_SECOND);

    //  Center cursor.
    pbf_press_button(context, BUTTON_X, 20, GameSettings::instance().OVERWORLD_TO_MENU_DELAY);
    pbf_press_button(context, BUTTON_X, 20, GameSettings::instance().MENU_TO_OVERWORLD_DELAY);

    //  Bring up menu
    pbf_press_button(context, BUTTON_ZL, 20, FLY_A_TO_X_DELAY - 20);
    pbf_press_button(context, BUTTON_X, 20, GameSettings::instance().OVERWORLD_TO_MENU_DELAY);

    //  Fly
    pbf_press_button(context, BUTTON_ZL, 20, 10 * TICKS_PER_SECOND);

    //  Enter Pokemon center.
    pbf_press_dpad(context, DPAD_UP, 50, 5 * TICKS_PER_SECOND);
    pbf_move_left_joystick(context, 255, 128, 125, 0);
    pbf_move_left_joystick(context, 128, 255, 125, 125);

    //  Move cursor back to default location for "Pokemon".
    pbf_move_right_joystick(context, 128, 0, 20, 20);
    pbf_move_right_joystick(context, 0, 128, 20, 20);
    pbf_move_right_joystick(context, 0, 128, 20, 20);
}



}
}
}
