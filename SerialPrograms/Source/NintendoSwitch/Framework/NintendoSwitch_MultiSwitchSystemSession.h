/*  Multi-Switch System Session
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  This class holds the run-time state for multiple Switch systems.
 *
 *  This class is fully thread-safe. You can call any functions from anywhere at
 *  anytime.
 *
 *  Warning: Constructing this class requires an "option" parameter. It is not
 *  safe to modify this "option" parameter during the lifetime of this class.
 *
 */

#ifndef PokemonAutomationn_NintendoSwitch_MultiSwitchSystemSession_H
#define PokemonAutomationn_NintendoSwitch_MultiSwitchSystemSession_H

#include "Common/Cpp/Containers/FixedLimitVector.h"
#include "NintendoSwitch_SwitchSystemSession.h"
#include "NintendoSwitch_MultiSwitchSystemOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


class MultiSwitchSystemSession{
public:
    struct Listener{
        //  Sent before the Switch sessions are destroyed. Listeners should
        //  their references to them before returning.
        virtual void shutdown() = 0;

        //  Sent after new Switches are started up.
        //  This is called immediately when attaching a listener to give the
        //  current switch count. The listener must drop all references to the
        //  switch sessions before detaching.
        virtual void startup(size_t switch_count) = 0;
    };
    void add_listener(Listener& listener);
    void remove_listener(Listener& listener);


public:
    ~MultiSwitchSystemSession();
    MultiSwitchSystemSession(
        MultiSwitchSystemOption& option,
        uint64_t program_id
    );

    //  Lock/unlock the Switch count.
    void lock();
    void unlock();

    //  Returns true only on success.
    bool set_switch_count(size_t count);

    size_t min_switches() const{ return m_option.min_switches(); }
    size_t max_switches() const{ return m_option.max_switches(); }

public:
    //  Note that these are not thread-safe with changing the # of switches.
    size_t count() const{ return m_consoles.size(); }
    SwitchSystemSession& operator[](size_t index){ return m_consoles[index]; }


private:
    MultiSwitchSystemOption& m_option;
    const uint64_t m_program_id;

    std::mutex m_lock;
    bool m_switch_count_locked;
    FixedLimitVector<SwitchSystemSession> m_consoles;
    std::set<Listener*> m_listeners;
};




}
}
#endif
