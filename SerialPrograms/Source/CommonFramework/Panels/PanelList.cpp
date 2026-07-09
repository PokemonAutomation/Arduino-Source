/*  Panel List
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "PanelList.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{



PanelListDescriptor::PanelListDescriptor(
    std::string name,
    ImageRGB32 icon,
    bool enabled
)
    : m_name(std::move(name))
    , m_icon(std::move(icon))
    , m_enabled(enabled)
{}




}


