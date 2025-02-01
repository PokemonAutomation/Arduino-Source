/*  Serial Port (PABotBase) Handle
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <set>
#include <QtGlobal>
#include <QMessageBox>
#include "Common/Cpp/PrettyPrint.h"
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/PanicDump.h"
#include "Common/Cpp/Concurrency/SpinPause.h"
#include "Common/Cpp/Options/TimeExpressionOption.h"
#include "Common/Microcontroller/DeviceRoutines.h"
#include "Common/NintendoSwitch/NintendoSwitch_ControllerDefs.h"
#include "ClientSource/Libraries/MessageConverter.h"
#include "ClientSource/Connection/SerialConnection.h"
//#include "ClientSource/Connection/BotBase.h"
#include "ClientSource/Connection/PABotBase.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/Options/Environment/ThemeSelectorOption.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Messages_Device.h"
#include "SerialPABotBase.h"
#include "SerialPABotBase_Handle.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{

using namespace SerialPABotBase;



void BotBaseHandle::process_device_protocol(uint32_t& version, uint8_t& program_id){
    static const std::set<uint32_t> COMPATIBLE_PROTOCOLS{
        20210526,   //  Old version
        20231219,   //  Fewer RPCs, has queue size.
    };

    Logger& logger = m_logger;


    logger.log("Checking device protocol compatibility...");
    uint32_t protocol = Microcontroller::protocol_version(*m_botbase);
    logger.log("Checking device protocol compatibility... Protocol = " + std::to_string(protocol));
    if (!COMPATIBLE_PROTOCOLS.contains(protocol / 100)){
        throw SerialProtocolException(
            logger, PA_CURRENT_FUNCTION,
            "Incompatible protocol. Device: " + std::to_string(protocol) + "<br>"
            "Please install the .hex that came with this version of the program."
        );
    }

    //  PABotBase Level
    logger.log("Checking Program ID...");
    program_id = Microcontroller::program_id(*m_botbase);
    logger.log("Checking Program ID... Program ID = " + std::to_string(program_id));
//    PABotBaseLevel level = program_id_to_botbase_level(program_id);
//    for (uint8_t c = 0; c <= (uint8_t)level; c++){
//        m_capabilities.insert(program_id_to_string(c));
//    }
    m_capabilities = program_id_to_features(program_id);

//    cout << "m_requirements.size() = " << m_requirements.map().size() << endl;

    std::string missing_feature = m_requirements.check_compatibility(
        SerialPABotBase::NintendoSwitch_Basic,
        m_capabilities
    );
    if (!missing_feature.empty()){
        throw SerialProtocolException(
            logger, PA_CURRENT_FUNCTION,
            "Missing Feature: " + missing_feature
        );
    }
//    if (!m_requirements.is_compatible_with(SerialPABotBase::INTERFACE_NAME, m_capabilities)){
//        throw SerialProtocolException(
//            logger, PA_CURRENT_FUNCTION,
//            "PABotBase level not met. (" + program_name(program_id) + ")"
//        );
//    }

    //  Program Version
    logger.log("Checking Firmware Version...");
    version = Microcontroller::program_version(*m_botbase);
    logger.log("Checking Firmware Version... Version = " + std::to_string(version));

    //  Queue Size
    if ((protocol / 100 == 20210526 && version == 2023121900) ||
        (protocol / 100 == 20231219)
    ){
        logger.log("Device supports queue size. Requesting queue size...", COLOR_BLUE);
        uint8_t queue_limit = Microcontroller::device_queue_size(*m_botbase);
        logger.log("Setting queue size to: " + std::to_string(queue_limit), COLOR_BLUE);
        m_botbase->set_queue_limit(queue_limit);
    }else{
        logger.log("Queue size not supported. Defaulting to size 4.", COLOR_RED);
    }

}




BotBaseHandle::BotBaseHandle(
    SerialLogger& logger,
    const QSerialPortInfo* port,
    const ControllerRequirements& requirements
)
    : m_logger(logger)
    , m_port(port)
    , m_requirements(requirements)
    , m_state(State::NOT_CONNECTED)
    , m_allow_user_commands(true)
    , m_label("<font color=\"red\">Not Connected</font>")
{
    reset(port);
}
BotBaseHandle::~BotBaseHandle(){
    stop();
    m_botbase.reset();
    m_state.store(State::NOT_CONNECTED, std::memory_order_release);
    emit on_not_connected("<font color=\"orange\">Disconnecting...</font>");
}

BotBaseController* BotBaseHandle::botbase(){
    BotBaseController* ret = m_botbase.get();
    if (ret == nullptr){
        m_logger.log("BotBaseHandle::botbase() called with null botbase...", COLOR_RED);
    }
    return ret;
}

BotBaseHandle::State BotBaseHandle::state() const{
    return m_state.load(std::memory_order_acquire);
}
bool BotBaseHandle::accepting_commands() const{
    return state() == State::READY;
}
std::string BotBaseHandle::label() const{
    std::lock_guard<std::mutex> lg(m_lock);
    return m_label;
}

void BotBaseHandle::set_allow_user_commands(bool allow){
    m_allow_user_commands.store(allow, std::memory_order_release);
}

const char* BotBaseHandle::check_accepting_commands(){
    //  Must call under the lock.
    if (state() != State::READY){
        return "Console is not accepting commands right now.";
    }
    if (!m_allow_user_commands.load(std::memory_order_acquire)){
        return "Handle is not accepting commands right now.";
    }
    return nullptr;
}
const char* BotBaseHandle::try_reset(){
    std::unique_lock<std::mutex> lg(m_lock, std::defer_lock);
    if (!lg.try_lock()){
        return "Console is busy.";
    }
    if (state() != State::READY){
        return "Console is not accepting commands right now.";
    }
    if (!m_allow_user_commands.load(std::memory_order_acquire)){
        return "Cannot reset while a program is running.";
    }
    reset_unprotected(m_port);
    return nullptr;
}
const char* BotBaseHandle::try_send_request(const BotBaseRequest& request){
    std::unique_lock<std::mutex> lg(m_lock, std::defer_lock);
    if (!lg.try_lock()){
        return "Console is busy.";
    }
    const char* error = check_accepting_commands();
    if (error){
        return error;
    }
    if (!botbase()->try_issue_request(request)){
        return "Command dropped.";
    }
    return nullptr;
}
const char* BotBaseHandle::try_stop_commands(){
    std::unique_lock<std::mutex> lg(m_lock, std::defer_lock);
    if (!lg.try_lock()){
        return "Console is busy.";
    }
    const char* error = check_accepting_commands();
    if (error){
        return error;
    }
    if (!botbase()->try_stop_all_commands()){
        return "Command dropped.";
    }
    return nullptr;
}
const char* BotBaseHandle::try_next_interrupt(){
    std::unique_lock<std::mutex> lg(m_lock, std::defer_lock);
    if (!lg.try_lock()){
        return "Console is busy.";
    }
    const char* error = check_accepting_commands();
    if (error){
        return error;
    }
    if (!botbase()->try_next_command_interrupt()){
        return "Command dropped.";
    }
    return nullptr;
}

void BotBaseHandle::stop_unprotected(){
    m_capabilities.clear();

    {
        State state = m_state.load(std::memory_order_acquire);
        if (state == State::NOT_CONNECTED){
            return;
        }
        if (state == State::SHUTDOWN){
            while (m_state.load(std::memory_order_acquire) != State::NOT_CONNECTED){
                pause();
            }
            return;
        }

        m_state.store(State::SHUTDOWN, std::memory_order_release);
        emit on_stopped("");
        m_botbase->stop();

        std::lock_guard<std::mutex> lg(m_sleep_lock);
        m_cv.notify_all();
    }

    if (m_status_thread.joinable()){
        m_status_thread.join();
    }

    m_state.store(State::NOT_CONNECTED, std::memory_order_release);
    m_label = "<font color=\"red\">Not Connected</font>";
    emit on_not_connected(m_label);
}
void BotBaseHandle::reset_unprotected(const QSerialPortInfo* port){
    using namespace PokemonAutomation;

    stop_unprotected();
    if (port == nullptr || port->isNull()){
        return;
    }

    std::string name = port->systemLocation().toUtf8().data();
    std::string error;


    if (port->description().indexOf("Prolific") != -1){
        QMessageBox box;
        box.critical(
            nullptr,
            "Error",
            "Cannot select Prolific controller.<br><br>"
            "Prolific controllers do not work for Arduino and similar microntrollers.<br>"
            "You were warned of this in the setup instructions. Please buy a CP210x controller instead."
        );
        std::string text = "Cannot connect to Prolific controller.";
        m_logger.log(text, COLOR_RED);
        m_label = html_color_text(text, COLOR_RED);
        emit on_not_connected(m_label);
        return;
    }

    try{
        std::unique_ptr<SerialConnection> connection(new SerialConnection(name, PABB_BAUD_RATE));
        m_botbase.reset(new PABotBase(m_logger, std::move(connection), nullptr));
//        m_capabilities.insert(program_id_to_string(0));
    }catch (const ConnectionException& e){
        error = e.message();
    }catch (const SerialProtocolException& e){
        error = e.message();
    }catch (const ProgramCancelledException& e){
        error = e.message();
    }
    if (error.empty()){
        m_state.store(State::CONNECTING, std::memory_order_release);
        m_label = "<font color=\"green\">Connecting...</font>";
        emit on_connecting(name);
    }else{
        m_label = html_color_text("Unable to open port.", COLOR_RED);
        emit on_not_connected(m_label);
//        m_logger.log(error, Color());
        return;
    }

    m_status_thread = std::thread(run_with_catch, "BotBaseHandle::thread_body()", [this]{ thread_body(); });
}

void BotBaseHandle::stop(){
    std::lock_guard<std::mutex> lg(m_lock);
    stop_unprotected();
}
void BotBaseHandle::reset(const QSerialPortInfo* port){
    std::lock_guard<std::mutex> lg(m_lock);
    reset_unprotected(port);
}


void BotBaseHandle::thread_body(){
    using namespace PokemonAutomation;

    m_botbase->set_sniffer(&m_logger);

    //  Connect
    {
        std::string error;
        try{
            m_botbase->connect();
        }catch (InvalidConnectionStateException&){
            m_botbase->stop();
            m_label = "";
            emit on_stopped("");
            return;
        }catch (SerialProtocolException& e){
            error = e.message();
        }catch (ConnectionException& e){
            error = e.message();
        }
        if (!error.empty()){
            m_botbase->stop();
            m_label = html_color_text(error, COLOR_RED);
            emit on_stopped(m_label);
            return;
        }
    }

    //  Check protocol and version.
    {
        uint32_t version = 0;
        uint8_t program_id = 0;
        std::string error;
        try{
            process_device_protocol(version, program_id);
        }catch (InvalidConnectionStateException&){
            return;
        }catch (SerialProtocolException& e){
            error = e.message();
        }catch (ConnectionException& e){
            error = e.message();
        }
        if (error.empty()){
            m_state.store(State::READY, std::memory_order_release);
            std::string text = "Program: " + program_name(program_id) + " (" + std::to_string(version) + ")";
            m_label = html_color_text(text, theme_friendly_darkblue());
//            cout << "on_ready(): " << m_label << endl;
            emit on_ready(m_label);
        }else{
            m_state.store(State::STOPPED, std::memory_order_release);
            m_label = html_color_text(error, COLOR_RED);
            emit on_stopped(m_label);
            m_botbase->stop();
            return;
        }
    }

    //  Stop pending commands.
    m_botbase->stop_all_commands();

    std::thread watchdog([this]{
        while (true){
            if (m_state.load(std::memory_order_acquire) != State::READY){
                break;
            }

            auto last = current_time() - m_botbase->last_ack();
            std::chrono::duration<double> seconds = last;
            if (last > 2 * SERIAL_REFRESH_RATE){
                std::string text = "Last Ack: " + tostr_fixed(seconds.count(), 3) + " seconds ago";
                emit uptime_status(html_color_text(text, COLOR_RED));
//                m_logger.log("Connection issue detected. Turning on all logging...");
//                settings.log_everything.store(true, std::memory_order_release);
            }

            std::unique_lock<std::mutex> lg(m_sleep_lock);
            if (m_state.load(std::memory_order_acquire) != State::READY){
                break;
            }
            m_cv.wait_for(lg, SERIAL_REFRESH_RATE);
        }
    });

    BotBaseControllerContext context(*m_botbase);

    while (true){
        if (m_state.load(std::memory_order_acquire) != State::READY){
            break;
        }

        std::string str;
        std::string error;
        try{
//            cout << "system_clock()" << endl;
            pabb_MsgAckRequestI32 response;
            context.issue_request_and_wait(
                NintendoSwitch::DeviceRequest_system_clock()
            ).convert<PABB_MSG_ACK_REQUEST_I32>(context.controller().logger(), response);
            uint32_t wallclock = response.data;

//            uint32_t wallclock = NintendoSwitch::system_clock(context);
//            cout << "system_clock() - done" << endl;
            str = ticks_to_time(NintendoSwitch::TICKS_PER_SECOND, wallclock);
        }catch (InvalidConnectionStateException&){
            break;
        }catch (SerialProtocolException& e){
            error = e.message();
        }catch (ConnectionException& e){
            error = e.message();
        }
        if (error.empty()){
            std::string text = "Up Time: " + str;
            emit uptime_status(html_color_text(text, theme_friendly_darkblue()));
        }else{
            emit uptime_status(html_color_text(error, COLOR_RED));
            error.clear();
            std::lock_guard<std::mutex> lg(m_lock);
            State state = m_state.load(std::memory_order_acquire);
            if (state == State::READY){
                m_state.store(State::ERRORED, std::memory_order_release);
            }
            break;
        }

//        cout << "lock()" << endl;
        std::unique_lock<std::mutex> lg(m_sleep_lock);
//        cout << "lock() - done" << endl;
        if (m_state.load(std::memory_order_acquire) != State::READY){
            break;
        }
        m_cv.wait_for(lg, SERIAL_REFRESH_RATE);
    }

    {
        std::unique_lock<std::mutex> lg(m_sleep_lock);
        m_cv.notify_all();
    }
    watchdog.join();
}



}
