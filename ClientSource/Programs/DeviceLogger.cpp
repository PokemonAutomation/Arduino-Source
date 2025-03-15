/*  Device Logger
 * 
 *  From: https://github.com/PokemonAutomation/
 * 
 *      Run this program and it will automatically begin logging status updates
 *  from whatever program is running on the device.
 * 
 */

#include <iostream>
#include "Common/NintendoSwitch/NintendoSwitch_PushButtons.h"
#include "ClientSource/Libraries/Utilities.h"

namespace PokemonAutomation{


void device_logger(const std::string& device_name){
    std::cout << "Starting Device Logger..." << std::endl;
    std::cout << std::endl;
    std::unique_ptr<PABotBase> pabotbase = start_connection(false, device_name);
//    global_connection = pabotbase.get();

    std::cout << "Begin Message Logging..." << std::endl;
    MessageLogger logger;
    pabotbase->set_sniffer(&logger);


    //  Wait Forever
    while (true){
        std::string x;
        std::cin >> x;
    }
}


}
