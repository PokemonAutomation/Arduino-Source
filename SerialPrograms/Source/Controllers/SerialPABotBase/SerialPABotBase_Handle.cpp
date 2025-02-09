/*  Serial Port (PABotBase) Handle
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <set>
#include <QtGlobal>
#include <QSerialPortInfo>
#include <QMessageBox>
#include "Common/Cpp/PrettyPrint.h"
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/PanicDump.h"
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

//  REMOVE
#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{

using namespace SerialPABotBase;





BotBaseHandle::BotBaseHandle(SerialLogger& logger, const QSerialPortInfo* port)
    : m_logger(logger)
    , m_port(port)
    , m_ready(false)
{
    set_label_text("Not Connected", COLOR_RED);

    //  No port selected.
    if (port == nullptr || port->isNull()){
        return;
    }

    //  Prolific is banned.
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
        set_label_text(text, COLOR_RED);
        return;
    }

    std::string name = port->systemLocation().toUtf8().data();
    std::string error;
    try{
        set_label_text("Connecting...", COLOR_DARKGREEN);

        std::unique_ptr<SerialConnection> connection(new SerialConnection(name, PABB_BAUD_RATE));
        m_botbase.reset(new PABotBase(m_logger, std::move(connection), nullptr));
    }catch (const ConnectionException& e){
        error = e.message();
    }catch (const SerialProtocolException& e){
        error = e.message();
    }catch (const ProgramCancelledException& e){
        error = e.message();
    }
    if (!error.empty()){
        set_label_text("Unable to open port.", COLOR_RED);
        return;
    }

    m_status_thread = std::thread(run_with_catch, "BotBaseHandle::thread_body()", [this]{ thread_body(); });
}
BotBaseHandle::~BotBaseHandle(){
    m_ready.store(false, std::memory_order_release);
    signal_pre_not_ready();
    if (m_botbase == nullptr){
        return;
    }
    m_botbase->stop();
    {
        std::lock_guard<std::mutex> lg(m_lock);
        m_cv.notify_all();
    }
    if (m_status_thread.joinable()){
        m_status_thread.join();
    }
    m_botbase.reset();
}

BotBaseController* BotBaseHandle::botbase(){
    BotBaseController* ret = m_botbase.get();
    if (ret == nullptr){
        m_logger.log("BotBaseHandle::botbase() called with null botbase...", COLOR_RED);
    }
    return ret;
}

bool BotBaseHandle::is_ready() const{
    return m_ready.load(std::memory_order_relaxed);
}


void BotBaseHandle::set_label_text(const std::string& text, Color color){
    m_label = html_color_text(text, color);

    std::string status = m_label;
    if (!m_uptime.empty()){
        if (!status.empty()){
            status += "<br>";
        }
        status += m_uptime;
    }
    set_status(status);
}
void BotBaseHandle::set_uptime_text(const std::string& text, Color color){
    m_uptime = html_color_text(text, color);

    std::string status = m_label;
    if (!m_uptime.empty()){
        if (!status.empty()){
            status += "<br>";
        }
        status += m_uptime;
    }
    set_status(status);
}




std::set<std::string> BotBaseHandle::read_device_specs(uint32_t& protocol, uint8_t& program_id){
    Logger& logger = m_logger;
    std::set<std::string> capabilities;


    //  Protocol
    logger.log("Checking device protocol compatibility...");
    protocol = Microcontroller::protocol_version(*m_botbase);
    logger.log("Checking device protocol compatibility... Protocol = " + std::to_string(protocol));

    //  (protocol_requested / 100) == (protocol_device / 100)
    //  (protocol_requested % 100) <= (protocol_device / 100)
    auto protocol_iter = SUPPORTED_VERSIONS.upper_bound(protocol);
    if (protocol_iter == SUPPORTED_VERSIONS.begin()){
        throw SerialProtocolException(
            logger, PA_CURRENT_FUNCTION,
            "Incompatible protocol. Device: " + std::to_string(protocol) + "<br>"
            "Please install the .hex that came with this version of the program."
        );
    }
    --protocol_iter;
    if (protocol_iter->first < protocol / 100 * 100){
        throw SerialProtocolException(
            logger, PA_CURRENT_FUNCTION,
            "Incompatible protocol. Device: " + std::to_string(protocol) + "<br>"
            "Please install the .hex that came with this version of the program."
        );
    }
    const std::map<uint32_t, std::set<Features>>& PROGRAM_IDS = protocol_iter->second;


    //  Program ID
    logger.log("Checking Program ID...");
    program_id = Microcontroller::program_id(*m_botbase);
    logger.log("Checking Program ID... Program ID = " + std::to_string(program_id));

    auto program_iter = PROGRAM_IDS.find(program_id);
    if (program_iter == PROGRAM_IDS.end()){
        throw SerialProtocolException(
            logger, PA_CURRENT_FUNCTION,
            "Unrecognized Program ID: " + std::to_string(program_id) + "<br>"
            "Please install the .hex that came with this version of the program."
        );
    }
    const std::set<Features>& FEATURES = program_iter->second;


    //  Program Version
    logger.log("Checking Firmware Version...");
    uint32_t version = Microcontroller::program_version(*m_botbase);
    logger.log("Checking Firmware Version... Version = " + std::to_string(version));


    //  REMOVE: Temporary for migration.
    if (protocol / 100 == 20210526 && version == 2023121900){
        capabilities.insert(to_string(Features::QueueSize));
    }


    //  Compile list of capabilities.
    for (Features feature : program_iter->second){
        capabilities.insert(to_string(feature));
    }
    if (FEATURES.contains(Features::QueueSize)){
        capabilities.insert(to_string(Features::QueueSize));
    }


    //  Enable queue size (if available).
    if (capabilities.contains(to_string(Features::QueueSize))){
        logger.log("Device supports queue size. Requesting queue size...", COLOR_BLUE);
        uint8_t queue_limit = Microcontroller::device_queue_size(*m_botbase);
        logger.log("Setting queue size to: " + std::to_string(queue_limit), COLOR_BLUE);
        m_botbase->set_queue_limit(queue_limit);
    }else{
        logger.log("Queue size not supported. Defaulting to size 4.", COLOR_RED);
    }

    return capabilities;
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
            set_label_text("");
            return;
        }catch (SerialProtocolException& e){
            error = e.message();
        }catch (ConnectionException& e){
            error = e.message();
        }
        if (!error.empty()){
            m_botbase->stop();
            set_label_text(error, COLOR_RED);
            return;
        }
    }

    //  Check protocol and version.
    {
        uint32_t protocol = 0;
        uint8_t program_id = 0;
        std::set<std::string> capabilities;
        std::string error;
        try{
            capabilities = read_device_specs(protocol, program_id);
            std::lock_guard<std::mutex> lg(m_lock);
            m_capabilities = capabilities;
        }catch (InvalidConnectionStateException&){
            return;
        }catch (SerialProtocolException& e){
            error = e.message();
        }catch (ConnectionException& e){
            error = e.message();
        }
        if (error.empty()){
            m_ready.store(true, std::memory_order_release);
            std::string text = "Program: " + program_name(program_id) + " (" + std::to_string(protocol) + ")";
            set_label_text(text, theme_friendly_darkblue());
            signal_post_ready(capabilities);
        }else{
            m_ready.store(false, std::memory_order_relaxed);
            set_label_text(error, COLOR_RED);
            signal_pre_not_ready();
            m_botbase->stop();
            return;
        }
    }

    //  Stop pending commands.
    m_botbase->stop_all_commands();

    std::thread watchdog([this]{
        while (true){
            if (!m_ready.load(std::memory_order_relaxed)){
                break;
            }

            auto last = current_time() - m_botbase->last_ack();
            std::chrono::duration<double> seconds = last;
            if (last > 2 * SERIAL_REFRESH_RATE){
                std::string text = "Last Ack: " + tostr_fixed(seconds.count(), 3) + " seconds ago";
                set_uptime_text(text, COLOR_RED);
//                m_logger.log("Connection issue detected. Turning on all logging...");
//                settings.log_everything.store(true, std::memory_order_release);
            }

            std::unique_lock<std::mutex> lg(m_sleep_lock);
            if (!m_ready.load(std::memory_order_relaxed)){
                break;
            }
            m_cv.wait_for(lg, SERIAL_REFRESH_RATE);
        }
    });

    CancellableHolder<CancellableScope> scope;

    while (true){
        if (!m_ready.load(std::memory_order_relaxed)){
            break;
        }

        std::string str;
        std::string error;
        try{
            pabb_MsgAckRequestI32 response;
            m_botbase->issue_request_and_wait(
                NintendoSwitch::DeviceRequest_system_clock(),
                &scope
            ).convert<PABB_MSG_ACK_REQUEST_I32>(m_logger, response);
            uint32_t wallclock = response.data;
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
            set_uptime_text(text, theme_friendly_darkblue());
        }else{
            set_uptime_text(error, COLOR_RED);
            error.clear();
            m_ready.store(false, std::memory_order_relaxed);
            break;
        }

//        cout << "lock()" << endl;
        std::unique_lock<std::mutex> lg(m_sleep_lock);
//        cout << "lock() - done" << endl;
        if (!m_ready.load(std::memory_order_relaxed)){
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
