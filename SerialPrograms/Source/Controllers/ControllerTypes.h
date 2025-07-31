/*  Controller Types
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Controllers_ControllerTypes_H
#define PokemonAutomation_Controllers_ControllerTypes_H

namespace PokemonAutomation{


//  This is used only for the UI to indicate if a program may be faster on
//  tick-precise controllers. There is no functional effect.
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

    //  This is for all the generic wired controllers.
    //  These do not support gyro or rumble.
    NintendoSwitch_WiredController,

    //  The official Pro Controller, connected over USB.
//    NintendoSwitch_WiredProController,

    NintendoSwitch_WirelessProController,
    NintendoSwitch_LeftJoycon,
    NintendoSwitch_RightJoycon,
};


enum class ControllerFeature{
    TickPrecise,

    //  If we add support gyro or rumble, we will need to split this feature
    //  since the Pokken controller doesn't support those.
    NintendoSwitch_ProController,

    NintendoSwitch_LeftJoycon,
    NintendoSwitch_RightJoycon,

    //  This is leftover from the days of RPCs. But we keep it here as a hack to
    //  lock ESP32 out of the day skippers since it's too unstable to run those.
    NintendoSwitch_DateSkip,
};


enum class ControllerPerformanceClass{
    Unknown,
    SerialPABotBase_Wired,
    SerialPABotBase_Wireless,
    SysbotBase,
};




}
#endif
