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
    {ControllerType::HID_Keyboard,                          "HID: Keyboard"},
    {ControllerType::NintendoSwitch_WiredController,        "NS1: Wired Controller"},
    {ControllerType::NintendoSwitch_WiredProController,     "NS1: Wired Pro Controller"},
    {ControllerType::NintendoSwitch_WirelessProController,  "NS1: Wireless Pro Controller"},
    {ControllerType::NintendoSwitch_LeftJoycon,             "NS1: Left Joycon"},
    {ControllerType::NintendoSwitch_RightJoycon,            "NS1: Right Joycon"},
    {ControllerType::NintendoSwitch2_WiredController,       "NS2: Wired Controller"},
    {ControllerType::NintendoSwitch2_WiredProController,    "NS2: Wired Pro Controller"},
    {ControllerType::NintendoSwitch2_WirelessProController, "NS2: Wireless Pro Controller"},
    {ControllerType::NintendoSwitch2_LeftJoycon,            "NS2: Left Joycon"},
    {ControllerType::NintendoSwitch2_RightJoycon,           "NS2: Right Joycon"},
};

const EnumStringMap<ControllerClass>& CONTROLLER_CLASS_STRINGS(){
    static EnumStringMap<ControllerClass> database{
        {ControllerClass::None,                             "None"},
        {ControllerClass::HID_Keyboard,                     "HID: Keyboard"},
        {ControllerClass::NintendoSwitch_ProController,     "NS1: Pro Controller"},
        {ControllerClass::NintendoSwitch_LeftJoycon,        "NS1: Left Joycon"},
        {ControllerClass::NintendoSwitch_RightJoycon,       "NS1: Right Joycon"},
    };
    return database;
}




}
