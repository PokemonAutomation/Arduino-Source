/*  ML Panels
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/StaticGlobals.h"
#include "CommonFramework/Panels/PanelTools.h"
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
        ret.emplace_back(make_panel<LabelImages_Descriptor, LabelImages>());
        // ret.emplace_back(make_panel<RunYOLO_Descriptor, RunYOLO>());
        ret.emplace_back(NintendoSwitch::make_single_switch_program<RunYOLO_Descriptor, RunYOLO>());
        // ret.emplace_back(make_single_switch_program<ThreeSegmentDudunsparceFinder_Descriptor, ThreeSegmentDudunsparceFinder>());
    }

    return ret;
}




}
}
