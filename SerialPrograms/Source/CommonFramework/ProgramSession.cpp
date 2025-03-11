/*  Program Session
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/PanicDump.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/Panels/ProgramDescriptor.h"
#include "CommonFramework/ProgramSession.h"
#include "CommonFramework/ProgramStats/StatsDatabase.h"
#include "Integrations/ProgramTracker.h"

namespace PokemonAutomation{



void ProgramSession::add_listener(Listener& listener){
    m_listeners.add(listener);
}
void ProgramSession::remove_listener(Listener& listener){
    m_listeners.remove(listener);
}



ProgramSession::ProgramSession(const ProgramDescriptor& descriptor)
    : m_descriptor(descriptor)
    , m_instance_id(ProgramTracker::instance().add_program(*this))
//    , m_logger(global_logger_raw(), "Program:" + std::to_string(m_instance_id))
    , m_logger(global_logger_raw(), "Program")
    , m_timestamp(current_time())
    , m_state(ProgramState::STOPPED)
{
    load_historical_stats();
}
ProgramSession::~ProgramSession(){
    ProgramTracker::instance().remove_program(m_instance_id);
//    ProgramSession::request_program_stop();
//    join_program_thread();
}

void ProgramSession::join_program_thread(){
    if (m_thread.joinable()){
        m_thread.join();
    }
}



const std::string& ProgramSession::identifier() const{
    return m_descriptor.identifier();
}
std::string ProgramSession::current_stats() const{
    std::lock_guard<std::mutex> lg(m_lock);
    if (m_current_stats){
        return m_current_stats->to_str(StatsTracker::DISPLAY_ON_SCREEN);
    }
    return "";
}
std::string ProgramSession::historical_stats() const{
    std::lock_guard<std::mutex> lg(m_lock);
    if (m_historical_stats){
        return m_historical_stats->to_str(StatsTracker::DISPLAY_ON_SCREEN);
    }
    return "";
}
WallClock ProgramSession::timestamp() const{
    return m_timestamp.load(std::memory_order_relaxed);
}


void ProgramSession::report_stats_changed(){
    std::lock_guard<std::mutex> lg(m_lock);
    push_stats();
}
void ProgramSession::report_error(const std::string& message){
    std::lock_guard<std::mutex> lg(m_lock);
    push_error(message);
}


void ProgramSession::set_state(ProgramState state){
    switch (state){
    case ProgramState::NOT_READY:
        m_logger.log("Program State: NOT_READY");
        break;
    case ProgramState::STOPPED:
        m_logger.log("Program State: STOPPED");
        break;
    case ProgramState::RUNNING:
        m_logger.log("Program State: RUNNING");
        break;
    case ProgramState::STOPPING:
        m_logger.log("Program State: STOPPING");
        break;
    }
    m_state.store(state, std::memory_order_relaxed);
    m_listeners.run_method_unique(&Listener::state_change, state);
}
void ProgramSession::push_stats(){
    m_listeners.run_method_unique(
        &Listener::stats_update,
        m_current_stats.get(),
        m_historical_stats.get()
    );
}
void ProgramSession::push_error(const std::string& message){
    m_listeners.run_method_unique(&Listener::error, message);
}

void ProgramSession::load_historical_stats(){
    //  Load historical stats.
    std::unique_ptr<StatsTracker> stats = m_descriptor.make_stats();
    if (stats){
        m_logger.log("Loading historical stats...");
//        m_current_stats = m_descriptor.make_stats();
        StatSet set;
        set.open_from_file(GlobalSettings::instance().STATS_FILE);
        const std::string& identifier = m_descriptor.identifier();
        StatList& list = set[identifier];
        if (list.size() != 0){
            list.aggregate(*stats);
        }
        m_historical_stats = std::move(stats);
    }
}
void ProgramSession::update_historical_stats_with_current(){
    if (m_current_stats){
        m_logger.log("Saving historical stats...");
        bool ok = StatSet::update_file(
            GlobalSettings::instance().STATS_FILE,
            m_descriptor.identifier(),
            *m_current_stats
        );
        if (ok){
            m_logger.log("Stats successfully saved!", COLOR_BLUE);
        }else{
            m_logger.log("Unable to save stats.", COLOR_RED);
            push_error("Unable to save stats.");
        }
    }
}


std::string ProgramSession::start_program(){
    m_logger.log("Received Program Start Request");
    std::lock_guard<std::mutex> lg(m_lock);

    ProgramState state = this->current_state();
    switch (state){
    case ProgramState::NOT_READY:
        m_logger.log("Program is not ready.", COLOR_RED);
        return "Program is not ready.";

    case ProgramState::STOPPED:{
        std::string error = check_validity();
        if (!error.empty()){
            m_logger.log(error, COLOR_RED);
            return error;
        }

        //  Wait for previous program thread to finish.
        join_program_thread();

        //  Now start the program.
        m_logger.log("Starting program...");
        m_timestamp.store(current_time(), std::memory_order_relaxed);
        set_state(ProgramState::RUNNING);
        m_thread = std::thread(
            run_with_catch,
            "ProgramSession::start_program()",
            [this]{ run_program(); }
        );

        return "";
    }
    case ProgramState::RUNNING:
        m_logger.log("Program is already running.", COLOR_RED);
        return "Program is already running.";

    case ProgramState::STOPPING:
        m_logger.log("Program is currently stopping.", COLOR_RED);
        return "Program is currently stopping.";
    }

    throw InternalProgramError(
        &m_logger,
        PA_CURRENT_FUNCTION,
        "Invalid State: " + std::to_string((int)state)
    );
}
std::string ProgramSession::stop_program(){
    m_logger.log("Received Stop Request");
    {
        std::lock_guard<std::mutex> lg(m_lock);

        ProgramState state = this->current_state();
        switch (state){
        case ProgramState::NOT_READY:
            m_logger.log("Program is not ready.", COLOR_RED);
            return "Program is not ready.";
        case ProgramState::STOPPED:
            m_logger.log("Program is already stopped.", COLOR_RED);
            return "Program is already stopped.";
        case ProgramState::RUNNING:
            m_logger.log("Stopping program...");
            set_state(ProgramState::STOPPING);
            break;
        case ProgramState::STOPPING:
            m_logger.log("Program is already stopping.", COLOR_RED);
            return "Program is already stopping.";
        default:
            throw InternalProgramError(
                &m_logger,
                PA_CURRENT_FUNCTION,
                "Invalid State: " + std::to_string((int)state)
            );
        }
    }
    internal_stop_program();
    return "";
}



void ProgramSession::run_program(){
    {
        std::lock_guard<std::mutex> lg(m_lock);
        m_current_stats = m_descriptor.make_stats();
        load_historical_stats();
        push_stats();
    }
    internal_run_program();
    {
        std::lock_guard<std::mutex> lg(m_lock);
        push_stats();
        update_historical_stats_with_current();
        set_state(ProgramState::STOPPED);
    }
}














}
