/*  Pokemon RSE Panels
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/GlobalSettingsPanel.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonRSE_Panels.h"

#include "PokemonRSE_Settings.h"

//#include "Programs/ShinyHunting/PokemonRSE_AudioStarterReset.h"
//#include "Programs/ShinyHunting/PokemonRSE_StarterReset.h"
#include "Programs/TestPrograms/PokemonRSE_SoundListener.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonRSE{



PanelListFactory::PanelListFactory()
    : PanelListDescriptor("Pokemon Ruby and Sapphire, Pokemon Emerald")
{}

std::vector<PanelEntry> PanelListFactory::make_panels() const{
    std::vector<PanelEntry> ret;
    
    ret.emplace_back("---- Settings ----");
    ret.emplace_back(make_settings<GameSettings_Descriptor, GameSettingsPanel>());

    //ret.emplace_back("---- General ----");

    ret.emplace_back("---- Shiny Hunting ----");
    //ret.emplace_back(make_single_switch_program<AudioStarterReset_Descriptor, AudioStarterReset>());
    

    if (PreloadSettings::instance().DEVELOPER_MODE){
        ret.emplace_back("---- Test ----");
        //ret.emplace_back(make_single_switch_program<StarterReset_Descriptor, StarterReset>()); //outdated early test program

        ret.emplace_back("---- Developer Tools ----");
        ret.emplace_back(make_single_switch_program<SoundListener_Descriptor, SoundListener>());
    }

    return ret;
}




}
}
}
