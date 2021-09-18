/*  Program Environment
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "ProgramEnvironment.h"


namespace PokemonAutomation{


ProgramEnvironment::ProgramEnvironment(
//    std::string program_identifier,
    Logger& logger,
    StatsTracker* current_stats,
    const StatsTracker* historical_stats
)
    : m_enable_feedback(true)
    , m_stopping(false)
//    , m_program_identifier(program_identifier)
    , m_logger(logger)
    , m_current_stats(current_stats)
    , m_historical_stats(historical_stats)
{}



void ProgramEnvironment::update_stats(const std::string& override_current){
    std::string current;
    if (!override_current.empty()){
        current = override_current;
    }else if (m_current_stats){
        current = m_current_stats->to_str();
    }

    std::string historical;
    if (m_historical_stats){
        historical = m_historical_stats->to_str();
    }

    if (current.empty() && historical.empty()){
        set_status("");
        return;
    }

    if (!current.empty() && historical.empty()){
        QString str = QString::fromStdString(current);
        set_status(str);
        log(str);
        return;
    }
    if (current.empty() && !historical.empty()){
        QString str = QString::fromStdString(historical);
        set_status("<b>Past Runs</b> - " + str);
        return;
    }

    log(QString::fromStdString(current));

    std::string str;
    str += "<b>Current Run</b> - " + current;
    str += "<br>";
    str += "<b>Past Totals</b> - " + historical;

    set_status(QString::fromStdString(str));
}

bool ProgramEnvironment::is_stopping() const{
    return m_stopping.load(std::memory_order_acquire);
}
void ProgramEnvironment::check_stopping() const{
    if (is_stopping()){
        throw PokemonAutomation::CancelledException();
    }
}
void ProgramEnvironment::signal_stop(){
    m_stopping.store(true, std::memory_order_release);
//    std::lock_guard<std::mutex> lg(m_lock);
//    m_cv.notify_all();
    notify_all();
}

void ProgramEnvironment::notify_all(){
    std::lock_guard<std::mutex> lg(m_lock);
    m_cv.notify_all();
}




}
