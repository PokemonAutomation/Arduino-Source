/*  Utilities
 * 
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 * 
 */

#include <iostream>
#include "Common/Cpp/Exception.h"
#include "Common/Cpp/PrettyPrint.h"
#include "Common/SwitchFramework/Switch_PushButtons.h"
#include "Common/PokemonSwSh/PokemonProgramIDs.h"
#include "ClientSource/Connection/SerialConnection.h"
#include "ClientSource/Libraries/MessageConverter.h"
//#include "Libraries/Logging.h"
#include "Utilities.h"

namespace PokemonAutomation{


std::unique_ptr<PABotBase> start_connection(
    bool require_pabotbase,
    const std::string& device_name,
    MessageLogger* logger
){
    //  First open a serial connection to the device.
    std::unique_ptr<PABotBase> pabotbase;
    if (device_name.empty()){
        std::string name;
        std::cout << "Please enter the name of the serial connection." << std::endl;
        std::cout << std::endl;
#if _WIN32
        std::cout << "If you don't know what it is, go to Device Manager -> Ports (COM & LPT)" << std::endl;
        std::cout << "The name should be something like, \"COM5\"." << std::endl;
#else
        std::cout << "If you don't know what it is, type \"ls /sys/class/tty\" to list them." << std::endl;
        std::cout << "The name should be something like, \"/dev/ttyUSB0\"." << std::endl;
#endif
        std::cout << std::endl;
        std::cout << "Device Name: ";
        std::cin >> name;
        std::cout << std::endl;
        std::cout << std::endl;
        pabotbase = std::make_unique<PABotBase>(
            std::make_unique<SerialConnection>(name, PABB_BAUD_RATE), logger
        );
    }else{
        pabotbase = std::make_unique<PABotBase>(
            std::make_unique<SerialConnection>(device_name, PABB_BAUD_RATE), logger
        );
    }

    std::cout << "Connecting to device..." << std::endl;
    pabotbase->connect();


    //  Make sure the device is using the same protocol version as us.
    std::cout << "Verifying protocol compatibility..." << std::endl;
    uint32_t version = pabotbase->protocol_version();
    uint32_t version_hi = version / 100;
    uint32_t version_lo = version % 100;
    if (version_hi != PABB_PROTOCOL_VERSION / 100 || version_lo < PABB_PROTOCOL_VERSION % 100){
        PA_THROW_StringException(
            "Incompatible version. Client: " + std::to_string(PABB_PROTOCOL_VERSION) + ", Device: " + std::to_string(version)
        );
    }
    std::cout << std::endl;


    //  Fetch some random information about what's running on the device.
    uint8_t program_id = pabotbase->program_id();
    std::cout << "Program ID:       " << (unsigned)program_id << " (" << program_name(program_id) << ")" << std::endl;

    uint32_t program_version = pabotbase->program_version();
    std::cout << "Program Version:  " << program_version << std::endl;

    uint32_t wallclock = system_clock(*pabotbase);
    std::cout << "Device Up Time:   " << ticks_to_time(wallclock) << std::endl;
    std::cout << std::endl;


    //  If we're running an actual program, the device needs to be running PABotBase to work.
    if (require_pabotbase && (int)program_id_to_botbase_level(program_id) >= (int)PABB_PID_PABOTBASE_12KB){
        PA_THROW_StringException("The device must be running PABotBase for this program to work.");
    }

//    std::cout << "Begin Message Logging..." << std::endl;
//    pabotbase->set_sniffer(logger);

    return pabotbase;
}


}
