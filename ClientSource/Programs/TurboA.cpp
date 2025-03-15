/*  Turbo A
 * 
 *  From: https://github.com/PokemonAutomation/
 * 
 *  This program is identical to TurboA in the Arduino programs.
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


void program_TurboA(const std::string& device_name){
    std::cout << "Starting PABotBase - TurboA..." << std::endl;
    std::cout << std::endl;
    std::unique_ptr<PABotBase> pabotbase = start_connection(true, device_name);
//    global_connection = pabotbase.get();

    std::cout << "Begin Message Logging..." << std::endl;
    MessageLogger logger;
    pabotbase->set_sniffer(&logger);


    //  Start Program
    start_program_flash(*pabotbase, CONNECT_CONTROLLER_DELAY);
    grip_menu_connect_go_home(*pabotbase);
    resume_game_no_interact(*pabotbase, TOLERATE_SYSTEM_UPDATE_MENU_FAST);

    while (true){
        pbf_press_button(*pabotbase, BUTTON_A, 5, 5);
    }
}


}
}
