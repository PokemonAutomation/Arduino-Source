/*  SerialPABotBase: Wireless Controller
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_SerialPABotBase_WirelessController_H
#define PokemonAutomation_NintendoSwitch_SerialPABotBase_WirelessController_H

#include <cmath>
#include "Common/ControllerStates/NintendoSwitch_WirelessController_State.h"
#include "Controllers/SerialPABotBase/SerialPABotBase_StatusThread.h"
#include "Controllers/JoystickTools.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch_SerialPABotBase_Controller.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{



class SerialPABotBase_WirelessController :
    public SerialPABotBase_Controller,
    private SerialPABotBase::ControllerStatusThreadCallback
{
public:
    SerialPABotBase_WirelessController(
        Logger& logger,
        SerialPABotBase::SerialPABotBase_Connection& connection,
        ControllerType controller_type,
        ControllerResetMode reset_mode
    );
    ~SerialPABotBase_WirelessController();
    void stop();


public:
    Milliseconds ticksize() const{
        return Milliseconds(0);
    }
    Milliseconds cooldown() const{
        return Milliseconds(15);
    }
    Milliseconds timing_variation() const{
        return ConsoleSettings::instance().TIMING_OPTIONS.WIRELESS_ESP32;
    }


protected:
    void set_info();


protected:
    template <uint16_t min_threshold, uint16_t max_threshold>
    void encode_joystick(uint8_t data[3], uint8_t x, uint8_t y){
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

        double fx = JoystickTools::linear_u8_to_float(x);
        double fy = -JoystickTools::linear_u8_to_float(y);
//        cout << "fx = " << fx << ", fy = " << fy << endl;
        double mag_squared = fx*fx + fy*fy;

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

    static Button populate_report_buttons(
        pabb_NintendoSwitch_WirelessController_State0x30_Buttons& buttons,
        const SwitchControllerState& controller_state
    );
    static bool populate_report_gyro(
        pabb_NintendoSwitch_WirelessController_State0x30_Gyro& gyro,
        const SwitchControllerState& controller_state
    );

    void issue_report(
        const Cancellable* cancellable,
        WallDuration duration,
        const pabb_NintendoSwitch_WirelessController_State0x30_Buttons& buttons
    );
    void issue_report(
        const Cancellable* cancellable,
        WallDuration duration,
        const pabb_NintendoSwitch_WirelessController_State0x30_Buttons& buttons,
        const pabb_NintendoSwitch_WirelessController_State0x30_Gyro& gyro
    );


private:
    virtual void update_status(Cancellable& cancellable) override;
    virtual void stop_with_error(std::string message) override;


protected:
    const ControllerType m_controller_type;
private:
    std::unique_ptr<SerialPABotBase::ControllerStatusThread> m_status_thread;

    std::string m_color_html;
};



}
}
#endif
