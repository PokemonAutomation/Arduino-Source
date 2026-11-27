/*  PABotBase2: OEM Controller (Nintendo Switch 2)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_NintendoSwitch2_PABotBase2_OemController_H
#define PokemonAutomation_NintendoSwitch2_PABotBase2_OemController_H

//#include "Common/ControllerStates/NintendoSwitch2_OemController_State.h"
#include "Controllers/ControllerStatusThread.h"
//#include "Controllers/JoystickTools.h"
//#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/Controllers/Procon/NintendoSwitch_ProController.h"
#include "NintendoSwitch_PABotBase2_Controller.h"

namespace PokemonAutomation{
namespace NintendoSwitch2{



class PABotBase2_OemController :
    public NintendoSwitch::PABotBase2_Controller,
    private ControllerStatusThreadCallback
{
public:
    using ContextType = NintendoSwitch::ProControllerContext;
    using ControllerPlayerNumber = NintendoSwitch::ControllerPlayerNumber;

    static void add_message_loggers(PABotBase2::MessageLogger& message_logger);


public:
    PABotBase2_OemController(
        Logger& logger,
        RecursiveThrottler& logging_throttler,
        PABotBase2::Connection& connection,
        ControllerType controller_type,
        std::function<void(double magnitude)> on_rumble
    );
    ~PABotBase2_OemController();
    void stop();

    static bool run_preconnect_configure(
        Logger& logger,
        PABotBase2::Connection& connection,
        ControllerType controller_type
    );

    ControllerPlayerNumber get_player_number(Cancellable& cancellable);


protected:
//    static Button populate_report_buttons(
//        OemController_State0x30_Buttons& buttons,
//        const SwitchControllerState& controller_state
//    );

//    void issue_report(
//        Cancellable* cancellable,
//        WallDuration duration,
//        const OemController_State0x30_Buttons& buttons
//    );


private:
    virtual void update_status(Cancellable& cancellable) override;
    virtual void stop_with_error(std::string message) override;


protected:
    //  Do not change after construction.
    const ControllerType m_controller_type;
    ControllerPerformanceClass m_performance_class;
    Milliseconds m_ticksize;
    Milliseconds m_cooldown;
    Milliseconds m_timing_variation;

    std::function<void(double magnitude)> m_on_rumble;

    std::unique_ptr<ControllerStatusThread> m_status_thread;

//    std::string m_color_html;
    std::atomic<ControllerPlayerNumber> m_player_number;
};






}
}
#endif
