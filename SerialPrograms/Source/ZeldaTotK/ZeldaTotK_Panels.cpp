/*  Pokemon SV Panels
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/GlobalSettingsPanel.h"
#include "ZeldaTotK_Panels.h"

#include "ZeldaTotK_Settings.h"

#include "Programs/ZeldaTotK_BowItemDuper.h"

#ifdef PA_OFFICIAL
#include "../../Internal/SerialPrograms/NintendoSwitch_TestPrograms.h"
#endif

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace ZeldaTotK{



PanelListFactory::PanelListFactory()
    : PanelListDescriptor("Zelda: Tears of the Kingdom")
{}

std::vector<PanelEntry> PanelListFactory::make_panels() const{
    std::vector<PanelEntry> ret;
    
    ret.emplace_back("---- Settings ----");
    ret.emplace_back(make_settings<GameSettings_Descriptor, GameSettingsPanel>());

    ret.emplace_back("---- General ----");
    ret.emplace_back(make_single_switch_program<BowItemDuper_Descriptor, BowItemDuper>());

#ifdef PA_OFFICIAL
    if (PreloadSettings::instance().DEVELOPER_MODE){
        ret.emplace_back("---- Research ----");
        add_panels(ret);
    }
#endif

    return ret;
}




}
}
}
