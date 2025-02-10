/*  Null Controller
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Json/JsonValue.h"
#include "ControllerConnection.h"
#include "NullController.h"

namespace PokemonAutomation{

template <>
std::vector<std::shared_ptr<const ControllerDescriptor>> InterfaceType_t<NullControllerDescriptor>::list() const{
    std::vector<std::shared_ptr<const ControllerDescriptor>> ret;
    ret.emplace_back(new NullControllerDescriptor());
    return ret;
}


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
    Logger& logger
) const{
    return nullptr;
}
std::unique_ptr<AbstractController> NullControllerDescriptor::make_controller(
    Logger& logger,
    ControllerConnection& connection,
    ControllerType controller_type,
    const ControllerRequirements& requirements
) const{
    return nullptr;
}




}
