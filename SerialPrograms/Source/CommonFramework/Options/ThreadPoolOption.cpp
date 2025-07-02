/*  Thread Pool Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <thread>
#include "ThreadPoolOption.h"

namespace PokemonAutomation{


ThreadPoolOption::ThreadPoolOption(
    std::string label,
    ThreadPriority default_priority,
    double default_max_thread_ratio
)
    : GroupOption(std::move(label), LockMode::UNLOCK_WHILE_RUNNING)
    , m_default_max_threads(
        std::max(
            (size_t)(std::thread::hardware_concurrency() * default_max_thread_ratio),
            (size_t)1
        )
    )
    , HARDWARE_THREADS(
        "<b>Hardware Threads:</b>",
        LockMode::UNLOCK_WHILE_RUNNING,
        std::thread::hardware_concurrency()
    )
    , PRIORITY("<b>Thread Priority:</b>", default_priority)
    , MAX_THREADS(
        "<b>Maximum Threads:</b>",
        LockMode::UNLOCK_WHILE_RUNNING,
        m_default_max_threads
    )
{
    PA_ADD_OPTION(HARDWARE_THREADS);
    PA_ADD_OPTION(PRIORITY);
    PA_ADD_OPTION(MAX_THREADS);
    HARDWARE_THREADS.set_visibility(ConfigOptionState::HIDDEN);
}

void ThreadPoolOption::load_json(const JsonValue& json){
    GroupOption::load_json(json);

    //  Reset the max threads if the hardware threads has changed.
    if (HARDWARE_THREADS != std::thread::hardware_concurrency()){
        MAX_THREADS.set(m_default_max_threads);
    }
}



}
