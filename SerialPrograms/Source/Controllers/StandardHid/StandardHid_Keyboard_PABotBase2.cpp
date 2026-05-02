/*  PABotBase2: HID Keyboard
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/PABotBase2/Controllers/PABotBase2_Controller_HID_Keyboard.h"
#include "CommonFramework/Options/Environment/ThemeSelectorOption.h"
#include "StandardHid_Keyboard_PABotBase2.h"

namespace PokemonAutomation{
namespace StandardHid{

using namespace std::chrono_literals;



PABotBase2_Keyboard::PABotBase2_Keyboard(
    Logger& logger,
    PABotBase2::Connection& connection
)
    : Keyboard(logger)
    , KeyboardControllerWithScheduler(logger)
        , m_connection(connection)
{
    using namespace PABotBase2;

    //  Add controller-specific messages.
    connection.message_logger().add_message<pabb2_Message_Command_HID_Keyboard_State>(
        "PABB2_MESSAGE_CMD_HID_KEYBOARD_STATE",
        PABB2_MESSAGE_CMD_HID_KEYBOARD_STATE,
        false,
        [](const pabb2_Message_Command_HID_Keyboard_State* message){
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
PABotBase2_Keyboard::~PABotBase2_Keyboard(){
    stop();
}
void PABotBase2_Keyboard::stop(){
    m_status_thread.reset();
}



void PABotBase2_Keyboard::update_status(Cancellable& cancellable){
    PABotBase2::MessageHeader request;
    request.message_bytes = sizeof(request);
    request.opcode = PABB2_MESSAGE_OPCODE_REQUEST_STATUS;
    uint8_t id = m_connection.device().send_request_with_response(request);
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
void PABotBase2_Keyboard::stop_with_error(std::string error_message){
    try{
        WriteSpinLock lg(m_error_lock);
        m_error_string = error_message;
    }catch (...){}
    m_connection.cancel();
    m_connection.set_status_line1(std::move(error_message), COLOR_RED);
}


void PABotBase2_Keyboard::cancel_all_commands(){
    std::lock_guard<Mutex> lg(m_state_lock);
    if (!is_ready()){
        throw InvalidConnectionStateException(error_string());
    }
    m_logger.log("cancel_all_commands()", COLOR_DARKGREEN);
    m_connection.device().command_queue().send_cancel();
    m_scheduler.clear_on_next();
}
void PABotBase2_Keyboard::replace_on_next_command(){
    std::lock_guard<Mutex> lg(m_state_lock);
    if (!is_ready()){
        throw InvalidConnectionStateException(error_string());
    }
    m_logger.log("replace_on_next_command()", COLOR_DARKGREEN);
    m_connection.device().command_queue().send_replace_on_next();
    m_scheduler.clear_on_next();
}


void PABotBase2_Keyboard::wait_for_all(Cancellable* cancellable){
    SuperscalarScheduler::Schedule schedule;
    std::lock_guard<Mutex> lg0(m_issue_lock);
    {
        std::lock_guard<Mutex> lg1(m_state_lock);

        ThrottleScope scope(m_logging_throttler);
        if (scope){
            m_logger.log("wait_for_all()", COLOR_DARKGREEN);
        }

        if (!is_ready()){
            throw InvalidConnectionStateException(error_string());
        }

        m_scheduler.issue_wait_for_all(schedule);
    }
    execute_schedule(cancellable, schedule);
    m_connection.device().command_queue().wait_for_all(cancellable);
}


void PABotBase2_Keyboard::execute_state(
    Cancellable* cancellable,
    const SuperscalarScheduler::ScheduleEntry& entry
){
    std::map<KeyboardKey, uint64_t> state;
    for (const auto& item : entry.state){
        const KeyboardCommand& key = static_cast<const KeyboardCommand&>(*item);
//        cout << (int)key << endl;
        state.emplace(key.key(), key.seqnum());
    }

//    cout << "last = " << m_last_state.size() << ", now = " << state.size() << endl;

    //  Add the modifier keys.
    PABotBase2::pabb2_Message_Command_HID_Keyboard_State request;
    request.message_bytes = sizeof(request);
    request.opcode = PABB2_MESSAGE_CMD_HID_KEYBOARD_STATE;
    request.report = {};
    if (state.contains(KeyboardKey::KEY_LEFT_CTRL)){
        state.erase(KeyboardKey::KEY_LEFT_CTRL);
        request.report.modifiers |= 1 << 0;
    }
    if (state.contains(KeyboardKey::KEY_LEFT_SHIFT)){
        state.erase(KeyboardKey::KEY_LEFT_SHIFT);
        request.report.modifiers |= 1 << 1;
    }
    if (state.contains(KeyboardKey::KEY_LEFT_ALT)){
        state.erase(KeyboardKey::KEY_LEFT_ALT);
        request.report.modifiers |= 1 << 2;
    }
    if (state.contains(KeyboardKey::KEY_LEFT_META)){
        state.erase(KeyboardKey::KEY_LEFT_CTRL);
        request.report.modifiers |= 1 << 3;
    }
    if (state.contains(KeyboardKey::KEY_RIGHT_CTRL)){
        state.erase(KeyboardKey::KEY_RIGHT_CTRL);
        request.report.modifiers |= 1 << 4;
    }
    if (state.contains(KeyboardKey::KEY_RIGHT_SHIFT)){
        state.erase(KeyboardKey::KEY_RIGHT_SHIFT);
        request.report.modifiers |= 1 << 5;
    }
    if (state.contains(KeyboardKey::KEY_RIGHT_ALT)){
        state.erase(KeyboardKey::KEY_RIGHT_ALT);
        request.report.modifiers |= 1 << 6;
    }
    if (state.contains(KeyboardKey::KEY_RIGHT_META)){
        state.erase(KeyboardKey::KEY_RIGHT_META);
        request.report.modifiers |= 1 << 7;
    }

    //  Last State: Remove keys that are no longer pressed.
    //  Current State: Remove keys that were already pressed.
    for (auto iterL = m_last_state.begin(); iterL != m_last_state.end();){
        auto iterS = state.find(iterL->second);
        if (iterS == state.end()){
            iterL = m_last_state.erase(iterL);
        }else{
            state.erase(iterS);
            ++iterL;
        }
    }

    //  Add the new keys to the state.
    for (const auto& key : state){
        m_last_state.emplace(key.second, key.first);
    }

    //  Get a searchable set of all currently pressed keys.
    state.clear();
    for (const auto& item : m_last_state){
        state.emplace(item.second, item.first);
    }

//    cout << "modifiers = " << (int)report.modifiers << endl;

    //  Add the remaining keys.
    auto iter = m_last_state.begin();
    for (size_t c = 0; c < 6; c++){
        if (iter == m_last_state.end()){
            break;
        }
        request.report.key[c] = iter->second;
        ++iter;
    }

//    cout << m_last_state.size() << endl;
    if (m_last_state.size() > 6){
        m_logger.log("Unable to send more than 6 simultaneous keyboard presses due to HID limitation.", COLOR_RED);
    }

    //  Divide the controller state into smaller chunks that fit into the report
    //  duration.
    Milliseconds time_left = std::chrono::duration_cast<Milliseconds>(entry.duration);

    while (time_left > Milliseconds::zero()){
        Milliseconds current = std::min(time_left, 65535ms);
        request.milliseconds = current.count();
        m_connection.device().command_queue().send_command(cancellable, request);
        time_left -= current;
    }
}




}
}
