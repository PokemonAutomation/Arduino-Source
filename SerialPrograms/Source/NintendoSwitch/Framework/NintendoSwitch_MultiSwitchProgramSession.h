/*  Multi-Switch Program Session
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_MultiSwitchProgramSession_H
#define PokemonAutomation_NintendoSwitch_MultiSwitchProgramSession_H

#include "CommonFramework/ProgramSession.h"
#include "NintendoSwitch_MultiSwitchSystemSession.h"
#include "NintendoSwitch_MultiSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{

class MultiSwitchProgramOption;


class MultiSwitchProgramSession final : public ProgramSession, private MultiSwitchSystemSession::Listener{
public:
    //  This is temporary. Remove once configs have push notifications.
    struct Listener{
        virtual void redraw_options() = 0;
    };
    void add_listener(Listener& listener);
    void remove_listener(Listener& listener);

public:
    virtual ~MultiSwitchProgramSession();
    MultiSwitchProgramSession(MultiSwitchProgramOption& option);

    void restore_defaults();

public:
    MultiSwitchSystemSession& system(){ return m_system; }

private:
    virtual std::string check_validity() const override;

    virtual void internal_run_program() override;
    virtual void internal_stop_program() override;

    virtual void shutdown() override;
    virtual void startup(size_t switch_count) override;

private:
    void run_program_instance(const ProgramInfo& info);

private:
    MultiSwitchProgramOption& m_option;
    MultiSwitchSystemSession m_system;

    std::mutex m_lock;
    std::condition_variable m_cv;
    CancellableScope* m_scope = nullptr;

    std::set<Listener*> m_listeners;
};





}
}
#endif
