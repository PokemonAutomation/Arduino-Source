/*  Egg Hatcher
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Device.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "PokemonSwSh/Commands/PokemonSwSh_Commands_GameEntry.h"
#include "PokemonSwSh/Commands/PokemonSwSh_Commands_EggRoutines.h"
#include "PokemonSwSh_EggHelpers.h"
#include "PokemonSwSh_EggHatcher.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


EggHatcher_Descriptor::EggHatcher_Descriptor()
    : RunnableSwitchProgramDescriptor(
        "PokemonSwSh:EggHatcher",
        STRING_POKEMON + " SwSh", "Egg Hatcher",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSwSh/EggHatcher.md",
        "Hatch eggs from boxes.",
        FeedbackType::NONE, false,
        PABotBaseLevel::PABOTBASE_31KB
    )
{}



void withdraw_column(const BotBaseContext& context, uint8_t column){
    menu_to_box(context, false);
    party_to_column(context, column);
    pickup_column(context, false);
    column_to_party(context, column);
    ssf_press_button1(context, BUTTON_A, GameSettings::instance().BOX_PICKUP_DROP_DELAY);
    box_to_menu(context);
}
void deposit_column(const BotBaseContext& context, uint8_t column){
    menu_to_box(context, true);
    pickup_column(context, true);
    party_to_column(context, column);
    ssf_press_button1(context, BUTTON_A, GameSettings::instance().BOX_PICKUP_DROP_DELAY);
    box_to_menu(context);
}
uint8_t swap_party(const BotBaseContext& context, uint8_t column){
    menu_to_box(context, true);
    pickup_column(context, true);

    uint16_t BOX_PICKUP_DROP_DELAY = GameSettings::instance().BOX_PICKUP_DROP_DELAY;
    uint16_t BOX_SCROLL_DELAY = GameSettings::instance().BOX_SCROLL_DELAY;

    //  Move to column.
    party_to_column(context, column);
    ssf_press_button1(context, BUTTON_A, BOX_PICKUP_DROP_DELAY);

    //  Move to next column.
    column++;
    if (column < 6){
        ssf_press_dpad1(context, DPAD_RIGHT, BOX_SCROLL_DELAY);
    }else{
        column = 0;
        ssf_press_button1(context, BUTTON_R, GameSettings::instance().BOX_CHANGE_DELAY);
        ssf_press_dpad1(context, DPAD_RIGHT, BOX_SCROLL_DELAY);
        ssf_press_dpad1(context, DPAD_RIGHT, BOX_SCROLL_DELAY);
    }

    pickup_column(context, false);

    //  Move to party.
    column_to_party(context, column);
    ssf_press_button1(context, BUTTON_A, BOX_PICKUP_DROP_DELAY);

    //  Return to menu.
    box_to_menu(context);

    return column;
}


EggHatcher::EggHatcher(const EggHatcher_Descriptor& descriptor)
    : SingleSwitchProgramInstance(descriptor)
    , BOXES_TO_HATCH(
        "<b>Boxes to Hatch:</b>",
        3
    )
    , m_advanced_options(
        "<font size=4><b>Advanced Options:</b> You should not need to touch anything below here.</font>"
    )
    , SAFETY_TIME(
        "<b>Safety Time:</b><br>Additional time added to the spinning.",
        "8 * TICKS_PER_SECOND"
    )
    , HATCH_DELAY(
        "<b>Hatch Delay:</b><br>Total animation time for hatching 5 eggs when there are no shinies.",
        "88 * TICKS_PER_SECOND"
    )
{
    PA_ADD_OPTION(START_IN_GRIP_MENU);

    PA_ADD_OPTION(BOXES_TO_HATCH);
    PA_ADD_OPTION(STEPS_TO_HATCH);
    PA_ADD_STATIC(m_advanced_options);
    PA_ADD_OPTION(SAFETY_TIME);
    PA_ADD_OPTION(HATCH_DELAY);
}
void EggHatcher::program(SingleSwitchProgramEnvironment& env, CancellableScope& scope){
    //  Calculate upper bounds for incubation time.
    uint16_t INCUBATION_DELAY_UPPER = (uint16_t)((uint32_t)STEPS_TO_HATCH * (uint32_t)103180 >> 16);
    uint16_t TOTAL_DELAY = INCUBATION_DELAY_UPPER + HATCH_DELAY + SAFETY_TIME - TRAVEL_RIGHT_DURATION;

    if (START_IN_GRIP_MENU){
        grip_menu_connect_go_home(env.console);
        resume_game_back_out(env.console, ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST, 400);
    }else{
        pbf_press_button(env.console, BUTTON_B, 5, 5);
    }

    bool party_is_empty = true;
    for (uint8_t box = 0; box < BOXES_TO_HATCH; box++){
        for (uint8_t column = 0; column < 6; column++){
            //  Get eggs from box.
            pbf_press_button(env.console, BUTTON_X, 20, GameSettings::instance().OVERWORLD_TO_MENU_DELAY - 20);
            if (party_is_empty){
                withdraw_column(env.console, column);
                party_is_empty = false;
            }else if (column == 0){
                swap_party(env.console, 5);
            }else{
                swap_party(env.console, column - 1);
            }

            fly_home(env.console, false);

            //  Travel to spin location.
            pbf_move_left_joystick(env.console, STICK_MAX, STICK_CENTER, TRAVEL_RIGHT_DURATION, 0);

            //  Spin
#if 0
            spin_and_mash_A(TOTAL_DELAY);
#else
            if (TOTAL_DELAY >= END_BATCH_MASH_B_DURATION){
                spin_and_mash_A(env.console, TOTAL_DELAY - END_BATCH_MASH_B_DURATION);
                pbf_mash_button(env.console, BUTTON_B, END_BATCH_MASH_B_DURATION);
            }else{
                spin_and_mash_A(env.console, TOTAL_DELAY);
            }
#endif
        }
    }

    uint16_t OVERWORLD_TO_MENU_DELAY = GameSettings::instance().OVERWORLD_TO_MENU_DELAY;

    if (!party_is_empty){
        pbf_press_button(env.console, BUTTON_X, 20, OVERWORLD_TO_MENU_DELAY - 20);
        deposit_column(env.console, 5);
        pbf_press_button(env.console, BUTTON_X, 20, OVERWORLD_TO_MENU_DELAY - 20);
    }
    pbf_press_button(env.console, BUTTON_HOME, 10, GameSettings::instance().GAME_TO_HOME_DELAY_SAFE - 10);
}


}
}
}
