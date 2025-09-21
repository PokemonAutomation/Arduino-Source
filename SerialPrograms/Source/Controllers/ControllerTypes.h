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


enum class ControllerType{
    None,

    HID_Keyboard,

    NintendoSwitch_WiredController,     //  Generic 3rd party wired controller.
    NintendoSwitch_WiredProController,  //  The official Pro Controller, connected over USB.
    NintendoSwitch_WirelessProController,
    NintendoSwitch_LeftJoycon,
    NintendoSwitch_RightJoycon,
    NintendoSwitch2_WiredController,    //  Generic 3rd party wired controller.
    NintendoSwitch2_WiredProController, //  The official Pro Controller, connected over USB.
    NintendoSwitch2_WirelessProController,
    NintendoSwitch2_LeftJoycon,
    NintendoSwitch2_RightJoycon,
};


enum class ControllerPerformanceClass{
    Unknown,
    SerialPABotBase_Wired,
    SerialPABotBase_Wireless,
    SysbotBase,
};

enum class ControllerClass{
    NONE,
    LEFT_JOYCON,
    RIGHT_JOYCON,
    PRO_CONTROLLER,
    KEYBOARD,
};




}
#endif
