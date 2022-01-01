/*  Program Environment
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <mutex>
#include <condition_variable>
#include "Common/Cpp/Pimpl.tpp"
#include "Common/Cpp/AsyncDispatcher.h"
#include "ClientSource/Connection/BotBase.h"
#include "StatsTracking.h"
#include "ProgramEnvironment.h"


namespace PokemonAutomation{


struct ProgramEnvironmentData{
    const ProgramInfo m_program_info;

    std::atomic<bool> m_stopping;
    std::mutex m_lock;
    std::condition_variable m_cv;
    AsyncDispatcher m_dispatcher;

    ProgramEnvironmentData(
        ProgramInfo program_info
    )
        : m_program_info(std::move(program_info))
        , m_stopping(false)
    {}
};




ProgramEnvironment::~ProgramEnvironment(){}

ProgramEnvironment::ProgramEnvironment(
    ProgramInfo program_info,
    Logger& logger,
    StatsTracker* current_stats,
    const StatsTracker* historical_stats
)
    : m_logger(logger)
    , m_current_stats(current_stats)
    , m_historical_stats(historical_stats)
    , m_data(std::move(program_info))
{}

const ProgramInfo& ProgramEnvironment::program_info() const{
    return m_data->m_program_info;
}
AsyncDispatcher& ProgramEnvironment::dispatcher(){
    return m_data->m_dispatcher;
}


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
//        m_data->status_update("");
        return;
    }

    if (!current.empty() && historical.empty()){
        QString str = QString::fromStdString(current);
        set_status(str);
//        m_data->status_update(str);
        log(str);
        return;
    }
    if (current.empty() && !historical.empty()){
        QString str = QString::fromStdString(historical);
        set_status("<b>Past Runs</b> - " + str);
//        m_data->status_update("<b>Past Runs</b> - " + str);
        return;
    }

    log(QString::fromStdString(current));

    std::string str;
    str += "<b>Current Run</b> - " + current;
    str += "<br>";
    str += "<b>Past Totals</b> - " + historical;

    set_status(QString::fromStdString(str));
//    m_data->status_update(QString::fromStdString(str));
}

bool ProgramEnvironment::is_stopping() const{
    return m_data->m_stopping.load(std::memory_order_acquire);
}
void ProgramEnvironment::check_stopping() const{
    if (is_stopping()){
        throw PokemonAutomation::CancelledException();
    }
}
void ProgramEnvironment::signal_stop(){
    m_data->m_stopping.store(true, std::memory_order_release);
//    std::lock_guard<std::mutex> lg(m_lock);
//    m_cv.notify_all();
    notify_all();
}


std::mutex& ProgramEnvironment::lock(){
    return m_data->m_lock;
}
std::condition_variable& ProgramEnvironment::cv(){
    return m_data->m_cv;
}

void ProgramEnvironment::wait_for(std::chrono::milliseconds duration){
    check_stopping();

    auto start = std::chrono::system_clock::now();
    std::unique_lock<std::mutex> lg(m_data->m_lock);
    m_data->m_cv.wait_for(
        lg, duration,
        [=]{
            return std::chrono::system_clock::now() - start >= duration || is_stopping();
        }
    );

    check_stopping();
}
void ProgramEnvironment::notify_all(){
    std::lock_guard<std::mutex> lg(m_data->m_lock);
    m_data->m_cv.notify_all();
}




}
