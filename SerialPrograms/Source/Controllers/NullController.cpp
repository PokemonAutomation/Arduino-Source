/*  Null Controller
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Json/JsonValue.h"
#include "ControllerConnection.h"
#include "NullController.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{


template class InterfaceType_t<NullControllerDescriptor>;


const char NullController::NAME[] = "(none)";



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
std::unique_ptr<ControllerConnection> NullControllerDescriptor::open_connection(Logger& logger) const{
    return nullptr;
}
std::unique_ptr<AbstractController> NullControllerDescriptor::make_controller(
    Logger& logger,
    ControllerConnection& connection,
    ControllerType controller_type
) const{
    return nullptr;
}







}
