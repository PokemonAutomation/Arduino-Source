/*  Game Entry
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonBDSP/PokemonBDSP_Settings.h"
#include "PokemonBDSP/Inference/PokemonBDSP_MenuDetector.h"
#include "PokemonBDSP/Inference/BoxSystem/PokemonBDSP_BoxDetector.h"
#include "PokemonBDSP_GameNavigation.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{



//  Non-Feedback

void save_game(ProControllerContext& context){
    pbf_press_button(context, BUTTON_X, 80ms, GameSettings::instance().OVERWORLD_TO_MENU_DELAY0);
    pbf_press_button(context, BUTTON_R, 80ms, 2000ms);
    pbf_press_button(context, BUTTON_ZL, 80ms, 5000ms);
}
void menu_to_box(ProControllerContext& context){
    Milliseconds MENU_TO_POKEMON_DELAY = GameSettings::instance().MENU_TO_POKEMON_DELAY0;
    pbf_mash_button(context, BUTTON_ZL, 30);
    if (MENU_TO_POKEMON_DELAY > 240ms){
        pbf_wait(context, MENU_TO_POKEMON_DELAY - 240ms);
    }

    pbf_press_button(context, BUTTON_R, 160ms, GameSettings::instance().POKEMON_TO_BOX_DELAY1);
}
void overworld_to_box(ProControllerContext& context){
    pbf_press_button(context, BUTTON_X, 160ms, GameSettings::instance().OVERWORLD_TO_MENU_DELAY0);
//    pbf_press_button(context, BUTTON_ZL, 160ms, GameSettings::instance().MENU_TO_POKEMON_DELAY);

    menu_to_box(context);
}
void box_to_overworld(ProControllerContext& context){
    //  There are two states here which need to be merged:
    //      1.  The depositing column was empty. The party has been swapped and
    //          it's sitting in the box with no held pokemon.
    //      2.  The depositing column was not empty. The party swap failed and
    //          it's sitting in the box holding on the party pokemon.
    //
    //  Double press B quickly here to back out of the box.
    //  In state (1):   The 1st B will begin back out of the box. The 2nd B will
    //                  be swallowed by the animation.
    //  In state (2):   The 1st B will drop the party pokemon. The 2nd B will
    //                  back out of the box.

    pbf_press_button(context, BUTTON_B, 20, 30);
    pbf_press_button(context, BUTTON_B, 160ms, GameSettings::instance().BOX_TO_POKEMON_DELAY0);

    pbf_press_button(context, BUTTON_B, 160ms, GameSettings::instance().POKEMON_TO_MENU_DELAY0);
    pbf_press_button(context, BUTTON_X, 160ms, GameSettings::instance().MENU_TO_OVERWORLD_DELAY0);
}



//  Feedback

void overworld_to_menu(VideoStream& stream, ProControllerContext& context){
    pbf_press_button(context, BUTTON_X, 20, 105);
    context.wait_for_all_requests();
    {
        MenuWatcher detector;
        int ret = wait_until(
            stream, context, std::chrono::seconds(10),
            {{detector}}
        );
        if (ret < 0){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Menu not detected after 10 seconds.",
                stream
            );
        }
        stream.log("Detected menu.");
    }
    context.wait_for(std::chrono::milliseconds(100));
}

void save_game(VideoStream& stream, ProControllerContext& context){
    overworld_to_menu(stream, context);
    pbf_press_button(context, BUTTON_R, 10, 2 * TICKS_PER_SECOND);
    pbf_press_button(context, BUTTON_ZL, 10, 5 * TICKS_PER_SECOND);
}

void overworld_to_box(VideoStream& stream, ProControllerContext& context){
    //  Open menu.
    overworld_to_menu(stream, context);

    //  Enter Pokemon
    Milliseconds MENU_TO_POKEMON_DELAY = GameSettings::instance().MENU_TO_POKEMON_DELAY0;
    pbf_press_button(context, BUTTON_ZL, 160ms, MENU_TO_POKEMON_DELAY);

    //  Enter box system.
    pbf_press_button(context, BUTTON_R, 20, 105);
    context.wait_for_all_requests();
    {
        BoxWatcher detector;
        int ret = wait_until(
            stream, context, std::chrono::seconds(10),
            {{detector}}
        );
        if (ret < 0){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Box system not detected after 10 seconds.",
                stream
            );
        }
        stream.log("Detected box system.");
    }
    context.wait_for(std::chrono::milliseconds(500));
}
void box_to_overworld(VideoStream& stream, ProControllerContext& context){
    //  There are two states here which need to be merged:
    //      1.  The depositing column was empty. The party has been swapped and
    //          it's sitting in the box with no held pokemon.
    //      2.  The depositing column was not empty. The party swap failed and
    //          it's sitting in the box holding on the party pokemon.
    //
    //  Double press B quickly here to back out of the box.
    //  In state (1):   The 1st B will begin back out of the box. The 2nd B will
    //                  be swallowed by the animation.
    //  In state (2):   The 1st B will drop the party pokemon. The 2nd B will
    //                  back out of the box.
    pbf_press_button(context, BUTTON_B, 20, 30);
    pbf_press_button(context, BUTTON_B, 160ms, GameSettings::instance().BOX_TO_POKEMON_DELAY0);

    //  To menu.
    pbf_press_button(context, BUTTON_B, 20, 105);
    context.wait_for_all_requests();
    {
        MenuWatcher detector;
        int ret = wait_until(
            stream, context, std::chrono::seconds(10),
            {{detector}}
        );
        if (ret < 0){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Menu not detected after 10 seconds.",
                stream
            );
        }
        stream.log("Detected menu.");
    }

    //  To overworld.
    pbf_press_button(context, BUTTON_X, 160ms, GameSettings::instance().MENU_TO_OVERWORLD_DELAY0);
}





}
}
}
