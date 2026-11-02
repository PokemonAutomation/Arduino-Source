/*  Camera Info
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Options/DeviceDropdownAliases.h"
#include "Backends/CameraImplementations.h"
#include "CameraInfo.h"

namespace PokemonAutomation{

std::string CameraInfo::display_name() const{
    if (!*this){
        return std::string();
    }
    if (auto alias = device_dropdown_name(DeviceAliasType::VIDEO, m_device_name)){
        return *alias;
    }
    return get_camera_name(*this);
}

}
