/*  Pokemon BD/SP Panels
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/GlobalSettingsPanel.h"
#include "PokemonBDSP_Panels.h"

#include "PokemonBDSP_Settings.h"

#include "Programs/General/PokemonBDSP_MassRelease.h"
#include "Programs/General/PokemonBDSP_AutonomousBallThrower.h"

#include "Programs/Trading/PokemonBDSP_SelfBoxTrade.h"
#include "Programs/Trading/PokemonBDSP_SelfTouchTrade.h"

#include "Programs/Farming/PokemonBDSP_MoneyFarmerRoute210.h"
#include "Programs/Farming/PokemonBDSP_MoneyFarmerRoute212.h"
#include "Programs/Farming/PokemonBDSP_DoublesLeveling.h"
#include "Programs/Farming/PokemonBDSP_AmitySquarePickUpFarmer.h"
#include "Programs/Farming/PokemonBDSP_PoffinCooker.h"
#include "Programs/Farming/PokemonBDSP_GiftBerryReset.h"

#include "Programs/ShinyHunting/PokemonBDSP_StarterReset.h"
#include "Programs/ShinyHunting/PokemonBDSP_LegendaryReset.h"
#include "Programs/ShinyHunting/PokemonBDSP_ShinyHunt-Overworld.h"
#include "Programs/ShinyHunting/PokemonBDSP_ShinyHunt-Fishing.h"
#include "Programs/ShinyHunting/PokemonBDSP_ShinyHunt-Shaymin.h"

#include "Programs/Eggs/PokemonBDSP_EggFetcher.h"
#include "Programs/Eggs/PokemonBDSP_EggHatcher.h"
#include "Programs/Eggs/PokemonBDSP_EggAutonomous.h"

#include "Programs/Glitches/PokemonBDSP_ActivateMenuGlitch-1.1.3.h"
#include "Programs/Glitches/PokemonBDSP_ActivateMenuGlitch-1.1.2.h"
#include "Programs/Glitches/PokemonBDSP_CloneItemsBoxCopy2.h"
//#include "Programs/Glitches/PokemonBDSP_CloneItemsBoxCopy.h"
//#include "Programs/Glitches/PokemonBDSP_CloneItemsMenuOverlap.h"

#include "Programs/TestPrograms/PokemonBDSP_ShinyEncounterTester.h"
#include "Programs/TestPrograms/PokemonBDSP_SoundListener.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{



PanelListFactory::PanelListFactory()
    : PanelListDescriptor(Pokemon::STRING_POKEMON + " Brilliant Diamond and Shining Pearl")
{}

std::vector<PanelEntry> PanelListFactory::make_panels() const{
    std::vector<PanelEntry> ret;

    ret.emplace_back("---- Settings ----");
    ret.emplace_back(make_settings<GameSettings_Descriptor, GameSettingsPanel>());

    ret.emplace_back("---- General ----");
    ret.emplace_back(make_single_switch_program<MassRelease_Descriptor, MassRelease>());
    ret.emplace_back(make_single_switch_program<AutonomousBallThrower_Descriptor, AutonomousBallThrower>());

    ret.emplace_back("---- Trading ----");
    ret.emplace_back(make_multi_switch_program<SelfBoxTrade_Descriptor, SelfBoxTrade>());
    ret.emplace_back(make_multi_switch_program<SelfTouchTrade_Descriptor, SelfTouchTrade>());

    ret.emplace_back("---- Farming ----");
    ret.emplace_back(make_single_switch_program<MoneyFarmerRoute212_Descriptor, MoneyFarmerRoute212>());
    ret.emplace_back(make_single_switch_program<MoneyFarmerRoute210_Descriptor, MoneyFarmerRoute210>());
    ret.emplace_back(make_single_switch_program<DoublesLeveling_Descriptor, DoublesLeveling>());
    ret.emplace_back(make_single_switch_program<AmitySquarePickUpFarmer_Descriptor, AmitySquarePickUpFarmer>());
    ret.emplace_back(make_single_switch_program<GiftBerryReset_Descriptor, GiftBerryReset>());
    ret.emplace_back(make_single_switch_program<PoffinCooker_Descriptor, PoffinCooker>());

    ret.emplace_back("---- Shiny Hunting ----");
    ret.emplace_back(make_single_switch_program<StarterReset_Descriptor, StarterReset>());
    ret.emplace_back(make_single_switch_program<LegendaryReset_Descriptor, LegendaryReset>());
    ret.emplace_back(make_single_switch_program<ShinyHuntOverworld_Descriptor, ShinyHuntOverworld>());
    ret.emplace_back(make_single_switch_program<ShinyHuntFishing_Descriptor, ShinyHuntFishing>());
    ret.emplace_back(make_single_switch_program<ShinyHuntShaymin_Descriptor, ShinyHuntShaymin>());

    ret.emplace_back("---- Eggs ----");
    ret.emplace_back(make_single_switch_program<EggFetcher_Descriptor, EggFetcher>());
    ret.emplace_back(make_single_switch_program<EggHatcher_Descriptor, EggHatcher>());
    ret.emplace_back(make_single_switch_program<EggAutonomous_Descriptor, EggAutonomous>());

    ret.emplace_back("---- Glitches (v1.1.3) ----");
    ret.emplace_back(make_single_switch_program<ActivateMenuGlitch113_Descriptor, ActivateMenuGlitch113>());
    ret.emplace_back(make_single_switch_program<CloneItemsBoxCopy2_Descriptor, CloneItemsBoxCopy2>());

    ret.emplace_back("---- Glitches (v1.1.2) ----");
    ret.emplace_back(make_single_switch_program<ActivateMenuGlitch112_Descriptor, ActivateMenuGlitch112>());

    if (IS_BETA_VERSION || PreloadSettings::instance().DEVELOPER_MODE){
        ret.emplace_back("---- Untested/Beta/WIP ----");
    }
    if (PreloadSettings::instance().DEVELOPER_MODE){
        ret.emplace_back("---- Developer Tools ----");
        ret.emplace_back(make_single_switch_program<ShinyEncounterTester_Descriptor, ShinyEncounterTester>());
        ret.emplace_back(make_single_switch_program<SoundListener_Descriptor, SoundListener>());
    }

    return ret;
}



}
}
}
