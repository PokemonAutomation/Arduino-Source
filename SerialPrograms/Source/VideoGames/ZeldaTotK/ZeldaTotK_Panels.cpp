/*  Zelda: TotK Panels
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/GlobalAutoPaths.h"
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
    : PanelListDescriptor(
        "Zelda: Tears of the Kingdom",
        RESOURCE_PATH() + "CategoryIcons/ZeldaTotK.png"
    )
{}

std::vector<PanelEntry> PanelListFactory::make_panels() const{
    std::vector<PanelEntry> ret;
    
    // ret.emplace_back("---- Settings ----");
    // ret.emplace_back(make_settings<GameSettings_Descriptor, GameSettingsPanel>());

    //ret.emplace_back("---- General ----");

    ret.emplace_back("---- Glitches (v1.1.1) ----");
    ret.emplace_back(make_SingleSwitchProgram<BowItemDuper>());
    ret.emplace_back(make_SingleSwitchProgram<MineruItemDuper>());
    ret.emplace_back(make_SingleSwitchProgram<ParaglideItemDuper>());
    ret.emplace_back(make_SingleSwitchProgram<SurfItemDuper>());
    ret.emplace_back(make_SingleSwitchProgram<WeaponDuper>());

    return ret;
}




}
}
}
