/*  Serial Port (PABotBase) Connection
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QSerialPortInfo>
#include <QMessageBox>
#include "Common/Cpp/PanicDump.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/Tools/GlobalThreadPools.h"
#include "SerialPABotBase2_Connection.h"

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
    cancel(nullptr);
    m_ready.store(false, std::memory_order_release);


    m_connect_thread.wait_and_ignore_exceptions();
}




void SerialPABotBase2_Connection::connect_thread_body(bool set_to_null_controller){
    {
        std::string text = "Opening serial port...";
        m_logger.log(text);
        set_status_line0(text, COLOR_DARKGREEN);
    }

    if (cancelled()){
        return;
    }
    QSerialPortInfo info(QString::fromStdString(m_device_name));

    //  Port is invalid.
    if (info.isNull()){
        std::string text = "Serial port " + m_device_name + " is invalid.";
        m_logger.log(text, COLOR_RED);
        set_status_line0(text, COLOR_RED);
        return;
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
        return;
    }

    if (cancelled()){
        return;
    }
    m_unreliable_connection = std::make_unique<SerialConnection>(
        GlobalThreadPools::unlimited_realtime(),
        info.systemLocation().toStdString(),
        115200  //  Temporary. Will increase to 460800 in the future.
    );

    {
        std::string text = "Opening up reliable channel...";
        m_logger.log(text);
        set_status_line0(text, COLOR_DARKGREEN);
    }
    m_stream_connection = std::make_unique<ReliableStreamConnection>(
        this,
        m_logger, GlobalSettings::instance().LOG_EVERYTHING,
        GlobalThreadPools::unlimited_realtime(),
        *m_unreliable_connection,
        std::chrono::milliseconds(80),
        nullptr
    );

    m_logger.log("Resetting device connection state...");
    m_stream_connection->reset();

    //  TODO

}




}
}
