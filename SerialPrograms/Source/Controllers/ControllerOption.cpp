/*  Controller Interface
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "ControllerTypeStrings.h"
#include "ControllerDescriptor.h"
#include "ControllerOption.h"
#include "NullController.h"

namespace PokemonAutomation{



//
//  Here we store a map of all controller types in the program.
//
static std::map<ControllerInterface, std::unique_ptr<InterfaceType>>& ALL_CONTROLLER_INTERFACES(){
    static std::map<ControllerInterface, std::unique_ptr<InterfaceType>> instance;
    return instance;
}


void InterfaceType::register_factory(
    ControllerInterface controller_interface,
    std::unique_ptr<InterfaceType> factory
){
    auto ret = ALL_CONTROLLER_INTERFACES().emplace(controller_interface, std::move(factory));
    if (!ret.second){
        throw InternalProgramError(
            nullptr, PA_CURRENT_FUNCTION,
            "Duplicate Factory Name: " + CONTROLLER_INTERFACE_STRINGS.get_string(controller_interface)
        );
    }
}




ControllerOption::ControllerOption(bool default_enable_mode)
    : m_default_enable_mode(default_enable_mode)
    , m_enable_input(default_enable_mode)
    , m_descriptor(null_controller_descriptor())
    , m_sanitizer("ControllerOption")
{}


void ControllerOption::set_interface(ControllerInterface interface_type){
    m_sanitizer.check_scope();

    m_descriptor_cache[m_descriptor->interface_type] = m_descriptor;
    auto iter = m_descriptor_cache.find(interface_type);
    if (iter != m_descriptor_cache.end()){
        m_descriptor = iter->second;
    }else{
        m_descriptor = ALL_CONTROLLER_INTERFACES().find(interface_type)->second->make();
    }
}
void ControllerOption::set_descriptor(std::shared_ptr<ControllerDescriptor> descriptor){
    m_sanitizer.check_scope();

    m_descriptor_cache[descriptor->interface_type] = descriptor;
    m_descriptor = std::move(descriptor);
}

std::shared_ptr<ControllerDescriptor> ControllerOption::get_descriptor_from_cache(ControllerInterface interface_type) const{
    m_sanitizer.check_scope();

    auto iter = m_descriptor_cache.find(interface_type);
    if (iter == m_descriptor_cache.end()){
        return nullptr;
    }
    return iter->second;
}



void ControllerOption::load_json(const JsonValue& json){
    m_sanitizer.check_scope();

    std::shared_ptr<ControllerDescriptor> descriptor;
    do{
        if (json.is_null()){
            break;
        }

        const JsonObject* obj = json.to_object();
        if (obj == nullptr){
            break;
        }

        obj->read_boolean(m_enable_input, "EnableInput");

        const std::string* type = obj->get_string("Interface");
        if (type == nullptr){
            break;
        }

        for (const auto& item : ALL_CONTROLLER_INTERFACES()){
            const JsonValue* params = obj->get_value(CONTROLLER_INTERFACE_STRINGS.get_string(item.first));
            if (params == nullptr){
                continue;
            }
            m_descriptor_cache[item.first] = item.second->make(*params);
        }

        try{
            auto iter = m_descriptor_cache.find(CONTROLLER_INTERFACE_STRINGS.get_enum(*type));
            if (iter == m_descriptor_cache.end()){
                break;
            }
            descriptor = iter->second;
        }catch (ParseException&){
            break;
        }

    }while (false);

    if (descriptor == nullptr){
        descriptor = null_controller_descriptor();
    }

    m_descriptor = std::move(descriptor);
}
JsonValue ControllerOption::to_json() const{
    m_sanitizer.check_scope();

    if (!m_descriptor){
        return JsonValue();
    }
    JsonObject obj;
    obj["EnableInput"] = m_enable_input;
    obj["Interface"] = CONTROLLER_INTERFACE_STRINGS.get_string(m_descriptor->interface_type);

    for (const auto& item : m_descriptor_cache){
        obj[CONTROLLER_INTERFACE_STRINGS.get_string(item.first)] = item.second->to_json();
    }

    return obj;
}






}
