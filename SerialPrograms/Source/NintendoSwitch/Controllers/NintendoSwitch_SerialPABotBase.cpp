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
#include "NintendoSwitch/Commands/NintendoSwitch_Messages_Superscalar.h"
#include "NintendoSwitch_SerialPABotBase.h"

namespace PokemonAutomation{


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
    : m_logger(logger, GlobalSettings::instance().LOG_EVERYTHING)
    , m_handle(m_logger, &descriptor.port(), requirements)
    , m_serial(*m_handle.botbase())
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
    m_serial.wait_for_all_requests(cancellable);
}
void SwitchController_SerialPABotBase::cancel_all(const Cancellable* cancellable){
    m_serial.stop_all_commands();
}
void SwitchController_SerialPABotBase::replace_on_next_command(const Cancellable* cancellable){
    m_serial.next_command_interrupt();
}
void SwitchController_SerialPABotBase::send_wait_for_pending(const Cancellable* cancellable){
    m_serial.issue_request(
        DeviceRequest_ssf_flush_pipeline(),
        cancellable
    );
}
void SwitchController_SerialPABotBase::send_wait(const Cancellable* cancellable, uint16_t ticks){
    m_serial.issue_request(
        DeviceRequest_ssf_do_nothing(ticks),
        cancellable
    );
}
void SwitchController_SerialPABotBase::send_controller_state(
    const Cancellable* cancellable,
    Button button,
    DpadPosition position,
    uint8_t left_x, uint8_t left_y,
    uint8_t right_x, uint8_t right_y,
    uint16_t ticks
){
    // divide the controller state into smaller chunks of 255 ticks
    while (ticks > 0){
        uint16_t curr_ticks = std::min(ticks, (uint16_t)255);
        m_serial.issue_request(
            DeviceRequest_controller_state(button, position, left_x, left_y, right_x, right_y, (uint8_t)curr_ticks)
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
void SwitchController_SerialPABotBase::send_buttons(
    const Cancellable* cancellable,
    Button button,
    uint16_t delay, uint16_t hold, uint8_t cooldown
){
    m_serial.issue_request(
        DeviceRequest_ssf_press_button(button, delay, hold, cooldown),
        cancellable
    );
}
void SwitchController_SerialPABotBase::send_dpad(
    const Cancellable* cancellable,
    DpadPosition position,
    uint16_t delay, uint16_t hold, uint8_t cooldown
){
    m_serial.issue_request(
        DeviceRequest_ssf_press_dpad(position, delay, hold, cooldown),
        cancellable
    );
}
void SwitchController_SerialPABotBase::send_left_joystick(
    const Cancellable* cancellable,
    uint8_t x, uint8_t y,
    uint16_t delay, uint16_t hold, uint8_t cooldown
){
    m_serial.issue_request(
        DeviceRequest_ssf_press_joystick(true, x, y, delay, hold, cooldown),
        cancellable
    );
}
void SwitchController_SerialPABotBase::send_right_joystick(
    const Cancellable* cancellable,
    uint8_t x, uint8_t y,
    uint16_t delay, uint16_t hold, uint8_t cooldown
){
    m_serial.issue_request(
        DeviceRequest_ssf_press_joystick(false, x, y, delay, hold, cooldown),
        cancellable
    );
}
void SwitchController_SerialPABotBase::send_mash_button(
    const Cancellable* cancellable,
    Button button, uint16_t ticks
){
    m_serial.issue_request(
        DeviceRequest_ssf_mash1_button(button, ticks),
        cancellable
    );
}
void SwitchController_SerialPABotBase::send_mash_button(
    const Cancellable* cancellable,
    Button button0, Button button1, uint16_t ticks
){
    m_serial.issue_request(
        DeviceRequest_ssf_mash2_button(button0, button1, ticks),
        cancellable
    );
}
void SwitchController_SerialPABotBase::send_mash_AZs(
    const Cancellable* cancellable,
    uint16_t ticks
){
    m_serial.issue_request(
        DeviceRequest_ssf_mash_AZs(ticks),
        cancellable
    );
}
void SwitchController_SerialPABotBase::send_system_scroll(
    const Cancellable* cancellable,
    DpadPosition direction, //  Diagonals not allowed.
    uint16_t delay, uint16_t hold, uint8_t cooldown
){
    m_serial.issue_request(
        DeviceRequest_ssf_issue_scroll(direction, delay, hold, cooldown),
        cancellable
    );
}




}
}
