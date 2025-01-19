/*  Controller Capabilities
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Controllers_ControllerCapabilities_H
#define PokemonAutomation_Controllers_ControllerCapabilities_H

#include <string>
#include <set>
#include <map>
#include "Common/Cpp/LifetimeSanitizer.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{



class ControllerRequirements{
public:
#if 0
    template <class... Args>
    ControllerRequirements(Args&&... args)
        : m_map(std::forward<Args>(args)...)
        , m_sanitizer("ControllerRequirements")
    {}
#endif
    ControllerRequirements(std::initializer_list<std::map<std::string, std::set<std::string>>::value_type> args)
        : m_map(std::move(args))
        , m_sanitizer("ControllerRequirements")
    {}

    const std::map<std::string, std::set<std::string>>& map() const{
        auto scope_check = m_sanitizer.check_scope();
        return m_map;
    }

    bool is_compatible_with(const std::string& device, const std::set<std::string>& features) const{
        auto scope_check = m_sanitizer.check_scope();

        auto iter0 = m_map.find(device);
        if (iter0 == m_map.end()){
            return false;
        }

        const std::set<std::string>& required = iter0->second;
        for (const std::string& feature : required){
            if (features.find(feature) == features.end()){
                return false;
            }
        }
        return true;
    }


private:
    std::map<std::string, std::set<std::string>> m_map;

    LifetimeSanitizer m_sanitizer;
};




}
#endif
