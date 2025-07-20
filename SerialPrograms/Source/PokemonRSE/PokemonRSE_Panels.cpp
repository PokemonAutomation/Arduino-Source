/*  Pokemon RSE Panels
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/GlobalSettingsPanel.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonRSE_Panels.h"

#include "PokemonRSE_Settings.h"

#include "Programs/ShinyHunting/PokemonRSE_AudioStarterReset.h"
#include "Programs/ShinyHunting/PokemonRSE_LegendaryHunt-Emerald.h"
#include "Programs/ShinyHunting/PokemonRSE_ShinyHunt-Deoxys.h"
#include "Programs/ShinyHunting/PokemonRSE_ShinyHunt-Mew.h"

#include "Programs/ShinyHunting/PokemonRSE_StarterReset.h"
#include "Programs/TestPrograms/PokemonRSE_SoundListener.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonRSE{



PanelListFactory::PanelListFactory()
    : PanelListDescriptor(Pokemon::STRING_POKEMON + " Ruby, Sapphire, and Emerald")
{}

std::vector<PanelEntry> PanelListFactory::make_panels() const{
    std::vector<PanelEntry> ret;
    
    ret.emplace_back("---- Settings ----");
    ret.emplace_back(make_settings<GameSettings_Descriptor, GameSettingsPanel>());

    //ret.emplace_back("---- General ----");

    ret.emplace_back("---- Shiny Hunting (Ruby/Sapphire) ----");
    ret.emplace_back(make_single_switch_program<AudioStarterReset_Descriptor, AudioStarterReset>());

    ret.emplace_back("---- Shiny Hunting (Emerald) ----");
    ret.emplace_back(make_single_switch_program<LegendaryHuntEmerald_Descriptor, LegendaryHuntEmerald>());
    ret.emplace_back(make_single_switch_program<ShinyHuntDeoxys_Descriptor, ShinyHuntDeoxys>());
    ret.emplace_back(make_single_switch_program<ShinyHuntMew_Descriptor, ShinyHuntMew>());
    

    if (PreloadSettings::instance().DEVELOPER_MODE){
        ret.emplace_back("---- Test ----");
        ret.emplace_back(make_single_switch_program<StarterReset_Descriptor, StarterReset>()); //outdated early test program

        ret.emplace_back("---- Developer Tools ----");
        ret.emplace_back(make_single_switch_program<SoundListener_Descriptor, SoundListener>());
    }

    return ret;
}




}
}
}
