/*  Controller Interface
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "ControllerDescriptor.h"

#include "Controllers/SerialPABotBase/SerialPABotBase_Descriptor.h"
#include "ControllerConnection.h"

namespace PokemonAutomation{


std::unique_ptr<ControllerDescriptor> NullControllerDescriptor::clone() const{
    return std::unique_ptr<ControllerDescriptor>(new NullControllerDescriptor());
}
bool NullControllerDescriptor::operator==(const ControllerDescriptor& x) const{
    return typeid(*this) == typeid(x);
}
const char* NullControllerDescriptor::type_name() const{
    return "None";
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




ControllerOption::ControllerOption()
    : m_current(new NullControllerDescriptor())
{}

void ControllerOption::load_json(const JsonValue& json){
    if (json.is_null()){
        m_current.reset(new NullControllerDescriptor());
    }
    const JsonObject& obj = json.to_object_throw();
    const std::string& type = obj.get_string_throw("DeviceType");
    const JsonValue& params = obj.get_value_throw("Parameters");

    if (type == "None"){
        auto descriptor = std::make_unique<NullControllerDescriptor>();
        m_current = std::move(descriptor);
        return;
    }
    if (type == SerialPABotBase::SerialDescriptor::TYPENAME){
        auto descriptor = std::make_unique<SerialPABotBase::SerialDescriptor>();
        descriptor->load_json(params);
        m_current = std::move(descriptor);
        return;
    }

    throw ParseException("Invalid Device Connection Type: " + type);
}
JsonValue ControllerOption::to_json() const{
    if (!m_current){
        return JsonValue();
    }
    JsonObject obj;
    obj["DeviceType"] = m_current->type_name();
    obj["Parameters"] = m_current->to_json();
    return obj;
}









}
