/*  SerialPABotBase: Wireless Controller
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/Options/Environment/ThemeSelectorOption.h"
#include "Controllers/SerialPABotBase/SerialPABotBase.h"
#include "Controllers/SerialPABotBase/Messages/SerialPABotBase_MessageWrappers_BaseProtocol_ControllerMode.h"
#include "Controllers/SerialPABotBase/Messages/SerialPABotBase_MessageWrappers_BaseProtocol_Misc.h"
#include "Controllers/SerialPABotBase/Messages/SerialPABotBase_MessageWrappers_NS1_OemControllers.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch_SerialPABotBase_OemController.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{

using namespace std::chrono_literals;







SerialPABotBase_OemController::SerialPABotBase_OemController(
    Logger& logger,
    SerialPABotBase::SerialPABotBase_Connection& connection,
    ControllerType controller_type,
    ControllerResetMode reset_mode
)
    : SerialPABotBase_Controller(
        logger,
        controller_type,
        connection
    )
    , m_controller_type(controller_type)
{
//    cout << "controller_type = " << (int)controller_type << endl;

    using namespace SerialPABotBase;


    //  Add controller-specific messages.
    connection.add_message_printer<MessageType_NS1_PlayerLights>();
    connection.add_message_printer<MessageType_NS1_ReadSpi>();
    connection.add_message_printer<MessageType_NS1_WriteSpi>();
    connection.add_message_printer<MessageType_NS1_OemControllerStateButtons>();
    connection.add_message_printer<MessageType_NS1_OemControllerStateFull>();
    connection.add_message_printer<MessageType_NS1_OemControllerRumble>();


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

    switch (reset_mode){
    case PokemonAutomation::ControllerResetMode::DO_NOT_RESET:
        break;
    case PokemonAutomation::ControllerResetMode::SIMPLE_RESET:
        set_info();
        connection.botbase()->issue_request_and_wait(
            DeviceRequest_change_controller_mode(controller_type_to_id(controller_type)),
            nullptr
        );
        break;
    case PokemonAutomation::ControllerResetMode::RESET_AND_CLEAR_STATE:
        set_info();
        connection.botbase()->issue_request_and_wait(
            DeviceRequest_reset_to_controller(controller_type_to_id(controller_type)),
            nullptr
        );
        break;
    }

    //  Re-read the controller.
    ControllerType current_controller = connection.refresh_controller_type();
    if (current_controller != controller_type){
        throw SerialProtocolException(logger, PA_CURRENT_FUNCTION, "Failed to set controller type.");
    }

    m_status_thread.reset(new SerialPABotBase::ControllerStatusThread(
        connection, *this
    ));
}
SerialPABotBase_OemController::~SerialPABotBase_OemController(){
    stop();
}
void SerialPABotBase_OemController::stop(){
    m_status_thread.reset();
}

void SerialPABotBase_OemController::set_info(){
    using namespace SerialPABotBase;

    uint8_t controller_mac_address[6] = {};
    {
        BotBaseMessage response = m_serial->issue_request_and_wait(
            DeviceRequest_read_mac_address(controller_type_to_id(m_controller_type)),
            nullptr
        );
        if (response.body.size() == sizeof(seqnum_t) + sizeof(controller_mac_address)){
            memcpy(
                controller_mac_address,
                response.body.data() + sizeof(seqnum_t),
                sizeof(controller_mac_address)
            );

            std::string str = "Controller MAC Address: ";

            static const char HEX_DIGITS[] = "0123456789abcdef";
            for (size_t c = 0; c < 6; c++){
                uint8_t byte = controller_mac_address[c];
                str += " ";
                str += HEX_DIGITS[(byte >> 4)];
                str += HEX_DIGITS[byte & 15];
            }

            m_logger.log(str);

        }else{
            m_logger.log(
                "Invalid response size to PABB_MSG_ESP32_REQUEST_READ_SPI: body = " + std::to_string(response.body.size()),
                COLOR_RED
            );
        }
    }

    NintendoSwitch::ControllerProfile profile =
        PokemonAutomation::NintendoSwitch::ConsoleSettings::instance().CONTROLLER_SETTINGS.get_or_make_profile(
            controller_mac_address,
            m_handle.device_name(),
            m_controller_type
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


    m_serial->issue_request_and_wait(
        DeviceRequest_NS1_WriteSpi(
            m_controller_type,
            0x00006050, sizeof(PABB_NintendoSwitch_ControllerColors),
            &colors
        ),
        nullptr
    );
}





Button SerialPABotBase_OemController::populate_report_buttons(
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
bool SerialPABotBase_OemController::populate_report_gyro(
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


void SerialPABotBase_OemController::issue_report(
    Cancellable* cancellable,
    WallDuration duration,
    const pabb_NintendoSwitch_OemController_State0x30_Buttons& buttons
){
    //  We will not do any throttling or timing adjustments here. We'll defer
    //  to the microcontroller to do that for us.

    //  Divide the controller state into smaller chunks of 65535 milliseconds.
    Milliseconds time_left = std::chrono::duration_cast<Milliseconds>(duration);

    while (time_left > Milliseconds::zero()){
        Milliseconds current = std::min(time_left, 65535ms);
        m_serial->issue_request(
            DeviceRequest_ControllerStateButtons(
                (uint16_t)current.count(),
                buttons
            ),
            cancellable
        );
        time_left -= current;
    }
}
void SerialPABotBase_OemController::issue_report(
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

    while (time_left > Milliseconds::zero()){
        Milliseconds current = std::min(time_left, 65535ms);
        m_serial->issue_request(
            DeviceRequest_ControllerStateFull(
                (uint16_t)current.count(),
                buttons, gyro3
            ),
            cancellable
        );
        time_left -= current;
    }
}




void SerialPABotBase_OemController::update_status(Cancellable& cancellable){
    if (m_color_html.empty()){
        try{
            m_logger.log("Reading Controller Colors...");

            using ControllerColors = PABB_NintendoSwitch_ControllerColors;

            BotBaseMessage response = m_serial->issue_request_and_wait(
                DeviceRequest_NS1_ReadSpi(
                    m_controller_type,
                    0x00006050, sizeof(ControllerColors)
                ),
                &cancellable
            );

            ControllerColors colors{};
            if (response.body.size() == sizeof(seqnum_t) + sizeof(ControllerColors)){
                memcpy(&colors, response.body.data() + sizeof(seqnum_t), sizeof(ControllerColors));
            }else{
                m_logger.log(
                    "Invalid response size to PABB_MSG_ESP32_REQUEST_READ_SPI: body = " + std::to_string(response.body.size()),
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
        pabb_MsgAckRequestI32 response;
        m_serial->issue_request_and_wait(
            SerialPABotBase::MessageControllerStatus(),
            &cancellable
        ).convert<PABB_MSG_ACK_REQUEST_I32>(m_logger, response);
        status = response.data;
    }

    if (protocol_version() >= 2025120806){
        uint8_t mac_address[6] = {};
        BotBaseMessage response = m_serial->issue_request_and_wait(
            SerialPABotBase::DeviceRequest_paired_mac_address(m_controller_type),
            &cancellable
        );
        if (response.body.size() == sizeof(seqnum_t) + sizeof(mac_address)){
            memcpy(
                mac_address,
                response.body.data() + sizeof(seqnum_t),
                sizeof(mac_address)
            );
        }else{
            m_logger.log(
                "Invalid response size to PABB_MSG_ESP32_REQUEST_READ_SPI: body = " + std::to_string(response.body.size()),
                COLOR_RED
            );
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
    }else{
        bool status_paired = status & 4;
        str += "Paired: " + (status_paired
            ? html_color_text("Yes", theme_friendly_darkblue())
            : html_color_text("No", COLOR_RED)
        );
    }

    bool status_ready = status & 2;
    if (protocol_version() >= 2025120806 && status_ready){
        pabb_MsgAckRequestI8 response;
        m_serial->issue_request_and_wait(
            DeviceRequest_PlayerLights(m_controller_type),
            &cancellable
        ).convert<PABB_MSG_ACK_REQUEST_I8>(m_logger, response);

        uint8_t byte = response.data;
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

//    bool status_connected = status & 1;
#if 0
    str += "Connected: " + (status_connected
        ? html_color_text("Yes", theme_friendly_darkblue())
        : html_color_text("No", COLOR_RED)
    );
#endif

    m_handle.set_status_line1(str);
}

void SerialPABotBase_OemController::stop_with_error(std::string message){
    SerialPABotBase_Controller::stop_with_error(std::move(message));
}




}
}
