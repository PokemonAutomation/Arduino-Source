/*  Virtual Controller
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_VirtualController_H
#define PokemonAutomation_NintendoSwitch_VirtualController_H

#include <set>
#include <map>
#include <thread>
#include <Qt>
#include "Common/Cpp/SpinLock.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/Tools/BotBaseHandle.h"
#include "CommonFramework/Logging/Logger.h"
#include "NintendoSwitch_VirtualControllerMapping.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


class VirtualController{
public:
    VirtualController(
        Logger& logger,
        BotBaseHandle& botbase,
        bool allow_commands_while_running
    );
    ~VirtualController();

    void clear_state();

    void on_key_press(Qt::Key key);
    void on_key_release(Qt::Key key);

    ProgramState last_known_state() const;
    void on_state_changed(ProgramState state);

private:
    void thread_loop();

private:
    Logger& m_logger;
    BotBaseHandle& m_botbase;
    bool m_allow_commands_while_running;

    //  Keyboard State
    std::set<Qt::Key> m_pressed_buttons;

    //  Controller State
    VirtualControllerState m_controller_state;
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
