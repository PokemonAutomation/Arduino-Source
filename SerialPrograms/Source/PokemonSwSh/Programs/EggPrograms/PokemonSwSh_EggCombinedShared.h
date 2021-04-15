/*  Egg Combined Shared Libraries
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_EggCombinedShared_H
#define PokemonAutomation_PokemonSwSh_EggCombinedShared_H

#include "Common/SwitchFramework/FrameworkSettings.h"
#include "Common/PokemonSwSh/PokemonSwShGameEntry.h"
#include "Common/PokemonSwSh/PokemonSwShEggRoutines.h"
#include "Common/PokemonSwSh/PokemonSwShDateSpam.h"
#include "NintendoSwitch/Framework/SingleSwitchProgram.h"
#include "PokemonSwSh_EggHelpers.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

typedef struct{
    uint8_t fetches;
    uint16_t spin_duration;
    uint16_t total_duration;
} EggCombinedBlock;

struct EggCombinedSession{
    uint8_t     BOXES_TO_HATCH;
    uint16_t    STEPS_TO_HATCH;
    float       FETCHES_PER_BATCH;
    uint16_t    SAFETY_TIME;
    uint16_t    EARLY_HATCH_SAFETY;
    uint16_t    HATCH_DELAY;
    uint32_t    TOUCH_DATE_INTERVAL;

    EggCombinedBlock sanitize_fetch_count(uint8_t desired_fetches, uint16_t duration){
        //  The amount of time during egg-fetches that counts towards incubation.
        const uint16_t FETCH_TRAVEL_TIME = TRAVEL_RIGHT_DURATION + GO_TO_LADY_DURATION;

        EggCombinedBlock block;

        //  Cap the # of fetches to 7 because things can ugly at larger values.
        block.fetches = desired_fetches;
        if (desired_fetches == 0){
            block.spin_duration = 0;
            block.total_duration = TRAVEL_RIGHT_DURATION;
            return block;
        }
        if (block.fetches > 7){
            block.fetches = 7;
        }

        //  Divide the incubation time by the # of fetches. But don't let the fetch
        //  interval drop below the FETCH_TRAVEL_TIME.
        block.total_duration = duration / block.fetches;
        if (block.total_duration >= FETCH_TRAVEL_TIME){
            block.spin_duration = block.total_duration - FETCH_TRAVEL_TIME;
        }else{
            block.fetches = duration / FETCH_TRAVEL_TIME;
            block.spin_duration = 0;
            block.total_duration = block.fetches == 0
                ? TRAVEL_RIGHT_DURATION
                : FETCH_TRAVEL_TIME;
        }

        return block;
    }
    void withdraw_column_shiftR(uint8_t column){
        menu_to_box(false);
        party_to_column(column);
        pickup_column(false);
        ssf_press_button2(BUTTON_R, BOX_CHANGE_DELAY, EGG_BUTTON_HOLD_DELAY);
        column_to_party(column);
        ssf_press_button2(BUTTON_A, BOX_PICKUP_DROP_DELAY, EGG_BUTTON_HOLD_DELAY);
        box_to_menu();
    }
    void deposit_column_shiftL(uint8_t column){
        menu_to_box(true);
        pickup_column(true);
        party_to_column(column);
        ssf_press_button2(BUTTON_L, BOX_CHANGE_DELAY, EGG_BUTTON_HOLD_DELAY);
        ssf_press_button2(BUTTON_A, BOX_PICKUP_DROP_DELAY, EGG_BUTTON_HOLD_DELAY);
        box_to_menu();
    }
    uint8_t swap_party_shift(uint8_t column){
        menu_to_box(true);
        pickup_column(true);

        //  Move to column.
        party_to_column(column);
        ssf_press_button2(BUTTON_L, BOX_CHANGE_DELAY, EGG_BUTTON_HOLD_DELAY);
        ssf_press_button2(BUTTON_A, BOX_PICKUP_DROP_DELAY, EGG_BUTTON_HOLD_DELAY);

        //  Move to next column.
        column++;
        if (column < 6){
            ssf_press_dpad2(DPAD_RIGHT, BOX_SCROLL_DELAY, EGG_BUTTON_HOLD_DELAY);
        }else{
            column = 0;
            ssf_press_button2(BUTTON_R, BOX_CHANGE_DELAY, EGG_BUTTON_HOLD_DELAY);
            ssf_press_dpad2(DPAD_RIGHT, BOX_SCROLL_DELAY, EGG_BUTTON_HOLD_DELAY);
            ssf_press_dpad2(DPAD_RIGHT, BOX_SCROLL_DELAY, EGG_BUTTON_HOLD_DELAY);
        }

        pickup_column(false);

        //  Move to party.
        ssf_press_button2(BUTTON_R, BOX_CHANGE_DELAY, EGG_BUTTON_HOLD_DELAY);
        column_to_party(column);
        ssf_press_button2(BUTTON_A, BOX_PICKUP_DROP_DELAY, EGG_BUTTON_HOLD_DELAY);

        //  Return to menu.
        box_to_menu();

        return column;
    }

#define TRAVEL_TO_SPIN_SPOT_DURATION    (300)
//#define TRAVEL_BACK_TO_LADY_DURATION    (30 + 260 + (580) + 120 + 120 * 0)
#define TRAVEL_BACK_TO_LADY_DURATION    (30 + 260 + (620) + 120 + 120 * 0)

    void eggcombined2_run_batch(
        uint16_t INCUBATION_DELAY_LOWER,
        uint16_t remaining_travel_duration,
        uint8_t column,
        uint8_t fetches,
        bool last_batch
    ){
        const uint16_t MIN_TRAVEL_TIME = TRAVEL_TO_SPIN_SPOT_DURATION + TRAVEL_BACK_TO_LADY_DURATION;

        //  Trim off TRAVEL_TO_SPIN_SPOT_DURATION for the last iteration.
        uint16_t loop_incubation = INCUBATION_DELAY_LOWER < TRAVEL_TO_SPIN_SPOT_DURATION
            ? 0
            : INCUBATION_DELAY_LOWER - TRAVEL_TO_SPIN_SPOT_DURATION;

        while (fetches > 1 && loop_incubation >= MIN_TRAVEL_TIME){
            uint16_t period = loop_incubation / (fetches - 1);
            uint16_t spin = 0;
            if (period >= MIN_TRAVEL_TIME){
                spin = period - MIN_TRAVEL_TIME;
                spin /= 128;
                spin *= 128;
            }

            collect_egg();
            collect_egg_mash_out(AUTO_DEPOSIT);

            travel_to_spin_location();
            spin_and_mash_A(spin);
            travel_back_to_lady();

            fetches--;
            loop_incubation -= MIN_TRAVEL_TIME + spin;
            remaining_travel_duration -= MIN_TRAVEL_TIME + spin;
        }

        //  Last fetch.
        if (fetches > 0){
            collect_egg();
            collect_egg_mash_out(AUTO_DEPOSIT);
            fetches--;
        }
        travel_to_spin_location();

        //  Hatch eggs.
        if (remaining_travel_duration >= END_BATCH_MASH_B_DURATION){
            spin_and_mash_A(remaining_travel_duration - END_BATCH_MASH_B_DURATION);
            pbf_mash_button(BUTTON_B, END_BATCH_MASH_B_DURATION);
        }else{
            spin_and_mash_A(remaining_travel_duration);
        }

        if (fetches == 0){
            //  Swap party.
            ssf_press_button2(BUTTON_X, OVERWORLD_TO_MENU_DELAY, 20);
            if (last_batch){
                deposit_column_shiftL(column);
                ssf_press_button2(BUTTON_B, MENU_TO_OVERWORLD_DELAY, 20);
            }else{
                swap_party_shift(column);
                fly_home_goto_lady(false);
            }
            return;
        }

        //  Additional fetches.
        fly_home_goto_lady(true);
        while (fetches-- > 0){
            collect_egg();
            collect_egg_mash_out(AUTO_DEPOSIT);
            eggfetcher_loop();
        }

        //  Swap party.
        ssf_press_button2(BUTTON_X, OVERWORLD_TO_MENU_DELAY, 20);
        if (last_batch){
            deposit_column_shiftL(column);
        }else{
            swap_party_shift(column);
        }
        ssf_press_button2(BUTTON_B, MENU_TO_OVERWORLD_DELAY, 20);
    }

    void eggcombined2_body(SingleSwitchProgramEnvironment& env){
        if (BOXES_TO_HATCH == 0){
            ssf_press_button2(BUTTON_HOME, GAME_TO_HOME_DELAY_SAFE, 10);
            return;
        }

        //  Calculate lower/upper bounds for incubation time.
        const uint16_t INCUBATION_DELAY_UPPER = (uint16_t)((uint32_t)STEPS_TO_HATCH * (uint32_t)103180 >> 16);
        const uint16_t INCUBATION_DELAY_LOWER = INCUBATION_DELAY_UPPER >= EARLY_HATCH_SAFETY
            ? INCUBATION_DELAY_UPPER - EARLY_HATCH_SAFETY
            : 0;

        const uint16_t FINISH_DELAY = HATCH_DELAY + SAFETY_TIME;

        float fetches_per_batch = FETCHES_PER_BATCH;
        if (fetches_per_batch < 0){
            fetches_per_batch = 0;
        }

        float fetch_residual = 0;

        //  Withdraw party.
        ssf_press_button2(BUTTON_X, OVERWORLD_TO_MENU_DELAY, 20);
        withdraw_column_shiftR(0);
        fly_home_goto_lady(false);

        uint32_t last_touch = system_clock() - TOUCH_DATE_INTERVAL;
        for (uint8_t box = 0; box < BOXES_TO_HATCH; box++){
            for (uint8_t column = 0; column < 6; column++){
                //  Touch the date.
                if (TOUCH_DATE_INTERVAL > 0 && system_clock() - last_touch >= TOUCH_DATE_INTERVAL){
                    env.log("Touching date to prevent rollover.");
                    pbf_press_button(BUTTON_HOME, 10, GAME_TO_HOME_DELAY_SAFE);
                    touch_date_from_home(SETTINGS_TO_HOME_DELAY);
                    resume_game_no_interact(TOLERATE_SYSTEM_UPDATE_MENU_FAST);
                    last_touch += TOUCH_DATE_INTERVAL;
                }

                fetch_residual += fetches_per_batch;
                uint8_t fetches = (uint8_t)fetch_residual;
                eggcombined2_run_batch(
                    INCUBATION_DELAY_LOWER,
                    INCUBATION_DELAY_UPPER + FINISH_DELAY,
                    column,
                    fetches,
                    box == BOXES_TO_HATCH - 1 && column == 5
                );
                fetch_residual -= (float)fetches;
            }
        }

        //  Finish
        ssf_press_button2(BUTTON_HOME, GAME_TO_HOME_DELAY_SAFE, 10);
    }

};



}
}
}
#endif

