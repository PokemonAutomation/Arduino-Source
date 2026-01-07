/*  Pybind Switch Controller
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Integrations_PybindSwitchController_H
#define PokemonAutomation_Integrations_PybindSwitchController_H

#include <stdint.h>
#include <string>

namespace PokemonAutomation{
namespace NintendoSwitch{



class PybindSwitchProController{
    PybindSwitchProController(const PybindSwitchProController&) = delete;
    void operator=(const PybindSwitchProController&) = delete;

public:
    PybindSwitchProController(const std::string& port_name);
    ~PybindSwitchProController();

    bool is_ready() const;
    std::string current_status() const;

    void wait_for_all_requests();

    //  All times are in milliseconds.
    void wait(uint64_t duration);
    void push_button(uint64_t delay, uint64_t hold, uint64_t release, uint32_t bitfield);
    void push_dpad(uint64_t delay, uint64_t hold, uint64_t release, uint8_t position);
    void push_left_joystick(uint64_t delay, uint64_t hold, uint64_t release, double x, double y);
    void push_right_joystick(uint64_t delay, uint64_t hold, uint64_t release, double x, double y);

    void controller_state(
        uint64_t duration,
        uint32_t button_bitfield,
        uint8_t dpad_position,
        double left_x, double left_y,
        double right_x, double right_y
    );

private:
    void* m_internals;
};



}
}
#endif
