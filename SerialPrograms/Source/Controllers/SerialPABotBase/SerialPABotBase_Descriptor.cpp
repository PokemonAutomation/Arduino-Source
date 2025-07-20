/*  Serial Port (PABotBase) Interface
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QWidget>
#include "Common/Cpp/Json/JsonValue.h"
#include "Controllers/ControllerTypeStrings.h"
#include "SerialPABotBase_Descriptor.h"
#include "SerialPABotBase_SelectorWidget.h"

#include "NintendoSwitch/Controllers/SerialPABotBase/NintendoSwitch_SerialPABotBase_PokkenController.h"
#include "NintendoSwitch/Controllers/SerialPABotBase/NintendoSwitch_SerialPABotBase_WirelessProController.h"
#include "NintendoSwitch/Controllers/SerialPABotBase/NintendoSwitch_SerialPABotBase_WirelessJoycon.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{

template class InterfaceType_t<SerialPABotBase::SerialPABotBase_Descriptor>;

namespace SerialPABotBase{




bool SerialPABotBase_Descriptor::operator==(const ControllerDescriptor& x) const{
    if (typeid(*this) != typeid(x)){
        return false;
    }
    return m_port.portName() == static_cast<const SerialPABotBase_Descriptor&>(x).m_port.portName();
}


std::string SerialPABotBase_Descriptor::display_name() const{
    if (m_port.isNull()){
        return "";
    }
//    if (PreloadSettings::instance().DEVELOPER_MODE){
        return m_port.portName().toStdString();
//    }
//    return m_port.portName().toStdString() + " - " + m_port.description().toStdString();
}
void SerialPABotBase_Descriptor::load_json(const JsonValue& json){
    const std::string* name = json.to_string();
    if (name == nullptr || name->empty()){
        return;
    }
    m_port = (QSerialPortInfo(QString::fromStdString(*name)));
}
JsonValue SerialPABotBase_Descriptor::to_json() const{
    return m_port.isNull() ? "" : m_port.portName().toStdString();
}

std::unique_ptr<ControllerConnection> SerialPABotBase_Descriptor::open_connection(
    Logger& logger,
    std::optional<ControllerType> change_controller
) const{
    return std::unique_ptr<ControllerConnection>(
        new SerialPABotBase_Connection(logger, &m_port, change_controller)
    );
}
std::unique_ptr<AbstractController> SerialPABotBase_Descriptor::make_controller(
    Logger& logger,
    ControllerConnection& connection,
    ControllerType controller_type
) const{
    switch (controller_type){
    case ControllerType::NintendoSwitch_WiredProController:
        return std::unique_ptr<AbstractController>(
            new PokemonAutomation::NintendoSwitch::SerialPABotBase_PokkenController(
                logger,
                static_cast<SerialPABotBase_Connection&>(connection)
            )
        );

    case ControllerType::NintendoSwitch_WirelessProController:
        return std::unique_ptr<AbstractController>(
            new PokemonAutomation::NintendoSwitch::SerialPABotBase_WirelessProController(
                logger,
                static_cast<SerialPABotBase_Connection&>(connection)
            )
        );

    case ControllerType::NintendoSwitch_LeftJoycon:
    case ControllerType::NintendoSwitch_RightJoycon:
        return std::unique_ptr<AbstractController>(
            new PokemonAutomation::NintendoSwitch::SerialPABotBase_WirelessJoycon(
                logger,
                static_cast<SerialPABotBase_Connection&>(connection),
                controller_type
            )
        );

    default:;
    }

    logger.log(
        std::string("Unsupported Controller Type: ") + CONTROLLER_TYPE_STRINGS.get_string(controller_type),
        COLOR_RED
    );
    return nullptr;
}



QWidget* SerialPABotBase_Descriptor::make_selector_QtWidget(ControllerSelectorWidget& parent) const{
    return new SerialPABotBase_SelectorWidget(parent, this);
}




}
}
