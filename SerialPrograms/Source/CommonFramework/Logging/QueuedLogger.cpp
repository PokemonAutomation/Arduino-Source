/*  Queued Logger
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Unicode.h"
#include "QueuedLogger.h"

namespace PokemonAutomation{


#if 0
void QueuedLogger::log(const char* msg, Color color){
    log(std::string(msg), color);
}
void QueuedLogger::log(const std::string& msg, Color color){
    std::lock_guard<std::mutex> lg(m_lock);
    m_queue.emplace_back(new Entry{
        current_time(),
        color,
        msg
    });
    m_cv.notify_all();
}
void QueuedLogger::log(const QString& msg, Color color){
    log(msg.toUtf8().toStdString(), color);
}

std::unique_ptr<QueuedLogger::Entry> QueuedLogger::get(){
    std::unique_lock<std::mutex> lg(m_lock);
    if (m_queue.empty()){
        m_cv.wait(lg);
    }
    if (m_queue.empty()){
        return nullptr;
    }
    std::unique_ptr<QueuedLogger::Entry> entry = std::move(m_queue.front());
    m_queue.pop_front();
    return entry;
}
void QueuedLogger::signal(){
    std::lock_guard<std::mutex> lg(m_lock);
    m_cv.notify_all();
}
#endif


}
