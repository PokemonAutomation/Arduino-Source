/*  Failure Watchdog
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_CommonTools_FailureWatchdog_H
#define PokemonAutomation_CommonTools_FailureWatchdog_H

#include "Common/Cpp/AbstractLogger.h"
#include "Common/Cpp/Time.h"
#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/Language.h"

namespace PokemonAutomation{





class FailureWatchdog{
public:
    FailureWatchdog(
        Logger& logger,
        Language language,
//        std::string failure_message,
        uint64_t min_count = 5,
        double min_success_rate = 0.5,
        std::chrono::seconds time_limit = std::chrono::seconds(120)
    )
        : m_logger(logger)
        , m_language(language)
//        , m_failure_message(std::move(failure_message))
        , m_min_count(min_count)
        , m_min_success_rate(min_success_rate)
        , m_time_limit(time_limit)
    {
        restart();
    }
    void restart(){
        m_expiration = current_time() + m_time_limit;
        m_expired = false;
        m_successes = 0;
        m_total = 0;
    }

    void push_result(bool success){
        m_successes += success ? 1 : 0;
        m_total++;
        if (success || m_expired){
            return;
        }

        WallClock current = current_time();
        if (current >= m_expiration){
            m_expired = true;
        }

        if (m_total < m_min_count){
            return;
        }

        double threshold = (double)m_total * m_min_success_rate;
        if ((double)m_successes >= threshold){
            return;
        }


        throw UserSetupError(
            m_logger,
            "Too many text recognition errors. Did you set the correct language?\n"
            "Current Language: " + language_data(m_language).name
        );
    }


private:
    Logger& m_logger;
    Language m_language;
//    std::string m_failure_message;
    uint64_t m_min_count;
    double m_min_success_rate;
    WallDuration m_time_limit;
    WallClock m_expiration;
    bool m_expired;

    uint64_t m_successes;
    uint64_t m_total;
};





class OcrFailureWatchdog : public FailureWatchdog{
public:
    OcrFailureWatchdog(
        Logger& logger,
        Language language,
//        std::string failure_message = "Too many text recognition errors. Did you set the correct language?",
        uint64_t min_count = 5,
        double min_success_rate = 0.5,
        std::chrono::seconds time_limit = std::chrono::seconds(120)
    )
        : FailureWatchdog(
            logger,
            language,
//            std::move(failure_message),
            min_count,
            min_success_rate,
            time_limit
        )
    {}
};






}
#endif
