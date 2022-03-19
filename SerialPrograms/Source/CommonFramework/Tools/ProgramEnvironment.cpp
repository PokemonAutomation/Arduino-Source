/*  Program Environment
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <mutex>
#include <condition_variable>
#include "Common/Cpp/Pimpl.tpp"
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/AsyncDispatcher.h"
#include "ClientSource/Connection/BotBase.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "StatsTracking.h"
#include "ProgramEnvironment.h"


namespace PokemonAutomation{


struct ProgramEnvironmentData{
    const ProgramInfo m_program_info;

    std::atomic<bool> m_stopping;
    std::mutex m_lock;
    std::condition_variable m_cv;
    AsyncDispatcher m_realtime_dispatcher;
    AsyncDispatcher m_inference_dispatcher;

    std::map<std::condition_variable*, std::mutex*> m_stop_signals;

    ProgramEnvironmentData(
        ProgramInfo program_info
    )
        : m_program_info(std::move(program_info))
        , m_stopping(false)
        , m_realtime_dispatcher(
            [](){
                GlobalSettings::instance().REALTIME_THREAD_PRIORITY.set_on_this_thread();
            },
            0
        )
        , m_inference_dispatcher(
            [](){
                GlobalSettings::instance().INFERENCE_PRIORITY.set_on_this_thread();
            },
            0
        )
    {}
};




ProgramEnvironment::~ProgramEnvironment(){}

ProgramEnvironment::ProgramEnvironment(
    ProgramInfo program_info,
    LoggerQt& logger,
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
AsyncDispatcher& ProgramEnvironment::realtime_dispatcher(){
    return m_data->m_realtime_dispatcher;
}
AsyncDispatcher& ProgramEnvironment::inference_dispatcher(){
    return m_data->m_inference_dispatcher;
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
        emit set_status("");
        return;
    }

    if (!current.empty() && historical.empty()){
        QString str = QString::fromStdString(current);
        emit set_status(str);
        log(str);
        return;
    }
    if (current.empty() && !historical.empty()){
        QString str = QString::fromStdString(historical);
        emit set_status("<b>Past Runs</b> - " + str);
        return;
    }

    log(QString::fromStdString(current));

    std::string str;
    str += "<b>Current Run</b> - " + current;
    str += "<br>";
    str += "<b>Past Totals</b> - " + historical;

    emit set_status(QString::fromStdString(str));
}

bool ProgramEnvironment::is_stopping() const{
    return m_data->m_stopping.load(std::memory_order_acquire);
}
void ProgramEnvironment::check_stopping() const{
    if (is_stopping()){
        throw ProgramCancelledException();
    }
}
void ProgramEnvironment::register_stop_program_signal(std::mutex& lock, std::condition_variable& cv){
    std::unique_lock<std::mutex> lg(m_data->m_lock);
    m_data->m_stop_signals[&cv] = &lock;
}
void ProgramEnvironment::deregister_stop_program_signal(std::condition_variable& cv){
    std::unique_lock<std::mutex> lg(m_data->m_lock);
    m_data->m_stop_signals.erase(&cv);
}
void ProgramEnvironment::signal_stop(){
    m_data->m_stopping.store(true, std::memory_order_release);

    std::lock_guard<std::mutex> lg(m_data->m_lock);
    m_data->m_cv.notify_all();
    for (auto& item : m_data->m_stop_signals){
        std::lock_guard<std::mutex> lg0(*item.second);
        item.first->notify_all();
    }
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
void ProgramEnvironment::wait_until(std::chrono::system_clock::time_point stop){
    check_stopping();

    std::unique_lock<std::mutex> lg(m_data->m_lock);
    m_data->m_cv.wait_until(
        lg, stop,
        [=]{
            return std::chrono::system_clock::now() >= stop || is_stopping();
        }
    );

    check_stopping();
}
void ProgramEnvironment::notify_all(){
    std::lock_guard<std::mutex> lg(m_data->m_lock);
    m_data->m_cv.notify_all();
}




}
