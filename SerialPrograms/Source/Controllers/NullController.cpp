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
std::vector<std::shared_ptr<const ControllerDescriptor>> ControllerType_t<NullControllerDescriptor>::list() const{
    std::vector<std::shared_ptr<const ControllerDescriptor>> ret;
    ret.emplace_back(new NullControllerDescriptor());
    return ret;
}


template class ControllerType_t<NullControllerDescriptor>;
const char NullControllerDescriptor::TYPENAME[] = "None";


bool NullControllerDescriptor::operator==(const ControllerDescriptor& x) const{
    return typeid(*this) == typeid(x);
}
const char* NullControllerDescriptor::type_name() const{
    return TYPENAME;
}
std::string NullControllerDescriptor::display_name() const{
    return "(none)";
}
void NullControllerDescriptor::load_json(const JsonValue& json){

}
JsonValue NullControllerDescriptor::to_json() const{
    return JsonValue();
}
std::unique_ptr<ControllerConnection> NullControllerDescriptor::open(
    Logger& logger,
    const ControllerRequirements& requirements
) const{
    return nullptr;
}




}
