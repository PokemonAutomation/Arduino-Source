/*  Virtual Controller
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_VirtualController_H
#define PokemonAutomation_VirtualController_H

#include <set>
#include <thread>
#include <Qt>
#include "Common/SwitchFramework/SwitchControllerDefs.h"
#include "Common/Clientside/SpinLock.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/Tools/BotBaseHandle.h"
#include "CommonFramework/Tools/Logger.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


class VirtualController{
public:
    VirtualController(BotBaseHandle& botbase, Logger& logger);
    ~VirtualController();

    void clear_state();

    void on_key_press(Qt::Key key);
    void on_key_release(Qt::Key key);

    ProgramState last_known_state() const;
    void on_state_changed(ProgramState state);

private:
    void print();
    void thread_loop();

private:
    BotBaseHandle& m_botbase;
    Logger& m_logger;

    //  Keyboard State
    std::set<Qt::Key> m_pressed_buttons;

    //  Controller State
    Button m_buttons;
    int m_dpad_x;
    int m_dpad_y;
    int m_left_joystick_x;
    int m_left_joystick_y;
    int m_right_joystick_x;
    int m_right_joystick_y;
    std::chrono::time_point<std::chrono::system_clock> m_last;

    std::atomic<ProgramState> m_last_known_state;
    std::atomic<bool> m_stop;
    uint8_t m_granularity;

    SpinLock m_state_lock;
    std::mutex m_sleep_lock;
    std::condition_variable m_cv;
    std::thread m_thread;
};




}
}
#endif
