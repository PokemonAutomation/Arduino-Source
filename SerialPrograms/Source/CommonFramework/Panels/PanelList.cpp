/*  Panel List
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/Logging/Logger.h"
#include "PanelList.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{



PanelListDescriptor::PanelListDescriptor(
    std::string name,
    const std::string& icon_path,
    bool enabled
)
    : m_name(std::move(name))
    , m_enabled(enabled)
{
    if (!icon_path.empty()){
        try{
            m_icon = ImageRGB32(icon_path);
        }catch (FileException& e){
            e.log(global_logger_tagged());
        }
    }
}




}


