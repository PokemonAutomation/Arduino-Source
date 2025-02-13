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

//  REMOVE
#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{


//
//  Here we store a map of all controller types in the program.
//
std::map<ControllerInterface, std::unique_ptr<InterfaceType>> ALL_CONTROLLER_INTERFACES;


void InterfaceType::register_factory(
    ControllerInterface controller_interface,
    std::unique_ptr<InterfaceType> factory
){
    auto ret = ALL_CONTROLLER_INTERFACES.emplace(controller_interface, std::move(factory));
    if (!ret.second){
        throw InternalProgramError(
            nullptr, PA_CURRENT_FUNCTION,
            "Duplicate Factory Name: " + CONTROLLER_INTERFACE_STRINGS.get_string(controller_interface)
        );
    }
}



#if 0
std::vector<std::shared_ptr<const ControllerDescriptor>>
get_compatible_descriptors(const ControllerRequirements& requirements){
    std::vector<std::shared_ptr<const ControllerDescriptor>> ret;

    //  Null controller goes first.
    ret.emplace_back(new NullControllerDescriptor());

    //  Add all other controllers. We don't filter them at this time.
    for (const auto& controller_interface : ALL_CONTROLLER_INTERFACES){
        if (controller_interface.first == ControllerInterface::None){
            continue;
        }
        std::vector<std::shared_ptr<const ControllerDescriptor>> list = controller_interface.second->list();
        std::move(list.begin(), list.end(), std::back_inserter(ret));
    }

#if 0
    //  Find all the devices in common between the supported list and the
    //  required list. For each of those, enumerate all the descriptors and
    //  combine them into a single list.
    for (const auto& device : requirements.map()){
        auto iter = ALL_CONTROLLER_INTERFACES.find(device.first);
        if (iter != ALL_CONTROLLER_INTERFACES.end()){
            std::vector<std::shared_ptr<const ControllerDescriptor>> list = iter->second->list();
            std::move(list.begin(), list.end(), std::back_inserter(ret));
        }
    }
#endif

    return ret;
}
#endif






ControllerOption::ControllerOption()
    : m_controller_type(ControllerType::None)
    , m_descriptor(new NullControllerDescriptor())
{}


void ControllerOption::set_descriptor(std::shared_ptr<const ControllerDescriptor> descriptor){
    m_descriptor_cache[descriptor->interface_type] = descriptor;
    m_descriptor = std::move(descriptor);
}

std::shared_ptr<const ControllerDescriptor> ControllerOption::get_descriptor_from_cache(ControllerInterface interface_type) const{
    auto iter = m_descriptor_cache.find(interface_type);
    if (iter == m_descriptor_cache.end()){
        return nullptr;
    }
    return iter->second;
}



void ControllerOption::load_json(const JsonValue& json){
    std::shared_ptr<const ControllerDescriptor> descriptor;
    ControllerType controller_type = ControllerType::None;
    do{
        if (json.is_null()){
            break;
        }

        const JsonObject* obj = json.to_object();
        if (obj == nullptr){
            break;
        }
        const std::string* type = obj->get_string("Interface");
        if (type == nullptr){
            break;
        }
        const std::string* controller = obj->get_string("Controller");
        if (controller != nullptr){
            controller_type = CONTROLLER_TYPE_STRINGS.get_enum(*controller, ControllerType::None);
        }

        for (const auto& item : ALL_CONTROLLER_INTERFACES){
            const JsonValue* params = obj->get_value(CONTROLLER_INTERFACE_STRINGS.get_string(item.first));
            if (params == nullptr){
                continue;
            }
            m_descriptor_cache[item.first] = item.second->make(*params);
        }

//        const JsonValue* params = obj->get_value("Parameters");
//        if (params == nullptr){
//            break;
//        }

        auto iter = m_descriptor_cache.find(CONTROLLER_INTERFACE_STRINGS.get_enum(*type, ControllerInterface::None));
        if (iter == m_descriptor_cache.end()){
            break;
        }

        descriptor = iter->second;

    }while (false);

    if (descriptor == nullptr){
        descriptor.reset(new NullControllerDescriptor());
    }

    m_descriptor = std::move(descriptor);
    m_controller_type = controller_type;
}
JsonValue ControllerOption::to_json() const{
    if (!m_descriptor){
        return JsonValue();
    }
    JsonObject obj;
    obj["Interface"] = CONTROLLER_INTERFACE_STRINGS.get_string(m_descriptor->interface_type);
    obj["Controller"] = CONTROLLER_TYPE_STRINGS.get_string(m_controller_type);
//    obj["Parameters"] = m_descriptor->to_json();

    for (const auto& item : m_descriptor_cache){
        obj[CONTROLLER_INTERFACE_STRINGS.get_string(item.first)] = item.second->to_json();
    }


    return obj;
}









}
