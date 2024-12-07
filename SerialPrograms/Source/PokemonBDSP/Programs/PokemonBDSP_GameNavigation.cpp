/*  Game Entry
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonBDSP/PokemonBDSP_Settings.h"
#include "PokemonBDSP/Inference/PokemonBDSP_MenuDetector.h"
#include "PokemonBDSP/Inference/BoxSystem/PokemonBDSP_BoxDetector.h"
#include "PokemonBDSP_GameNavigation.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


//  Non-Feedback

void save_game(BotBaseContext& context){
    pbf_press_button(context, BUTTON_X, 10, GameSettings::instance().OVERWORLD_TO_MENU_DELAY);
    pbf_press_button(context, BUTTON_R, 10, 2 * TICKS_PER_SECOND);
    pbf_press_button(context, BUTTON_ZL, 10, 5 * TICKS_PER_SECOND);
}
void menu_to_box(BotBaseContext& context){
    uint16_t MENU_TO_POKEMON_DELAY = GameSettings::instance().MENU_TO_POKEMON_DELAY;
    pbf_mash_button(context, BUTTON_ZL, 30);
    if (MENU_TO_POKEMON_DELAY > 30){
        pbf_wait(context, MENU_TO_POKEMON_DELAY - 30);
    }

    pbf_press_button(context, BUTTON_R, 20, GameSettings::instance().POKEMON_TO_BOX_DELAY0);
}
void overworld_to_box(BotBaseContext& context){
    pbf_press_button(context, BUTTON_X, 20, GameSettings::instance().OVERWORLD_TO_MENU_DELAY);
//    pbf_press_button(context, BUTTON_ZL, 20, GameSettings::instance().MENU_TO_POKEMON_DELAY);

    menu_to_box(context);
}
void box_to_overworld(BotBaseContext& context){
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
    pbf_press_button(context, BUTTON_B, 20, GameSettings::instance().BOX_TO_POKEMON_DELAY);

    pbf_press_button(context, BUTTON_B, 20, GameSettings::instance().POKEMON_TO_MENU_DELAY);
    pbf_press_button(context, BUTTON_X, 20, GameSettings::instance().MENU_TO_OVERWORLD_DELAY);
}



//  Feedback

void overworld_to_menu(ConsoleHandle& console, BotBaseContext& context){
    pbf_press_button(context, BUTTON_X, 20, 105);
    context.wait_for_all_requests();
    {
        MenuWatcher detector;
        int ret = wait_until(
            console, context, std::chrono::seconds(10),
            {{detector}}
        );
        if (ret < 0){
            OperationFailedException::fire(
                console, ErrorReport::SEND_ERROR_REPORT,
                "Menu not detected after 10 seconds."
            );
        }
        console.log("Detected menu.");
    }
    context.wait_for(std::chrono::milliseconds(100));
}

void save_game(ConsoleHandle& console, BotBaseContext& context){
    overworld_to_menu(console, context);
    pbf_press_button(context, BUTTON_R, 10, 2 * TICKS_PER_SECOND);
    pbf_press_button(context, BUTTON_ZL, 10, 5 * TICKS_PER_SECOND);
}

void overworld_to_box(ConsoleHandle& console, BotBaseContext& context){
    //  Open menu.
    overworld_to_menu(console, context);

    //  Enter Pokemon
    uint16_t MENU_TO_POKEMON_DELAY = GameSettings::instance().MENU_TO_POKEMON_DELAY;
#if 0
//    pbf_mash_button(context, BUTTON_ZL, 30);
    if (MENU_TO_POKEMON_DELAY > 30){
        pbf_wait(context, MENU_TO_POKEMON_DELAY - 30);
    }
#else
    pbf_press_button(context, BUTTON_ZL, 20, MENU_TO_POKEMON_DELAY);
#endif

    //  Enter box system.
    pbf_press_button(context, BUTTON_R, 20, 105);
    context.wait_for_all_requests();
    {
        BoxWatcher detector;
        int ret = wait_until(
            console, context, std::chrono::seconds(10),
            {{detector}}
        );
        if (ret < 0){
            OperationFailedException::fire(
                console, ErrorReport::SEND_ERROR_REPORT,
                "Box system not detected after 10 seconds."
            );
        }
        console.log("Detected box system.");
    }
    context.wait_for(std::chrono::milliseconds(500));
}
void box_to_overworld(ConsoleHandle& console, BotBaseContext& context){
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
    pbf_press_button(context, BUTTON_B, 20, GameSettings::instance().BOX_TO_POKEMON_DELAY);

    //  To menu.
    pbf_press_button(context, BUTTON_B, 20, 105);
    context.wait_for_all_requests();
    {
        MenuWatcher detector;
        int ret = wait_until(
            console, context, std::chrono::seconds(10),
            {{detector}}
        );
        if (ret < 0){
            OperationFailedException::fire(
                console, ErrorReport::SEND_ERROR_REPORT,
                "Menu not detected after 10 seconds."
            );
        }
        console.log("Detected menu.");
    }

    //  To overworld.
    pbf_press_button(context, BUTTON_X, 20, GameSettings::instance().MENU_TO_OVERWORLD_DELAY);
}





}
}
}
