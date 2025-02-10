/*  Controller Capabilities
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "ControllerCapability.h"

namespace PokemonAutomation{



const EnumStringMap<ControllerInterface> CONTROLLER_INTERFACE_STRINGS{
    {ControllerInterface::None,                 "None"},
    {ControllerInterface::SerialPABotBase,      "SerialPABotBase"},
};

const EnumStringMap<ControllerType> CONTROLLER_TYPE_STRINGS{
    {ControllerType::None,                                  "None"},
    {ControllerType::NintendoSwitch_WiredProController,     "Switch: Wired Pro Controller"},
    {ControllerType::NintendoSwitch_WirelessProController,  "Switch: Wireless Pro Controller"},
    {ControllerType::NintendoSwitch_LeftJoycon,             "Switch: Left Joycon"},
    {ControllerType::NintendoSwitch_RightJoycon,            "Switch: Right Joycon"},
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
