/*  Controller Types
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Controllers_ControllerTypes_H
#define PokemonAutomation_Controllers_ControllerTypes_H

namespace PokemonAutomation{



enum class ControllerInterface{
    None,
    SerialPABotBase,
    TcpSysbotBase,
    UsbSysbotBase,
};

enum class ControllerPerformanceClass{
    Unknown,
    SerialPABotBase_Wired,
    SerialPABotBase_Wireless,
    SysbotBase,
};


enum class ControllerClass{
    None,
    HID_Keyboard,
    NintendoSwitch_ProController,
    NintendoSwitch_LeftJoycon,
    NintendoSwitch_RightJoycon,
};


enum class ControllerType{
    None,

    HID_Keyboard,

    NintendoSwitch_WiredController,     //  Generic 3rd party wired controller.
    NintendoSwitch_WiredProController,
    NintendoSwitch_WiredLeftJoycon,
    NintendoSwitch_WiredRightJoycon,
    NintendoSwitch_WirelessProController,
    NintendoSwitch_WirelessLeftJoycon,
    NintendoSwitch_WirelessRightJoycon,

    NintendoSwitch2_WiredController,    //  Generic 3rd party wired controller.
    NintendoSwitch2_WiredProController,
    NintendoSwitch2_WiredLeftJoycon,
    NintendoSwitch2_WiredRightJoycon,
    NintendoSwitch2_WirelessProController,
    NintendoSwitch2_WirelessLeftJoycon,
    NintendoSwitch2_WirelessRightJoycon,
};




}
#endif
