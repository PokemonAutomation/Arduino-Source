/*  sys-botbase Descriptor
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Json/JsonValue.h"
#include "SysbotBase_Descriptor.h"
#include "SysbotBase_Connection.h"
#include "SysbotBase_ProController.h"
#include "SysbotBase3_ProController.h"
#include "SysbotBase_SelectorWidget.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{

template class InterfaceType_t<SysbotBase::TcpSysbotBase_Descriptor>;

namespace SysbotBase{



bool TcpSysbotBase_Descriptor::operator==(const ControllerDescriptor& x) const{
    if (typeid(*this) != typeid(x)){
        return false;
    }
    return m_url == static_cast<const TcpSysbotBase_Descriptor&>(x).m_url;
}

std::string TcpSysbotBase_Descriptor::display_name() const{
    return m_url;
}

void TcpSysbotBase_Descriptor::load_json(const JsonValue& json){
    const std::string* url = json.to_string();
    if (url == nullptr || url->empty()){
        return;
    }
    m_url = *url;
}
JsonValue TcpSysbotBase_Descriptor::to_json() const{
    return m_url;
}



std::unique_ptr<ControllerConnection> TcpSysbotBase_Descriptor::open_connection(
    Logger& logger,
    bool set_to_null_controller
) const{
    return std::unique_ptr<ControllerConnection>(
        new TcpSysbotBase_Connection(logger, m_url)
    );
}
std::unique_ptr<AbstractController> TcpSysbotBase_Descriptor::make_controller(
    Logger& logger,
    ControllerConnection& connection,
    ControllerType controller_type,
    ControllerResetMode reset_mode
) const{
    TcpSysbotBase_Connection& sbb_connection = static_cast<TcpSysbotBase_Connection&>(connection);
    if (sbb_connection.supports_command_queue()){
        return std::unique_ptr<AbstractController>(
            new NintendoSwitch::ProController_SysbotBase3(logger, sbb_connection)
        );
    }else{
        return std::unique_ptr<AbstractController>(
            new NintendoSwitch::ProController_SysbotBase(logger, sbb_connection)
        );
    }
}



QWidget* TcpSysbotBase_Descriptor::make_selector_QtWidget(ControllerSelectorWidget& parent) const{
    return new TcpSysbotBase_SelectorWidget(parent, this);
}




}
}
