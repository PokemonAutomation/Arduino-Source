/*  Serial Port (PABotBase) Connection
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QSerialPortInfo>
#include <QMessageBox>
#include "Common/Cpp/PrettyPrint.h"
#include "Common/Cpp/PanicDump.h"
#include "Common/PABotBase2/ReliableConnectionLayer/PABotBase2_PacketProtocol.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/Options/Environment/ThemeSelectorOption.h"
#include "CommonFramework/Tools/GlobalThreadPools.h"
#include "Controllers/SerialPABotBase/SerialPABotBase.h"
#include "SerialPABotBase2_Connection.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace SerialPABotBase{




SerialPABotBase2_Connection::SerialPABotBase2_Connection(
    Logger& logger,
    std::string name,
    bool set_to_null_controller
)
    : m_logger(logger, GlobalSettings::instance().LOG_EVERYTHING)
    , m_device_name(std::move(name))
{
    set_status_line0("Not Connected", COLOR_RED);

    m_connect_thread = GlobalThreadPools::unlimited_normal().dispatch_now_blocking([=, this]{
        run_with_catch(
            "SerialPABotBase_Connection::connect_thread_body()",
            [=, this]{ connect_thread_body(set_to_null_controller); }
        );
    });
};
SerialPABotBase2_Connection::~SerialPABotBase2_Connection(){
    SerialPABotBase2_Connection::cancel(nullptr);
    m_device.reset();
}
bool SerialPABotBase2_Connection::cancel(std::exception_ptr exception) noexcept{
    if (Connection::cancel(std::move(exception))){
        return true;
    }
    m_ready.store(false, std::memory_order_release);
    m_connect_thread.wait_and_ignore_exceptions();
    return false;
}



ControllerType SerialPABotBase2_Connection::refresh_controller_type(){
    ControllerType ret = m_device->refresh_controller_type();
    m_current_controller.store(ret, std::memory_order_release);
    return ret;
}


bool SerialPABotBase2_Connection::open_serial_port(){
    {
        std::string text = "Opening serial port...";
        m_logger.log(text);
        set_status_line0(text, COLOR_DARKGREEN);
    }

    if (cancelled()){
        return false;
    }
    QSerialPortInfo info(QString::fromStdString(m_device_name));

    //  Port is invalid.
    if (info.isNull()){
        std::string text = "Serial port " + m_device_name + " is invalid.";
        m_logger.log(text, COLOR_RED);
        set_status_line0(text, COLOR_RED);
        return false;
    }

    //  Prolific is banned
    if (info.description().indexOf("Prolific") != -1){
        QMessageBox box;
        box.critical(
            nullptr,
            "Error",
            "Cannot select Prolific controller.<br><br>"
            "Prolific controllers do not work for Arduino and similar microcontrollers.<br>"
            "You were warned of this in the setup instructions. Please buy a CP210x controller instead."
        );
        std::string text = "Cannot connect to Prolific controller.";
        m_logger.log(text, COLOR_RED);
        set_status_line0(text, COLOR_RED);
        return false;
    }

    if (cancelled()){
        return false;
    }

    m_unreliable_connection = std::make_unique<SerialConnection>(
        GlobalThreadPools::unlimited_realtime(),
        info.systemLocation().toStdString(),
        115200
    );

    return true;
}
bool SerialPABotBase2_Connection::connect_to_device(){
    const uint32_t BAUD_RATES[] = {
        921600,
        115200,
    };
    std::string str;
    WallClock start = current_time();
    while (current_time() - start < std::chrono::seconds(5)){
        m_logger.log("Trying baud " + tostr_u_commas(921600) + " (no session ID)...");
        m_unreliable_connection->set_baud_rate(921600);
        if (m_stream_connection->reset(false, std::chrono::milliseconds(100))){
            return true;
        }

        for (size_t c = 0; c < sizeof(BAUD_RATES) / sizeof(uint32_t); c++){
            uint32_t baud_rate = BAUD_RATES[c];
            m_logger.log("Trying baud " + tostr_u_commas(baud_rate) + " (with session ID)...");
            m_unreliable_connection->set_baud_rate(baud_rate);
            if (m_stream_connection->reset(true, std::chrono::milliseconds(100))){
                return true;
            }
        }
    }
    str =
        "Unable to connect to controller.<br>"
        "Please make sure your microcontroller is flashed properly and<br>"
        "you are using the correct mode (PABotBase vs. PABotBase2).<br>" +
        make_text_url(ONLINE_DOC_URL_BASE + "SetupGuide/Reflash.html", "See documentation for more details.");
    set_status_line0(str, COLOR_RED);
    return false;
}
bool SerialPABotBase2_Connection::open_serial_connection(){
    {
        std::string text = "Opening up reliable channel...";
        m_logger.log(text);
        set_status_line0(text, COLOR_DARKGREEN);
    }
    m_stream_connection = std::make_unique<PABotBase2::ReliableStreamConnection>(
        static_cast<CancellableScope*>(this),
        m_logger, GlobalSettings::instance().LOG_EVERYTHING,
        GlobalThreadPools::unlimited_realtime(),
        *m_unreliable_connection,
        std::chrono::milliseconds(80),
        nullptr
    );

    {
        std::string text = "Connecting...";
        m_logger.log(text);
        set_status_line0(text, COLOR_DARKGREEN);
    }
    if (!connect_to_device()){
        return false;
    }

    m_stream_connection->send_request(PABB2_CONNECTION_OPCODE_ASK_VERSION);
    m_stream_connection->wait_for_pending();
    if (!m_stream_connection->remote_protocol_is_compatible()){
        std::string str =
            "Incompatible RSC protocol. Device: " + std::to_string(m_stream_connection->remote_protocol()) + "<br>"
            "Please flash your microcontroller (e.g. ESP32, Pico W...)<br>"
            "with the .bin/.uf2/.hex that came with this version of the program.<br>" +
            make_text_url(ONLINE_DOC_URL_BASE + "SetupGuide/Reflash.html", "See documentation for more details.");
        set_status_line0(str, COLOR_RED);
        return false;
    }
    std::string error = m_stream_connection->error_string();
    if (!error.empty()){
        set_status_line0(error, COLOR_RED);
        return false;
    }

    m_stream_connection->send_request(PABB2_CONNECTION_OPCODE_ASK_PACKET_SIZE);
    m_stream_connection->wait_for_pending();

    m_stream_connection->send_request(PABB2_CONNECTION_OPCODE_ASK_BUFFER_SLOTS);
    m_stream_connection->wait_for_pending();

    return true;
}
bool SerialPABotBase2_Connection::open_device_connection(bool set_to_null_controller){
    {
        std::string text = "Querying device...";
        m_logger.log(text);
        set_status_line0(text, COLOR_DARKGREEN);
    }
    {
//        std::lock_guard<Mutex> lg(m_lock);
        m_device = std::make_unique<PABotBase2::DeviceHandle>(
            static_cast<CancellableScope*>(this),
            m_logger, *m_stream_connection
        );
    }

    m_device->connect();
    m_controller_list = m_device->controller_list();

    set_status_line0(
        m_device->device_name() + " (" + std::to_string(m_device->device_firmware_version()) + ")",
        theme_friendly_darkblue()
    );


    //  Current Controller
    ControllerType current_controller = refresh_controller_type();

    if (set_to_null_controller && current_controller != ControllerType::None){
        PABotBase2::Message_u32 request;
        request.message_bytes = sizeof(request);
        request.opcode = PABB2_MESSAGE_OPCODE_CHANGE_CONTROLLER_MODE;
        request.data = SerialPABotBase::controller_type_to_id(ControllerType::None);
        m_device->send_request_with_no_response(request);

#if 0
        m_botbase->issue_request_and_wait(
            DeviceRequest_change_controller_mode(PABB_CID_NONE),
            nullptr
        );
#endif
        refresh_controller_type();
    }

    return true;
}
void SerialPABotBase2_Connection::connect_thread_body(bool set_to_null_controller){
    try{
        if (!open_serial_port()){
            return;
        }
        if (!open_serial_connection()){
            return;
        }
        if (!open_device_connection(set_to_null_controller)){
            return;
        }
        declare_ready();
    }catch (Exception& e){
        set_status_line0(e.message(), COLOR_RED);
    }
}




}
}
