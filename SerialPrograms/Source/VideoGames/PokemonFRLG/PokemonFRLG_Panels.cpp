/*  Pokemon FRLG Panels
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/StaticGlobals.h"
#include "CommonFramework/GlobalAutoPaths.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonFRLG_Panels.h"

#include "PokemonFRLG_Settings.h"
#include "Programs/Farming/PokemonFRLG_HeldItemFarmer-SafariZone.h"
#include "Programs/Farming/PokemonFRLG_ItemDuplication.h"
#include "Programs/Farming/PokemonFRLG_NuggetBridgeFarmer.h"
#include "Programs/Farming/PokemonFRLG_PickupFarmer.h"
#include "Programs/Farming/PokemonFRLG_EvTrainer.h"
#include "Programs/ShinyHunting/PokemonFRLG_GiftReset.h"
#include "Programs/ShinyHunting/PokemonFRLG_LegendaryReset.h"
#include "Programs/ShinyHunting/PokemonFRLG_LegendaryRunAway.h"
#include "Programs/ShinyHunting/PokemonFRLG_PrizeCornerReset.h"
#include "Programs/ShinyHunting/PokemonFRLG_ShinyHunt-Fishing.h"
#include "Programs/ShinyHunting/PokemonFRLG_ShinyHunt-Overworld.h"
#include "Programs/RngManipulation/PokemonFRLG_RngHelper.h"
#include "Programs/RngManipulation/PokemonFRLG_SidHelper.h"
#include "Programs/RngManipulation/PokemonFRLG_StarterRng.h"
#include "Programs/RngManipulation/PokemonFRLG_GiftRng.h"
#include "Programs/RngManipulation/PokemonFRLG_StaticRng.h"
#include "Programs/RngManipulation/PokemonFRLG_WildRng.h"
#include "Programs/RngManipulation/PokemonFRLG_RoamingLegendaryRng.h"
#include "Programs/RngManipulation/PokemonFRLG_EggRng.h"
#include "Programs/TestPrograms/PokemonFRLG_SoundListener.h"
#include "Programs/TestPrograms/PokemonFRLG_ReadStats.h"
#include "Programs/TestPrograms/PokemonFRLG_ReadBattleLevelUp.h"
#include "Programs/TestPrograms/PokemonFRLG_ReadTrainerId.h"
#include "Programs/TestPrograms/PokemonFRLG_ReadEncounter.h"
#include "Programs/TestPrograms/PokemonFRLG_SafariOptimalActionTest.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{



PanelListFactory::PanelListFactory()
    : PanelListDescriptor(
        Pokemon::STRING_POKEMON + " FireRed and LeafGreen",
        RESOURCE_PATH() + "CategoryIcons/PokemonFRLG.png"
    )
{}

std::vector<PanelEntry> PanelListFactory::make_panels() const{
    std::vector<PanelEntry> ret;
    
    ret.emplace_back("---- Settings ----");
    ret.emplace_back(make_OptionsPanel<GameSettingsPanel>());

    ret.emplace_back("---- Farming ----");
    ret.emplace_back(make_SingleSwitchProgram<NuggetBridgeFarmer>());
    ret.emplace_back(make_SingleSwitchProgram<PickupFarmer>());
    ret.emplace_back(make_SingleSwitchProgram<EvTrainer>());
    ret.emplace_back(make_SingleSwitchProgram<HeldItemFarmerSafariZone>());
    ret.emplace_back(make_SingleSwitchProgram<ItemDuplication>());

    //ret.emplace_back("---- General ----");

    ret.emplace_back("---- Shiny Hunting  ----");
    ret.emplace_back(make_SingleSwitchProgram<GiftReset>());
    ret.emplace_back(make_SingleSwitchProgram<LegendaryReset>());
    ret.emplace_back(make_SingleSwitchProgram<LegendaryRunAway>());
    ret.emplace_back(make_SingleSwitchProgram<ShinyHuntFishing>());
    ret.emplace_back(make_SingleSwitchProgram<ShinyHuntOverworld>());
    ret.emplace_back(make_SingleSwitchProgram<PrizeCornerReset>());

    ret.emplace_back("---- RNG Manipulation  ----");
    ret.emplace_back(make_SingleSwitchProgram<RngHelper>());
    ret.emplace_back(make_SingleSwitchProgram<SidHelper>());
    ret.emplace_back(make_SingleSwitchProgram<StarterRng>());
    ret.emplace_back(make_SingleSwitchProgram<GiftRng>());
    ret.emplace_back(make_SingleSwitchProgram<StaticRng>());
    ret.emplace_back(make_SingleSwitchProgram<WildRng>());
    ret.emplace_back(make_SingleSwitchProgram<RoamingLegendaryRng>());

    if (IS_BETA_VERSION || STATIC_GLOBALS.DEVELOPER_MODE){
        ret.emplace_back("---- Untested/Beta/WIP ----");
        ret.emplace_back(make_SingleSwitchProgram<EggRng>());
    }

    if (STATIC_GLOBALS.DEVELOPER_MODE){
        ret.emplace_back("---- Developer Tools ----");
        ret.emplace_back(make_SingleSwitchProgram<SoundListener>());
        ret.emplace_back(make_SingleSwitchProgram<ReadStats>());
        ret.emplace_back(make_SingleSwitchProgram<ReadBattleLevelUp>());
        ret.emplace_back(make_SingleSwitchProgram<ReadTrainerId>());
        ret.emplace_back(make_SingleSwitchProgram<ReadEncounter>());
        ret.emplace_back(make_SingleSwitchProgram<SafariOptimalActionTest>());
    }

    return ret;
}




}
}
}
