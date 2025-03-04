/*  Controller Capabilities
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "ControllerTypeStrings.h"
#include "ControllerCapability.h"

namespace PokemonAutomation{





ControllerRequirements::ControllerRequirements(std::initializer_list<ControllerFeature> args)
    : m_features(std::move(args))
    , m_sanitizer("ControllerRequirements")
{}

std::string ControllerRequirements::check_compatibility(const std::set<ControllerFeature>& features) const{
    auto scope_check = m_sanitizer.check_scope();

    for (ControllerFeature feature : m_features){
        if (features.find(feature) == features.end()){
            return CONTROLLER_FEATURE_STRINGS.get_string(feature);
        }
    }
    return "";
}




}
