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
    bool enabled
)
    : m_name(std::move(name))
    , m_enabled(enabled)
{}




}


