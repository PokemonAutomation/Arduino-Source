/*  Controller Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Controllers_ControllerOption_H
#define PokemonAutomation_Controllers_ControllerOption_H

#include <map>
#include "ControllerDescriptor.h"

namespace PokemonAutomation{



//
//  A configurable option type for the descriptor.
//
class ControllerOption{
public:
    ControllerOption(bool default_enable_mode);

    void set_interface(ControllerInterface interface_type);

    std::shared_ptr<ControllerDescriptor> descriptor() const{
        return m_descriptor;
    }
    void set_descriptor(std::shared_ptr<ControllerDescriptor> descriptor);

    //  Remember the last used descriptor for each interface type. That way when
    //  the user switches back-and-forth between two interfaces, it will reload
    //  the previous one.
    std::shared_ptr<ControllerDescriptor> get_descriptor_from_cache(ControllerInterface interface_type) const;


public:
    void load_json(const JsonValue& json);
    JsonValue to_json() const;


public:
    bool m_default_enable_mode;
    bool m_enable_input = true;

private:
    std::shared_ptr<ControllerDescriptor> m_descriptor;
    std::map<ControllerInterface, std::shared_ptr<ControllerDescriptor>> m_descriptor_cache;

    LifetimeSanitizer m_sanitizer;
};





}
#endif
