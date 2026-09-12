/*  Multi-Switch Program Session
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  This class holds the run-time state of a Switch program.
 *
 *  This class is fully thread-safe. You can call any functions from anywhere at
 *  anytime.
 *
 *  Warning: Constructing this class requires an "option" parameter. It is not
 *  safe to modify this "option" parameter during the lifetime of this class.
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_MultiSwitchProgramSession_H
#define PokemonAutomation_NintendoSwitch_MultiSwitchProgramSession_H

#include "CommonFramework/Panels/PanelSession.h"
#include "CommonFramework/ProgramSession.h"
#include "NintendoSwitch/NintendoSwitch_MultiSwitchProgram.h"
#include "NintendoSwitch_MultiSwitchSystemSession.h"

namespace PokemonAutomation{
namespace NintendoSwitch{

class MultiSwitchProgramOption;


class MultiSwitchProgramSession final
    : public UiState<MultiSwitchProgramSession, PanelSession>
    , public ProgramSession
    , private MultiSwitchSystemSession::Listener
{
public:
    //  This is temporary. Remove once configs have push notifications.
    struct Listener{
        virtual void redraw_options() = 0;
    };
    void add_listener(Listener& listener);
    void remove_listener(Listener& listener);


public:
    bool try_shutdown();
    ~MultiSwitchProgramSession();
    MultiSwitchProgramSession(const MultiSwitchProgramDescriptor& descriptor);


public:
    const MultiSwitchProgramDescriptor& descriptor() const{ return m_descriptor; }
    MultiSwitchSystemSession& system(){ return m_system; }
    ConfigOption& options();


private:
    virtual void internal_run_program() override;
    virtual void internal_stop_program() override;

    virtual void shutdown() override;
    virtual void startup(size_t switch_count) override;


public:
    void restore_defaults();
    virtual std::string check_validity() const override;
    virtual JsonValue to_json() const override;
    virtual void load_json(const JsonValue& json) override;


private:
    void run_program_instance(MultiSwitchProgramEnvironment& env, CancellableScope& scope);


private:

    const MultiSwitchProgramDescriptor& m_descriptor;

    MultiSwitchSystemOption m_system_option;
    MultiSwitchSystemSession m_system;

    std::unique_ptr<MultiSwitchProgramInstance> m_instance;

    std::atomic<CancellableScope*> m_scope;

    ListenerSet<Listener> m_listeners;

    LifetimeSanitizer m_sanitizer;
};





}
}
#endif
