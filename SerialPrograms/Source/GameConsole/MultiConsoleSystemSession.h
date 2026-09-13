/*  Multi-Console System Session
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_GameConsole_MultiConsoleSystemSession_H
#define PokemonAutomation_GameConsole_MultiConsoleSystemSession_H

#include "Common/Cpp/ListenerSet.h"
#include "Common/Cpp/UiWrapper.h"
#include "Common/Cpp/Containers/FixedLimitVector.h"
#include "GameConsole/ConsoleSystemSession.h"
#include "MultiConsoleSystemOption.h"

namespace PokemonAutomation{
namespace GameConsole{


class MultiConsoleSystemSession : public UiState<MultiConsoleSystemSession>{
public:
    struct Listener{
        //  Called when the console count is locked or unlocked.
        virtual void on_console_count_lock(bool locked) = 0;

        //  Sent before the console sessions are destroyed. Listeners should
        //  drop their references to them before returning.
        virtual void shutdown() = 0;

        //  Sent after new consoles are started up.
        //  This is called immediately when attaching a listener to give the
        //  current console count. The listener must drop all references to the
        //  console sessions before detaching.
        virtual void startup(size_t console_count) = 0;
    };
    void add_listener(Listener& listener);
    void remove_listener(Listener& listener);


public:
    bool try_shutdown();
    virtual ~MultiConsoleSystemSession();

    MultiConsoleSystemSession(
        MultiConsoleSystemOption& option,
        bool allow_commands_while_locked,
        std::optional<uint64_t> program_tracking_id = {}
    );
    MultiConsoleSystemSession(
        MultiConsoleSystemOption& option,
        std::function<
            std::unique_ptr<ConsoleSystemSession>(
                ConsoleSystemOption& option,
                size_t console_index
            )
        > factory
    );


public:
    //  These are not thread-safe with "resize()".
    size_t min_consoles() const{ return m_option.min_consoles(); }
    size_t max_consoles() const{ return m_option.max_consoles(); }

    size_t active_consoles() const{ return m_consoles.size(); }
    ConsoleSystemSession& operator[](size_t index){ return *m_consoles[index]; }


public:
    void lock_controllers(const std::string& reason);
    void unlock_controllers();


public:
    //  Lock/unlock the console count.
    void add_console_count_lock(Listener& listener);
    void remove_console_count_lock(Listener& listener);

    void set_active_consoles(size_t count);


public:
    JsonValue to_json() const;
    void load_json(const JsonValue& json);


private:
    MultiConsoleSystemOption& m_option;

    Mutex m_resize_lock;
    FixedLimitVector<std::unique_ptr<ConsoleSystemSession>> m_consoles;

    std::function<
        std::unique_ptr<ConsoleSystemSession>(
            ConsoleSystemOption& option,
            size_t console_index
        )
    > m_factory;

    //  Listeners who are currently locking the console count.
    //  It is not safe to change the console count if this is not zero.
    mutable Mutex m_state_lock;
    ConditionVariable m_cv;
    std::set<Listener*> m_count_locks;

    ListenerSet<Listener> m_listeners;
};



}
}
#endif
