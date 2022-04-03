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
    const ProgramInfo& m_program_info;

    std::atomic<bool> m_stopping;
    std::mutex m_lock;
    std::condition_variable m_cv;
    AsyncDispatcher m_realtime_dispatcher;
    AsyncDispatcher m_inference_dispatcher;

    ProgramEnvironmentData(
        const ProgramInfo& program_info
    )
        : m_program_info(program_info)
        , m_stopping(false)
        , m_realtime_dispatcher(
            [](){
                GlobalSettings::instance().REALTIME_THREAD_PRIORITY0.set_on_this_thread();
            },
            0
        )
        , m_inference_dispatcher(
            [](){
                GlobalSettings::instance().INFERENCE_PRIORITY0.set_on_this_thread();
            },
            0
        )
    {}
};




ProgramEnvironment::~ProgramEnvironment(){}

ProgramEnvironment::ProgramEnvironment(
    const ProgramInfo& program_info,
    LoggerQt& logger,
    StatsTracker* current_stats,
    const StatsTracker* historical_stats
)
    : m_logger(logger)
    , m_current_stats(current_stats)
    , m_historical_stats(historical_stats)
    , m_data(program_info)
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
    std::string str = stats_to_bar(m_logger, m_historical_stats, m_current_stats, override_current);
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
void ProgramEnvironment::signal_stop(){
    m_data->m_stopping.store(true, std::memory_order_release);

    if (m_scope.cancel()){
        return;
    }

    std::lock_guard<std::mutex> lg(m_data->m_lock);
    m_data->m_cv.notify_all();
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




}
