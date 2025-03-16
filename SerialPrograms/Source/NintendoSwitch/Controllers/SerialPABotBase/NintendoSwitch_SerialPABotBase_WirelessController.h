/*  SerialPABotBase: Wireless Controller
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_SerialPABotBase_WirelessController_H
#define PokemonAutomation_NintendoSwitch_SerialPABotBase_WirelessController_H

#include "Common/SerialPABotBase/SerialPABotBase_Messages_ESP32.h"
#include "Controllers/JoystickTools.h"
#include "NintendoSwitch_SerialPABotBase_Controller.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{



class SerialPABotBase_WirelessController : public SerialPABotBase_Controller{
public:
    SerialPABotBase_WirelessController(
        Logger& logger,
        SerialPABotBase::SerialPABotBase_Connection& connection,
        ControllerType controller_type
    );
    ~SerialPABotBase_WirelessController();
    void stop();


public:
    Milliseconds ticksize() const{
        return Milliseconds(15);
    }
    Milliseconds cooldown() const{
        return Milliseconds(15);
    }
    Milliseconds timing_variation() const{
        return Milliseconds(0);
    }


protected:
    void encode_joystick(uint8_t data[3], uint8_t x, uint8_t y){
        //  2048 is the neutral position.
        //
        //  1897 is the point where the joystick calibrator will register it as
        //  off-center.
        //
        //  ~320 is where it reaches the maximum value.
        //
        //  It we linearly interpolate between 1897 and 320, we seem to match
        //  the wired controller's behavior.
        //
        //  I suspect the need to offset by 151 from 2048 -> 1897 is Nintendo's
        //  way to alleviate the joycon drift problem.
        const uint16_t min = 1897;
//        const uint16_t max = 320;
        const uint16_t max = 275;   //  REMOVE: TODO: Fix the clipping for real.

        const double lo = 1 - min / 2048.;
        const double hi = 1 - max / 2048.;

        double fx = JoystickTools::linear_u8_to_float(x);
        double fy = -JoystickTools::linear_u8_to_float(y);
//        cout << "fx = " << fx << ", fy = " << fy << endl;

        uint16_t wx = JoystickTools::linear_float_to_u12(lo, hi, fx);
        uint16_t wy = JoystickTools::linear_float_to_u12(lo, hi, fy);
//        cout << "wx = " << wx << ", wy = " << wy << endl;

        data[0] = (uint8_t)wx;
        data[1] = (uint8_t)(wx >> 8 | wy << 4);
        data[2] = (uint8_t)(wy >> 4);
    }

    void issue_report(
        const Cancellable* cancellable,
        const SerialPABotBase::NintendoSwitch_ButtonState& buttons,
        WallDuration duration
    );

private:
    template <typename Type>
    PA_FORCE_INLINE Type milliseconds_to_ticks_15ms(Type milliseconds){
        return milliseconds / 15 + (milliseconds % 15 + 14) / 15;
    }

    void status_thread();

protected:
    const ControllerType m_controller_type;
private:
    CancellableHolder<CancellableScope> m_scope;
    std::atomic<bool> m_stopping;
    std::mutex m_sleep_lock;
    std::condition_variable m_cv;
    std::thread m_status_thread;
};



}
}
#endif
