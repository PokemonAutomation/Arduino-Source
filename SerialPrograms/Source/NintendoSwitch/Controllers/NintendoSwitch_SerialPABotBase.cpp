/*  Nintendo Switch Controller (Serial PABotBase)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/Options/Environment/ThemeSelectorOption.h"
#include "Controllers/ControllerCapability.h"
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
//    return "Wired Controller - Serial (PABotBase): " + m_port.portName().toStdString() + " - " + m_port.description().toStdString();
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

std::unique_ptr<ControllerConnection> SwitchController_SerialPABotBase_Descriptor::open_connection(
    Logger& logger
) const{
    return std::unique_ptr<ControllerConnection>(new BotBaseHandle(logger, &m_port));
}
std::unique_ptr<AbstractController> SwitchController_SerialPABotBase_Descriptor::make_controller(
    Logger& logger,
    ControllerConnection& connection,
    ControllerType controller_type,
    const ControllerRequirements& requirements
) const{
    if (controller_type != ControllerType::NintendoSwitch_WiredProController){
        throw InternalProgramError(
            nullptr, PA_CURRENT_FUNCTION,
            std::string("Unsupported Controller Type: ") + to_string(controller_type)
        );
    }
    BotBaseHandle& actual = static_cast<BotBaseHandle&>(connection);
    return std::unique_ptr<AbstractController>(
        new SwitchController_SerialPABotBase(logger, actual, requirements)
    );
}






SwitchController_SerialPABotBase::~SwitchController_SerialPABotBase(){
//    m_handle.remove_status_listener(*this);
}
SwitchController_SerialPABotBase::SwitchController_SerialPABotBase(
    Logger& logger,
    BotBaseHandle& connection,
    const ControllerRequirements& requirements
)
    : SwitchControllerWithScheduler(logger)
    , m_logger(logger)
    , m_requirements(requirements)
    , m_handle(connection)
    , m_serial(m_handle.botbase())
{
    if (!m_handle.is_ready()){
        return;
    }

    //  Check compatibility.

    const std::map<ControllerType, std::set<ControllerFeature>>& controllers = m_handle.supported_controllers();
    auto iter = controllers.find(ControllerType::NintendoSwitch_WiredProController);

    std::string missing_feature;
    do{
        if (iter == controllers.end()){
            missing_feature = "NintendoSwitch_WiredProController";
            break;
        }

        missing_feature = m_requirements.check_compatibility(
            SerialPABotBase::NintendoSwitch_Basic,
            iter->second
        );

        if (missing_feature.empty()){
            m_handle.update_with_capabilities(iter->second);
            return;
        }

    }while (false);

    m_error_string = html_color_text("Missing Feature: " + missing_feature, COLOR_RED);
}




void SwitchController_SerialPABotBase::wait_for_all(const Cancellable* cancellable){
//    cout << "wait_for_all() - enter" << endl;
    if (!is_ready()){
        throw InvalidConnectionStateException();
    }
    issue_barrier(cancellable);
    m_serial->wait_for_all_requests(cancellable);
//    cout << "wait_for_all() - exit" << endl;
}
void SwitchController_SerialPABotBase::cancel_all_commands(const Cancellable* cancellable){
    if (!is_ready()){
        throw InvalidConnectionStateException();
    }
    m_serial->stop_all_commands();
    this->clear_on_next();
}
void SwitchController_SerialPABotBase::replace_on_next_command(const Cancellable* cancellable){
    if (!is_ready()){
        throw InvalidConnectionStateException();
    }
    m_serial->next_command_interrupt();
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
    if (!is_ready()){
        throw InvalidConnectionStateException();
    }

    //  Divide the controller state into smaller chunks of 255 ticks.
    while (duration > Milliseconds::zero()){
        Milliseconds current_ms = std::min(duration, 255 * 8ms);
        uint8_t current_ticks = (uint8_t)milliseconds_to_ticks_8ms(current_ms.count());
        m_serial->issue_request(
            DeviceRequest_controller_state(button, position, left_x, left_y, right_x, right_y, current_ticks),
            cancellable
        );
        duration -= current_ms;
    }
}



void SwitchController_SerialPABotBase::send_botbase_request(
    const Cancellable* cancellable,
    const BotBaseRequest& request
){
    if (!is_ready()){
        throw InvalidConnectionStateException();
    }
    m_serial->issue_request(request, cancellable);
}
BotBaseMessage SwitchController_SerialPABotBase::send_botbase_request_and_wait(
    const Cancellable* cancellable,
    const BotBaseRequest& request
){
    if (!is_ready()){
        throw InvalidConnectionStateException();
    }
    return m_serial->issue_request_and_wait(request, cancellable);
}











}
}
