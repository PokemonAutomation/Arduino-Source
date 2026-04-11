/*  PABotBase2: OEM Controller (Nintendo Switch 1)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/PABotBase2/Controllers/PABotBase2_Controller_NS1_OemController.h"
#include "CommonFramework/Options/Environment/ThemeSelectorOption.h"
#include "Controllers/SerialPABotBase/SerialPABotBase.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch_PABotBase2_OemController.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{

using namespace std::chrono_literals;



PABotBase2_OemController::PABotBase2_OemController(
    Logger& logger,
    PABotBase2::Connection& connection,
    ControllerType controller_type
)
    : PABotBase2_Controller(logger, connection)
    , m_controller_type(controller_type)
{
    using namespace PABotBase2;


    switch (controller_type){
    case ControllerType::NintendoSwitch_WiredProController:
    case ControllerType::NintendoSwitch_WiredLeftJoycon:
    case ControllerType::NintendoSwitch_WiredRightJoycon:
        m_performance_class = ControllerPerformanceClass::SerialPABotBase_Wired;
        m_ticksize = 0ms;
        m_cooldown = 8ms;
        m_timing_variation = ConsoleSettings::instance().TIMING_OPTIONS.WIRED;
        break;
    case ControllerType::NintendoSwitch_WirelessProController:
    case ControllerType::NintendoSwitch_WirelessLeftJoycon:
    case ControllerType::NintendoSwitch_WirelessRightJoycon:
        m_performance_class = ControllerPerformanceClass::SerialPABotBase_Wireless;
        m_ticksize = 0ms;
        m_cooldown = 15ms;
        m_timing_variation = ConsoleSettings::instance().TIMING_OPTIONS.WIRELESS;
        break;

    default:
        throw InternalProgramError(&logger, PA_CURRENT_FUNCTION, "Unsupported controller.");
    }


    //  Add controller-specific messages.
    connection.device().message_logger().add_message<pabb2_Message_NS1_OemController_Spi>(
        "PABB_MSG_REQ_NS1_OEM_CONTROLLER_READ_SPI",
        PABB_MSG_REQ_NS1_OEM_CONTROLLER_READ_SPI,
        true,
        [](const pabb2_Message_NS1_OemController_Spi* message){
            std::string str;
            str += "id = " + std::to_string(message->id);
            str += ", controller_type = " + std::to_string(message->controller_type);
            str += ", address = 0x" + tostr_hex(message->address);
            str += ", bytes = " + std::to_string(message->bytes);
            return str;
        }
    );
    connection.device().message_logger().add_message<pabb2_Message_NS1_OemController_Spi>(
        "PABB_MSG_REQ_NS1_OEM_CONTROLLER_WRITE_SPI",
        PABB_MSG_REQ_NS1_OEM_CONTROLLER_WRITE_SPI,
        true,
        [](const pabb2_Message_NS1_OemController_Spi* message){
            std::string str;
            str += "id = " + std::to_string(message->id);
            str += ", controller_type = " + std::to_string(message->controller_type);
            str += ", address = 0x" + tostr_hex(message->address);
            str += ", bytes = " + std::to_string(message->bytes);
            return str;
        }
    );
    connection.device().message_logger().add_message<MessageHeader>(
        "PABB2_MESSAGE_REQ_NS1_OEM_CONTROLLER_PLAYER_LIGHTS",
        PABB2_MESSAGE_REQ_NS1_OEM_CONTROLLER_PLAYER_LIGHTS,
        true,
        [](const MessageHeader* header){
            std::string str;
            str += "id = " + std::to_string(header->id);
            return str;
        }
    );
    connection.device().message_logger().add_message<Message_u32>(
        "PABB2_MESSAGE_INFO_NS1_OEM_CONTROLLER_PLAYER_LIGHTS",
        PABB2_MESSAGE_INFO_NS1_OEM_CONTROLLER_PLAYER_LIGHTS,
        true,
        [](const Message_u32* message){
            std::string str;
            str += "id = " + std::to_string(message->id);
            str += ", lights = 0x" + tostr_hex(message->data);
            return str;
        }
    );
    connection.device().message_logger().add_message<MessageHeader>(
        "PABB2_MESSAGE_INFO_NS1_OEM_CONTROLLER_USB_DISALLOWED",
        PABB2_MESSAGE_INFO_NS1_OEM_CONTROLLER_USB_DISALLOWED,
        true,
        [](const MessageHeader* header){
            std::string str;
            str += "id = " + std::to_string(header->id);
            return str;
        }
    );
    connection.device().message_logger().add_message<pabb2_Message_Feedback_NS1_OemController_Rumble>(
        "CONTROLLER_RUMBLE",
        PABB2_MESSAGE_INFO_NS1_OEM_CONTROLLER_RUMBLE,
        true,
        [](const pabb2_Message_Feedback_NS1_OemController_Rumble* message){
            std::string str;
            str += tostr_hexbytes(&message->data, sizeof(pabb_NintendoSwitch_Rumble));
            return str;
        }
    );
    connection.device().message_logger().add_message<pabb2_Message_Command_NS1_OemController_Buttons>(
        "PABB2_MESSAGE_CMD_NS1_OEM_CONTROLLER_BUTTONS",
        PABB2_MESSAGE_CMD_NS1_OEM_CONTROLLER_BUTTONS,
        false,
        [](const pabb2_Message_Command_NS1_OemController_Buttons* message){
            std::string str;
            str += "id = " + std::to_string(message->id);
            str += ", ms = " + std::to_string(message->milliseconds);
            return str;
        }
    );
    connection.device().message_logger().add_message<pabb2_Message_Command_NS1_OemController_FullState>(
        "PABB2_MESSAGE_CMD_NS1_OEM_CONTROLLER_FULL_STATE",
        PABB2_MESSAGE_CMD_NS1_OEM_CONTROLLER_FULL_STATE,
        false,
        [](const pabb2_Message_Command_NS1_OemController_FullState* message){
            std::string str;
            str += "id = " + std::to_string(message->id);
            str += ", ms = " + std::to_string(message->milliseconds);
            return str;
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


void PABotBase2_OemController::run_preconnect_configure(
    Logger& logger,
    PABotBase2::Connection& connection,
    ControllerType controller_type
){
    using namespace PABotBase2;

    uint8_t controller_mac_address[6] = {};
    {
        Message_u32 request;
        request.message_bytes = sizeof(request);
        request.opcode = PABB2_MESSAGE_OPCODE_CONTROLLER_MAC_ADDRESS;
        request.data = SerialPABotBase::controller_type_to_id(controller_type);
        uint8_t id = connection.device().send_request(request);
        std::string response = connection.device().wait_for_request_response(id);
        if (response.size() == sizeof(MessageHeader) + sizeof(controller_mac_address)){
            memcpy(
                controller_mac_address,
                response.data() + sizeof(MessageHeader),
                sizeof(controller_mac_address)
            );
            logger.log("Controller MAC Address:" + tostr_hexbytes(controller_mac_address, sizeof(controller_mac_address)));
        }else{
            logger.log(
                "Invalid response size to PABB2_MESSAGE_OPCODE_PAIRED_MAC_ADDRESS: body = " + std::to_string(response.size()),
                COLOR_RED
            );
        }
    }

    NintendoSwitch::ControllerProfile profile =
        PokemonAutomation::NintendoSwitch::ConsoleSettings::instance().CONTROLLER_SETTINGS.get_or_make_profile(
            controller_mac_address,
            "",
            controller_type
        );

    PABB_NintendoSwitch_ControllerColors colors;
    {
        Color color(profile.body_color);
        colors.body[0] = color.red();
        colors.body[1] = color.green();
        colors.body[2] = color.blue();
    }
    {
        Color color(profile.button_color);
        colors.buttons[0] = color.red();
        colors.buttons[1] = color.green();
        colors.buttons[2] = color.blue();
    }
    {
        Color color(profile.left_grip);
        colors.left_grip[0] = color.red();
        colors.left_grip[1] = color.green();
        colors.left_grip[2] = color.blue();
    }
    {
        Color color(profile.right_grip);
        colors.right_grip[0] = color.red();
        colors.right_grip[1] = color.green();
        colors.right_grip[2] = color.blue();
    }


    pabb2_Message_NS1_OemController_Spi request;
    request.message_bytes = sizeof(request) + sizeof(colors);
    request.opcode = PABB_MSG_REQ_NS1_OEM_CONTROLLER_WRITE_SPI;
    request.controller_type = SerialPABotBase::controller_type_to_id(controller_type);
    request.address = 0x00006050;
    request.bytes = sizeof(PABB_NintendoSwitch_ControllerColors);

    connection.device().connection().reliable_send(&request, sizeof(request), Milliseconds(100));
    connection.device().connection().reliable_send(&colors, sizeof(colors), Milliseconds(100));
}




Button PABotBase2_OemController::populate_report_buttons(
    pabb_NintendoSwitch_OemController_State0x30_Buttons& buttons,
    const SwitchControllerState& controller_state
){
    //  https://github.com/dekuNukem/Nintendo_Switch_Reverse_Engineering/blob/master/bluetooth_hid_notes.md

    Button all_buttons = BUTTON_NONE;
    for (size_t c = 0; c < TOTAL_BUTTONS; c++){
        Button button = (Button)((ButtonFlagType)1 << c);
        if (!(controller_state.buttons & button)){
            continue;
        }

        all_buttons |= button;
        switch (button){
        //  Right
        case BUTTON_Y:          buttons.button3 |= 1 << 0; break;
        case BUTTON_X:          buttons.button3 |= 1 << 1; break;
        case BUTTON_B:          buttons.button3 |= 1 << 2; break;
        case BUTTON_A:          buttons.button3 |= 1 << 3; break;
        case BUTTON_RIGHT_SR:   buttons.button3 |= 1 << 4; break;
        case BUTTON_RIGHT_SL:   buttons.button3 |= 1 << 5; break;
        case BUTTON_R:          buttons.button3 |= 1 << 6; break;
        case BUTTON_ZR:         buttons.button3 |= 1 << 7; break;

        //  Shared
        case BUTTON_MINUS:      buttons.button4 |= 1 << 0; break;
        case BUTTON_PLUS:       buttons.button4 |= 1 << 1; break;
        case BUTTON_RCLICK:     buttons.button4 |= 1 << 2; break;
        case BUTTON_LCLICK:     buttons.button4 |= 1 << 3; break;
        case BUTTON_HOME:       buttons.button4 |= 1 << 4; break;
        case BUTTON_CAPTURE:    buttons.button4 |= 1 << 5; break;

        //  Left
        case BUTTON_DOWN:       buttons.button5 |= 1 << 0; break;
        case BUTTON_UP:         buttons.button5 |= 1 << 1; break;
        case BUTTON_RIGHT:      buttons.button5 |= 1 << 2; break;
        case BUTTON_LEFT:       buttons.button5 |= 1 << 3; break;
        case BUTTON_LEFT_SR:    buttons.button5 |= 1 << 4; break;
        case BUTTON_LEFT_SL:    buttons.button5 |= 1 << 5; break;
        case BUTTON_L:          buttons.button5 |= 1 << 6; break;
        case BUTTON_ZL:         buttons.button5 |= 1 << 7; break;

        default:;
        }
    }
    return all_buttons;
}
bool PABotBase2_OemController::populate_report_gyro(
    pabb_NintendoSwitch_OemController_State0x30_Gyro& gyro,
    const SwitchControllerState& controller_state
){
    gyro.accel_x = controller_state.gyro[0];
    gyro.accel_y = controller_state.gyro[1];
    gyro.accel_z = controller_state.gyro[2];
    gyro.rotation_x = controller_state.gyro[3];
    gyro.rotation_y = controller_state.gyro[4];
    gyro.rotation_z = controller_state.gyro[5];

    bool gyro_active = false;
    gyro_active |= gyro.accel_x != 0;
    gyro_active |= gyro.accel_y != 0;
    gyro_active |= gyro.accel_z != 0;
    gyro_active |= gyro.rotation_x != 0;
    gyro_active |= gyro.rotation_y != 0;
    gyro_active |= gyro.rotation_z != 0;
//    cout << "gyro_active = " << gyro_active << endl;
    return gyro_active;
}


void PABotBase2_OemController::issue_report(
    Cancellable* cancellable,
    WallDuration duration,
    const pabb_NintendoSwitch_OemController_State0x30_Buttons& buttons
){
    //  We will not do any throttling or timing adjustments here. We'll defer
    //  to the microcontroller to do that for us.

    //  Divide the controller state into smaller chunks of 65535 milliseconds.
    Milliseconds time_left = std::chrono::duration_cast<Milliseconds>(duration);

    PABotBase2::pabb2_Message_Command_NS1_OemController_Buttons request;
    request.message_bytes = sizeof(request);
    request.opcode = PABB2_MESSAGE_CMD_NS1_OEM_CONTROLLER_BUTTONS;
    request.buttons = buttons;

    while (time_left > Milliseconds::zero()){
        Milliseconds current = std::min(time_left, 65535ms);
        request.milliseconds = current.count();
        m_connection.device().command_queue().send_command(request);
        time_left -= current;
    }
}
void PABotBase2_OemController::issue_report(
    Cancellable* cancellable,
    WallDuration duration,
    const pabb_NintendoSwitch_OemController_State0x30_Buttons& buttons,
    const pabb_NintendoSwitch_OemController_State0x30_Gyro& gyro
){
    //  TODO: For now we duplicate the gyro data to all 3 5ms segments.
    pabb_NintendoSwitch_OemController_State0x30_GyroX3 gyro3{
        gyro, gyro, gyro
    };

#if 0
    //  Purturb results to show the Switch that they are not stuck.
    if (gyro3.time1.accel_x > 0){
        gyro3.time0.accel_x--;
        gyro3.time2.accel_x--;
    }else if (gyro3.time1.accel_x < 0){
        gyro3.time0.accel_x++;
        gyro3.time2.accel_x++;
    }else{
        gyro3.time0.accel_x--;
        gyro3.time2.accel_x++;
    }
    if (gyro3.time1.accel_y > 0){
        gyro3.time0.accel_y--;
        gyro3.time2.accel_y--;
    }else if (gyro3.time1.accel_y < 0){
        gyro3.time0.accel_y++;
        gyro3.time2.accel_y++;
    }else{
        gyro3.time0.accel_y--;
        gyro3.time2.accel_y++;
    }
    if (gyro3.time1.accel_z > 0){
        gyro3.time0.accel_z--;
        gyro3.time2.accel_z--;
    }else if (gyro3.time1.accel_z < 0){
        gyro3.time0.accel_z++;
        gyro3.time2.accel_z++;
    }else{
        gyro3.time0.accel_z--;
        gyro3.time2.accel_z++;
    }
#endif

    //  We will not do any throttling or timing adjustments here. We'll defer
    //  to the microcontroller to do that for us.

    //  Divide the controller state into smaller chunks of 65535 milliseconds.
    Milliseconds time_left = std::chrono::duration_cast<Milliseconds>(duration);

    PABotBase2::pabb2_Message_Command_NS1_OemController_FullState request;
    request.message_bytes = sizeof(request);
    request.opcode = PABB2_MESSAGE_CMD_NS1_OEM_CONTROLLER_FULL_STATE;
    request.state.buttons = buttons;
    request.state.gyro = gyro3;

    while (time_left > Milliseconds::zero()){
        Milliseconds current = std::min(time_left, 65535ms);
        request.milliseconds = current.count();
        m_connection.device().command_queue().send_command(request);
        time_left -= current;
    }
}


void PABotBase2_OemController::update_status(Cancellable& cancellable){
    using namespace PABotBase2;

    if (m_color_html.empty()){
        try{
            m_logger.log("Reading Controller Colors...");

            using ControllerColors = PABB_NintendoSwitch_ControllerColors;

            pabb2_Message_NS1_OemController_Spi message;
            message.message_bytes = sizeof(pabb2_Message_NS1_OemController_Spi);
            message.opcode = PABB_MSG_REQ_NS1_OEM_CONTROLLER_READ_SPI;
            message.controller_type = SerialPABotBase::controller_type_to_id(m_controller_type);
            message.address = 0x00006050;
            message.bytes = sizeof(ControllerColors);

            uint8_t id = m_connection.device().send_request(message);
            std::string response = m_connection.device().wait_for_request_response(id);

            ControllerColors colors{};
            if (response.size() == sizeof(MessageHeader) + sizeof(ControllerColors)){
                memcpy(&colors, response.data() + sizeof(MessageHeader), sizeof(ControllerColors));
            }else{
                m_logger.log(
                    "Invalid response size to PABB_MSG_REQ_NS1_OEM_CONTROLLER_READ_SPI: body = " + std::to_string(response.size()),
                    COLOR_RED
                );
//                m_handle.set_status_line1("Error: See log for more information.", COLOR_RED);
                return;
            }
            m_logger.log("Reading Controller Colors... Done");

            switch (m_controller_type){
            case ControllerType::NintendoSwitch_WiredProController:
            case ControllerType::NintendoSwitch_WirelessProController:{
                Color left(colors.left_grip[0], colors.left_grip[1], colors.left_grip[2]);
                Color body(colors.body[0], colors.body[1], colors.body[2]);
                Color right(colors.right_grip[0], colors.right_grip[1], colors.right_grip[2]);
                m_color_html += html_color_text("&#x2b24;", left);
                m_color_html += " " + html_color_text("&#x2b24;", body);
                m_color_html += " " + html_color_text("&#x2b24;", right);
                break;
            }
            case ControllerType::NintendoSwitch_WiredLeftJoycon:
            case ControllerType::NintendoSwitch_WiredRightJoycon:
            case ControllerType::NintendoSwitch_WirelessLeftJoycon:
            case ControllerType::NintendoSwitch_WirelessRightJoycon:{
                Color body(colors.body[0], colors.body[1], colors.body[2]);
                m_color_html = html_color_text("&#x2b24;", body);
                break;
            }
            default:;
            }

        }catch (Exception& e){
            e.log(m_logger);
            throw;
        }
    }

    std::string str;
    str += m_color_html + " - ";

    uint32_t status;
    {
        MessageHeader request;
        request.message_bytes = sizeof(request);
        request.opcode = PABB2_MESSAGE_OPCODE_REQUEST_STATUS;
        uint8_t id = m_connection.device().send_request(request);
        Message_u32 response;
        m_connection.device().wait_for_request_response<Message_u32, PABB2_MESSAGE_OPCODE_RET_U32>(
            response, id
        );
        status = response.data;
    }

    uint8_t mac_address[6] = {};
    {
        Message_u32 request;
        request.message_bytes = sizeof(request);
        request.opcode = PABB2_MESSAGE_OPCODE_PAIRED_MAC_ADDRESS;
        request.data = SerialPABotBase::controller_type_to_id(m_controller_type);
        uint8_t id = m_connection.device().send_request(request);
        std::string response = m_connection.device().wait_for_request_response(id);
        if (response.size() == sizeof(MessageHeader) + sizeof(mac_address)){
            memcpy(
                mac_address,
                response.data() + sizeof(MessageHeader),
                sizeof(mac_address)
            );
        }else{
            m_logger.log(
                "Invalid response size to PABB2_MESSAGE_OPCODE_PAIRED_MAC_ADDRESS: body = " + std::to_string(response.size()),
                COLOR_RED
            );
        }

    }

    str += "Paired: ";
    if (std::all_of(mac_address, mac_address + 6, [](uint8_t x){ return x == 0; })){
        str += html_color_text("No", COLOR_RED);
    }else{
        str += html_color_text(
            tostr_hex(mac_address[4]) + ":" +
            tostr_hex(mac_address[5]),
            theme_friendly_darkblue()
        );
    }

    bool status_ready = status & 2;
    if (status_ready){
        uint8_t byte = (uint8_t)(status >> 24);
        byte = (byte | (byte >> 4)) & 0x0f;
        str += " - Connected: ";
        for (int c = 0; c < 4; c++){
            str += html_color_text("\u258d", byte & (1 << c) ? COLOR_GREEN : COLOR_BLACK);
        }
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
