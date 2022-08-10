/*  Multi-Switch System Session
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomationn_NintendoSwitch_MultiSwitchSystemSession_H
#define PokemonAutomationn_NintendoSwitch_MultiSwitchSystemSession_H

#include "Common/Cpp/FixedLimitVector.h"
#include "CommonFramework/Logging/LoggerQt.h"
#include "NintendoSwitch_SingleSwitchProgramSession.h"
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
        virtual void startup(size_t switch_count) = 0;
    };
    void add_listener(Listener& listener);
    void remove_listener(Listener& listener);

public:
    ~MultiSwitchSystemSession();
    MultiSwitchSystemSession(
        MultiSwitchSystemOption& option,
        Logger& raw_logger,
        uint64_t program_id
    );

    void set_switch_count(size_t count);

    SwitchSystemSession& operator[](size_t index){ return m_consoles[index]; }


private:
    MultiSwitchSystemOption& m_option;
    Logger& m_raw_logger;
    const uint64_t m_program_id;

    std::mutex m_lock;
    FixedLimitVector<SwitchSystemSession> m_consoles;
    std::set<Listener*> m_listeners;
};




}
}
#endif
