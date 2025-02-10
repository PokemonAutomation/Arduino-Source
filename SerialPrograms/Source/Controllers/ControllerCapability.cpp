/*  Controller Capabilities
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "ControllerCapability.h"

namespace PokemonAutomation{



const EnumStringMap<ControllerInterface> CONTROLLER_INTERFACE_STRINGS{
    {ControllerInterface::SerialPABotBase,      "SerialPABotBase"},
};

const EnumStringMap<ControllerType> CONTROLLER_TYPE_STRINGS{
    {ControllerType::None,                                  "None"},
    {ControllerType::NintendoSwitch_WiredProController,     "NintendoSwitch_WiredProController"},
    {ControllerType::NintendoSwitch_WirelessProController,  "NintendoSwitch_WirelessProController"},
    {ControllerType::NintendoSwitch_LeftJoycon,             "NintendoSwitch_LeftJoycon"},
    {ControllerType::NintendoSwitch_RightJoycon,            "NintendoSwitch_RightJoycon"},
};

const EnumStringMap<ControllerFeature> CONTROLLER_FEATURE_STRINGS{
    {ControllerFeature::TickPrecise,                        "TickPrecise"},
    {ControllerFeature::QueryTickSize,                      "QueryTickSize"},
    {ControllerFeature::QueryCommandQueueSize,              "QueryCommandQueueSize"},
    {ControllerFeature::NintendoSwitch_ProController,       "NintendoSwitch_ProController"},
    {ControllerFeature::NintendoSwitch_DateSkip,            "NintendoSwitch_DateSkip"},
};






ControllerRequirements::ControllerRequirements(std::initializer_list<ControllerFeature> args)
    : m_features(std::move(args))
    , m_sanitizer("ControllerRequirements")
{}

std::string ControllerRequirements::check_compatibility(const std::set<ControllerFeature>& features) const{
    auto scope_check = m_sanitizer.check_scope();

    for (ControllerFeature feature : m_features){
        if (features.find(feature) == features.end()){
            return CONTROLLER_FEATURE_STRINGS.get_string(feature);
        }
    }
    return "";
}




}
