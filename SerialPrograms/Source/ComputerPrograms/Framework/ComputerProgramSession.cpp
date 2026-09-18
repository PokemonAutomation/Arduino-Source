/*  Computer Program Session
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Json/JsonValue.h"
#include "CommonFramework/Exceptions/FatalProgramException.h"
#include "CommonFramework/Exceptions/ProgramFinishedException.h"
#include "CommonFramework/Exceptions/OperationFailedExceptionWithScreenshot.h"
#include "CommonFramework/Notifications/ProgramInfo.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "ComputerProgramSession.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{


ComputerProgramSession::ComputerProgramSession(const ComputerProgramDescriptor& descriptor)
    : UiState<ComputerProgramSession, PanelSession>(descriptor)
    , ProgramSession(descriptor)
    , m_descriptor(descriptor)
{
    m_instance = descriptor.make_instance();
}

ComputerProgramSession::~ComputerProgramSession(){
    ComputerProgramSession::internal_stop_program();
    join_program_thread();
}


ConfigOption& ComputerProgramSession::options(){
    return m_instance->options();
}
std::string ComputerProgramSession::check_validity() const{
    return m_instance->check_validity();
}




void ComputerProgramSession::restore_defaults(){
    std::lock_guard<Mutex> lg(program_lock());
    if (current_state() != ProgramState::STOPPED){
        logger().log("Cannot change settings while program is running.", COLOR_RED);
        return;
    }
    logger().log("Restoring settings to defaults...");
    m_instance->restore_defaults();
}
JsonValue ComputerProgramSession::to_json() const{
    return m_instance->to_json();
}
void ComputerProgramSession::load_json(const JsonValue& json){
    m_instance->load_json(json);
}


std::unique_ptr<ProgramEnvironment> ComputerProgramSession::make_env(const ProgramInfo& program_info){
    return std::make_unique<ProgramEnvironment>(
        program_info,
        *this,
        current_stats_tracker(), historical_stats_tracker()
    );
}
void ComputerProgramSession::internal_run_program(ProgramEnvironment& env){
    ComputerProgramInstance& instance = static_cast<ComputerProgramInstance&>(*m_instance);
    instance.program(env, *m_scope);
}



}
