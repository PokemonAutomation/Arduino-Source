/*  Controller Capabilities
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *      Represents optional features that each controller
 *  interface may have.
 *
 *  For example, PABotBase has 3 different levels:
 *      -   Not PABotBase (protocol only, no commands)
 *      -   PABotBase-12KB (basic commands only)
 *      -   PABotBase-31KB (basic commands + advanced RPC)
 *
 *  This class allows programs to declare required features so that the
 *  controller interface can error if it lacks them.
 *
 */

#ifndef PokemonAutomation_Controllers_ControllerCapabilities_H
#define PokemonAutomation_Controllers_ControllerCapabilities_H

#include <initializer_list>
#include <string>
#include <set>
#include "Common/Cpp/LifetimeSanitizer.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{



enum class ControllerInterface{
    SerialPABotBase,
};


enum class ControllerType{
    None,
    NintendoSwitch_WiredProController,
    NintendoSwitch_WirelessProController,
    NintendoSwitch_LeftJoycon,
    NintendoSwitch_RightJoycon,
};
const char* to_string(ControllerType type);


enum class ControllerFeature{
    TickPrecise,
    QueryTickSize,
    QueryCommandQueueSize,
    NintendoSwitch_ProController,
    NintendoSwitch_DateSkip,
};
const char* to_string(ControllerFeature feature);




class ControllerRequirements{
public:
    ControllerRequirements(std::initializer_list<ControllerFeature> args);

    //  Check compatibility. If compatible, returns empty string.
    //  Otherwise returns one of the missing features.
    std::string check_compatibility(const std::set<ControllerFeature>& features) const;


private:
    std::set<ControllerFeature> m_features;

    LifetimeSanitizer m_sanitizer;
};




}
#endif
