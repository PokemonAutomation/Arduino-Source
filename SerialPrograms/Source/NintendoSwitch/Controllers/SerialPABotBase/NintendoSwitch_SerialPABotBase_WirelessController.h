/*  SerialPABotBase: Wireless Controller
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_SerialPABotBase_WirelessController_H
#define PokemonAutomation_NintendoSwitch_SerialPABotBase_WirelessController_H

#include "Common/NintendoSwitch/NintendoSwitch_Protocol_ESP32.h"
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
    Milliseconds timing_variation() const{
        return Milliseconds::zero();
    }


public:
    class MessageControllerStatus;
    class MessageControllerState;


protected:
    void encode_joystick(uint8_t data[3], uint8_t x, uint8_t y){
        const uint16_t min = 1897;
        const uint16_t max = 320;

        const double SHIFT = 2048 - min;
        const double RATIO = (min - max) / 127.;

        double dx = x - 128.;
        double dy = y - 128.;

        dx *= RATIO;
        dy *= RATIO;

        if (dx != 0){
            dx += dx >= 0 ? SHIFT : -SHIFT;
        }
        if (dy != 0){
            dy += dy >= 0 ? SHIFT : -SHIFT;
        }

        uint16_t wx = (uint16_t)(2048 + dx + 0.5);
        uint16_t wy = (uint16_t)(2048 - dy + 0.5);

//        wx = 320;
//        cout << "wx = " << wx << endl;

        data[0] = (uint8_t)wx;
        data[1] = (uint8_t)(wx >> 8 | wy << 4);
        data[2] = (uint8_t)(wy >> 4);
    }

    void issue_report(
        const Cancellable* cancellable,
        const ESP32Report0x30& report,
        WallDuration duration
    );

private:
    template <typename Type>
    PA_FORCE_INLINE Type milliseconds_to_ticks_15ms(Type milliseconds){
        return milliseconds / 15 + (milliseconds % 15 + 14) / 15;
    }

    void status_thread();


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
