/*  PABotBase2: Wired Controller (Nintendo Switch)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/PABotBase2/Controllers/PABotBase2_Controller_NS_WiredController.h"
#include "CommonFramework/Options/Environment/ThemeSelectorOption.h"
#include "Controllers/JoystickTools.h"
#include "Controllers/SerialPABotBase/SerialPABotBase.h"
#include "NintendoSwitch_PABotBase2_WiredController.h"

//  REMOVE
#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{

using namespace PABotBase2;

using namespace std::chrono_literals;



class PABotBase2_MessageHandler_WiredController : public PABotBase2::FixedLengthMesssageHandler<
    PABB2_MESSAGE_CMD_NS_WIRED_CONTROLLER_STATE,
    pabb2_Message_Command_NS_WiredController_State
>{
public:
    virtual std::string tostr(const MessageHeader* header) const override{
        const MessageType* message = (const MessageType*)header;
        std::string str;
        str += "PABB2_MESSAGE_CMD_NS_WIRED_CONTROLLER_STATE: id = ";
        str += std::to_string(message->id);
        str += ", ms = " + std::to_string(message->milliseconds);
        return str;
    }

};



PABotBase2_WiredController::PABotBase2_WiredController(
    Logger& logger,
    PABotBase2::Connection& connection,
    ControllerType controller_type,
    ControllerResetMode reset_mode
)
    : ProController(logger)
    , PABotBase2_Controller(logger, connection)
{
    using namespace PABotBase2;

    cout << "PABotBase2_WiredController()" << endl;

    connection.device().add_message_handler<PABotBase2_MessageHandler_WiredController>();

    switch (reset_mode){
    case PokemonAutomation::ControllerResetMode::DO_NOT_RESET:
        break;
    case PokemonAutomation::ControllerResetMode::SIMPLE_RESET:{
        PABotBase2::Message_u32 message;
        message.message_bytes = sizeof(message);
        message.opcode = PABB2_MESSAGE_OPCODE_CHANGE_CONTROLLER_MODE;
        message.data = SerialPABotBase::controller_type_to_id(controller_type);
        uint8_t id = connection.device().send_request(message);
        cout << "wait... start" << endl;
        connection.device().wait_for_request_response(id);
        cout << "wait... done" << endl;
        break;
    }
    case PokemonAutomation::ControllerResetMode::RESET_AND_CLEAR_STATE:{
        PABotBase2::Message_u32 message;
        message.message_bytes = sizeof(message);
        message.opcode = PABB2_MESSAGE_OPCODE_RESET_TO_CONTROLLER;
        message.data = SerialPABotBase::controller_type_to_id(controller_type);
        uint8_t id = connection.device().send_request(message);
        cout << "wait... start" << endl;
        connection.device().wait_for_request_response(id);
        cout << "wait... done" << endl;
        break;
    }
    }

    //  Re-read the controller.
    ControllerType current_controller = connection.device().refresh_controller_type();
    if (current_controller != controller_type){
        cout << "Failed to set controller type." << endl;
        throw SerialProtocolException(logger, PA_CURRENT_FUNCTION, "Failed to set controller type.");
    }

    cout << "Starting status thread" << endl;

#if 0   //  REMOVE
    m_status_thread.reset(new ControllerStatusThread(
        connection, *this
    ));
#endif
}
PABotBase2_WiredController::~PABotBase2_WiredController(){
    stop();
}
void PABotBase2_WiredController::stop(){
    m_status_thread.reset();
}




void PABotBase2_WiredController::execute_state(
    Cancellable* cancellable,
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

    PABotBase2::pabb2_Message_Command_NS_WiredController_State request;
    request.message_bytes = sizeof(request);
    request.opcode = PABB2_MESSAGE_CMD_NS_WIRED_CONTROLLER_STATE;
    request.report.buttons0 = (uint8_t)buttons;
    request.report.buttons1 = (uint8_t)(buttons >> 8);
    request.report.dpad_byte = dpad_byte;
    request.report.left_joystick_x = JoystickTools::linear_float_to_u8(controller_state.left_joystick.x);
    request.report.left_joystick_y = JoystickTools::linear_float_to_u8(-controller_state.left_joystick.y);
    request.report.right_joystick_x = JoystickTools::linear_float_to_u8(controller_state.right_joystick.x);
    request.report.right_joystick_y = JoystickTools::linear_float_to_u8(-controller_state.right_joystick.y);

    //  Divide the controller state into smaller chunks that fit into the report
    //  duration.
    Milliseconds time_left = std::chrono::duration_cast<Milliseconds>(entry.duration);

    while (time_left > Milliseconds::zero()){
        Milliseconds current = std::min(time_left, 65535ms);
        request.milliseconds = current.count();
        m_connection.device().command_queue().send_command(request);
        time_left -= current;
    }
}



void PABotBase2_WiredController::update_status(Cancellable& cancellable){
    PABotBase2::MessageHeader request;
    request.message_bytes = sizeof(request);
    request.opcode = PABB2_MESSAGE_OPCODE_REQUEST_STATUS;
    uint8_t id = m_connection.device().send_request(request);
    PABotBase2::Message_u32 response;
    m_connection.device().wait_for_request_response<PABotBase2::Message_u32, PABB2_MESSAGE_OPCODE_RET_U32>(
        response, id
    );

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

    m_connection.set_status_line1(str);
}
void PABotBase2_WiredController::stop_with_error(std::string message){
    PABotBase2_Controller::stop_with_error(std::move(message));
}



}
}
