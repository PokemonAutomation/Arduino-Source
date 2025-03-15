/*  Beam Reset
 * 
 *  From: https://github.com/PokemonAutomation/
 * 
 *  This program is identical to BeamReset in the Arduino programs.
 *  Please follow the same start instructions.
 * 
 */

#include <iostream>
#include "Common/NintendoSwitch/FrameworkSettings.h"
#include "Common/NintendoSwitch/NintendoSwitch_Device.h"
#include "Common/NintendoSwitch/NintendoSwitch_PushButtons.h"
#include "Common/PokemonSwSh/PokemonSettings.h"
#include "Common/PokemonSwSh/PokemonSwShGameEntry.h"
#include "ClientSource/Libraries/Utilities.h"

namespace PokemonAutomation{
namespace NintendoSwitch{



//  Wait in the Home menu for this long before resetting the game.
const uint16_t DELAY_BEFORE_RESET   =   5 * TICKS_PER_SECOND;

//  Talking to the den has an extra line in German. Set this to true if this
//  is the case.
const bool EXTRA_LINE   =   false;




void program_BeamReset(const std::string& device_name){
    std::cout << "Starting PABotBase - BeamReset..." << std::endl;
    std::cout << std::endl;
    std::unique_ptr<PABotBase> pabotbase = start_connection(true, device_name);
//    global_connection = pabotbase.get();

    std::cout << "Begin Message Logging..." << std::endl;
    MessageLogger logger;
    pabotbase->set_sniffer(&logger);


    //  Start Program
    start_program_flash(*pabotbase, CONNECT_CONTROLLER_DELAY);
    grip_menu_connect_go_home(*pabotbase);

    resume_game_front_of_den_nowatts(*pabotbase, TOLERATE_SYSTEM_UPDATE_MENU_SLOW);
    pbf_mash_button(*pabotbase, BUTTON_B, 100);

    while (true){
        //  Talk to den.
        pbf_press_button(*pabotbase, BUTTON_A, 10, 450);
        if (EXTRA_LINE){
            pbf_press_button(*pabotbase, BUTTON_A, 10, 300);
        }
        pbf_press_button(*pabotbase, BUTTON_A, 10, 300);

        //  Drop wishing piece.
        pbf_press_button(*pabotbase, BUTTON_A, 10, 70);
        pbf_press_button(*pabotbase, BUTTON_HOME, 10, GAME_TO_HOME_DELAY_FAST);

        for (uint16_t c = 0; c < 4; c++){
            pbf_press_button(*pabotbase, BUTTON_HOME, 10, 10);
            pbf_press_button(*pabotbase, BUTTON_HOME, 10, 220);
        }
        pbf_wait(*pabotbase, DELAY_BEFORE_RESET);

        reset_game_from_home(*pabotbase, TOLERATE_SYSTEM_UPDATE_MENU_SLOW);
    }

    end_program_callback(*pabotbase);
    end_program_loop(*pabotbase);
}



}
}
