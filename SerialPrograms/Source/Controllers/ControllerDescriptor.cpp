/*  Controller Interface
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <iterator>
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "ControllerCapability.h"
#include "ControllerDescriptor.h"
#include "NullController.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{


//
//  Here we store a map of all controller types in the program.
//
std::map<std::string, std::unique_ptr<ControllerType>> CONTROLLER_TYPES;


void ControllerType::register_factory(
    const std::string& name,
    std::unique_ptr<ControllerType> factory
){
    auto ret = CONTROLLER_TYPES.emplace(name, std::move(factory));
    if (!ret.second){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Duplicate Factory Name: " + name);
    }
}



std::vector<std::shared_ptr<const ControllerDescriptor>>
get_compatible_descriptors(const ControllerRequirements& requirements){
    std::vector<std::shared_ptr<const ControllerDescriptor>> ret;

    //  Find all the devices in common between the supported list and the
    //  required list. For each of those, enumerate all the descriptors and
    //  combine them into a single list.
    for (const auto& device : requirements.map()){
        auto iter = CONTROLLER_TYPES.find(device.first);
        if (iter != CONTROLLER_TYPES.end()){
            std::vector<std::shared_ptr<const ControllerDescriptor>> list = iter->second->list();
            std::move(list.begin(), list.end(), std::back_inserter(ret));
        }
    }

    return ret;
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

    auto iter = CONTROLLER_TYPES.find(type);
    if (iter == CONTROLLER_TYPES.end()){
        m_current.reset(new NullControllerDescriptor());
        return;
    }

    m_current = iter->second->make(params);
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
