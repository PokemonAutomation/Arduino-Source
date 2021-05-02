/*  Watt Farmer
 * 
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 * 
 *  This program is identical to DateSpam-WattFarmer in the Arduino programs.
 *  Please follow the same start instructions.
 * 
 */

#include <iostream>
#include "Common/SwitchFramework/FrameworkSettings.h"
#include "Common/SwitchFramework/Switch_PushButtons.h"
#include "Common/PokemonSwSh/PokemonSettings.h"
#include "Common/PokemonSwSh/PokemonSwShDateSpam.h"
#include "ClientSource/Libraries/Utilities.h"

namespace PokemonAutomation{



//  Grab watts this many times. You can set this number if you're also date
//  skipping to a particular den frame and you don't want to overshoot it.
//
//  Be aware that this program isn't intended to be an accurate date skipper.
//  It will occasionally miss frames causing it to fall short.
const uint32_t SKIPS            =   33334;

//  Save the game every this number of iterations.
//  If set to zero, no saving is done.
const uint16_t SAVE_ITERATIONS  =   0;



void program_DateSpam_WattFarmer(const std::string& device_name){
    std::cout << "Starting PABotBase - DateSpam-WattFarmer..." << std::endl;
    std::cout << std::endl;
    std::unique_ptr<PABotBase> pabotbase = start_connection(true, device_name);
//    global_connection = pabotbase.get();

    std::cout << "Begin Message Logging..." << std::endl;
    MessageLogger logger;
    pabotbase->set_sniffer(&logger);



    start_program_flash(*pabotbase, CONNECT_CONTROLLER_DELAY);
    grip_menu_connect_go_home(*pabotbase);

    uint8_t year = MAX_YEAR;
    uint16_t save_count = 0;
    for (uint32_t c = 0; c < SKIPS; c++){
//        pabb_send_info_i32(c);
        log("Frames Skipped: " + std::to_string(c));
        home_roll_date_enter_game_autorollback(*pabotbase, &year);
        pbf_mash_button(*pabotbase, BUTTON_B, 90);

        pbf_press_button(*pabotbase, BUTTON_A, 5, 5);
        pbf_mash_button(*pabotbase, BUTTON_B, 215);

        if (SAVE_ITERATIONS != 0){
            save_count++;
            if (save_count >= SAVE_ITERATIONS){
                save_count = 0;
                pbf_mash_button(*pabotbase, BUTTON_B, 2 * TICKS_PER_SECOND);
                pbf_press_button(*pabotbase, BUTTON_X, 20, OVERWORLD_TO_MENU_DELAY);
                pbf_press_button(*pabotbase, BUTTON_R, 20, 2 * TICKS_PER_SECOND);
                pbf_press_button(*pabotbase, BUTTON_ZL, 20, 3 * TICKS_PER_SECOND);
            }
        }

        //  Tap HOME and quickly spam B. The B spamming ensures that we don't
        //  accidentally update the system if the system update window pops up.
        pbf_press_button(*pabotbase, BUTTON_HOME, 10, 5);
        pbf_mash_button(*pabotbase, BUTTON_B, GAME_TO_HOME_DELAY_FAST - 15);
    }

    end_program_callback(*pabotbase);
    end_program_loop(*pabotbase);
}


}
