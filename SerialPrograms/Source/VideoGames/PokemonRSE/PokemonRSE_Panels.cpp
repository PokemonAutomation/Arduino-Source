/*  Pokemon RSE Panels
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/StaticGlobals.h"
#include "CommonFramework/GlobalAutoPaths.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonRSE_Panels.h"

#include "PokemonRSE_Settings.h"

#include "Programs/ShinyHunting/PokemonRSE_AudioStarterReset.h"
#include "Programs/ShinyHunting/PokemonRSE_GiftReset.h"
#include "Programs/ShinyHunting/PokemonRSE_LegendaryReset.h"
#include "Programs/ShinyHunting/PokemonRSE_LegendaryRunAway-Emerald.h"
#include "Programs/ShinyHunting/PokemonRSE_ShinyHunt-Deoxys.h"
#include "Programs/ShinyHunting/PokemonRSE_ShinyHunt-Mew.h"

#include "Programs/ShinyHunting/PokemonRSE_StarterReset.h"
#include "Programs/TestPrograms/PokemonRSE_SoundListener.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonRSE{



PanelListFactory::PanelListFactory()
    : PanelListDescriptor(
        Pokemon::STRING_POKEMON + " Ruby, Sapphire, and Emerald",
        RESOURCE_PATH() + "CategoryIcons/PokemonRS.png"
    )
{}

std::vector<PanelEntry> PanelListFactory::make_panels() const{
    std::vector<PanelEntry> ret;
    
    ret.emplace_back("---- Settings ----");
    ret.emplace_back(make_OptionsPanel<GameSettingsPanel>());

    //ret.emplace_back("---- General ----");

    ret.emplace_back("---- Shiny Hunting (Ruby/Sapphire) ----"); //Remove RS only if E is fixed
    ret.emplace_back(make_SingleSwitchProgram<AudioStarterReset>());
    ret.emplace_back(make_SingleSwitchProgram<LegendaryReset>());
    ret.emplace_back(make_SingleSwitchProgram<GiftReset>());

    ret.emplace_back("---- Shiny Hunting (Emerald only) ----");
    ret.emplace_back(make_SingleSwitchProgram<LegendaryRunAwayEmerald>());
    ret.emplace_back(make_SingleSwitchProgram<ShinyHuntDeoxys>());
    ret.emplace_back(make_SingleSwitchProgram<ShinyHuntMew>());
    

    if (STATIC_GLOBALS.DEVELOPER_MODE){
        ret.emplace_back("---- Test ----");
        ret.emplace_back(make_SingleSwitchProgram<StarterReset>()); //outdated early test program

        ret.emplace_back("---- Developer Tools ----");
        ret.emplace_back(make_SingleSwitchProgram<SoundListener>());
    }

    return ret;
}




}
}
}
