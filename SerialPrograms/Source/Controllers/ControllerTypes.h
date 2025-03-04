/*  Controller Types
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
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
    QueryTickSize,
    QueryCommandQueueSize,
    NintendoSwitch_ProController,
    NintendoSwitch_DateSkip,
};





}
#endif
