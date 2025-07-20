/*  Controller Type Strings
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "ControllerTypeStrings.h"

namespace PokemonAutomation{



const EnumStringMap<ControllerInterface> CONTROLLER_INTERFACE_STRINGS{
    {ControllerInterface::None,                 "None"},
    {ControllerInterface::SerialPABotBase,      "Serial: PABotBase"},
    {ControllerInterface::TcpSysbotBase,        "TCP: sys-botbase"},
    {ControllerInterface::UsbSysbotBase,        "USB: sys-botbase"},
};

const EnumStringMap<ControllerType> CONTROLLER_TYPE_STRINGS{
    {ControllerType::None,                                  "None"},
    {ControllerType::NintendoSwitch_WiredProController,     "NS: Wired Pro Controller"},
    {ControllerType::NintendoSwitch_WirelessProController,  "NS: Wireless Pro Controller"},
    {ControllerType::NintendoSwitch_LeftJoycon,             "NS: Left Joycon"},
    {ControllerType::NintendoSwitch_RightJoycon,            "NS: Right Joycon"},
};

const EnumStringMap<ControllerFeature> CONTROLLER_FEATURE_STRINGS{
    {ControllerFeature::TickPrecise,                        "TickPrecise"},
    {ControllerFeature::NintendoSwitch_ProController,       "NintendoSwitch_ProController"},
    {ControllerFeature::NintendoSwitch_LeftJoycon,          "NintendoSwitch_LeftJoycon"},
    {ControllerFeature::NintendoSwitch_RightJoycon,         "NintendoSwitch_RightJoycon"},
    {ControllerFeature::NintendoSwitch_DateSkip,            "NintendoSwitch_DateSkip"},
};




}
