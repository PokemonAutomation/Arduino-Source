/*  Virtual Controller
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_VirtualController_H
#define PokemonAutomation_NintendoSwitch_VirtualController_H

#include <thread>
#include <QKeyEvent>
#include "Common/Cpp/Concurrency/SpinLock.h"
#include "CommonFramework/Globals.h"
#include "Controllers/ControllerSession.h"
#include "Controllers/KeyboardStateTracker.h"

namespace PokemonAutomation{
namespace NintendoSwitch{





class VirtualController{
public:
    VirtualController(
        Logger& logger,
        ControllerSession& session,
        bool allow_commands_while_running
    );
    ~VirtualController();

public:
    //  These 3 must be called from UI thread.
    void clear_state();

    //  Returns false if key is not handled. (pass it up to next handler)
    bool on_key_press(const QKeyEvent& key);
    bool on_key_release(const QKeyEvent& key);

public:
    //  Call from any thread.
    ProgramState last_known_state() const;
    void on_state_changed(ProgramState state);

private:
    bool try_stop_commands();
    bool try_next_interrupt();

    void thread_loop();

private:
    Logger& m_logger;
    ControllerSession& m_session;
    const bool m_allow_commands_while_running;

    //  Controller State
    SpinLock m_state_lock;
    KeyboardStateTracker m_state_tracker;

    std::atomic<ProgramState> m_last_known_state;
    std::atomic<bool> m_stop;

    std::mutex m_sleep_lock;
    std::condition_variable m_cv;
    std::thread m_thread;
};




}
}
#endif
