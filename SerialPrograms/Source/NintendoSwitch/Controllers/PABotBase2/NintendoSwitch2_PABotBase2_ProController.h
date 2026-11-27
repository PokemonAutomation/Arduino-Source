/*  PABotBase2: Pro Controller (Nintendo Switch 2)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_NintendoSwitch2_PABotBase2_ProController_H
#define PokemonAutomation_NintendoSwitch2_PABotBase2_ProController_H

#include <algorithm>
#include "Common/PABotBase2/Controllers/PABotBase2_Controller_NS2_OemController.h"
#include "NintendoSwitch/Controllers/Procon/NintendoSwitch_ProController.h"
#include "NintendoSwitch2_PABotBase2_OemController.h"

namespace PokemonAutomation{
namespace NintendoSwitch2{




class PABotBase2_ProController final :
    public NintendoSwitch::ProController,
    public PABotBase2_OemController
{
    using PABotBase2_Controller = NintendoSwitch::PABotBase2_Controller;

    //  TODO: Exact values TBD
    static constexpr uint16_t JOYSTICK_MIN_THRESHOLD = 1874;
    static constexpr uint16_t JOYSTICK_MAX_THRESHOLD = 320;


public:
    PABotBase2_ProController(
        Logger& logger,
        PABotBase2::Connection& connection,
        ControllerType controller_type
    )
        : ProController(logger)
        , PABotBase2_OemController(
            logger, logging_throttler(),
            connection, controller_type,
            [this](double magnitude){ on_rumble(magnitude); }
        )
    {}
    ~PABotBase2_ProController(){
        PABotBase2_OemController::stop();
    }

    virtual Logger& logger() override{
        return m_logger;
    }
    virtual bool is_ready() const override{
        return PABotBase2_Controller::is_ready();
    }
    virtual ControllerPlayerNumber get_player_number(Cancellable& cancellable) override{
        return PABotBase2_OemController::get_player_number(cancellable);
    }


public:
    virtual ControllerPerformanceClass performance_class() const override{
        return m_performance_class;
    }
    virtual Milliseconds ticksize() const override{
        return m_ticksize;
    }
    virtual Milliseconds cooldown() const override{
        return m_cooldown;
    }
    virtual Milliseconds timing_variation() const override{
        return m_timing_variation;
    }
    virtual bool atomic_multibutton() const override{
        return true;
    }


public:
    virtual void cancel_all_commands() override{
        return PABotBase2_Controller::cancel_all_commands();
    }
    virtual void replace_on_next_command() override{
        PABotBase2_Controller::replace_on_next_command();
    }
    virtual bool cancel_all_and_wait(Milliseconds timeout) override{
        return PABotBase2_Controller::cancel_all_and_wait(timeout);
    }

    virtual void wait_for_all(Cancellable* cancellable) override{
        PABotBase2_Controller::wait_for_all(cancellable);
    }


public:
    //  Superscalar Commands (the "ssf" framework)

    virtual void issue_barrier(Cancellable* cancellable) override{
        ControllerWithScheduler::issue_barrier(cancellable);
    }
    virtual void issue_nop(Cancellable* cancellable, Milliseconds duration) override{
        ControllerWithScheduler::issue_nop(cancellable, duration);
    }

    virtual void issue_buttons(
        Cancellable* cancellable,
        Milliseconds delay, Milliseconds hold, Milliseconds cooldown,
        NintendoSwitch::Button button
    ) override{
        ControllerWithScheduler::issue_buttons(cancellable, delay, hold, cooldown, button);
    }
    virtual void issue_dpad(
        Cancellable* cancellable,
        Milliseconds delay, Milliseconds hold, Milliseconds cooldown,
        NintendoSwitch::DpadPosition position
    ) override{
        ControllerWithScheduler::issue_dpad(cancellable, delay, hold, cooldown, position);
    }
    virtual void issue_left_joystick(
        Cancellable* cancellable,
        Milliseconds delay, Milliseconds hold, Milliseconds cooldown,
        const JoystickPosition& position
    ) override{
        ControllerWithScheduler::issue_left_joystick(cancellable, delay, hold, cooldown, position);
    }
    virtual void issue_right_joystick(
        Cancellable* cancellable,
        Milliseconds delay, Milliseconds hold, Milliseconds cooldown,
        const JoystickPosition& position
    ) override{
        ControllerWithScheduler::issue_right_joystick(cancellable, delay, hold, cooldown, position);
    }

    virtual void issue_full_controller_state(
        Cancellable* cancellable,
        bool enable_logging,
        Milliseconds duration,
        NintendoSwitch::Button button,
        NintendoSwitch::DpadPosition dpad,
        const JoystickPosition& left_joystick,
        const JoystickPosition& right_joystick
    ) override{
        ControllerWithScheduler::issue_full_controller_state(
            cancellable,
            enable_logging,
            duration,
            button,
            dpad,
            left_joystick,
            right_joystick
        );
    }


public:
    //  High speed RPCs.

    virtual void issue_mash_button(
        Cancellable* cancellable,
        Milliseconds duration,
        NintendoSwitch::Button button,
        Milliseconds delay, Milliseconds hold, Milliseconds cooldown
    ) override{
        ControllerWithScheduler::issue_mash_button(cancellable, duration, button, delay, hold, cooldown);
    }
    virtual void issue_mash_button(
        Cancellable* cancellable,
        Milliseconds duration,
        NintendoSwitch::Button button0, NintendoSwitch::Button button1
    ) override{
        ControllerWithScheduler::issue_mash_button(cancellable, duration, button0, button1);
    }
    virtual void issue_mash_AZs(
        Cancellable* cancellable,
        Milliseconds duration
    ) override{
        ControllerWithScheduler::issue_mash_AZs(cancellable, duration);
    }
    virtual void issue_system_scroll(
        Cancellable* cancellable,
        Milliseconds delay, Milliseconds hold, Milliseconds cooldown,
        NintendoSwitch::DpadPosition direction  //  Diagonals not allowed.
    ) override{
        ControllerWithScheduler::issue_system_scroll(cancellable, delay, hold, cooldown, direction);
    }


private:
    virtual void execute_state(
        Cancellable* cancellable,
        const SuperscalarScheduler::ScheduleEntry& entry
    ) override{
        using namespace NintendoSwitch;

        SwitchControllerState controller_state;
        for (auto& item : entry.state){
            static_cast<const SwitchCommand&>(*item).apply(controller_state);
        }

        //  The Pro Controller 2's native report (0x09): the 3 button
        //  bytes plus the two 12-bit sticks. The motion payload is not
        //  populated yet; the firmware zero-fills the rest of the
        //  state (motion_length = 0).
        OemController_State0x09_Buttons buttons{};

        populate_report_buttons(buttons, controller_state);

        {
            SplitDpad dpad =
                convert_unified_to_split_dpad(controller_state.dpad);
            buttons.buttons[1] |= (dpad.down  ? 1 : 0) << 0;
            buttons.buttons[1] |= (dpad.right ? 1 : 0) << 1;
            buttons.buttons[1] |= (dpad.left  ? 1 : 0) << 2;
            buttons.buttons[1] |= (dpad.up    ? 1 : 0) << 3;
        }

        //  Left Stick
        encode_joystick<JOYSTICK_MIN_THRESHOLD, JOYSTICK_MAX_THRESHOLD>(
            buttons.left_stick,
            controller_state.left_joystick
        );

        //  Right Stick
        encode_joystick<JOYSTICK_MIN_THRESHOLD, JOYSTICK_MAX_THRESHOLD>(
            buttons.right_stick,
            controller_state.right_joystick
        );

        issue_report(cancellable, entry.duration, buttons);
    }

    //  Report 0x09's button bytes, MSB -> LSB per byte:
    //
    //      byte0: Rstickclick Plus ZR R | X Y A B
    //      byte1: Lstickclick Minus ZL L | Up Left Right Down
    //      byte2: - - - C GL GR Capture Home
    //
    //  (The Pro Controller 2 has no SL/SR.)
    static NintendoSwitch::Button populate_report_buttons(
        OemController_State0x09_Buttons& buttons,
        const NintendoSwitch::SwitchControllerState& controller_state
    ){
        using namespace NintendoSwitch;

        Button all_buttons = BUTTON_NONE;
        for (size_t c = 0; c < TOTAL_BUTTONS; c++){
            Button button =
                (Button)((ButtonFlagType)1 << c);
            if (!(controller_state.buttons & button)){
                continue;
            }

            all_buttons |= button;
            switch (button){
            //  Right: byte0
            case BUTTON_B:          buttons.buttons[0] |= 1 << 0; break;
            case BUTTON_A:          buttons.buttons[0] |= 1 << 1; break;
            case BUTTON_Y:          buttons.buttons[0] |= 1 << 2; break;
            case BUTTON_X:          buttons.buttons[0] |= 1 << 3; break;
            case BUTTON_R:          buttons.buttons[0] |= 1 << 4; break;
            case BUTTON_ZR:         buttons.buttons[0] |= 1 << 5; break;
            case BUTTON_PLUS:       buttons.buttons[0] |= 1 << 6; break;
            case BUTTON_RCLICK:     buttons.buttons[0] |= 1 << 7; break;

            //  Left + dpad: byte1
            case BUTTON_DOWN:       buttons.buttons[1] |= 1 << 0; break;
            case BUTTON_RIGHT:      buttons.buttons[1] |= 1 << 1; break;
            case BUTTON_LEFT:       buttons.buttons[1] |= 1 << 2; break;
            case BUTTON_UP:         buttons.buttons[1] |= 1 << 3; break;
            case BUTTON_L:          buttons.buttons[1] |= 1 << 4; break;
            case BUTTON_ZL:         buttons.buttons[1] |= 1 << 5; break;
            case BUTTON_MINUS:      buttons.buttons[1] |= 1 << 6; break;
            case BUTTON_LCLICK:     buttons.buttons[1] |= 1 << 7; break;

            //  Shared: byte2
            case BUTTON_HOME:       buttons.buttons[2] |= 1 << 0; break;
            case BUTTON_CAPTURE:    buttons.buttons[2] |= 1 << 1; break;
            case BUTTON_GR:         buttons.buttons[2] |= 1 << 2; break;
            case BUTTON_GL:         buttons.buttons[2] |= 1 << 3; break;
            case BUTTON_C:          buttons.buttons[2] |= 1 << 4; break;

            default:;
            }
        }
        return all_buttons;
    }

    void issue_report(
        Cancellable* cancellable,
        WallDuration duration,
        const OemController_State0x09_Buttons& buttons
    ){
        //  No throttling or timing adjustments here - the
        //  microcontroller does that. Split the state into 65535
        //  millisecond chunks.
        Milliseconds time_left = std::chrono::duration_cast<Milliseconds>(duration);

        PABotBase2::Message_Command_NS2_OemController_Buttons0x09 request;
        request.message_bytes = sizeof(request);
        request.opcode = PABB2_MESSAGE_CMD_NS2_OEM_CONTROLLER_BUTTONS_0X09;
        request.buttons = buttons;

        while (time_left > Milliseconds::zero()){
            Milliseconds current = std::min(time_left, Milliseconds(65535));
            request.milliseconds = (uint16_t)current.count();
            m_connection.device().command_queue().send_command(cancellable, request);
            time_left -= current;
        }
    }






};




}
}
#endif
