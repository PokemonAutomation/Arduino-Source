/*  Zelda: TotK Panels
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

//#include "CommonFramework/GlobalSettingsPanel.h"
#include "ZeldaTotK_Panels.h"

//#include "ZeldaTotK_Settings.h"

#include "Programs/ZeldaTotK_BowItemDuper.h"
#include "Programs/ZeldaTotK_MineruItemDuper.h"
#include "Programs/ZeldaTotK_ParaglideItemDuper.h"
#include "Programs/ZeldaTotK_SurfItemDuper.h"
#include "Programs/ZeldaTotK_WeaponDuper.h"

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

    //ret.emplace_back("---- General ----");

    ret.emplace_back("---- Glitches (v1.1.1) ----");
    ret.emplace_back(make_single_switch_program<BowItemDuper_Descriptor, BowItemDuper>());
    ret.emplace_back(make_single_switch_program<MineruItemDuper_Descriptor, MineruItemDuper>());
    ret.emplace_back(make_single_switch_program<ParaglideItemDuper_Descriptor, ParaglideItemDuper>());
    ret.emplace_back(make_single_switch_program<SurfItemDuper_Descriptor, SurfItemDuper>());
    ret.emplace_back(make_single_switch_program<WeaponDuper_Descriptor, WeaponDuper>());

    return ret;
}




}
}
}
