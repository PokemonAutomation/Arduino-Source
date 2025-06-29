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

    //  There's a difference between the generic 3rd party wired controllers and
    //  a pro controller connected over USB. If/when we support the latter, we
    //  will need to split this controller type.
    NintendoSwitch_WiredProController,

    NintendoSwitch_WirelessProController,
    NintendoSwitch_LeftJoycon,
    NintendoSwitch_RightJoycon,
};


enum class ControllerFeature{
    TickPrecise,
    TimingFlexibleMilliseconds,

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
    SerialPABotBase_Wired_125Hz,
    SerialPABotBase_Wireless_ESP32,
    SysbotBase,
};




}
#endif
