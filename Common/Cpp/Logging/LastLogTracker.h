/*  Last Log Tracker
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  A simple ring buffer that stores the last N log lines. Thread-safe reads
 *  require external synchronization (the caller must hold the appropriate lock).
 */

#ifndef PokemonAutomation_Logging_LastLogTracker_H
#define PokemonAutomation_Logging_LastLogTracker_H

#include <deque>
#include <vector>
#include "Common/Cpp/Concurrency/SpinLock.h"
#include "AbstractLogger.h"

namespace PokemonAutomation{


// A bounded queue that keeps track of the most recent log lines.
// When the queue exceeds max_lines, the oldest lines are discarded.
// This class is NOT thread-safe; callers must provide external synchronization.
class LastLogTracker : public Logger{
public:
    LastLogTracker(size_t max_lines = 10000)
        : m_max_lines(max_lines)
    {}

    // Add a log line to the tracker. If the tracker exceeds max_lines,
    // the oldest lines are removed.
    virtual void log(const std::string& msg, Color color = Color()) override;
    virtual void log(std::string&& msg, Color color = Color()) override;
    virtual void log(const char* msg, Color color = Color()) override;

    // Return a copy of all stored log lines, oldest first.
    std::vector<LogLine> get_recent(size_t lines) const;

private:
    const size_t m_max_lines;

    mutable SpinLock m_lock;
    std::deque<LogLine> m_lines;
};


}
#endif
