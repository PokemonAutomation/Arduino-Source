/*  ML Panels
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/StaticGlobals.h"
#include "CommonFramework/Panels/PanelTools.h"
#include "GameConsole/ConsolePanel.h"
#include "Programs/ML_LabelImages.h"
#include "Programs/ML_RunYOLO.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "ML_Panels.h"


namespace PokemonAutomation{
namespace ML{



PanelListFactory::PanelListFactory()
    : PanelListDescriptor("ML")
{}

std::vector<PanelEntry> PanelListFactory::make_panels() const{
    std::vector<PanelEntry> ret;
    if (STATIC_GLOBALS.DEVELOPER_MODE){
        ret.emplace_back("---- Developer Tools ----");
        ret.emplace_back(GameConsole::make_ConsolePanel<LabelImages>());
        // ret.emplace_back(make_panel<RunYOLO_Descriptor, RunYOLO>());
        ret.emplace_back(NintendoSwitch::make_SingleSwitchProgram<RunYOLO>());
        // ret.emplace_back(make_SingleSwitchProgram<ThreeSegmentDudunsparceFinder_Descriptor, ThreeSegmentDudunsparceFinder>());
    }

    return ret;
}




}
}
