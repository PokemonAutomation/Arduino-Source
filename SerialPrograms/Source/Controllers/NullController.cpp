/*  Null Controller
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QWidget>
#include "Common/Cpp/Json/JsonValue.h"
#include "ControllerConnection.h"
#include "ControllerSelectorWidget.h"
#include "NullController.h"

namespace PokemonAutomation{


template class InterfaceType_t<NullControllerDescriptor>;


bool NullControllerDescriptor::operator==(const ControllerDescriptor& x) const{
    return typeid(*this) == typeid(x);
}
std::string NullControllerDescriptor::display_name() const{
    return "(none)";
}
void NullControllerDescriptor::load_json(const JsonValue& json){

}
JsonValue NullControllerDescriptor::to_json() const{
    return JsonValue();
}
std::unique_ptr<ControllerConnection> NullControllerDescriptor::open_connection(
    Logger& logger,
    bool set_to_null_controller
) const{
    return nullptr;
}
std::unique_ptr<AbstractController> NullControllerDescriptor::make_controller(
    Logger& logger,
    ControllerConnection& connection,
    ControllerType controller_type,
    ControllerResetMode reset_mode
) const{
    return nullptr;
}
QWidget* NullControllerDescriptor::make_selector_QtWidget(ControllerSelectorWidget& parent) const{
    return new QWidget(&parent);
}




}
