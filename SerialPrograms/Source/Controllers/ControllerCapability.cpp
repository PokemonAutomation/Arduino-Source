/*  Controller Capabilities
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "ControllerTypeStrings.h"
#include "ControllerCapability.h"

namespace PokemonAutomation{



std::string ControllerFeatures::contains_all(const ControllerFeatures& features) const{
    auto scope_check = m_sanitizer.check_scope();
    for (ControllerFeature feature : features.m_features){
        if (!contains(feature)){
            return CONTROLLER_FEATURE_STRINGS.get_string(feature);
        }
    }
    return "";
}





}
