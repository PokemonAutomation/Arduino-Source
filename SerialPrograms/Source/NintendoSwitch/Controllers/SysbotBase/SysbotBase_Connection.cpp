/*  sys-botbase Connection
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Time.h"
#include "Common/Cpp/ColoredText.h"
#include "CommonFramework/Logging/Logger.h"
#include "CommonFramework/Tools/GlobalThreadPools.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "SysbotBase_Connection.h"

#include <charconv>
#include <cstdint>

#include "Common/Cpp/TestRunners/UnitTest.h"
#include "Common/Cpp/TestRunners/UnitTestDatabase.h"

//#include <iostream>
//using std::cout;
//using std::endl;


namespace PokemonAutomation{
namespace SysbotBase{

using namespace std::chrono_literals;



bool parse_ip_and_port(const std::string& str, QHostAddress& address, int& port){
    //  IPv4
    const size_t colon = str.find(':');
    if (colon == std::string::npos || str.find(':', colon + 1) != std::string::npos){
        return false;
    }
    address = QHostAddress(QString::fromStdString(str.substr(0, colon)));
    bool ok;
    port = QString::fromStdString(str.substr(colon + 1)).toUInt(&ok);
    if (!ok){
        return false;
    }

    //  TODO: IPv6

    return true;
}




TcpSysbotBase_Connection::TcpSysbotBase_Connection(
    Logger& logger,
    const std::string& url
)
    : m_logger(logger)
    , m_socket(GlobalThreadPools::unlimited_realtime())
    , m_last_ping_send(WallClock::min())
    , m_last_ping_receive(WallClock::min())
{
    m_controller_list.emplace_back(ControllerType::NintendoSwitch_WiredController);
    m_current_controller = ControllerType::NintendoSwitch_WiredController;

    QHostAddress address;
    int port;
    if (!parse_ip_and_port(url, address, port)){
        set_status_line0(html_color_text("Invalid IP address + port.", COLOR_RED));
        return;
    }

    set_status_line0(html_color_text("Connecting...", COLOR_DARKGREEN));

    m_connecting_message =
        "Connecting To: " + address.toString().toStdString() +
        " - Port: " + std::to_string(port);
    m_logger.log(m_connecting_message);

    //  Attach ourselves as listener first since it might immediately return as
    //  connected.
    m_socket.add_listener(*this);

    try{
        m_socket.connect(address.toString().toStdString(), port);
    }catch (...){
        m_socket.remove_listener(*this);
        throw;
    }
}

TcpSysbotBase_Connection::~TcpSysbotBase_Connection(){
    try{
        write_data("detachController\r\n");
    }catch (...){}
    m_socket.remove_listener(*this);
    m_socket.close();
    {
        std::lock_guard<Mutex> lg(m_lock);
        m_cv.notify_all();
    }
    m_thread.wait_and_ignore_exceptions();
}


void TcpSysbotBase_Connection::write_data(const std::string& data){
    WriteSpinLock lg(m_send_lock, "TcpSysbotBase_Connection::write_data()");
//    cout << "Sending: " << data << endl;
    m_socket.send(data.data(), data.size());
}


std::string pretty_print(uint64_t x){
    auto ms = x / 1000;
    auto us = x - ms * 1000;
    std::string str_ms = std::to_string(ms);
    std::string str_us = std::to_string(us);
    return str_ms + "." + std::string(3 - str_us.size(), '0') + str_us;
}


void TcpSysbotBase_Connection::thread_loop(){
    std::unique_lock<Mutex> lg(m_lock);
    m_last_ping_send = current_time();
    while (true){
        ClientSocket::State state = m_socket.state();
        if (state == ClientSocket::State::DESTRUCTING || state == ClientSocket::State::NOT_RUNNING){
            break;
        }

        WallClock now = current_time();

        std::chrono::milliseconds time_since = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_last_ping_receive);
        if (time_since > std::chrono::seconds(5)){
            std::string str = "Last Ack: " + pretty_print(time_since.count()) + " seconds ago";
            set_status_line1(str, COLOR_RED);
        }

        m_last_ping_send = current_time();
        if (NintendoSwitch::ConsoleSettings::instance().ENABLE_SBB3_PINGS){
            //  If we're more than 60 pings behind, just start clearing them.
            while (m_active_pings.size() > 60){
                m_active_pings.erase(m_active_pings.begin());
            }

            m_active_pings[m_ping_seqnum] = m_last_ping_send;
            write_data("ping " + std::to_string(m_ping_seqnum) + "\r\n");
            m_ping_seqnum++;
        }else{
            write_data("getVersion\r\n");
        }
        m_cv.wait_for(lg, std::chrono::seconds(1));
    }
}



void TcpSysbotBase_Connection::on_connect_finished(const std::string& error_message){
    try{
        if (!error_message.empty()){
            m_logger.log(m_connecting_message + " - " + error_message, COLOR_RED);
            set_status_line0(m_connecting_message, COLOR_RED);
            return;
        }

        m_logger.log(m_connecting_message + " (Success)", COLOR_BLUE);

        write_data("configure echoCommands 0\r\n");
        write_data("getVersion\r\n");
    }catch (...){}
}
void TcpSysbotBase_Connection::on_disconnect(const std::string& error_message){
    try{
        m_logger.log("sys-botbase disconnected: " + error_message, COLOR_RED);
        set_status_line1("Disconnected: " + error_message, COLOR_RED);
    }catch (...){}
}
void TcpSysbotBase_Connection::on_receive_data(const void* data, size_t bytes){
//    cout << "on_receive_data(): " << std::string((const char*)data, bytes - 2) << endl;

    WallClock now = current_time();

    try{
        const char* ptr = (const char*)data;
        for (size_t c = 0; c < bytes; c++){
            char ch = ptr[c];
            if (ch == '\r'){
                continue;
            }
            if (ch != '\n'){
                m_receive_buffer.emplace_back(ch);
                continue;
            }
            process_message(
                std::string(
                    m_receive_buffer.begin(),
                    m_receive_buffer.end()
                ),
                now
            );
            m_receive_buffer.clear();
        }

//        m_listeners.run_method(&Listener::on_receive_data, data, bytes);

    }catch (...){}
}
void TcpSysbotBase_Connection::process_message(const std::string& message, WallClock timestamp){
    if (LOG_EVERYTHING()){
        m_logger.log("Received: " + message, COLOR_DARKGREEN);
    }

    m_listeners.run_method(&Listener::on_message, message);

    //  Version #
    std::string str = message;
    if (str.find('.') != std::string::npos){
        while (!str.empty() && str.back() <= 32){
            str.pop_back();
        }
        set_status_line0("sys-botbase: Version " + str, COLOR_BLUE);

        std::lock_guard<Mutex> lg(m_lock);
        m_last_ping_receive = timestamp;

        if (m_last_ping_send != WallClock::min()){
            std::chrono::microseconds latency = std::chrono::duration_cast<std::chrono::microseconds>(timestamp - m_last_ping_send);
            std::string text = "Response Time: " + pretty_print(latency.count()) + " ms";
            if (latency < 10ms){
                set_status_line1(text, COLOR_BLUE);
            }else if (latency < 50ms){
                set_status_line1(text, COLOR_DARKGREEN);
            }else{
                set_status_line1(text, COLOR_ORANGE);
            }
        }

        if (!m_thread){
            set_mode(str);
        }
    }

    uint64_t ping_seqnum = 0;
    if (parse_ping_response(str, ping_seqnum)){
        std::lock_guard<Mutex> lg(m_lock);
        m_last_ping_receive = timestamp;
        auto iter = m_active_pings.find(ping_seqnum);
        if (iter == m_active_pings.end()){
            m_logger.log("Received Unexpected Ping: " + std::to_string(ping_seqnum));
            return;
        }

        std::chrono::microseconds latency = std::chrono::duration_cast<std::chrono::microseconds>(timestamp - iter->second);
        std::string text = "Response Time: " + pretty_print(latency.count()) + " ms";
        if (latency < 10ms){
            set_status_line1(text, COLOR_BLUE);
        }else if (latency < 50ms){
            set_status_line1(text, COLOR_DARKGREEN);
        }else{
            set_status_line1(text, COLOR_ORANGE);
        }

        m_active_pings.erase(iter);
    }

}


bool TcpSysbotBase_Connection::parse_ping_response(const std::string& str, uint64_t& seqnum){
    size_t pos = str.find("ping");
    if (pos == std::string::npos){
        return false;
    }

    //  The sequence number, if any, follows the "ping" token. A message may
    //  end right after "ping", so the scan must stop at the string's end:
    //  letting it run past the terminating NUL would read out of bounds, and
    //  the number parse that follows would then read arbitrary memory.
    size_t cursor = pos + 4;
    while (cursor < str.size() && str[cursor] <= 32){
        cursor++;
    }
    if (cursor >= str.size()){
        //  Nothing after "ping" to parse.
        return false;
    }

    //  Parse the bounded numeric token. Only a complete, in-range unsigned
    //  integer counts; an overflow or a non-numeric token cannot be one of
    //  our sequence numbers, so it is rejected.
    auto parsed = std::from_chars(str.data() + cursor, str.data() + str.size(), seqnum);
    return parsed.ec == std::errc();
}
void TcpSysbotBase_Connection::set_mode(const std::string& sbb_version){
    if (sbb_version.rfind("2.", 0) == 0){
        set_status_line1("sbb2 is no longer supported. Please upgrade to sbb3.", COLOR_RED);
        return;
    }else if (sbb_version.rfind("3.", 0) == 0){
        m_logger.log("Detected sbb3. Using CC command queue.", COLOR_BLUE);
        if (NintendoSwitch::ConsoleSettings::instance().ENABLE_SBB3_LOGGING){
            write_data("configure enableLogs 1\r\n");
        }
        write_data("configure enablePA 1\r\n");
    }else{
        m_logger.log("Unrecognized sbb version: " + sbb_version, COLOR_RED);
        return;
    }

    m_thread = GlobalThreadPools::unlimited_realtime().dispatch_now_blocking(
        [this]{ thread_loop(); }
    );
    declare_ready();
}


namespace{
class PingParseTest : public UnitTest{
public:
    PingParseTest(const std::string& name, const std::string& message, bool expected_found, uint64_t expected_seqnum);

    virtual UnitTestResult run(Logger&, CancellableScope&) const override;

private:
    std::string m_message;
    bool m_expected_found;
    uint64_t m_expected_seqnum;
};
}


PingParseTest::PingParseTest(
    const std::string& name,
    const std::string& message,
    bool expected_found,
    uint64_t expected_seqnum
)
    : UnitTest("NintendoSwitch::SysbotBase - " + name)
    , m_message(message)
    , m_expected_found(expected_found)
    , m_expected_seqnum(expected_seqnum)
{}

UnitTestResult PingParseTest::run(Logger&, CancellableScope&) const{
    uint64_t seqnum = 0;
    bool found = TcpSysbotBase_Connection::parse_ping_response(m_message, seqnum);
    if (found != m_expected_found){
        return "parse_ping_response(\"" + m_message + "\") returned " + (found ? "true" : "false")
            + ", expected " + (m_expected_found ? "true" : "false") + ".";
    }
    if (found && seqnum != m_expected_seqnum){
        return "parse_ping_response(\"" + m_message + "\") returned seqnum " + std::to_string(seqnum)
            + ", expected " + std::to_string(m_expected_seqnum) + ".";
    }
    return true;
}



void add_tests_SysbotBasePing(UnitTestDatabase& database){
    database.add<PingParseTest>("PingParse(valid)", "ping 42", true, 42);
    database.add<PingParseTest>("PingParse(zero)", "ping 0", true, 0);
    database.add<PingParseTest>("PingParse(offset ping)", "prefix ping 7", true, 7);
    database.add<PingParseTest>("PingParse(uint64 max)", "ping 18446744073709551615", true, UINT64_MAX);
    database.add<PingParseTest>("PingParse(ends in ping)", std::string(128, 'A') + "ping", false, 0);
    database.add<PingParseTest>("PingParse(bare ping)", "ping", false, 0);
    database.add<PingParseTest>("PingParse(trailing whitespace)", "ping   ", false, 0);
    database.add<PingParseTest>("PingParse(overflow)", "ping 18446744073709551616", false, 0);
    database.add<PingParseTest>("PingParse(non-numeric)", "ping abc", false, 0);
    database.add<PingParseTest>("PingParse(negative)", "ping -5", false, 0);
}







}
}
