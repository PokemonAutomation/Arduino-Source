/*  Controller Types
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Controllers_ControllerTypes_H
#define PokemonAutomation_Controllers_ControllerTypes_H

namespace PokemonAutomation{


enum class FasterIfTickPrecise{
    NOT_FASTER,
    FASTER,
    MUCH_FASTER,
};



enum class ControllerInterface{
    None,
    SerialPABotBase,
    TcpSysbotBase,
    UsbSysbotBase,
};


enum class ControllerType{
    None,
    NintendoSwitch_WiredProController,
    NintendoSwitch_WirelessProController,
    NintendoSwitch_LeftJoycon,
    NintendoSwitch_RightJoycon,
};


enum class ControllerFeature{
    TickPrecise,
    TimingFlexibleMilliseconds,
    QueryTickSize,
    QueryCommandQueueSize,
    NintendoSwitch_ProController,
    NintendoSwitch_LeftJoycon,
    NintendoSwitch_RightJoycon,
    NintendoSwitch_DateSkip,
};


enum class ControllerPerformanceClass{
    Unknown,
    SerialPABotBase_Wired_125Hz,
    SerialPABotBase_Wireless_ESP32,
    SysbotBase,
};




}
#endif
