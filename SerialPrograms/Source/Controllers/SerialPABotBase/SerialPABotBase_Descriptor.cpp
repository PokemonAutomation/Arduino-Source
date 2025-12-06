/*  Serial Port (PABotBase) Interface
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QSerialPortInfo>
#include <QWidget>
#include "Common/Cpp/Json/JsonValue.h"
#include "Controllers/ControllerTypeStrings.h"
#include "SerialPABotBase_Descriptor.h"
#include "SerialPABotBase_SelectorWidget.h"

#include "Controllers/StandardHid/StandardHid_Keyboard_SerialPABotBase.h"
#include "NintendoSwitch/Controllers/SerialPABotBase/NintendoSwitch_SerialPABotBase_WiredController.h"
#include "NintendoSwitch/Controllers/SerialPABotBase/NintendoSwitch_SerialPABotBase_ProController.h"
#include "NintendoSwitch/Controllers/SerialPABotBase/NintendoSwitch_SerialPABotBase_Joycon.h"

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
    return m_name == static_cast<const SerialPABotBase_Descriptor&>(x).m_name;
}


std::string SerialPABotBase_Descriptor::display_name() const{
#if 0
    QSerialPortInfo info(QString::fromStdString(m_name));
    if (info.isNull()){
        return m_name;
    }
    return m_name + " - " + info.manufacturer().toStdString();
#else
    return m_name;
#endif
}
void SerialPABotBase_Descriptor::load_json(const JsonValue& json){
    const std::string* name = json.to_string();
    if (name == nullptr || name->empty()){
        return;
    }
    m_name = *name;
}
JsonValue SerialPABotBase_Descriptor::to_json() const{
    return m_name;
}

std::unique_ptr<ControllerConnection> SerialPABotBase_Descriptor::open_connection(
    Logger& logger,
    bool set_to_null_controller
) const{
    return std::unique_ptr<ControllerConnection>(
        new SerialPABotBase_Connection(logger, m_name, set_to_null_controller)
    );
}
std::unique_ptr<AbstractController> SerialPABotBase_Descriptor::make_controller(
    Logger& logger,
    ControllerConnection& connection,
    ControllerType controller_type,
    ControllerResetMode reset_mode
) const{
    switch (controller_type){
    case ControllerType::HID_Keyboard:
        return std::unique_ptr<AbstractController>(
            new PokemonAutomation::StandardHid::SerialPABotBase_Keyboard(
                logger,
                static_cast<SerialPABotBase_Connection&>(connection),
                reset_mode
            )
        );

    case ControllerType::NintendoSwitch_WiredController:
    case ControllerType::NintendoSwitch2_WiredController:
        return std::unique_ptr<AbstractController>(
            new PokemonAutomation::NintendoSwitch::SerialPABotBase_WiredController(
                logger,
                static_cast<SerialPABotBase_Connection&>(connection),
                controller_type,
                reset_mode
            )
        );

    case ControllerType::NintendoSwitch_WiredProController:
    case ControllerType::NintendoSwitch_WirelessProController:
        return std::unique_ptr<AbstractController>(
            new PokemonAutomation::NintendoSwitch::SerialPABotBase_ProController(
                logger,
                static_cast<SerialPABotBase_Connection&>(connection),
                controller_type,
                reset_mode
            )
        );

    case ControllerType::NintendoSwitch_WiredLeftJoycon:
    case ControllerType::NintendoSwitch_WirelessLeftJoycon:
        return std::unique_ptr<AbstractController>(
            new PokemonAutomation::NintendoSwitch::SerialPABotBase_LeftJoycon(
                logger,
                static_cast<SerialPABotBase_Connection&>(connection),
                controller_type,
                reset_mode
            )
        );

    case ControllerType::NintendoSwitch_WiredRightJoycon:
    case ControllerType::NintendoSwitch_WirelessRightJoycon:
        return std::unique_ptr<AbstractController>(
            new PokemonAutomation::NintendoSwitch::SerialPABotBase_RightJoycon(
                logger,
                static_cast<SerialPABotBase_Connection&>(connection),
                controller_type,
                reset_mode
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
