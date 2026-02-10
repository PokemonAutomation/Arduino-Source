/*  Queued Logger
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Logging_QueuedLogger_H
#define PokemonAutomation_Logging_QueuedLogger_H

#include <deque>
#include "Common/Cpp/Concurrency/Mutex.h"
#include "Common/Cpp/Concurrency/ConditionVariable.h"
#include "Logger.h"

namespace PokemonAutomation{


#if 0
class QueuedLogger : public Logger{
public:
    struct Entry{
        WallClock timestamp;
        Color color;
        std::string line;
    };

public:
    virtual void log(const char* msg, Color color = QColor()) override;
    virtual void log(const std::string& msg, Color color = QColor()) override;
    virtual void log(const QString& msg, Color color = QColor()) override;

    //  Get a message from the queue. If it is empty, wait for a message.
    //  May return nullptr.
    std::unique_ptr<Entry> get();

    //  Signal any pending "get()" calls to return.
    void signal();

private:
    Mutex m_lock;
    ConditionV m_cv;
    std::deque<std::unique_ptr<Entry>> m_queue;
};
#endif



}
#endif
