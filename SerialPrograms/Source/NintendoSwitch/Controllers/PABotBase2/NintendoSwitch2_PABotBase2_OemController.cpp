/*  PABotBase2: OEM Controller (Nintendo Switch 2)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/ColoredText.h"
#include "Common/PABotBase2/Controllers/PABotBase2_Controller_NS2_OemController.h"
#include "Controllers/SerialPort/SerialPABotBase.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch2_PABotBase2_OemController.h"

namespace PokemonAutomation{
namespace NintendoSwitch2{

using namespace std::chrono_literals;

void PABotBase2_OemController::add_message_loggers(PABotBase2::MessageLogger& message_logger){
    using namespace PABotBase2;

    message_logger.add_message<Message_NS2_OemController_Flash>(
        "PABB2_MESSAGE_REQ_NS2_OEM_CONTROLLER_READ_FLASH",
        PABB2_MESSAGE_REQ_NS2_OEM_CONTROLLER_READ_FLASH,
        true,
        [](const Message_NS2_OemController_Flash* message){
            std::string str;
            str += "id = " + std::to_string(message->id);
            str += ", controller_type = 0x" + tostr_hex(message->controller_type);
            str += ", address = 0x" + tostr_hex(message->address);
            str += ", bytes = " + std::to_string(message->bytes);
            return str;
        }
    );
    message_logger.add_message_min_length<Message_NS2_OemController_Flash>(
        "PABB2_MESSAGE_REQ_NS2_OEM_CONTROLLER_WRITE_FLASH",
        PABB2_MESSAGE_REQ_NS2_OEM_CONTROLLER_WRITE_FLASH,
        true,
        [](const Message_NS2_OemController_Flash* message){
            std::string str;
            str += "id = " + std::to_string(message->id);
            str += ", controller_type = 0x" + tostr_hex(message->controller_type);
            str += ", address = 0x" + tostr_hex(message->address);
            str += ", bytes = " + std::to_string(message->bytes);
            return str;
        }
    );
    message_logger.add_message<Message_Command_NS2_OemController_Buttons0x05>(
        "PABB2_MESSAGE_CMD_NS2_OEM_CONTROLLER_BUTTONS_0X05",
        PABB2_MESSAGE_CMD_NS2_OEM_CONTROLLER_BUTTONS_0X05,
        false,
        [](const Message_Command_NS2_OemController_Buttons0x05* message){
            std::string str;
            str += "id = " + std::to_string(message->id);
            str += ", ms = " + std::to_string(message->milliseconds);
            return str;
        }
    );
    message_logger.add_message<Message_Command_NS2_OemController_FullState0x05>(
        "PABB2_MESSAGE_CMD_NS2_OEM_CONTROLLER_FULL_STATE_0X05",
        PABB2_MESSAGE_CMD_NS2_OEM_CONTROLLER_FULL_STATE_0X05,
        false,
        [](const Message_Command_NS2_OemController_FullState0x05* message){
            std::string str;
            str += "id = " + std::to_string(message->id);
            str += ", ms = " + std::to_string(message->milliseconds);
            return str;
        }
    );
    message_logger.add_message<Message_Command_NS2_OemController_Buttons0x07>(
        "PABB2_MESSAGE_CMD_NS2_OEM_CONTROLLER_BUTTONS_0X07",
        PABB2_MESSAGE_CMD_NS2_OEM_CONTROLLER_BUTTONS_0X07,
        false,
        [](const Message_Command_NS2_OemController_Buttons0x07* message){
            std::string str;
            str += "id = " + std::to_string(message->id);
            str += ", ms = " + std::to_string(message->milliseconds);
            return str;
        }
    );
    message_logger.add_message<Message_Command_NS2_OemController_FullState0x07>(
        "PABB2_MESSAGE_CMD_NS2_OEM_CONTROLLER_FULL_STATE_0X07",
        PABB2_MESSAGE_CMD_NS2_OEM_CONTROLLER_FULL_STATE_0X07,
        false,
        [](const Message_Command_NS2_OemController_FullState0x07* message){
            std::string str;
            str += "id = " + std::to_string(message->id);
            str += ", ms = " + std::to_string(message->milliseconds);
            return str;
        }
    );
    message_logger.add_message<Message_Command_NS2_OemController_Buttons0x08>(
        "PABB2_MESSAGE_CMD_NS2_OEM_CONTROLLER_BUTTONS_0X08",
        PABB2_MESSAGE_CMD_NS2_OEM_CONTROLLER_BUTTONS_0X08,
        false,
        [](const Message_Command_NS2_OemController_Buttons0x08* message){
            std::string str;
            str += "id = " + std::to_string(message->id);
            str += ", ms = " + std::to_string(message->milliseconds);
            return str;
        }
    );
    message_logger.add_message<Message_Command_NS2_OemController_FullState0x08>(
        "PABB2_MESSAGE_CMD_NS2_OEM_CONTROLLER_FULL_STATE_0X08",
        PABB2_MESSAGE_CMD_NS2_OEM_CONTROLLER_FULL_STATE_0X08,
        false,
        [](const Message_Command_NS2_OemController_FullState0x08* message){
            std::string str;
            str += "id = " + std::to_string(message->id);
            str += ", ms = " + std::to_string(message->milliseconds);
            return str;
        }
    );
    message_logger.add_message<Message_Command_NS2_OemController_Buttons0x09>(
        "PABB2_MESSAGE_CMD_NS2_OEM_CONTROLLER_BUTTONS_0X09",
        PABB2_MESSAGE_CMD_NS2_OEM_CONTROLLER_BUTTONS_0X09,
        false,
        [](const Message_Command_NS2_OemController_Buttons0x09* message){
            std::string str;
            str += "id = " + std::to_string(message->id);
            str += ", ms = " + std::to_string(message->milliseconds);
            return str;
        }
    );
    message_logger.add_message<Message_Command_NS2_OemController_FullState0x09>(
        "PABB2_MESSAGE_CMD_NS2_OEM_CONTROLLER_FULL_STATE_0X09",
        PABB2_MESSAGE_CMD_NS2_OEM_CONTROLLER_FULL_STATE_0X09,
        false,
        [](const Message_Command_NS2_OemController_FullState0x09* message){
            std::string str;
            str += "id = " + std::to_string(message->id);
            str += ", ms = " + std::to_string(message->milliseconds);
            return str;
        }
    );
    message_logger.add_message<Message_Command_NS2_OemController_Buttons0x0A>(
        "PABB2_MESSAGE_CMD_NS2_OEM_CONTROLLER_BUTTONS_0X0A",
        PABB2_MESSAGE_CMD_NS2_OEM_CONTROLLER_BUTTONS_0X0A,
        false,
        [](const Message_Command_NS2_OemController_Buttons0x0A* message){
            std::string str;
            str += "id = " + std::to_string(message->id);
            str += ", ms = " + std::to_string(message->milliseconds);
            return str;
        }
    );
    message_logger.add_message<Message_Command_NS2_OemController_FullState0x0A>(
        "PABB2_MESSAGE_CMD_NS2_OEM_CONTROLLER_FULL_STATE_0X0A",
        PABB2_MESSAGE_CMD_NS2_OEM_CONTROLLER_FULL_STATE_0X0A,
        false,
        [](const Message_Command_NS2_OemController_FullState0x0A* message){
            std::string str;
            str += "id = " + std::to_string(message->id);
            str += ", ms = " + std::to_string(message->milliseconds);
            return str;
        }
    );
}





PABotBase2_OemController::PABotBase2_OemController(
    Logger& logger,
    RecursiveThrottler& logging_throttler,
    PABotBase2::Connection& connection,
    ControllerType controller_type,
    std::function<void(double magnitude)> on_rumble
)
    : PABotBase2_Controller(logger, logging_throttler, connection)
    , m_controller_type(controller_type)
    , m_on_rumble(std::move(on_rumble))
    , m_player_number(ControllerPlayerNumber::UNKNOWN)
{
    using namespace PABotBase2;


    switch (controller_type){
    case ControllerType::NintendoSwitch2_WiredProController:
    case ControllerType::NintendoSwitch2_WiredLeftJoycon:
    case ControllerType::NintendoSwitch2_WiredRightJoycon:
        m_performance_class = ControllerPerformanceClass::SerialPABotBase_Wired;
        m_ticksize = 0ms;
        m_cooldown = 5ms;
        m_timing_variation = NintendoSwitch::ConsoleSettings::instance().TIMING_OPTIONS.WIRED;
        break;
    case ControllerType::NintendoSwitch2_WirelessProController:
    case ControllerType::NintendoSwitch2_WirelessLeftJoycon:
    case ControllerType::NintendoSwitch2_WirelessRightJoycon:
        m_performance_class = ControllerPerformanceClass::SerialPABotBase_Wireless;
        m_ticksize = 0ms;
        m_cooldown = 5ms;
        m_timing_variation = NintendoSwitch::ConsoleSettings::instance().TIMING_OPTIONS.WIRELESS;
        break;

    default:
        throw InternalProgramError(&logger, PA_CURRENT_FUNCTION, "Unsupported controller.");
    }


    //  Add controller-specific messages.

    add_message_loggers(connection.message_logger());

    connection.device().add_message_handler(
        PABB2_MESSAGE_OPCODE_CONSOLE_DISCONNECT,
        [this](const MessageHeader* header){
            if (header->message_bytes != sizeof(Message_u32)){
                m_logger.log(
                    "PABB2_MESSAGE_OPCODE_CONSOLE_DISCONNECT: **(invalid size = " + std::to_string(header->message_bytes) + ")**",
                    COLOR_RED
                );
                return;
            }
            const auto* message = (const Message_u32*)header;
            WriteSpinLock lg(m_error_lock);
            switch ((DisconnectReason)message->data){
            case DisconnectReason::HOST_SHUTDOWN:
                m_error_string = "Console turned off.";
                break;
            case DisconnectReason::CONNECTION_REJECTED:
                m_error_string = "Rejected by console. Please re-pair the controller.";
                break;
            default:
                m_error_string = "Disconnected by console.";
            }

        }
    );


    m_status_thread.reset(new ControllerStatusThread(
        connection, *this
    ));
}
PABotBase2_OemController::~PABotBase2_OemController(){
    stop();
}
void PABotBase2_OemController::stop(){
    m_status_thread.reset();
}


bool PABotBase2_OemController::run_preconnect_configure(
    Logger& logger,
    PABotBase2::Connection& connection,
    ControllerType controller_type
){
    using namespace PABotBase2;

    add_message_loggers(connection.message_logger());

    WallClock deadline = current_time() + std::chrono::milliseconds(100);

    uint8_t controller_mac_address[6] = {};
    {
        Message_u32 request;
        request.message_bytes = sizeof(request);
        request.opcode = PABB2_MESSAGE_OPCODE_CONTROLLER_MAC_ADDRESS;
        request.data = SerialPABotBase::controller_type_to_id(controller_type);
        std::optional<uint8_t> id = connection.device().try_send_request_with_response(
            request,
            deadline
        );
        if (!id.has_value()){
            return false;
        }
        std::string response = connection.device().wait_for_request_response(
            id.value(),
            deadline
        );
        if (response.size() == sizeof(MessageHeader) + sizeof(controller_mac_address)){
            memcpy(
                controller_mac_address,
                response.data() + sizeof(MessageHeader),
                sizeof(controller_mac_address)
            );
            logger.log("Controller MAC Address: " + tostr_hexbytes(controller_mac_address, sizeof(controller_mac_address)));
        }else if (response.empty()){
            logger.log(
                "Timed out waiting for response to PABB2_MESSAGE_OPCODE_PAIRED_MAC_ADDRESS.",
                COLOR_RED
            );
            return false;
        }else{
            logger.log(
                "Invalid response size to PABB2_MESSAGE_OPCODE_PAIRED_MAC_ADDRESS: body = " + std::to_string(response.size()),
                COLOR_RED
            );
            return false;
        }
    }

    return true;
}



NintendoSwitch::ControllerPlayerNumber PABotBase2_OemController::get_player_number(Cancellable& cancellable){
    update_status(cancellable);
    return m_player_number.load(std::memory_order_relaxed);
}








void PABotBase2_OemController::update_status(Cancellable& cancellable){
    using namespace PABotBase2;

//    cout << m_connection.device().dump_pending_requests() << endl;



    std::string str;
//    str += m_color_html + " - ";


    PABotBase2::MessageHeader request;
    request.message_bytes = sizeof(request);
    request.opcode = PABB2_MESSAGE_OPCODE_REQUEST_STATUS;
    uint8_t id = m_connection.device().send_request_with_response(request);
    std::string response = m_connection.device().wait_for_request_response_min_size<
        PABotBase2::Message_u32, PABB2_MESSAGE_OPCODE_RET_U32_DATA
    >(id);

    const PABotBase2::Message_u32* header = (const PABotBase2::Message_u32*)response.data();

    switch (header->data){
    case PABB_CID_NintendoSwitch2_WiredProController:
    case PABB_CID_NintendoSwitch2_WiredLeftJoycon:
    case PABB_CID_NintendoSwitch2_WiredRightJoycon:
    case PABB_CID_NintendoSwitch2_WirelessProController:
    case PABB_CID_NintendoSwitch2_WirelessLeftJoycon:
    case PABB_CID_NintendoSwitch2_WirelessRightJoycon:
        break;
    default:
        WriteSpinLock lg(m_error_lock);
        if (m_error_string.empty()){
            m_connection.set_status_line1("");
        }else{
            m_connection.set_status_line1(m_error_string, COLOR_RED);
        }
        return;
    }

    constexpr size_t EXPECTED_SIZE = sizeof(PABotBase2::Message_u32) + sizeof(OemController_Status);
    if (response.size() != EXPECTED_SIZE){
        throw SerialProtocolException(
            m_logger, PA_CURRENT_FUNCTION,
            "Received Incorrect Response Size: Expected = " + std::to_string(EXPECTED_SIZE) +
            ", Actual = " + std::to_string(response.size())
        );
    }

    const OemController_Status& status = *(const OemController_Status*)(header + 1);

    str += "Paired: ";
    if (status.status & 4){
        str += html_color_text(
            tostr_hex_padded(2, status.paired_mac_address[4]) + ":" +
            tostr_hex_padded(2, status.paired_mac_address[5]),
            theme_friendly_darkblue()
        );
    }else{
        str += html_color_text("No", COLOR_RED);
    }

    bool status_ready = status.status & 2;
    if (status_ready){
        uint8_t byte = status.player_lights;
        byte = (byte | (byte >> 4)) & 0x0f;
        str += " - Connected: ";
        for (int c = 0; c < 4; c++){
            str += html_color_text("\u258d", byte & (1 << c) ? COLOR_GREEN : COLOR_BLACK);
        }
        ControllerPlayerNumber player = ControllerPlayerNumber::UNKNOWN;
        switch (byte){
        case 0b0000: player = ControllerPlayerNumber::DISCONNECTED; break;
        case 0b0001: player = ControllerPlayerNumber::PLAYER1; break;
        case 0b0011: player = ControllerPlayerNumber::PLAYER2; break;
        case 0b0111: player = ControllerPlayerNumber::PLAYER3; break;
        case 0b1111: player = ControllerPlayerNumber::PLAYER4; break;
        case 0b1001: player = ControllerPlayerNumber::PLAYER5; break;
        case 0b0101: player = ControllerPlayerNumber::PLAYER6; break;
        case 0b1101: player = ControllerPlayerNumber::PLAYER7; break;
        case 0b0110: player = ControllerPlayerNumber::PLAYER8; break;
        }
        m_player_number.store(player, std::memory_order_relaxed);
    }else{
        str += " - Connected: " + (status_ready
            ? html_color_text("Yes", theme_friendly_darkblue())
            : html_color_text("No", COLOR_RED)
        );
    }

    WriteSpinLock lg(m_error_lock);
    if (m_error_string.empty()){
        m_connection.set_status_line1(str);
    }else{
        m_connection.set_status_line1(m_error_string, COLOR_RED);
    }
}
void PABotBase2_OemController::stop_with_error(std::string message){
    PABotBase2_Controller::stop_with_error(std::move(message));
}














}
}
