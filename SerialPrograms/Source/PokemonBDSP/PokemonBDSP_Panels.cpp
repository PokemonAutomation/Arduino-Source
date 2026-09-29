/*  Pokemon BD/SP Panels
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/StaticGlobals.h"
#include "CommonFramework/GlobalAutoPaths.h"
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

#include "Programs/RngManipulation/PokemonBDSP_BedroomSeedFinder.h"
#include "Programs/RngManipulation/PokemonBDSP_IntroSeedFinder.h"

#include "Programs/TestPrograms/PokemonBDSP_ShinyEncounterTester.h"
#include "Programs/TestPrograms/PokemonBDSP_SoundListener.h"
#include "Programs/TestPrograms/PokemonBDSP_SummaryReaderTester.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{



PanelListFactory::PanelListFactory()
    : PanelListDescriptor(
        Pokemon::STRING_POKEMON + " Brilliant Diamond and Shining Pearl",
        RESOURCE_PATH() + "CategoryIcons/PokemonBDSP.png"
    )
{}

std::vector<PanelEntry> PanelListFactory::make_panels() const{
    std::vector<PanelEntry> ret;

    ret.emplace_back("---- Settings ----");
    ret.emplace_back(make_OptionsPanel<GameSettingsPanel>());

    ret.emplace_back("---- General ----");
    ret.emplace_back(make_SingleSwitchProgram<MassRelease>());
    ret.emplace_back(make_SingleSwitchProgram<AutonomousBallThrower>());

    ret.emplace_back("---- Trading ----");
    ret.emplace_back(make_MultiSwitchProgram<SelfBoxTrade>());
    ret.emplace_back(make_MultiSwitchProgram<SelfTouchTrade>());

    ret.emplace_back("---- Farming ----");
    ret.emplace_back(make_SingleSwitchProgram<MoneyFarmerRoute212>());
    ret.emplace_back(make_SingleSwitchProgram<MoneyFarmerRoute210>());
    ret.emplace_back(make_SingleSwitchProgram<DoublesLeveling>());
    ret.emplace_back(make_SingleSwitchProgram<AmitySquarePickUpFarmer>());
    ret.emplace_back(make_SingleSwitchProgram<GiftBerryReset>());
    ret.emplace_back(make_SingleSwitchProgram<PoffinCooker>());

    ret.emplace_back("---- Shiny Hunting ----");
    ret.emplace_back(make_SingleSwitchProgram<StarterReset>());
    ret.emplace_back(make_SingleSwitchProgram<LegendaryReset>());
    ret.emplace_back(make_SingleSwitchProgram<ShinyHuntOverworld>());
    ret.emplace_back(make_SingleSwitchProgram<ShinyHuntFishing>());
    ret.emplace_back(make_SingleSwitchProgram<ShinyHuntShaymin>());

    ret.emplace_back("---- Eggs ----");
    ret.emplace_back(make_SingleSwitchProgram<EggFetcher>());
    ret.emplace_back(make_SingleSwitchProgram<EggHatcher>());
    ret.emplace_back(make_SingleSwitchProgram<EggAutonomous>());

    ret.emplace_back("---- Glitches (v1.1.3) ----");
    ret.emplace_back(make_SingleSwitchProgram<ActivateMenuGlitch113>());
    ret.emplace_back(make_SingleSwitchProgram<CloneItemsBoxCopy2>());

    ret.emplace_back("---- Glitches (v1.1.2) ----");
    ret.emplace_back(make_SingleSwitchProgram<ActivateMenuGlitch112>());

    if (IS_BETA_VERSION || STATIC_GLOBALS.DEVELOPER_MODE){
        ret.emplace_back("---- Untested/Beta/WIP ----");
        ret.emplace_back(make_SingleSwitchProgram<IntroSeedFinder>());
        ret.emplace_back(make_SingleSwitchProgram<BedroomSeedFinder>());
    }
    if (STATIC_GLOBALS.DEVELOPER_MODE){
        ret.emplace_back("---- Developer Tools ----");
        ret.emplace_back(make_SingleSwitchProgram<ShinyEncounterTester>());
        ret.emplace_back(make_SingleSwitchProgram<SoundListener>());
        ret.emplace_back(make_SingleSwitchProgram<SummaryReaderTester>());
    }

    return ret;
}



}
}
}
