/*  SerialPABotBase: Wireless Controller
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Options/Environment/ThemeSelectorOption.h"
#include "Controllers/SerialPABotBase/SerialPABotBase.h"
#include "Controllers/SerialPABotBase/SerialPABotBase_Routines_Protocol.h"
#include "Controllers/SerialPABotBase/SerialPABotBase_Routines_NS1_WirelessControllers.h"
#include "NintendoSwitch_SerialPABotBase_WirelessController.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{

using namespace std::chrono_literals;







SerialPABotBase_WirelessController::SerialPABotBase_WirelessController(
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
    using namespace SerialPABotBase;

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
SerialPABotBase_WirelessController::~SerialPABotBase_WirelessController(){
    stop();
}
void SerialPABotBase_WirelessController::stop(){
    m_status_thread.reset();
}

void SerialPABotBase_WirelessController::set_info(){
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
        MessageControllerWriteSpi(
            m_controller_type,
            0x00006050, sizeof(PABB_NintendoSwitch_ControllerColors),
            &colors
        ),
        nullptr
    );
}





Button SerialPABotBase_WirelessController::populate_report_buttons(
    pabb_NintendoSwitch_WirelessController_State0x30_Buttons& buttons,
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
bool SerialPABotBase_WirelessController::populate_report_gyro(
    pabb_NintendoSwitch_WirelessController_State0x30_Gyro& gyro,
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
    return gyro_active;
}


void SerialPABotBase_WirelessController::issue_report(
    const Cancellable* cancellable,
    WallDuration duration,
    const pabb_NintendoSwitch_WirelessController_State0x30_Buttons& buttons
){
    //  We will not do any throttling or timing adjustments here. We'll defer
    //  to the microcontroller to do that for us.

    //  Divide the controller state into smaller chunks of 65535 milliseconds.
    Milliseconds time_left = std::chrono::duration_cast<Milliseconds>(duration);

    while (time_left > Milliseconds::zero()){
        Milliseconds current = std::min(time_left, 65535ms);
        m_serial->issue_request(
            SerialPABotBase::MessageControllerStateButtons(
                (uint16_t)current.count(),
                buttons
            ),
            cancellable
        );
        time_left -= current;
    }
}
void SerialPABotBase_WirelessController::issue_report(
    const Cancellable* cancellable,
    WallDuration duration,
    const pabb_NintendoSwitch_WirelessController_State0x30_Buttons& buttons,
    const pabb_NintendoSwitch_WirelessController_State0x30_Gyro& gyro
){
    //  TODO: For now we duplicate the gyro data to all 3 5ms segments.
    pabb_NintendoSwitch_WirelessController_State0x30_GyroX3 gyro3{
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
            SerialPABotBase::MessageControllerStateFull(
                (uint16_t)current.count(),
                buttons, gyro3
            ),
            cancellable
        );
        time_left -= current;
    }
}




void SerialPABotBase_WirelessController::update_status(Cancellable& cancellable){
    if (m_color_html.empty()){
        try{
            m_logger.log("Reading Controller Colors...");

            using ControllerColors = PABB_NintendoSwitch_ControllerColors;

            BotBaseMessage response = m_serial->issue_request_and_wait(
                SerialPABotBase::MessageControllerReadSpi(
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
            case ControllerType::NintendoSwitch_WirelessProController:{
                Color left(colors.left_grip[0], colors.left_grip[1], colors.left_grip[2]);
                Color body(colors.body[0], colors.body[1], colors.body[2]);
                Color right(colors.right_grip[0], colors.right_grip[1], colors.right_grip[2]);
                m_color_html += html_color_text("&#x2b24;", left);
                m_color_html += " " + html_color_text("&#x2b24;", body);
                m_color_html += " " + html_color_text("&#x2b24;", right);
                break;
            }
            case ControllerType::NintendoSwitch_LeftJoycon:
            case ControllerType::NintendoSwitch_RightJoycon:{
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


    pabb_MsgAckRequestI32 response;
    m_serial->issue_request_and_wait(
        SerialPABotBase::MessageControllerStatus(),
        &cancellable
    ).convert<PABB_MSG_ACK_REQUEST_I32>(m_logger, response);

    uint32_t status = response.data;
//            bool status_connected = status & 1;
    bool status_ready     = status & 2;
    bool status_paired    = status & 4;

    std::string str;
    str += "Paired: " + (status_paired
        ? html_color_text("Yes", theme_friendly_darkblue())
        : html_color_text("No", COLOR_RED)
    );
#if 0
    str += "Connected: " + (status_connected
        ? html_color_text("Yes", theme_friendly_darkblue())
        : html_color_text("No", COLOR_RED)
    );
#endif
    str += " - Connected: " + (status_ready
        ? html_color_text("Yes", theme_friendly_darkblue())
        : html_color_text("No", COLOR_RED)
    );
    str += " - " + m_color_html;

    m_handle.set_status_line1(str);
}

void SerialPABotBase_WirelessController::stop_with_error(std::string message){
    SerialPABotBase_Controller::stop_with_error(std::move(message));
}




}
}
