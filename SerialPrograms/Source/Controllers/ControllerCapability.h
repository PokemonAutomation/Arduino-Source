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
#include "Common/Cpp/EnumStringMap.h"
#include "Common/Cpp/LifetimeSanitizer.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{







enum class ControllerInterface{
    SerialPABotBase,
};
extern EnumStringMap<ControllerInterface> CONTROLLER_INTERFACE_STRINGS;


enum class ControllerType{
    None,
    NintendoSwitch_WiredProController,
    NintendoSwitch_WirelessProController,
    NintendoSwitch_LeftJoycon,
    NintendoSwitch_RightJoycon,
};
extern EnumStringMap<ControllerType> CONTROLLER_TYPE_STRINGS;


enum class ControllerFeature{
    TickPrecise,
    QueryTickSize,
    QueryCommandQueueSize,
    NintendoSwitch_ProController,
    NintendoSwitch_DateSkip,
};
extern EnumStringMap<ControllerFeature> CONTROLLER_FEATURE_STRINGS;




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
