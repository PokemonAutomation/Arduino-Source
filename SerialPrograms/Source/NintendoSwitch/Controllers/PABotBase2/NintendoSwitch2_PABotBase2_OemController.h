/*  PABotBase2: OEM Controller (Nintendo Switch 2)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_NintendoSwitch2_PABotBase2_OemController_H
#define PokemonAutomation_NintendoSwitch2_PABotBase2_OemController_H

#include "Controllers/ControllerStatusThread.h"
#include "Controllers/JoystickTools.h"
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
    //  Shared Utils

    //  The S2 native reports pack the sticks 12-bit into 3 bytes
    //  (X = b0 | ((b1 & 0x0F) << 8); Y = (b1 >> 4) | (b2 << 4)),
    //  which is the same packing as the S1 0x30 report, so the
    //  encoder is the same. 2048 is the neutral position.
    template <uint16_t min_threshold, uint16_t max_threshold>
    static void encode_joystick(uint8_t data[3], const JoystickPosition& position){
        double fx = position.x;
        double fy = position.y;
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

        data[0] = (uint8_t)wx;
        data[1] = (uint8_t)(wx >> 8 | wy << 4);
        data[2] = (uint8_t)(wy >> 4);
    }


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
