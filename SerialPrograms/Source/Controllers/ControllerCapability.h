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

#include <string>
#include <set>
#include <map>
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




//
//  This class is a double map.
//  The first level is keyed on a string representing the interface type. (serial, bluetooth, etc...)
//  The second level is a set of features for that interface type.
//
//  Programs will specify which interfaces they support and which features are
//  required for each interface.
//
//  The list of interfaces will filter which connections are shown in the UI
//  dropdown. The feature set will be enforced after connecting since you won't
//  know the feature set until after talking to the controller.
//
class ControllerRequirements{
public:
    ControllerRequirements(std::initializer_list<std::map<std::string, std::set<ControllerFeature>>::value_type> args)
        : m_map(std::move(args))
        , m_sanitizer("ControllerRequirements")
    {}

    const std::map<std::string, std::set<ControllerFeature>>& map() const{
        auto scope_check = m_sanitizer.check_scope();
        return m_map;
    }

    bool contains_device(const std::string& device) const{
        auto scope_check = m_sanitizer.check_scope();
        return m_map.find(device) != m_map.end();
    }

    //  Check compatibility. If compatible, returns empty string.
    //  Otherwise returns one of the missing features.
    std::string check_compatibility(const std::string& device, const std::set<ControllerFeature>& features) const{
        auto scope_check = m_sanitizer.check_scope();

        auto iter0 = m_map.find(device);
        if (iter0 == m_map.end()){
            return device;
        }

        const std::set<ControllerFeature>& required = iter0->second;
        for (ControllerFeature feature : required){
            if (features.find(feature) == features.end()){
                return to_string(feature);
            }
        }
        return "";
    }


private:
    std::map<std::string, std::set<ControllerFeature>> m_map;

    LifetimeSanitizer m_sanitizer;
};




}
#endif
