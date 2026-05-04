/*  Pokemon FRLG Panels
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/GlobalSettingsPanel.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonFRLG_Panels.h"

#include "PokemonFRLG_Settings.h"
#include "Programs/Farming/PokemonFRLG_ItemDuplication.h"
#include "Programs/Farming/PokemonFRLG_LuckyEggFarmer.h"
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
#include "Programs/TestPrograms/PokemonFRLG_SoundListener.h"
#include "Programs/TestPrograms/PokemonFRLG_ReadStats.h"
#include "Programs/TestPrograms/PokemonFRLG_ReadBattleLevelUp.h"
#include "Programs/TestPrograms/PokemonFRLG_ReadTrainerId.h"
#include "Programs/TestPrograms/PokemonFRLG_ReadEncounter.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{



PanelListFactory::PanelListFactory()
    : PanelListDescriptor(Pokemon::STRING_POKEMON + " FireRed and LeafGreen")
{}

std::vector<PanelEntry> PanelListFactory::make_panels() const{
    std::vector<PanelEntry> ret;
    
    ret.emplace_back("---- Settings ----");
    ret.emplace_back(make_settings<GameSettings_Descriptor, GameSettingsPanel>());

    ret.emplace_back("---- Farming ----");
    ret.emplace_back(make_single_switch_program<NuggetBridgeFarmer_Descriptor, NuggetBridgeFarmer>());
    ret.emplace_back(make_single_switch_program<PickupFarmer_Descriptor, PickupFarmer>());
    ret.emplace_back(make_single_switch_program<EvTrainer_Descriptor, EvTrainer>());
    ret.emplace_back(make_single_switch_program<LuckyEggFarmer_Descriptor, LuckyEggFarmer>());
    ret.emplace_back(make_single_switch_program<ItemDuplication_Descriptor, ItemDuplication>());

    //ret.emplace_back("---- General ----");

    ret.emplace_back("---- Shiny Hunting  ----");
    ret.emplace_back(make_single_switch_program<GiftReset_Descriptor, GiftReset>());
    ret.emplace_back(make_single_switch_program<LegendaryReset_Descriptor, LegendaryReset>());
    ret.emplace_back(make_single_switch_program<LegendaryRunAway_Descriptor, LegendaryRunAway>());
    ret.emplace_back(make_single_switch_program<ShinyHuntFishing_Descriptor, ShinyHuntFishing>());
    ret.emplace_back(make_single_switch_program<ShinyHuntOverworld_Descriptor, ShinyHuntOverworld>());
    ret.emplace_back(make_single_switch_program<PrizeCornerReset_Descriptor, PrizeCornerReset>());

    ret.emplace_back("---- RNG Manipulation  ----");
    ret.emplace_back(make_single_switch_program<RngHelper_Descriptor, RngHelper>());

    if (IS_BETA_VERSION || PreloadSettings::instance().DEVELOPER_MODE){
        ret.emplace_back("---- Untested/Beta/WIP ----");
        ret.emplace_back(make_single_switch_program<SidHelper_Descriptor, SidHelper>());
        ret.emplace_back(make_single_switch_program<StarterRng_Descriptor, StarterRng>());
        ret.emplace_back(make_single_switch_program<GiftRng_Descriptor, GiftRng>());
        ret.emplace_back(make_single_switch_program<StaticRng_Descriptor, StaticRng>());
        ret.emplace_back(make_single_switch_program<WildRng_Descriptor, WildRng>());
    }

    if (PreloadSettings::instance().DEVELOPER_MODE){
        ret.emplace_back("---- Developer Tools ----");
        ret.emplace_back(make_single_switch_program<SoundListener_Descriptor, SoundListener>());
        ret.emplace_back(make_single_switch_program<ReadStats_Descriptor, ReadStats>());
        ret.emplace_back(make_single_switch_program<ReadBattleLevelUp_Descriptor, ReadBattleLevelUp>());
        ret.emplace_back(make_single_switch_program<ReadTrainerId_Descriptor, ReadTrainerId>());
        ret.emplace_back(make_single_switch_program<ReadEncounter_Descriptor, ReadEncounter>());    
    }

    return ret;
}




}
}
}
