/*  PABotBase2: OEM Controller (Nintendo Switch 1)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_PABotBase2_OemController_H
#define PokemonAutomation_NintendoSwitch_PABotBase2_OemController_H

#include "Common/ControllerStates/NintendoSwitch_OemController_State.h"
#include "Controllers/ControllerStatusThread.h"
#include "Controllers/JoystickTools.h"
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


public:
    PABotBase2_OemController(
        Logger& logger,
        PABotBase2::Connection& connection,
        ControllerType controller_type,
        std::function<void(double magnitude)> on_rumble
    );
    ~PABotBase2_OemController();
    void stop();

    static void run_preconnect_configure(
        Logger& logger,
        PABotBase2::Connection& connection,
        ControllerType controller_type
    );


protected:
    static Button populate_report_buttons(
        pabb_NintendoSwitch_OemController_State0x30_Buttons& buttons,
        const SwitchControllerState& controller_state
    );
    static bool populate_report_gyro(
        pabb_NintendoSwitch_OemController_State0x30_Gyro& gyro,
        const SwitchControllerState& controller_state
    );

    void issue_report(
        Cancellable* cancellable,
        WallDuration duration,
        const pabb_NintendoSwitch_OemController_State0x30_Buttons& buttons
    );
    void issue_report(
        Cancellable* cancellable,
        WallDuration duration,
        const pabb_NintendoSwitch_OemController_State0x30_Buttons& buttons,
        const pabb_NintendoSwitch_OemController_State0x30_Gyro& gyro
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

    template <uint16_t min_threshold, uint16_t max_threshold>
    static void encode_joystick(uint8_t data[3], const JoystickPosition& position){
        //  2048 is the neutral position.
        //
        //  1897 is the point where the joystick calibrator will register it as
        //  off-center.
        //
        //  ~320 is where it reaches the maximum value.
        //
        //  If we linearly interpolate between 1897 and 320, we seem to match
        //  the wired controller's behavior.
        //
        //  I suspect the need to offset by 151 from 2048 -> 1897 is Nintendo's
        //  way to alleviate the joycon drift problem.
        //
        //  The values 320 and 1897 are for the pro controller. Joycons are
        //  slightly different.
        //

        double fx = position.x;
        double fy = position.y;
//        cout << "fx = " << fx << ", fy = " << fy << endl;
        double mag_squared = fx*fx + fy*fy;

        fx = std::min(fx, +1.0);
        fx = std::max(fx, -1.0);
        fy = std::min(fy, +1.0);
        fy = std::max(fy, -1.0);

        uint16_t wx, wy;
        if (mag_squared == 0){
            wx = 2048;
            wy = 2048;
        }else if (mag_squared >= 1){
            JoystickTools::max_out_magnitude(fx, fy);
            wx = JoystickTools::linear_float_to_u12(fx);
            wy = JoystickTools::linear_float_to_u12(fy);
        }else{
            constexpr double lo = 1 - min_threshold / 2048.;
            constexpr double hi = 1 - max_threshold / 2048.;

            double true_mag = std::sqrt(mag_squared);
            double report_mag = JoystickTools::project_to_range(true_mag, lo, hi);
            double scale = report_mag / true_mag;
            wx = JoystickTools::linear_float_to_u12(fx * scale);
            wy = JoystickTools::linear_float_to_u12(fy * scale);
        }

//        cout << "wx = " << wx << ", wy = " << wy << endl;
//        wy = 2048;
//        wx = 1874;
//        wx = 320;

        data[0] = (uint8_t)wx;
        data[1] = (uint8_t)(wx >> 8 | wy << 4);
        data[2] = (uint8_t)(wy >> 4);
    }

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
};



}
}
#endif
