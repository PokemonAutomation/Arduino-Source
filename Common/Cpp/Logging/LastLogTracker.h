/*  Last Log Tracker
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  A simple ring buffer that stores the last N log lines. Thread-safe reads
 *  require external synchronization (the caller must hold the appropriate lock).
 */

#ifndef PokemonAutomation_Logging_LastLogTracker_H
#define PokemonAutomation_Logging_LastLogTracker_H

#include <string>
#include <deque>
#include <vector>

namespace PokemonAutomation{


// A bounded queue that keeps track of the most recent log lines.
// When the queue exceeds max_lines, the oldest lines are discarded.
// This class is NOT thread-safe; callers must provide external synchronization.
class LastLogTracker{
public:
    LastLogTracker(size_t max_lines = 10000)
        : m_max_lines(max_lines)
    {}

    // Add a log line to the tracker. If the tracker exceeds max_lines,
    // the oldest lines are removed.
    void operator+=(std::string line);

    // Return a copy of all stored log lines, oldest first.
    std::vector<std::string> snapshot() const;

private:
    size_t m_max_lines;
    std::deque<std::string> m_lines;
};


}
#endif
