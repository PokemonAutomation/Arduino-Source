/*  List of all Panels
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PanelList_H
#define PokemonAutomation_PanelList_H

#include <memory>
#include <vector>
#include <map>
#include "CommonFramework/Panels/RightPanel.h"

namespace PokemonAutomation{


const std::vector<std::unique_ptr<RightPanel>>& SETTINGS_LIST();
const std::map<QString, RightPanel*>& SETTINGS_MAP();

const std::vector<std::unique_ptr<RightPanel>>& PROGRAM_LIST();
const std::map<QString, RightPanel*>& PROGRAM_MAP();


}
#endif
