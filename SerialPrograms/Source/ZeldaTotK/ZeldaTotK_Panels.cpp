/*  Zelda: TotK Panels
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/GlobalSettingsPanel.h"
#include "ZeldaTotK_Panels.h"

#include "ZeldaTotK_Settings.h"

#include "Programs/ZeldaTotK_BowItemDuper.h"


namespace PokemonAutomation{
namespace NintendoSwitch{
namespace ZeldaTotK{



PanelListFactory::PanelListFactory()
    : PanelListDescriptor("Zelda: Tears of the Kingdom")
{}

std::vector<PanelEntry> PanelListFactory::make_panels() const{
    std::vector<PanelEntry> ret;
    
    // ret.emplace_back("---- Settings ----");
    // ret.emplace_back(make_settings<GameSettings_Descriptor, GameSettingsPanel>());

    ret.emplace_back("---- General ----");
    ret.emplace_back(make_single_switch_program<BowItemDuper_Descriptor, BowItemDuper>());

    return ret;
}




}
}
}
