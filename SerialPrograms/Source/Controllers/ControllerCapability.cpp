/*  Controller Capabilities
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "ControllerCapability.h"

namespace PokemonAutomation{


const char* to_string(ControllerType type){
    switch (type){
    case ControllerType::None:
        return "None";
    case ControllerType::NintendoSwitch_WiredProController:
        return "NintendoSwitch_WiredProController";
    case ControllerType::NintendoSwitch_WirelessProController:
        return "NintendoSwitch_WirelessProController";
    case ControllerType::NintendoSwitch_LeftJoycon:
        return "NintendoSwitch_LeftJoycon";
    case ControllerType::NintendoSwitch_RightJoycon:
        return "NintendoSwitch_RightJoycon";
    }
    return nullptr;
}



const char* to_string(ControllerFeature feature){
    switch (feature){
    case ControllerFeature::TickPrecise:
        return "TickPrecise";
    case ControllerFeature::QueryTickSize:
        return "QueryTickSize";
    case ControllerFeature::QueryCommandQueueSize:
        return "QueryCommandQueueSize";
    case ControllerFeature::NintendoSwitch_ProController:
        return "NintendoSwitch_ProController";
    case ControllerFeature::NintendoSwitch_DateSkip:
        return "NintendoSwitch_DateSkip";
    }
    return nullptr;
}






ControllerRequirements::ControllerRequirements(std::initializer_list<ControllerFeature> args)
    : m_features(std::move(args))
    , m_sanitizer("ControllerRequirements")
{}

std::string ControllerRequirements::check_compatibility(const std::set<ControllerFeature>& features) const{
    auto scope_check = m_sanitizer.check_scope();

    for (ControllerFeature feature : m_features){
        if (features.find(feature) == features.end()){
            return to_string(feature);
        }
    }
    return "";
}




}
