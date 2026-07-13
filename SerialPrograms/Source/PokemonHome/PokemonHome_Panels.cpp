/*  Pokemon Home Panels
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Globals.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonHome_Panels.h"

#include "Programs/PokemonHome_PageSwap.h"
#include "Programs/PokemonHome_BoxSorter.h"
#include "Programs/PokemonHome_BoxSorterLivingDex.h"

#include "Programs/PokemonHome_GenerateNameOCR.h"
#include "Programs/TestPrograms/PokemonHome_ReadSummaryScreen.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonHome{



PanelListFactory::PanelListFactory()
    : PanelListDescriptor(
        Pokemon::STRING_POKEMON + " Home",
        RESOURCE_PATH() + "CategoryIcons/PokemonHome.png"
    )
{}

std::vector<PanelEntry> PanelListFactory::make_panels() const{
    std::vector<PanelEntry> ret;

//    ret.emplace_back("---- Settings ----");
//    ret.emplace_back(make_settings<GameSettings_Descriptor, GameSettingsPanel>());
    ret.emplace_back("---- General ----");
    ret.emplace_back(make_single_switch_program<PokemonHome::PageSwap_Descriptor, PokemonHome::PageSwap>());
    ret.emplace_back(make_single_switch_program<PokemonHome::BoxSorter_Descriptor, PokemonHome::BoxSorter>());
    ret.emplace_back(make_single_switch_program<PokemonHome::BoxSorterLivingDex_Descriptor, PokemonHome::BoxSorterLivingDex>());
    if (IS_BETA_VERSION || PreloadSettings::instance().DEVELOPER_MODE){
    }
//    ret.emplace_back("---- Trading ----");

//    ret.emplace_back("---- Farming ----");

//    ret.emplace_back("---- Shiny Hunting ----");

    if (PreloadSettings::instance().DEVELOPER_MODE){
        ret.emplace_back("---- Developer Tools ----");
        ret.emplace_back(make_single_switch_program<PokemonHome::GenerateNameOCRData_Descriptor, PokemonHome::GenerateNameOCRData>());
        ret.emplace_back(make_single_switch_program<PokemonHome::ReadSummaryScreen_Descriptor, PokemonHome::ReadSummaryScreen>());
    }

    return ret;
}




}
}
}
