/*  Console Program Session
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/ScopeExit.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "Common/Cpp/Containers/FixedLimitVector.tpp"
#include "ConsoleProgramSession.h"

namespace PokemonAutomation{
namespace GameConsole{


bool ConsoleProgramSession::try_shutdown(){
    ConsoleProgramSession::internal_stop_program();
    join_program_thread();
    return m_system.try_shutdown();
}
ConsoleProgramSession::~ConsoleProgramSession(){
    blocking_shutdown(
        logger(),
        "ConsoleProgramSession",
        [this]{ return try_shutdown(); }
    );
}

ConsoleProgramSession::ConsoleProgramSession(
    const ConsoleProgramDescriptor& descriptor,
    std::unique_ptr<ConfigOption> extra_option
)
    : UiState<ConsoleProgramSession, PanelSession>(descriptor)
    , ProgramSession(descriptor)
    , m_descriptor(descriptor)
    , m_system_option(1, std::move(extra_option))
    , m_system(
        m_system_option,
        descriptor.allow_commands_while_running(),
        0,
        instance_id()
    )
{
    m_instance = descriptor.make_instance(m_system);
}
ConfigOption& ConsoleProgramSession::options(){
    return m_instance->options();
}
std::string ConsoleProgramSession::check_validity() const{
    return m_instance->check_validity();
}
void ConsoleProgramSession::restore_defaults(){
    std::lock_guard<Mutex> lg(program_lock());
    if (current_state() != ProgramState::STOPPED){
        logger().log("Cannot change settings while program is running.", COLOR_RED);
        return;
    }
    logger().log("Restoring settings to defaults...");
    m_instance->restore_defaults();
}
JsonValue ConsoleProgramSession::to_json() const{
    JsonObject obj = std::move(*m_instance->to_json().to_object());
    obj["ConsoleSetup"] = m_system_option.to_json();
    return obj;
}
void ConsoleProgramSession::load_json(const JsonValue& json){
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


std::unique_ptr<ProgramEnvironment> ConsoleProgramSession::make_env(const ProgramInfo& program_info){
    auto env = std::make_unique<ConsoleProgramEnvironment>(
        program_info,
        *m_scope,
        *this,
        current_stats_tracker(),
        historical_stats_tracker(),
        m_system
    );
    return env;
}
void ConsoleProgramSession::internal_run_program(ProgramEnvironment& env){
    ConsoleProgramInstance& instance = static_cast<ConsoleProgramInstance&>(*m_instance);
    ConsoleProgramEnvironment& lenv = static_cast<ConsoleProgramEnvironment&>(env);

    size_t controllers = lenv.console().controllers();

    //  Attach all the controllers to the scope so they can be cancelled from the top.
    FixedLimitVector<ControllerContext<AbstractController>> contexts(controllers);
    for (size_t c = 0; c < controllers; c++){
        contexts.emplace_back(*m_scope, lenv.console().controller(c));
    }

    ScopeExit on_exit([&]{
        lenv.console().cancel_all_controllers();
    });

    instance.program(lenv, *m_scope);
    lenv.console().wait_for_all_controllers();
}









}
}
