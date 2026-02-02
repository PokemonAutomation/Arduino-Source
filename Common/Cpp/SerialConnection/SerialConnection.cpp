/*  Serial Connection
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <iostream>
#include "Common/Cpp/Time.h"
#include "Common/Cpp/Concurrency/Mutex.h"
#include "SerialConnection.h"

namespace PokemonAutomation{


Mutex serial_logging_lock;


void serial_debug_log(const std::string& msg){
    std::lock_guard<Mutex> lg(serial_logging_lock);
    std::cout << current_time_to_str() << " - " << msg << std::endl;
}


}
