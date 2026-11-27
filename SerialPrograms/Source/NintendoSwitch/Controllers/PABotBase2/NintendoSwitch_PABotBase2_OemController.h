/*  PABotBase2: OEM Controller (Nintendo Switch 1)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_PABotBase2_OemController_H
#define PokemonAutomation_NintendoSwitch_PABotBase2_OemController_H

#include "Common/ControllerStates/NintendoSwitch_OemController_State.h"
#include "Controllers/ControllerStatusThread.h"
//#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/Controllers/Procon/NintendoSwitch_ProController.h"
#include "NintendoSwitch_PABotBase2_Controller.h"

namespace PokemonAutomation{
namespace NintendoSwitch{



class PABotBase2_OemController :
    public PABotBase2_Controller,
    private ControllerStatusThreadCallback
{
public:
    using ContextType = ProControllerContext;

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
    static Button populate_report_buttons(
        OemController_State0x30_Buttons& buttons,
        const SwitchControllerState& controller_state
    );
    static bool populate_report_gyro(
        OemController_State0x30_Gyro& gyro,
        const SwitchControllerState& controller_state
    );

    void issue_report(
        Cancellable* cancellable,
        WallDuration duration,
        const OemController_State0x30_Buttons& buttons
    );
    void issue_report(
        Cancellable* cancellable,
        WallDuration duration,
        const OemController_State0x30_Buttons& buttons,
        const OemController_State0x30_Gyro& gyro
    );


private:
    virtual void update_status(Cancellable& cancellable) override;
    virtual void stop_with_error(std::string message) override;

#if 0
    virtual void execute_state(
        Cancellable* cancellable,
        const SuperscalarScheduler::ScheduleEntry& entry
    ) override;
#endif


protected:
    //  Shared Utils

    struct RumbleData{
        double lo_freq;
        double lo_amp;
        double hi_freq;
        double hi_amp;
    };
    static double rumble_index_to_amp(uint8_t index);
    static double rumble_index_to_freq(double index);
    static RumbleData parse_rumble(const uint8_t data[4]);
    static std::string rumble_to_str(const RumbleData& data);


protected:
    //  Do not change after construction.
    const ControllerType m_controller_type;
    ControllerPerformanceClass m_performance_class;
    Milliseconds m_ticksize;
    Milliseconds m_cooldown;
    Milliseconds m_timing_variation;

    std::function<void(double magnitude)> m_on_rumble;

    std::unique_ptr<ControllerStatusThread> m_status_thread;

    std::string m_color_html;
    std::atomic<ControllerPlayerNumber> m_player_number;
};



}
}
#endif
