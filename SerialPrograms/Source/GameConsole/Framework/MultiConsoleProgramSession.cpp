/*  Multi-Console Program Session
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/ScopeExit.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "MultiConsoleProgramSession.h"

namespace PokemonAutomation{
namespace GameConsole{



bool MultiConsoleProgramSession::try_shutdown(){
    MultiConsoleProgramSession::internal_stop_program();
    join_program_thread();
    return m_system.try_shutdown();
}
MultiConsoleProgramSession::~MultiConsoleProgramSession(){
    blocking_shutdown(
        logger(),
        "MultiConsoleProgramSession",
        [this]{ return try_shutdown(); }
    );

    m_system.remove_listener(*this);

    //  The instance may hold references to m_system.
    //  So it must be destroyed first.
    m_instance.reset();
}
MultiConsoleProgramSession::MultiConsoleProgramSession(
    const MultiConsoleProgramDescriptor& descriptor,
        std::unique_ptr<MultiConsoleSystemOption> option
)
    : UiState<MultiConsoleProgramSession, PanelSession>(descriptor)
    , ProgramSession(descriptor)
    , m_descriptor(descriptor)
    , m_system_option(
        option != nullptr
            ? std::move(option)
            : std::make_unique<MultiConsoleSystemOption>(
                descriptor.min_consoles(),
                descriptor.max_consoles(),
                descriptor.default_consoles()
            )
    )
    , m_system(*m_system_option, descriptor.allow_commands_while_running(), instance_id())
{
    auto instance = descriptor.make_instance(m_system);
    instance->update_active_consoles(m_system_option->active_consoles());
    m_instance = std::move(instance);
    m_system.add_listener(*this);
}



ConfigOption& MultiConsoleProgramSession::options(){
    return m_instance->options();
}
std::string MultiConsoleProgramSession::check_validity() const{
    return m_instance->check_validity();
}
void MultiConsoleProgramSession::restore_defaults(){
    std::lock_guard<Mutex> lg(program_lock());
    if (current_state() != ProgramState::STOPPED){
        logger().log("Cannot change settings while program is running.", COLOR_RED);
        return;
    }
    logger().log("Restoring settings to defaults...");
    m_instance->restore_defaults();
}
JsonValue MultiConsoleProgramSession::to_json() const{
    JsonObject obj = std::move(*m_instance->to_json().to_object());
    obj["ConsoleSetup"] = m_system_option->to_json();
    return obj;
}
void MultiConsoleProgramSession::load_json(const JsonValue& json){
    const JsonObject* obj = json.to_object();
    if (obj == nullptr){
        return;
    }
    const JsonValue* value = obj->get_value("ConsoleSetup");
    if (value){
        m_system.load_json(*value);
    }
    m_instance->load_json(json);
}


std::unique_ptr<ProgramEnvironment> MultiConsoleProgramSession::make_env(const ProgramInfo& program_info){
    size_t consoles = m_system.active_consoles();
    std::vector<std::unique_ptr<ConsoleHandle>> handles(consoles);
    for (size_t c = 0; c < consoles; c++){
        ConsoleSystemSession& session = m_system[c];
        handles.emplace_back(std::make_unique<ConsoleHandle>(session));
    }
    return std::make_unique<MultiConsoleProgramEnvironment>(
        program_info,
        *m_scope,
        *this,
        current_stats_tracker(), historical_stats_tracker(),
        std::move(handles)
    );
}
void MultiConsoleProgramSession::internal_run_program(ProgramEnvironment& env){
    MultiConsoleProgramInstance& instance = static_cast<MultiConsoleProgramInstance&>(*m_instance);
    MultiConsoleProgramEnvironment& lenv = static_cast<MultiConsoleProgramEnvironment&>(env);

    std::deque<ControllerContext<AbstractController>> contexts;
    size_t total_consoles = m_system.active_consoles();
    for (size_t console = 0; console < total_consoles; console++){
        size_t total_controllers = lenv.console(console).controllers();

        //  Attach all the controllers to the scope so they can be cancelled from the top.
        for (size_t controller = 0; controller < total_controllers; controller++){
            contexts.emplace_back(*m_scope, lenv.console(console).controller(controller));
        }
    }

    {
        std::lock_guard<Mutex> lg(program_lock());
        if (current_state() != ProgramState::RUNNING){
            return;
        }
    }

    ScopeExit on_exit([&]{
        for (size_t c = 0; c < total_consoles; c++){
            lenv.console(c).cancel_all_controllers();
        }
    });

    instance.program(lenv, *m_scope);
    for (size_t c = 0; c < total_consoles; c++){
        lenv.console(c).wait_for_all_controllers();
    }
}




void MultiConsoleProgramSession::shutdown(){
    internal_stop_program();
}
void MultiConsoleProgramSession::startup(size_t switch_count){
    MultiConsoleProgramInstance& instance = static_cast<MultiConsoleProgramInstance&>(*m_instance);
    instance.update_active_consoles(switch_count);
//    m_listeners.run_method(&Listener::redraw_options);
}













}
}
