/*  BBQ Diagnostic Logger
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_BBQLogger_H
#define PokemonAutomation_PokemonSV_BBQLogger_H

#include <chrono>
#include <string>
#include <vector>
#include "Common/Cpp/Logging/AbstractLogger.h"
#include "VideoGames/PokemonSV/Options/PokemonSV_BBQuests.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


// Formats one-line "BBQ ..." diagnostic events through an existing Logger.
// Does no image I/O and no controller input.
class BBQLogger{
public:
    explicit BBQLogger(Logger& logger) : m_logger(logger) {}

    // The quest board as read from one panel snapshot: whether the panel was
    // detected, and the quests in board order.
    void board(bool panel_detected, const std::vector<BBQuests>& quests);

    // A catch quest gave up waiting for the battle menu after navigating to its target.
    void battle_entry_timeout(BBQuests quest);

    // A quest used all of its attempts and the rest of the batch will be skipped.
    void retry_exhausted(BBQuests quest);

private:
    Logger& m_logger;
};


// The event sequence and monotonic timing of one quest attempt.
// Timestamps default to now; tests pass fixed values.
class BBQAttemptLog{
public:
    using Clock = std::chrono::steady_clock;

    // Logs the start of attempt number `attempt` out of `limit`.
    BBQAttemptLog(
        Logger& logger, BBQuests quest,
        unsigned attempt, unsigned limit,
        Clock::time_point start = Clock::now()
    );

    // The quest routine returned; logs the time since start.
    void route_return(Clock::time_point end = Clock::now());

    // The board was re-read. `completed` means the quest is no longer on it.
    // Logs the verification time since route_return() and the total time.
    void result(bool completed, Clock::time_point end = Clock::now());

private:
    Logger& m_logger;
    const Clock::time_point m_start;
    Clock::time_point m_route_end;
    std::string m_attempt_description;
};


}
}
}
#endif
