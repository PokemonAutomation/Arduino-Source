/*  SerialPABotBase: Status Thread Helper
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  This is a helper class to reuse the code for the status updater thread.
 *
 */

#ifndef PokemonAutomation_SerialPABotBase_StatusThread_H
#define PokemonAutomation_SerialPABotBase_StatusThread_H

#include "Common/Cpp/PrettyPrint.h"
#include "SerialPABotBase_Connection.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace SerialPABotBase{




struct ControllerStatusThreadCallback{
    virtual void update_status(Cancellable& cancellable) = 0;
    virtual void stop_with_error(std::string message) = 0;
};




class ControllerStatusThread{
public:
    ControllerStatusThread(
        SerialPABotBase_Connection& connection,
        ControllerStatusThreadCallback& callback
    )
        : m_connection(connection)
        , m_callback(callback)
        , m_stopping(false)
        , m_error(false)
        , m_status_thread(&ControllerStatusThread::status_thread, this)
    {}
    ~ControllerStatusThread(){
        if (m_stopping.exchange(true)){
            return;
        }
        m_scope.cancel(nullptr);
        {
            std::unique_lock<std::mutex> lg(m_sleep_lock);
            BotBaseController* botbase = m_connection.botbase();
            if (botbase){
                botbase->notify_all();
            }
            m_cv.notify_all();
        }
        m_status_thread.join();
    }

private:
    void status_thread(){
        constexpr std::chrono::milliseconds PERIOD(1000);
        std::atomic<WallClock> last_ack(current_time());

        std::thread watchdog([&, this]{
            WallClock next_ping = current_time();
            while (true){
                if (m_stopping.load(std::memory_order_relaxed) ||
                    m_error.load(std::memory_order_acquire) ||
                    !m_connection.is_ready()
                ){
                    break;
                }

                auto last = current_time() - last_ack.load(std::memory_order_relaxed);
                std::chrono::duration<double> seconds = last;
                if (last > 2 * PERIOD){
                    std::string text = "Last Ack: " + tostr_fixed(seconds.count(), 3) + " seconds ago";
                    m_connection.set_status_line1(text, COLOR_RED);
    //                m_logger.log("Connection issue detected. Turning on all logging...");
    //                settings.log_everything.store(true, std::memory_order_release);
                }

                std::unique_lock<std::mutex> lg(m_sleep_lock);
                if (m_stopping.load(std::memory_order_relaxed) ||
                    m_error.load(std::memory_order_acquire) ||
                    !m_connection.is_ready()
                ){
                    break;
                }

                WallClock now = current_time();
                next_ping += PERIOD;
                if (now + PERIOD < next_ping){
                    next_ping = now + PERIOD;
                }
                m_cv.wait_until(lg, next_ping);
            }
        });


        WallClock next_ping = current_time();
        while (true){
            if (m_stopping.load(std::memory_order_relaxed) || !m_connection.is_ready()){
                break;
            }

            std::string error;
            try{
                m_callback.update_status(m_scope);
                last_ack.store(current_time(), std::memory_order_relaxed);
            }catch (OperationCancelledException&){
                break;
            }catch (InvalidConnectionStateException& e){
                error = e.message();
            }catch (SerialProtocolException& e){
                error = e.message();
            }catch (ConnectionException& e){
                error = e.message();
            }catch (...){
                error = "Unknown error.";
            }
            if (!error.empty()){
                m_error.store(true, std::memory_order_release);
                m_connection.set_status_line1(error, COLOR_RED);
                m_callback.stop_with_error(std::move(error));
            }

            std::unique_lock<std::mutex> lg(m_sleep_lock);
            if (m_stopping.load(std::memory_order_relaxed) || !m_connection.is_ready()){
                break;
            }

            WallClock now = current_time();
            next_ping += PERIOD;
            if (now + PERIOD < next_ping){
                next_ping = now + PERIOD;
            }
            m_cv.wait_until(lg, next_ping);
        }

        {
            std::unique_lock<std::mutex> lg(m_sleep_lock);
            m_cv.notify_all();
        }
        watchdog.join();
    }

private:
    SerialPABotBase::SerialPABotBase_Connection& m_connection;
    ControllerStatusThreadCallback& m_callback;
    CancellableHolder<CancellableScope> m_scope;
    std::atomic<bool> m_stopping;
    std::atomic<bool> m_error;
    std::mutex m_sleep_lock;
    std::condition_variable m_cv;
    std::thread m_status_thread;
};






}
}
#endif
