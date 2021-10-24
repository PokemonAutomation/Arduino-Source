/*  Clothing Buyer
 * 
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 * 
 *  This program is identical to ClothingBuyer in the Arduino programs.
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



//  Rotate categories. This will allow the program to eventually buy out the
//  entire store, but it will take a lot longer than doing separate runs on
//  each category individually.
const bool CATEGORY_ROTATION    =   true;



void program_ClothingBuyer(const std::string& device_name){
    std::cout << "Starting PABotBase - ClothingBuyer..." << std::endl;
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
        pbf_press_button(*pabotbase, BUTTON_A, 10, 90);
        if (CATEGORY_ROTATION){
            pbf_press_dpad(*pabotbase, DPAD_RIGHT, 10, 40);
        }
        pbf_press_button(*pabotbase, BUTTON_A, 10, 90);
        pbf_press_button(*pabotbase, BUTTON_A, 10, 90);
        pbf_press_dpad(*pabotbase, DPAD_DOWN, 10, 40);
    }
}



}
}
