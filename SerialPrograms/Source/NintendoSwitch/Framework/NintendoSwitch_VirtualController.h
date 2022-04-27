/*  Virtual Controller
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_VirtualController_H
#define PokemonAutomation_NintendoSwitch_VirtualController_H

#include <deque>
#include <set>
#include <map>
#include <thread>
#include <Qt>
#include "Common/Cpp/SpinLock.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/Tools/BotBaseHandle.h"
#include "CommonFramework/Logging/LoggerQt.h"
#include "NintendoSwitch_VirtualControllerMapping.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{


//  If you hold down a button, it doesn't actually stay down. You will get
//  release/press pairs that come with very little time between.
//  This class will debounce that.
class KeyboardDebouncer{
public:
    void add_event(bool press, VirtualControllerState state);

    //  Returns when the consumer thread should wakt up again.
    WallClock get_current_state(VirtualControllerState& state);


private:
    struct Entry{
        WallClock timestamp;
        bool press;
        VirtualControllerState state;
    };

    SpinLock m_lock;
    VirtualControllerState m_last;
    std::deque<Entry> m_history;
};




class VirtualController{
public:
    VirtualController(
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
    // LoggerQt& m_logger;
    BotBaseHandle& m_botbase;
    const bool m_allow_commands_while_running;

    //  Keyboard State
    std::set<Qt::Key> m_pressed_buttons;

    //  Controller State
    VirtualControllerState m_controller_state;
    KeyboardDebouncer m_history;

    std::atomic<ProgramState> m_last_known_state;
    std::atomic<bool> m_stop;

    std::mutex m_sleep_lock;
    std::condition_variable m_cv;
    std::thread m_thread;
};




}
}
#endif
