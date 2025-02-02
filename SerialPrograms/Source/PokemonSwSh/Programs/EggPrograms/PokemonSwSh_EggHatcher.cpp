/*  Egg Hatcher
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "Pokemon/Pokemon_Strings.h"
//#include "PokemonSwSh/Commands/PokemonSwSh_Commands_GameEntry.h"
#include "PokemonSwSh/Commands/PokemonSwSh_Commands_EggRoutines.h"
#include "PokemonSwSh/Programs/PokemonSwSh_GameEntry.h"
#include "PokemonSwSh_EggHelpers.h"
#include "PokemonSwSh_EggHatcher.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
    using namespace Pokemon;


EggHatcher_Descriptor::EggHatcher_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSwSh:EggHatcher",
        STRING_POKEMON + " SwSh", "Egg Hatcher",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSwSh/EggHatcher.md",
        "Hatch eggs from boxes.",
        FeedbackType::NONE,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        {SerialPABotBase::OLD_NINTENDO_SWITCH_DEFAULT_REQUIREMENTS}
    )
{}



void withdraw_column(SwitchControllerContext& context, uint8_t column){
    menu_to_box(context, false);
    party_to_column(context, column);
    pickup_column(context, false);
    column_to_party(context, column);
    ssf_press_button(context, BUTTON_A, GameSettings::instance().BOX_PICKUP_DROP_DELAY0);
    box_to_menu(context);
}
void deposit_column(SwitchControllerContext& context, uint8_t column){
    menu_to_box(context, true);
    pickup_column(context, true);
    party_to_column(context, column);
    ssf_press_button(context, BUTTON_A, GameSettings::instance().BOX_PICKUP_DROP_DELAY0);
    box_to_menu(context);
}
uint8_t swap_party(SwitchControllerContext& context, uint8_t column){
    menu_to_box(context, true);
    pickup_column(context, true);

    Milliseconds BOX_PICKUP_DROP_DELAY = GameSettings::instance().BOX_PICKUP_DROP_DELAY0;
    Milliseconds BOX_SCROLL_DELAY = GameSettings::instance().BOX_SCROLL_DELAY0;

    //  Move to column.
    party_to_column(context, column);
    ssf_press_button(context, BUTTON_A, BOX_PICKUP_DROP_DELAY);

    //  Move to next column.
    column++;
    if (column < 6){
        ssf_press_dpad(context, DPAD_RIGHT, BOX_SCROLL_DELAY);
    }else{
        column = 0;
        ssf_press_button(context, BUTTON_R, GameSettings::instance().BOX_CHANGE_DELAY0);
        ssf_press_dpad(context, DPAD_RIGHT, BOX_SCROLL_DELAY);
        ssf_press_dpad(context, DPAD_RIGHT, BOX_SCROLL_DELAY);
    }

    pickup_column(context, false);

    //  Move to party.
    column_to_party(context, column);
    ssf_press_button(context, BUTTON_A, BOX_PICKUP_DROP_DELAY);

    //  Return to menu.
    box_to_menu(context);

    return column;
}


EggHatcher::EggHatcher()
    : BOXES_TO_HATCH(
        "<b>Boxes to Hatch:</b>",
        LockMode::LOCK_WHILE_RUNNING,
        3
    )
    , m_advanced_options(
        "<font size=4><b>Advanced Options:</b> You should not need to touch anything below here.</font>"
    )
    , SAFETY_TIME0(
        "<b>Safety Time:</b><br>Additional time added to the spinning.",
        LockMode::LOCK_WHILE_RUNNING,
        TICKS_PER_SECOND,
        "12 * TICKS_PER_SECOND"
    )
    , HATCH_DELAY(
        "<b>Hatch Delay:</b><br>Total animation time for hatching 5 eggs when there are no shinies.",
        LockMode::LOCK_WHILE_RUNNING,
        TICKS_PER_SECOND,
        "88 * TICKS_PER_SECOND"
    )
    , NOTIFICATIONS({
        &NOTIFICATION_PROGRAM_FINISH,
    })
{
    PA_ADD_OPTION(START_LOCATION);
    PA_ADD_OPTION(BOXES_TO_HATCH);
    PA_ADD_OPTION(STEPS_TO_HATCH);
    PA_ADD_STATIC(m_advanced_options);
    PA_ADD_OPTION(SAFETY_TIME0);
    PA_ADD_OPTION(HATCH_DELAY);
    PA_ADD_OPTION(NOTIFICATIONS);
}
void EggHatcher::program(SingleSwitchProgramEnvironment& env, SwitchControllerContext& context){
    //  Calculate upper bounds for incubation time.
    uint16_t INCUBATION_DELAY_UPPER = (uint16_t)((uint32_t)STEPS_TO_HATCH * 2 * (uint32_t)103180 >> 16);
    uint16_t TOTAL_DELAY = INCUBATION_DELAY_UPPER + HATCH_DELAY + SAFETY_TIME0 - TRAVEL_RIGHT_DURATION;

    if (START_LOCATION.start_in_grip_menu()){
        grip_menu_connect_go_home(context);
        resume_game_back_out(env.console, context, ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST, 400);
    }else{
        pbf_press_button(context, BUTTON_B, 5, 5);
    }

    bool party_is_empty = true;
    for (uint8_t box = 0; box < BOXES_TO_HATCH; box++){
        for (uint8_t column = 0; column < 6; column++){
            //  Get eggs from box.
            ssf_press_button(context, BUTTON_X, GameSettings::instance().OVERWORLD_TO_MENU_DELAY0);
            if (party_is_empty){
                withdraw_column(context, column);
                party_is_empty = false;
            }else if (column == 0){
                swap_party(context, 5);
            }else{
                swap_party(context, column - 1);
            }

            fly_home(context, false);

            //  Travel to spin location.
            pbf_move_left_joystick(context, STICK_MAX, STICK_CENTER, TRAVEL_RIGHT_DURATION, 0);

            //  Spin
#if 0
            spin_and_mash_A(TOTAL_DELAY);
#else
            if (TOTAL_DELAY >= END_BATCH_MASH_B_DURATION){
                spin_and_mash_A(context, TOTAL_DELAY - END_BATCH_MASH_B_DURATION);
                pbf_mash_button(context, BUTTON_B, END_BATCH_MASH_B_DURATION);
            }else{
                spin_and_mash_A(context, TOTAL_DELAY);
            }
#endif
        }
    }

    Milliseconds OVERWORLD_TO_MENU_DELAY = GameSettings::instance().OVERWORLD_TO_MENU_DELAY0;

    if (!party_is_empty){
        ssf_press_button(context, BUTTON_X, OVERWORLD_TO_MENU_DELAY);
        deposit_column(context, 5);
        ssf_press_button(context, BUTTON_X, OVERWORLD_TO_MENU_DELAY);
    }
    ssf_press_button(context, BUTTON_HOME, GameSettings::instance().GAME_TO_HOME_DELAY_SAFE0);

    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}


}
}
}
