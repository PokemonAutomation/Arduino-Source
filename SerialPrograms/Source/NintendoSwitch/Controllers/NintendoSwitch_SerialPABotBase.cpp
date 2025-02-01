/*  Nintendo Switch Controller (Serial PABotBase)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Json/JsonValue.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "Controllers/ControllerDescriptor.h"
#include "Controllers/SerialPABotBase/SerialPABotBase.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Messages_PushButtons.h"
//#include "NintendoSwitch/Commands/NintendoSwitch_Messages_Superscalar.h"
#include "NintendoSwitch_SerialPABotBase.h"

//#include <iostream>
//using std::cout;
//using std::endl;


namespace PokemonAutomation{


using namespace std::chrono_literals;



template <>
std::vector<std::shared_ptr<const ControllerDescriptor>>
ControllerType_t<NintendoSwitch::SwitchController_SerialPABotBase_Descriptor>::list() const{
    std::vector<std::shared_ptr<const ControllerDescriptor>> ret;
    for (QSerialPortInfo& port : QSerialPortInfo::availablePorts()){
#ifdef _WIN32
        //  COM1 is never the correct port on Windows.
        if (port.portName() == "COM1"){
            continue;
        }
#endif
        ret.emplace_back(new NintendoSwitch::SwitchController_SerialPABotBase_Descriptor(port));
    }
    return ret;
}
template class ControllerType_t<NintendoSwitch::SwitchController_SerialPABotBase_Descriptor>;




namespace NintendoSwitch{





const char* SwitchController_SerialPABotBase_Descriptor::TYPENAME = SerialPABotBase::NintendoSwitch_Basic;



bool SwitchController_SerialPABotBase_Descriptor::operator==(const ControllerDescriptor& x) const{
    if (typeid(*this) != typeid(x)){
        return false;
    }
    return m_port.portName() == static_cast<const SwitchController_SerialPABotBase_Descriptor&>(x).m_port.portName();
}


const char* SwitchController_SerialPABotBase_Descriptor::type_name() const{
    return TYPENAME;
}
std::string SwitchController_SerialPABotBase_Descriptor::display_name() const{
    if (m_port.isNull()){
        return "";
    }
    return m_port.portName().toStdString() + " - " + m_port.description().toStdString();
//    return "Serial (PABotBase): " + m_port.portName().toStdString() + " - " + m_port.description().toStdString();;
}
void SwitchController_SerialPABotBase_Descriptor::load_json(const JsonValue& json){
    const std::string* name = json.to_string();
    if (name == nullptr || name->empty()){
        return;
    }
    m_port = (QSerialPortInfo(QString::fromStdString(*name)));
}
JsonValue SwitchController_SerialPABotBase_Descriptor::to_json() const{
    return m_port.isNull() ? "" : m_port.portName().toStdString();
}

std::unique_ptr<ControllerConnection> SwitchController_SerialPABotBase_Descriptor::open(
    Logger& logger,
    const ControllerRequirements& requirements
) const{
    return std::unique_ptr<ControllerConnection>(new SwitchController_SerialPABotBase(logger, *this, requirements));
}







SwitchController_SerialPABotBase::SwitchController_SerialPABotBase(
    Logger& logger,
    const SwitchController_SerialPABotBase_Descriptor& descriptor,
    const ControllerRequirements& requirements
)
    : SuperscalarScheduler(
        logger, Milliseconds(4),
        {
            &m_buttons[ 0],
            &m_buttons[ 1],
            &m_buttons[ 2],
            &m_buttons[ 3],
            &m_buttons[ 4],
            &m_buttons[ 5],
            &m_buttons[ 6],
            &m_buttons[ 7],
            &m_buttons[ 8],
            &m_buttons[ 9],
            &m_buttons[10],
            &m_buttons[11],
            &m_buttons[12],
            &m_buttons[13],
            &m_dpad,
            &m_left_joystick,
            &m_right_joystick,
        }
    )
    , m_logger(logger, GlobalSettings::instance().LOG_EVERYTHING)
    , m_handle(m_logger, &descriptor.port(), requirements)
    , m_serial(*m_handle.botbase())
    , m_logging_suppress(0)
{
    m_handle.connect(
        &m_handle, &BotBaseHandle::on_not_connected,
        &m_handle, [this](std::string error){
//            cout << "BotBaseHandle::on_not_connected: " << error << endl;
            m_status = std::move(error);
            m_ready.store(false, std::memory_order_release);
            update_status_string();
            signal_ready_changed(false);
        }
    );
    m_handle.connect(
        &m_handle, &BotBaseHandle::on_connecting,
        &m_handle, [this](const std::string& port_name){
//            cout << "BotBaseHandle::on_connecting" << endl;
            m_status = "<font color=\"green\">Connecting...</font>";
            m_ready.store(false, std::memory_order_release);
            update_status_string();
            signal_ready_changed(false);
        }
    );
    m_handle.connect(
        &m_handle, &BotBaseHandle::on_ready,
        &m_handle, [this](std::string description){
//            cout << "BotBaseHandle::on_ready: " << description << endl;
            m_status = std::move(description);
            m_ready.store(true, std::memory_order_release);
            update_status_string();
            signal_ready_changed(true);
        }
    );
    m_handle.connect(
        &m_handle, &BotBaseHandle::on_stopped,
        &m_handle, [this](std::string error){
//            cout << "BotBaseHandle::on_stopped: " << error << endl;
            m_status = std::move(error);
            m_ready.store(false, std::memory_order_release);
            update_status_string();
            signal_ready_changed(false);
        }
    );
    m_handle.connect(
        &m_handle, &BotBaseHandle::uptime_status,
        &m_handle, [this](std::string status){
            m_uptime = std::move(status);
            update_status_string();
        }
    );

    m_status_text = m_handle.label();
}

void SwitchController_SerialPABotBase::push_state(const Cancellable* cancellable, WallDuration duration){
    Button buttons = BUTTON_NONE;
    for (size_t c = 0; c < 14; c++){
        buttons |= m_buttons[c].is_busy()
            ? (Button)((uint16_t)1 << c)
            : BUTTON_NONE;
    }

    uint8_t left_x = 128;
    uint8_t left_y = 128;
    uint8_t right_x = 128;
    uint8_t right_y = 128;
    if (m_left_joystick.is_busy()){
        left_x = m_left_joystick.x;
        left_y = m_left_joystick.y;
    }
    if (m_right_joystick.is_busy()){
        right_x = m_right_joystick.x;
        right_y = m_right_joystick.y;
    }

    issue_controller_state(
        cancellable,
        buttons,
        m_dpad.is_busy() ? m_dpad.position : DPAD_NONE,
        left_x, left_y,
        right_x, right_y,
        std::chrono::duration_cast<Milliseconds>(duration)
    );
}


void SwitchController_SerialPABotBase::update_status_string(){
    std::string str;
    str += m_status;
    if (!str.empty() && !m_uptime.empty()){
        str += "<br>";
    }
    str += m_uptime;
    {
        SpinLockGuard lg(m_status_text_lock);
        m_status_text = str;
    }

//    cout << "status: " << str << endl;

    signal_status_text_changed(str);
}




void SwitchController_SerialPABotBase::wait_for_all(const Cancellable* cancellable){
//    cout << "wait_for_all() - enter" << endl;
    {
        WriteSpinLock lg(m_lock);
        this->issue_wait_for_all(cancellable);
    }
    m_serial.wait_for_all_requests(cancellable);
//    cout << "wait_for_all() - exit" << endl;
}
void SwitchController_SerialPABotBase::cancel_all(const Cancellable* cancellable){
    m_serial.stop_all_commands();
    this->clear_on_next();
}
void SwitchController_SerialPABotBase::replace_on_next_command(const Cancellable* cancellable){
    m_serial.next_command_interrupt();
    this->clear_on_next();
}
void SwitchController_SerialPABotBase::issue_controller_state(
    const Cancellable* cancellable,
    Button button,
    DpadPosition position,
    uint8_t left_x, uint8_t left_y,
    uint8_t right_x, uint8_t right_y,
    Milliseconds duration
){
    uint32_t ticks = milliseconds_to_ticks_8ms(duration.count());
    //  Divide the controller state into smaller chunks of 255 ticks.
    while (ticks > 0){
        uint16_t curr_ticks = (uint16_t)std::min(ticks, (uint32_t)255);
        m_serial.issue_request(
            DeviceRequest_controller_state(button, position, left_x, left_y, right_x, right_y, (uint8_t)curr_ticks),
            cancellable
        );
        ticks -= curr_ticks;
    } // end while loop, and function
}
void SwitchController_SerialPABotBase::send_botbase_request(
    const Cancellable* cancellable,
    const BotBaseRequest& request
){
    m_serial.issue_request(request, cancellable);
}
BotBaseMessage SwitchController_SerialPABotBase::send_botbase_request_and_wait(
    const Cancellable* cancellable,
    const BotBaseRequest& request
){
    return m_serial.issue_request_and_wait(request, cancellable);
}

void SwitchController_SerialPABotBase::issue_barrier(const Cancellable* cancellable){
    if (m_logging_suppress.load(std::memory_order_relaxed) == 0){
        m_logger.log("issue_barrier()", COLOR_DARKGREEN);
    }
    WriteSpinLock lg(m_lock);
    this->issue_wait_for_all(cancellable);
}
void SwitchController_SerialPABotBase::issue_nop(const Cancellable* cancellable, Milliseconds duration){
    if (cancellable){
        cancellable->throw_if_cancelled();
    }
    if (m_logging_suppress.load(std::memory_order_relaxed) == 0){
        m_logger.log(
            "issue_nop(): duration = " + std::to_string(duration.count()) + "ms",
            COLOR_DARKGREEN
        );
    }
    WriteSpinLock lg(m_lock);
    this->SuperscalarScheduler::issue_nop(cancellable, WallDuration(duration));
}
void SwitchController_SerialPABotBase::issue_buttons(
    const Cancellable* cancellable,
    Button button,
    Milliseconds delay, Milliseconds hold, Milliseconds cooldown
){
    if (cancellable){
        cancellable->throw_if_cancelled();
    }
    if (m_logging_suppress.load(std::memory_order_relaxed) == 0){
        m_logger.log(
            "issue_buttons(): " + button_to_string(button) +
            ", delay = " + std::to_string(delay.count()) + "ms" +
            ", hold = " + std::to_string(hold.count()) + "ms" +
            ", cooldown = " + std::to_string(delay.count()) + "ms",
            COLOR_DARKGREEN
        );
    }
    WriteSpinLock lg(m_lock);
    for (size_t c = 0; c < 14; c++){
        uint16_t mask = (uint16_t)1 << c;
        if (button & mask){
            this->wait_for_resource(cancellable, m_buttons[c]);
        }
    }
    for (size_t c = 0; c < 14; c++){
        uint16_t mask = (uint16_t)1 << c;
        if (button & mask){
            this->issue_to_resource(
                cancellable, m_buttons[c],
                WallDuration::zero(), hold, cooldown
            );
        }
    }
    this->SuperscalarScheduler::issue_nop(cancellable, delay);
}
void SwitchController_SerialPABotBase::issue_dpad(
    const Cancellable* cancellable,
    DpadPosition position,
    Milliseconds delay, Milliseconds hold, Milliseconds cooldown
){
    if (cancellable){
        cancellable->throw_if_cancelled();
    }
    if (m_logging_suppress.load(std::memory_order_relaxed) == 0){
        m_logger.log(
            "issue_dpad(): " + dpad_to_string(position) +
            ", delay = " + std::to_string(delay.count()) + "ms" +
            ", hold = " + std::to_string(hold.count()) + "ms" +
            ", cooldown = " + std::to_string(delay.count()) + "ms",
            COLOR_DARKGREEN
        );
    }
    WriteSpinLock lg(m_lock);
    this->wait_for_resource(cancellable, m_dpad);
    m_dpad.position = position;
    this->issue_to_resource(cancellable, m_dpad, delay, hold, cooldown);
}
void SwitchController_SerialPABotBase::issue_left_joystick(
    const Cancellable* cancellable,
    uint8_t x, uint8_t y,
    Milliseconds delay, Milliseconds hold, Milliseconds cooldown
){
    if (cancellable){
        cancellable->throw_if_cancelled();
    }
    if (m_logging_suppress.load(std::memory_order_relaxed) == 0){
        m_logger.log(
            "issue_left_joystick(): (" + std::to_string(x) + "," + std::to_string(y) + ")" +
            ", delay = " + std::to_string(delay.count()) + "ms" +
            ", hold = " + std::to_string(hold.count()) + "ms" +
            ", cooldown = " + std::to_string(delay.count()) + "ms",
            COLOR_DARKGREEN
        );
    }
    WriteSpinLock lg(m_lock);
    this->wait_for_resource(cancellable, m_left_joystick);
    m_left_joystick.x = x;
    m_left_joystick.y = y;
    this->issue_to_resource(cancellable, m_left_joystick, delay, hold, cooldown);
//    cout << "(" << (unsigned)x << "," << (unsigned)y << "), hold = " << hold / 8 << endl;
}
void SwitchController_SerialPABotBase::issue_right_joystick(
    const Cancellable* cancellable,
    uint8_t x, uint8_t y,
    Milliseconds delay, Milliseconds hold, Milliseconds cooldown
){
    if (cancellable){
        cancellable->throw_if_cancelled();
    }
    if (m_logging_suppress.load(std::memory_order_relaxed) == 0){
        m_logger.log(
            "issue_right_joystick(): (" + std::to_string(x) + "," + std::to_string(y) + ")" +
            ", delay = " + std::to_string(delay.count()) + "ms" +
            ", hold = " + std::to_string(hold.count()) + "ms" +
            ", cooldown = " + std::to_string(delay.count()) + "ms",
            COLOR_DARKGREEN
        );
    }
    WriteSpinLock lg(m_lock);
    this->wait_for_resource(cancellable, m_right_joystick);
    m_right_joystick.x = x;
    m_right_joystick.y = y;
    this->issue_to_resource(cancellable, m_right_joystick, delay, hold, cooldown);
}


void SwitchController_SerialPABotBase::issue_mash_button(
    const Cancellable* cancellable,
    Button button, Milliseconds duration
){
    if (cancellable){
        cancellable->throw_if_cancelled();
    }
    if (m_logging_suppress.load(std::memory_order_relaxed) == 0){
        m_logger.log(
            "issue_mash_button(): " + button_to_string(button) +
            ", duration = " + std::to_string(duration.count()) + "ms",
            COLOR_DARKGREEN
        );
    }
    LoggingSuppressScope scope(m_logging_suppress);

    while (duration > Milliseconds::zero()){
        issue_buttons(cancellable, button, 8*8ms, 5*8ms, 3*8ms);
        duration = duration >= 8*8ms
            ? duration - 8*8ms
            : Milliseconds::zero();
    }
}
void SwitchController_SerialPABotBase::issue_mash_button(
    const Cancellable* cancellable,
    Button button0, Button button1, Milliseconds duration
){
    if (cancellable){
        cancellable->throw_if_cancelled();
    }
    if (m_logging_suppress.load(std::memory_order_relaxed) == 0){
        m_logger.log(
            "issue_mash_button(): (" + button_to_string(button0) +
            "), (" + button_to_string(button1) +
            "), duration = " + std::to_string(duration.count()) + "ms",
            COLOR_DARKGREEN
        );
    }
    LoggingSuppressScope scope(m_logging_suppress);

    while (duration > Milliseconds::zero()){
        issue_buttons(cancellable, button0, Milliseconds(4*8), 5*8ms, 3*8ms);
        issue_buttons(cancellable, button1, Milliseconds(4*8), 5*8ms, 3*8ms);
        duration = duration >= 8*8ms
            ? duration - 8*8ms
            : Milliseconds::zero();
    }
}
void SwitchController_SerialPABotBase::issue_mash_AZs(
    const Cancellable* cancellable,
    Milliseconds duration
){
    if (cancellable){
        cancellable->throw_if_cancelled();
    }
    if (m_logging_suppress.load(std::memory_order_relaxed) == 0){
        m_logger.log(
            "issue_mash_AZs(): duration = " + std::to_string(duration.count()) + "ms",
            COLOR_DARKGREEN
        );
    }
    LoggingSuppressScope scope(m_logging_suppress);

    while (duration > Milliseconds::zero()){
        issue_buttons(cancellable, BUTTON_A, 3*8ms, 5*8ms, 3*8ms);
        issue_buttons(cancellable, BUTTON_ZL, 3*8ms, 5*8ms, 3*8ms);
        issue_buttons(cancellable, BUTTON_ZR, 3*8ms, 5*8ms, 3*8ms);
        duration = duration >= Milliseconds(9*8)
            ? duration - Milliseconds(9*8)
            : Milliseconds::zero();
    }
}
void SwitchController_SerialPABotBase::issue_system_scroll(
    const Cancellable* cancellable,
    DpadPosition direction, //  Diagonals not allowed.
    Milliseconds delay, Milliseconds hold, Milliseconds cooldown
){
    if (cancellable){
        cancellable->throw_if_cancelled();
    }
    if (m_logging_suppress.load(std::memory_order_relaxed) == 0){
        m_logger.log(
            "issue_system_scroll(): " + dpad_to_string(direction) +
            ", delay = " + std::to_string(delay.count()) + "ms" +
            ", hold = " + std::to_string(hold.count()) + "ms" +
            ", cooldown = " + std::to_string(delay.count()) + "ms",
            COLOR_DARKGREEN
        );
    }
    LoggingSuppressScope scope(m_logging_suppress);

    WallClock dpad = m_dpad.free_time();
    WallClock left_joystick = m_left_joystick.free_time();
    WallClock right_joystick = m_right_joystick.free_time();

    if (dpad <= left_joystick && dpad <= right_joystick){
        issue_dpad(
            cancellable, direction,
            Milliseconds(delay), Milliseconds(hold), Milliseconds(cooldown)
        );
        return;
    }

    uint8_t x = 128;
    uint8_t y = 128;
    switch (direction){
    case DPAD_NONE:
        x = 128;
        y = 128;
        break;
    case DPAD_UP:
        x = 128;
        y = 0;
        break;
    case DPAD_RIGHT:
        x = 255;
        y = 128;
        break;
    case DPAD_DOWN:
        x = 128;
        y = 255;
        break;
    case DPAD_LEFT:
        x = 0;
        y = 128;
        break;

    //  These diagonal ones probably don't work.
    case DPAD_UP_RIGHT:
        x = 255;
        y = 0;
        break;
    case DPAD_DOWN_RIGHT:
        x = 255;
        y = 255;
        break;
    case DPAD_DOWN_LEFT:
        x = 0;
        y = 255;
        break;
    case DPAD_UP_LEFT:
        x = 0;
        y = 0;
        break;
    }

    if (left_joystick <= dpad && left_joystick <= right_joystick){
        issue_left_joystick(
            cancellable, x, y,
            Milliseconds(delay), Milliseconds(hold), Milliseconds(cooldown)
        );
        return;
    }

    issue_right_joystick(
        cancellable, x, y,
        Milliseconds(delay), Milliseconds(hold), Milliseconds(cooldown)
    );
}



}
}
