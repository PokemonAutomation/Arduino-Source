/*  sys-botbase Descriptor
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Json/JsonValue.h"
#include "SysbotBase_Descriptor.h"
#include "SysbotBase_Connection.h"
#include "SysbotBase_ProController.h"
#include "SysbotBase_SelectorWidget.h"

namespace PokemonAutomation{

template class InterfaceType_t<SysbotBase::SysbotBaseNetwork_Descriptor>;

namespace SysbotBase{



bool SysbotBaseNetwork_Descriptor::operator==(const ControllerDescriptor& x) const{
    if (typeid(*this) != typeid(x)){
        return false;
    }
    return m_url == static_cast<const SysbotBaseNetwork_Descriptor&>(x).m_url;
}

std::string SysbotBaseNetwork_Descriptor::display_name() const{
    return m_url;
}

void SysbotBaseNetwork_Descriptor::load_json(const JsonValue& json){
    const std::string* url = json.to_string();
    if (url == nullptr || url->empty()){
        return;
    }
    m_url = *url;
}
JsonValue SysbotBaseNetwork_Descriptor::to_json() const{
    return m_url;
}



std::unique_ptr<ControllerConnection> SysbotBaseNetwork_Descriptor::open_connection(Logger& logger) const{
    return std::unique_ptr<ControllerConnection>(
        new SysbotBaseNetwork_Connection(logger, m_url)
    );
}
std::unique_ptr<AbstractController> SysbotBaseNetwork_Descriptor::make_controller(
    Logger& logger,
    ControllerConnection& connection,
    ControllerType controller_type,
    const ControllerRequirements& requirements
) const{
    return std::unique_ptr<AbstractController>(
        new NintendoSwitch::ProController_SysbotBase(
            logger,
            static_cast<SysbotBaseNetwork_Connection&>(connection),
            requirements
        )
    );
}



QWidget* SysbotBaseNetwork_Descriptor::make_selector_QtWidget(ControllerSelectorWidget& parent) const{
    return new SysbotBaseNetwork_SelectorWidget(parent, this);
}




}
}
