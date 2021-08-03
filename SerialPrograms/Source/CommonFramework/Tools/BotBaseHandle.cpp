/*  Bot-Base Wrapper Handle
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QtGlobal>
#include "Common/Cpp/PrettyPrint.h"
#include "Common/Cpp/PanicDump.h"
#include "Common/SwitchFramework/Switch_PushButtons.h"
#include "ClientSource/Libraries/MessageConverter.h"
#include "ClientSource/Connection/SerialConnection.h"
#include "ClientSource/Connection/PABotBase.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/PersistentSettings.h"
#include "BotBaseHandle.h"

namespace PokemonAutomation{


BotBaseHandle::BotBaseHandle(
    const QSerialPortInfo& port,
    PABotBaseLevel minimum_pabotbase,
    MessageSniffer& logger
)
    : m_minimum_pabotbase(minimum_pabotbase)
    , m_current_pabotbase(PABotBaseLevel::NOT_PABOTBASE)
    , m_state(State::NOT_CONNECTED)
    , m_logger(logger)
{
    reset(port);
    connect(
        this, static_cast<void(BotBaseHandle::*)(uint8_t, std::string)>(&BotBaseHandle::async_try_send_request),
        this, [=](uint8_t msg_type, std::string body){
            if (try_send_request(msg_type, &body[0], body.size())){
                return;
            }
            m_logger.log("BotBaseHandle::async_try_send_request() - Request dropped.");
        }
    );
}
BotBaseHandle::~BotBaseHandle(){
    stop();
    m_botbase.reset();
    m_state.store(State::NOT_CONNECTED, std::memory_order_release);
    on_not_connected("");
}

BotBase* BotBaseHandle::botbase(){
    return m_botbase.get();
}

BotBaseHandle::State BotBaseHandle::state() const{
    return m_state.load(std::memory_order_acquire);
}
bool BotBaseHandle::accepting_commands() const{
    return state() == State::READY &&
        m_current_pabotbase.load(std::memory_order_acquire) > PABotBaseLevel::NOT_PABOTBASE;
}


bool BotBaseHandle::try_send_request(uint8_t msg_type, void* params, size_t bytes){
    std::unique_lock<std::mutex> lg(m_lock, std::defer_lock);
    if (!lg.try_lock()){
        return false;
    }
    if (!accepting_commands()){
        return false;
    }
    return botbase()->try_issue_request(nullptr, msg_type, params, bytes);
}
void BotBaseHandle::async_try_send_request(uint8_t msg_type, void* params, size_t bytes){
    async_try_send_request(msg_type, std::string((char*)params, bytes));
}

void BotBaseHandle::stop_unprotected(){
    {
        State state = m_state.load(std::memory_order_acquire);
        if (state == State::NOT_CONNECTED){
            return;
        }
        if (state == State::SHUTDOWN){
            while (m_state.load(std::memory_order_acquire) != State::NOT_CONNECTED){
                _mm_pause();
            }
            return;
        }

        m_state.store(State::SHUTDOWN, std::memory_order_release);
        on_stopped("");
        m_botbase->stop();

        std::lock_guard<std::mutex> lg(m_cv_lock);
        m_cv.notify_all();
    }

    if (m_status_thread.joinable()){
        m_status_thread.join();
    }

    m_state.store(State::NOT_CONNECTED, std::memory_order_release);
    on_not_connected("");
}
void BotBaseHandle::stop(){
    std::lock_guard<std::mutex> lg(m_lock);
    stop_unprotected();
}
void BotBaseHandle::reset(const QSerialPortInfo& port){
    using namespace PokemonAutomation;

    std::lock_guard<std::mutex> lg(m_lock);

    stop_unprotected();
    if (port.isNull()){
        return;
    }

    std::string name = port.systemLocation().toUtf8().data();
    std::string error;

    try{
        std::unique_ptr<SerialConnection> connection(new SerialConnection(name, PABB_BAUD_RATE));
        m_botbase.reset(new PABotBase(std::move(connection), nullptr));
        m_current_pabotbase.store(PABotBaseLevel::NOT_PABOTBASE, std::memory_order_release);
    }catch (const StringException& e){
        error = e.message();
    }
    if (error.empty()){
        m_state.store(State::CONNECTING, std::memory_order_release);
        on_connecting();
    }else{
        on_not_connected("<font color=\"red\">Unable to open port.</font>");
        m_logger.log(error);
        return;
    }

    m_status_thread = std::thread(run_with_catch, "BotBaseHandle::thread_body()", [=]{ thread_body(); });
}

void BotBaseHandle::verify_protocol(){
    uint32_t protocol = m_botbase->protocol_version();
    uint32_t version_hi = protocol / 100;
    uint32_t version_lo = protocol % 100;
    if (version_hi != PABB_PROTOCOL_VERSION / 100 || version_lo < PABB_PROTOCOL_VERSION % 100){
        PA_THROW_StringException(
            "Incompatible version. Client: " + std::to_string(PABB_PROTOCOL_VERSION) + ", Device: " + std::to_string(protocol)
        );
    }
}
uint8_t BotBaseHandle::verify_pabotbase(){
    using namespace PokemonAutomation;

    uint8_t program_id = m_botbase->program_id();
    PABotBaseLevel type = program_id_to_botbase_level(program_id);
    m_current_pabotbase.store(type, std::memory_order_release);
    if (type < m_minimum_pabotbase){
        PA_THROW_StringException(
            "PABotBase level not met. (" + program_name(program_id) + ")"
        );
    }
    return program_id;
}
void BotBaseHandle::thread_body(){
    using namespace PokemonAutomation;

    m_botbase->set_sniffer(&m_logger);

    //  Connect
    {
        QString error;
        try{
            m_botbase->connect();
        }catch (CancelledException&){
            m_botbase->stop();
            on_stopped("");
            return;
        }catch (const StringException& e){
            error = e.message_qt();
        }
        if (!error.isEmpty()){
            m_botbase->stop();
            on_stopped("<font color=\"red\">" + error + "</font>");
            return;
        }
    }

    //  Check protocol and version.
    {
        uint8_t program_id = 0;
        uint32_t version = 0;
        QString error;
        try{
            verify_protocol();
            program_id = verify_pabotbase();
            version = m_botbase->program_version();
        }catch (CancelledException&){
            return;
        }catch (const StringException& e){
            error = e.message_qt();
        }
        if (error.isEmpty()){
            m_state.store(State::READY, std::memory_order_release);
            on_ready(QString::fromStdString(
                "<font color=\"blue\">Program: " +
                program_name(program_id) +
                " (" + std::to_string(version) + ")</font>"
            ));
        }else{
            m_state.store(State::STOPPED, std::memory_order_release);
            on_stopped("<font color=\"red\">" + error + "</font>");
            m_botbase->stop();
            return;
        }
    }

    std::thread watchdog([=]{
        while (true){
            if (m_state.load(std::memory_order_acquire) != State::READY){
                break;
            }

            auto last = std::chrono::system_clock::now() - m_botbase->last_ack();
            std::chrono::duration<double> seconds = last;
            if (last > 2 * SERIAL_REFRESH_RATE){
                uptime_status(
                    QString("<font color=\"red\">Last Ack: ") +
                    QString::fromStdString(tostr_fixed(seconds.count(), 3)) + " seconds ago</font>"
                );
//                m_logger.log("Connection issue detected. Turning on all logging...");
//                settings.log_everything.store(true, std::memory_order_release);
            }

            std::unique_lock<std::mutex> lg(m_cv_lock);
            if (m_state.load(std::memory_order_acquire) != State::READY){
                break;
            }
            m_cv.wait_for(lg, SERIAL_REFRESH_RATE);
        }
    });

    BotBaseContext context(*m_botbase);
    while (true){
        if (m_state.load(std::memory_order_acquire) != State::READY){
            break;
        }

        std::string str;
        QString error;
        try{
//            cout << "system_clock()" << endl;
            uint32_t wallclock = system_clock(context);
//            cout << "system_clock() - done" << endl;
            str = ticks_to_time(wallclock);
        }catch (CancelledException&){
            break;
        }catch (const StringException& e){
            error = e.message_qt();
        }
        if (error.isEmpty()){
            uptime_status("<font color=\"blue\">Up Time: " + QString::fromStdString(str) + "</font>");
        }else{
            uptime_status("<font color=\"red\">Up Time: " + error + "</font>");
            error.clear();
        }

//        cout << "lock()" << endl;
        std::unique_lock<std::mutex> lg(m_cv_lock);
//        cout << "lock() - done" << endl;
        if (m_state.load(std::memory_order_acquire) != State::READY){
            break;
        }
        m_cv.wait_for(lg, SERIAL_REFRESH_RATE);
    }

    {
        std::unique_lock<std::mutex> lg(m_cv_lock);
        m_cv.notify_all();
    }
    watchdog.join();
}



}
