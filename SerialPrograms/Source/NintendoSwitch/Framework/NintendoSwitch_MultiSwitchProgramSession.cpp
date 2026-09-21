/*  Multi-Switch Program Session
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/ScopeExit.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "Common/Cpp/EarlyShutdown.h"
#include "Common/Cpp/Containers/FixedLimitVector.tpp"
#include "CommonFramework/Exceptions/FatalProgramException.h"
#include "CommonFramework/Exceptions/OperationFailedExceptionWithScreenshot.h"
#include "CommonFramework/Exceptions/ProgramFinishedException.h"
#include "CommonFramework/Notifications/ProgramInfo.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/Options/NintendoSwitch_ModelType.h"
#include "NintendoSwitch_MultiSwitchProgramSession.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{



void MultiSwitchProgramSession::add_listener(Listener& listener){
    auto ScopeCheck = m_sanitizer.check_scope();
    m_listeners.add(listener);
}
void MultiSwitchProgramSession::remove_listener(Listener& listener){
    auto ScopeCheck = m_sanitizer.check_scope();
    m_listeners.remove(listener);
}




MultiSwitchProgramSession::MultiSwitchProgramSession(const MultiSwitchProgramDescriptor& descriptor)
    : UiState<MultiSwitchProgramSession, PanelSession>(descriptor)
    , ProgramSession(descriptor)
    , m_descriptor(descriptor)
    , m_system_option(
        descriptor.min_switches(),
        descriptor.max_switches(),
        descriptor.default_switches(),
            [](size_t console_index){
                return std::make_unique<GameConsole::ConsoleSystemOption>(
                    1, std::make_unique<ConsoleModelCell>()
                );
            }
    )
    , m_system(m_system_option, descriptor.allow_commands_while_running(), instance_id())
    , m_sanitizer("MultiSwitchProgramSession")
{
    auto instance = descriptor.make_instance();
    instance->update_active_consoles(m_system_option.active_consoles());
    m_instance = std::move(instance);
    m_system.add_listener(*this);
}

bool MultiSwitchProgramSession::try_shutdown(){
    MultiSwitchProgramSession::internal_stop_program();
    m_system.remove_listener(*this);
    join_program_thread();
    return m_system.try_shutdown();
}
MultiSwitchProgramSession::~MultiSwitchProgramSession(){
    blocking_shutdown(
        logger(),
        "MultiSwitchProgramSession",
        [this]{ return try_shutdown(); }
    );

    //  The instance may hold references to m_system.
    //  So it must be destroyed first.
    m_instance.reset();
}

ConfigOption& MultiSwitchProgramSession::options(){
    return m_instance->options();
}



std::string MultiSwitchProgramSession::check_validity() const{
    auto ScopeCheck = m_sanitizer.check_scope();
    return m_instance->check_validity();
}
void MultiSwitchProgramSession::restore_defaults(){
    auto ScopeCheck = m_sanitizer.check_scope();
    std::lock_guard<Mutex> lg(program_lock());
    if (current_state() != ProgramState::STOPPED){
        logger().log("Cannot change settings while program is running.", COLOR_RED);
        return;
    }
    logger().log("Restoring settings to defaults...");
    m_instance->restore_defaults();
}
JsonValue MultiSwitchProgramSession::to_json() const{
    JsonObject obj = std::move(*m_instance->to_json().to_object());
    obj["SwitchSetup"] = m_system_option.to_json();
    return obj;
}
void MultiSwitchProgramSession::load_json(const JsonValue& json){
//    cout << "MultiSwitchProgramSession::load_json()" << endl;
    const JsonObject* obj = json.to_object();
    if (obj == nullptr){
        return;
    }
    const JsonValue* value = obj->get_value("SwitchSetup");
    if (value){
        m_system.load_json(*value);
    }
    m_instance->load_json(json);
}




std::unique_ptr<ProgramEnvironment> MultiSwitchProgramSession::make_env(const ProgramInfo& program_info){
    size_t consoles = m_system.active_consoles();
    FixedLimitVector<ConsoleHandle> handles(consoles);
    for (size_t c = 0; c < consoles; c++){
        GameConsole::ConsoleSystemSession& session = m_system[c];
        handles.emplace_back(session);

        ConsoleModelCell& console_type = static_cast<ConsoleModelCell&>(*session.extra_option());

        ConsoleState& state = handles.back().state();
        if (ConsoleSettings::instance().TRUST_USER_CONSOLE_SELECTION){
            state.set_console_type(handles.back().logger(), console_type);
        }else{
            state.set_console_type_user(console_type);
        }
    }
    return std::make_unique<MultiSwitchProgramEnvironment>(
        program_info,
        *m_scope,
        *this,
        current_stats_tracker(), historical_stats_tracker(),
        std::move(handles)
    );
}
void MultiSwitchProgramSession::internal_run_program(ProgramEnvironment& env){
    auto ScopeCheck = m_sanitizer.check_scope();

    MultiSwitchProgramInstance& instance = static_cast<MultiSwitchProgramInstance&>(*m_instance);
    MultiSwitchProgramEnvironment& lenv = static_cast<MultiSwitchProgramEnvironment&>(env);

    std::deque<ControllerContext<AbstractController>> contexts;
    size_t consoles = m_system.active_consoles();
    for (size_t console = 0; console < consoles; console++){
        size_t controllers = lenv.consoles[console].controllers();

        //  Startup Checks
        if (controllers > 0){
            instance.start_program_controller_check(
                m_system[console], console
            );
        }
        instance.start_program_feedback_check(
            lenv.consoles[console], console,
           m_descriptor.feedback()
        );
        instance.start_program_border_check(
            lenv.consoles[console], console,
            m_descriptor.feedback()
        );

        //  Attach all the controllers to the scope so they can be cancelled from the top.
        for (size_t controller = 0; controller < controllers; controller++){
            contexts.emplace_back(*m_scope, lenv.consoles[console].controller(controller));
        }
    }

    {
        std::lock_guard<Mutex> lg(program_lock());
        if (current_state() != ProgramState::RUNNING){
            return;
        }
    }

    ScopeExit on_exit([&]{
        for (size_t c = 0; c < consoles; c++){
            lenv.consoles[c].cancel_all_controllers();
        }
    });

    instance.program(lenv, *m_scope);
    for (size_t c = 0; c < consoles; c++){
        lenv.consoles[c].wait_for_all_controllers();
    }
}


void MultiSwitchProgramSession::shutdown(){
    auto ScopeCheck = m_sanitizer.check_scope();
    internal_stop_program();
}
void MultiSwitchProgramSession::startup(size_t switch_count){
    auto ScopeCheck = m_sanitizer.check_scope();
    MultiSwitchProgramInstance& instance = static_cast<MultiSwitchProgramInstance&>(*m_instance);
    instance.update_active_consoles(switch_count);
    m_listeners.run_method(&Listener::redraw_options);
}






}
}
