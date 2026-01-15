/*  Serial Connection
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <mutex>
#include <iostream>
#include "Common/Cpp/Time.h"
#include "SerialConnection.h"

namespace PokemonAutomation{


std::mutex serial_logging_lock;


void serial_debug_log(const std::string& msg){
    std::lock_guard<std::mutex> lg(serial_logging_lock);
    std::cout << current_time_to_str() << " - " << msg << std::endl;
}


}
