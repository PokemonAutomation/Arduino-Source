/*  Ball Thrower
 * 
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 * 
 *  This program is identical to BallThrower in the Arduino programs.
 *  Please follow the same start instructions.
 * 
 */

#include <iostream>
#include "Common/NintendoSwitch/FrameworkSettings.h"
#include "Common/NintendoSwitch/NintendoSwitch_PushButtons.h"
#include "Common/PokemonSwSh/PokemonSettings.h"
#include "Common/PokemonSwSh/PokemonSwShGameEntry.h"
#include "ClientSource/Libraries/Utilities.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


void program_BallThrower(const std::string& device_name){
    std::cout << "Starting PABotBase - Ballthrower..." << std::endl;
    std::cout << std::endl;
    std::unique_ptr<PABotBase> pabotbase = start_connection(true, device_name);
//    global_connection = pabotbase.get();

    std::cout << "Begin Message Logging..." << std::endl;
    MessageLogger logger;
    pabotbase->set_sniffer(&logger);



    //  Start Program
    start_program_flash(*pabotbase, CONNECT_CONTROLLER_DELAY);
    grip_menu_connect_go_home(*pabotbase);
    pbf_press_button(*pabotbase, BUTTON_HOME, 10, HOME_TO_GAME_DELAY);

    while (true){
        pbf_press_button(*pabotbase, BUTTON_X, 50, 50);
        pbf_press_button(*pabotbase, BUTTON_A, 50, 50);
        pbf_mash_button(*pabotbase, BUTTON_B, 100);
    }

//    pbf_press_button(*pabotbase, BUTTON_HOME, 10, GAME_TO_HOME_DELAY_SAFE);
//    end_program_callback(*pabotbase);
//    end_program_loop(*pabotbase);
}


}
}
