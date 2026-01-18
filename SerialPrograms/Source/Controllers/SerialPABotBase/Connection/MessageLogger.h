/*  Message Logger
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_MessageLogger_H
#define PokemonAutomation_MessageLogger_H

#include "Common/Cpp/Logger/AbstractLogger.h"
#include "Common/Cpp/Time.h"
#include "Common/Cpp/Containers/CircularBuffer.h"
#include "Common/Cpp/Concurrency/SpinLock.h"

namespace PokemonAutomation{



class SerialLogger : public Logger{
public:
    SerialLogger(Logger& logger, bool log_everything);

    virtual void log(const char* msg, Color color = COLOR_DARKGREEN) override;
    virtual void log(const std::string& msg, Color color = COLOR_DARKGREEN) override;

private:
    bool ok_to_log();

private:
    Logger& m_logger;
    SpinLock m_lock;
    size_t m_messages_dropped = 0;
    CircularBuffer<WallClock> m_history;
};




}
#endif
