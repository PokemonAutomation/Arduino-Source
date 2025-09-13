/*  SerialPABotBase: Wired Controller (Switch 1)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/Options/Environment/ThemeSelectorOption.h"
#include "Controllers/SerialPABotBase/SerialPABotBase.h"
#include "Controllers/SerialPABotBase/SerialPABotBase_Routines_Protocol.h"
#include "Controllers/SerialPABotBase/SerialPABotBase_Routines_NS2_WiredController.h"
#include "NintendoSwitch_SerialPABotBase_WiredController.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{

using namespace std::chrono_literals;




SerialPABotBase_WiredController::SerialPABotBase_WiredController(
    Logger& logger,
    SerialPABotBase::SerialPABotBase_Connection& connection,
    ControllerType controller_type,
    ControllerResetMode reset_mode
)
    : ProController(logger)
    , SerialPABotBase_Controller(
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
        connection.botbase()->issue_request_and_wait(
            DeviceRequest_change_controller_mode(controller_type_to_id(controller_type)),
            nullptr
        );
        break;
    case PokemonAutomation::ControllerResetMode::RESET_AND_CLEAR_STATE:
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
SerialPABotBase_WiredController::~SerialPABotBase_WiredController(){
    stop();
}
void SerialPABotBase_WiredController::stop(){
    ProController::stop();
    m_status_thread.reset();
}




void SerialPABotBase_WiredController::execute_state(
    const Cancellable* cancellable,
    const SuperscalarScheduler::ScheduleEntry& entry
){
    if (!is_ready()){
        throw InvalidConnectionStateException(error_string());
    }

    SwitchControllerState controller_state;
    for (auto& item : entry.state){
        static_cast<const SwitchCommand&>(*item).apply(controller_state);
    }

    int dpad_x = 0;
    int dpad_y = 0;
    uint16_t buttons = 0;
    uint8_t dpad_byte = 0;

    buttons = (uint16_t)controller_state.buttons;
    if (controller_state.buttons & Button::BUTTON_UP)       dpad_y--;
    if (controller_state.buttons & Button::BUTTON_RIGHT)    dpad_x++;
    if (controller_state.buttons & Button::BUTTON_DOWN)     dpad_y++;
    if (controller_state.buttons & Button::BUTTON_LEFT)     dpad_x--;
    if (controller_state.buttons & Button::BUTTON_C)        dpad_byte |= 0x80;

    //  Merge the dpad states.
    DpadPosition dpad = controller_state.dpad;
    {
        switch (dpad){
        case DpadPosition::DPAD_UP:
            dpad_y--;
            break;
        case DpadPosition::DPAD_UP_RIGHT:
            dpad_x++;
            dpad_y--;
            break;
        case DpadPosition::DPAD_RIGHT:
            dpad_x++;
            break;
        case DpadPosition::DPAD_DOWN_RIGHT:
            dpad_x++;
            dpad_y++;
            break;
        case DpadPosition::DPAD_DOWN:
            dpad_y++;
            break;
        case DpadPosition::DPAD_DOWN_LEFT:
            dpad_x--;
            dpad_y++;
            break;
        case DpadPosition::DPAD_LEFT:
            dpad_x--;
            break;
        case DpadPosition::DPAD_UP_LEFT:
            dpad_x--;
            dpad_y--;
            break;
        default:;
        }

        if (dpad_x < 0){
            if (dpad_y < 0){
                dpad = DpadPosition::DPAD_UP_LEFT;
            }else if (dpad_y > 0){
                dpad = DpadPosition::DPAD_DOWN_LEFT;
            }else{
                dpad = DpadPosition::DPAD_LEFT;
            }
        }else if (dpad_x > 0){
            if (dpad_y < 0){
                dpad = DpadPosition::DPAD_UP_RIGHT;
            }else if (dpad_y > 0){
                dpad = DpadPosition::DPAD_DOWN_RIGHT;
            }else{
                dpad = DpadPosition::DPAD_RIGHT;
            }
        }else{
            if (dpad_y < 0){
                dpad = DpadPosition::DPAD_UP;
            }else if (dpad_y > 0){
                dpad = DpadPosition::DPAD_DOWN;
            }else{
                dpad = DPAD_NONE;
            }
        }
    }
    dpad_byte |= dpad;

    //  Divide the controller state into smaller chunks that fit into the report
    //  duration.
    Milliseconds time_left = std::chrono::duration_cast<Milliseconds>(entry.duration);

    while (time_left > Milliseconds::zero()){
        Milliseconds current = std::min(time_left, 65535ms);
        m_serial->issue_request(
            SerialPABotBase::DeviceRequest_NS2_WiredController_ControllerStateMs(
                (uint16_t)current.count(),
                buttons,
                dpad_byte,
                controller_state.left_stick_x, controller_state.left_stick_y,
                controller_state.right_stick_x, controller_state.right_stick_y
            ),
            cancellable
        );
        time_left -= current;
    }
}




void SerialPABotBase_WiredController::update_status(Cancellable& cancellable){
    pabb_MsgAckRequestI32 response;
    m_serial->issue_request_and_wait(
        SerialPABotBase::MessageControllerStatus(),
        &cancellable
    ).convert<PABB_MSG_ACK_REQUEST_I32>(m_logger, response);

    uint32_t status = response.data;
    bool status_connected = status & 1;
    bool status_ready     = status & 2;

    std::string str;
    str += "Connected: " + (status_connected
        ? html_color_text("Yes", theme_friendly_darkblue())
        : html_color_text("No", COLOR_RED)
    );
    str += " - Ready: " + (status_ready
        ? html_color_text("Yes", theme_friendly_darkblue())
        : html_color_text("No", COLOR_RED)
    );

    m_handle.set_status_line1(str);
}
void SerialPABotBase_WiredController::stop_with_error(std::string message){
    SerialPABotBase_Controller::stop_with_error(std::move(message));
}







}
}
