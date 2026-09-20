/*  Pokemon Sword/Shield Panels
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/StaticGlobals.h"
#include "CommonFramework/GlobalAutoPaths.h"
#include "PokemonSwSh_Panels.h"

#include "PokemonSwSh_Settings.h"

#include "Programs/QoLMacros/PokemonSwSh_FastCodeEntry.h"
#include "Programs/QoLMacros/PokemonSwSh_FriendSearchDisconnect.h"

#include "Programs/General/PokemonSwSh_MassRelease.h"
#include "Programs/General/PokemonSwSh_SurpriseTrade.h"
#include "Programs/General/PokemonSwSh_TradeBot.h"
#include "Programs/General/PokemonSwSh_ClothingBuyer.h"
#include "Programs/General/PokemonSwSh_AutonomousBallThrower.h"
#include "Programs/General/PokemonSwSh_DexRecFinder.h"
#include "Programs/General/PokemonSwSh_BoxReorderNationalDex.h"

#include "Programs/DateSpamFarmers/PokemonSwSh_DateSpam-WattFarmer.h"
#include "Programs/DateSpamFarmers/PokemonSwSh_DateSpam-BerryFarmer.h"
#include "Programs/DateSpamFarmers/PokemonSwSh_DateSpam-BerryFarmer2.h"
#include "Programs/DateSpamFarmers/PokemonSwSh_DateSpam-LotoFarmer.h"
#include "Programs/DateSpamFarmers/PokemonSwSh_DateSpam-StowOnSideFarmer.h"
#include "Programs/DateSpamFarmers/PokemonSwSh_DateSpam-DailyHighlightFarmer.h"
#include "Programs/DateSpamFarmers/PokemonSwSh_DateSpam-PokeJobsFarmer.h"
#include "Programs/DateSpamFarmers/PokemonSwSh_DateSpam-WattTraderFarmer.h"

#include "Programs/DenHunting/PokemonSwSh_PurpleBeamFinder.h"
#include "Programs/DenHunting/PokemonSwSh_EventBeamFinder.h"
#include "Programs/DenHunting/PokemonSwSh_DaySkipperJPN.h"
#include "Programs/DenHunting/PokemonSwSh_DaySkipperEU.h"
#include "Programs/DenHunting/PokemonSwSh_DaySkipperUS.h"
#include "Programs/DenHunting/PokemonSwSh_DaySkipperJPN-7.8k.h"

#include "Programs/Hosting/PokemonSwSh_DenRoller.h"
#include "Programs/Hosting/PokemonSwSh_AutoHost-Rolling.h"
#include "Programs/Hosting/PokemonSwSh_AutoHost-MultiGame.h"

#include "Programs/NonShinyHunting/PokemonSwSh_StatsReset.h"
#include "Programs/NonShinyHunting/PokemonSwSh_StatsReset-Calyrex.h"
#include "Programs/NonShinyHunting/PokemonSwSh_StatsReset-Moltres.h"
#include "Programs/NonShinyHunting/PokemonSwSh_StatsReset-Regi.h"

#include "Programs/EggPrograms/PokemonSwSh_EggAutonomous.h"
#include "Programs/EggPrograms/PokemonSwSh_EggFetcher2.h"
#include "Programs/EggPrograms/PokemonSwSh_EggFetcherMultiple.h"
#include "Programs/EggPrograms/PokemonSwSh_EggHatcher.h"
#include "Programs/EggPrograms/PokemonSwSh_GodEggDuplication.h"
#include "Programs/EggPrograms/PokemonSwSh_GodEggItemDupe.h"

#include "Programs/ShinyHuntUnattended/PokemonSwSh_MultiGameFossil.h"
#include "Programs/ShinyHuntUnattended/PokemonSwSh_CurryHunter.h"

#include "Programs/ShinyHuntAutonomous/PokemonSwSh_ShinyHuntAutonomous-Regi.h"
#include "Programs/ShinyHuntAutonomous/PokemonSwSh_ShinyHuntAutonomous-SwordsOfJustice.h"
#include "Programs/ShinyHuntAutonomous/PokemonSwSh_ShinyHuntAutonomous-StrongSpawn.h"
#include "Programs/ShinyHuntAutonomous/PokemonSwSh_ShinyHuntAutonomous-Regigigas2.h"
#include "Programs/ShinyHuntAutonomous/PokemonSwSh_ShinyHuntAutonomous-IoATrade.h"
#include "Programs/ShinyHuntAutonomous/PokemonSwSh_ShinyHuntAutonomous-BerryTree.h"
#include "Programs/ShinyHuntAutonomous/PokemonSwSh_ShinyHuntAutonomous-Whistling.h"
#include "Programs/ShinyHuntAutonomous/PokemonSwSh_ShinyHuntAutonomous-Fishing.h"
#include "Programs/OverworldBot/PokemonSwSh_ShinyHuntAutonomous-Overworld.h"

#include "Programs/RNG/PokemonSwSh_CramomaticRNG.h"
#include "Programs/RNG/PokemonSwSh_DailyHighlightRNG.h"
#include "Programs/RNG/PokemonSwSh_SeedFinder.h"

#include "Programs/PokemonSwSh_SynchronizedSpinning.h"
#include "Programs/PokemonSwSh_RaidItemFarmerOKHO.h"

#include "MaxLair/PokemonSwSh_MaxLair_Standard.h"
#include "MaxLair/PokemonSwSh_MaxLair_StrongBoss.h"
#include "MaxLair/PokemonSwSh_MaxLair_BossFinder.h"

#include "Programs/TestPrograms/PokemonSwSh_ShinyEncounterTester.h"
#include "InferenceTraining/PokemonSwSh_GenerateIVCheckerOCR.h"
#include "InferenceTraining/PokemonSwSh_GenerateNameOCRPokedex.h"
#include "InferenceTraining/PokemonSwSh_GeneratePokedexSprites.h"


namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{



PanelListFactory::PanelListFactory()
    : PanelListDescriptor(
        Pokemon::STRING_POKEMON + " Sword and Shield",
        RESOURCE_PATH() + "CategoryIcons/PokemonSwSh.png"
    )
{}

std::vector<PanelEntry> PanelListFactory::make_panels() const{
    std::vector<PanelEntry> ret;

    ret.emplace_back("---- Settings ----");
    ret.emplace_back(make_OptionsPanel<GameSettingsPanel>());

    ret.emplace_back("---- QoL Macros ----");
    ret.emplace_back(make_SingleSwitchProgram<FastCodeEntry>());
    ret.emplace_back(make_SingleSwitchProgram<FriendSearchDisconnect>());

    ret.emplace_back("---- General ----");
    ret.emplace_back(make_SingleSwitchProgram<MassRelease>());
    ret.emplace_back(make_SingleSwitchProgram<SurpriseTrade>());
    ret.emplace_back(make_SingleSwitchProgram<TradeBot>());
    ret.emplace_back(make_SingleSwitchProgram<ClothingBuyer>());
    ret.emplace_back(make_SingleSwitchProgram<AutonomousBallThrower>());
    ret.emplace_back(make_SingleSwitchProgram<DexRecFinder>());
    ret.emplace_back(make_SingleSwitchProgram<BoxReorderNationalDex>());

    ret.emplace_back("---- Date-Spam Farmers ----");
    ret.emplace_back(make_SingleSwitchProgram<WattFarmer>());
    ret.emplace_back(make_SingleSwitchProgram<BerryFarmer>());
    ret.emplace_back(make_SingleSwitchProgram<BerryFarmer2>());
    ret.emplace_back(make_SingleSwitchProgram<LotoFarmer>());
    ret.emplace_back(make_SingleSwitchProgram<StowOnSideFarmer>());
    ret.emplace_back(make_SingleSwitchProgram<DailyHighlightFarmer>());
    ret.emplace_back(make_SingleSwitchProgram<PokeJobsFarmer>());
    if (STATIC_GLOBALS.DEVELOPER_MODE){
        ret.emplace_back(make_SingleSwitchProgram<WattTraderFarmer>());
    }

    ret.emplace_back("---- Den Hunting ----");
    ret.emplace_back(make_SingleSwitchProgram<PurpleBeamFinder>());
    ret.emplace_back(make_SingleSwitchProgram<EventBeamFinder>());
    ret.emplace_back(make_SingleSwitchProgram<DaySkipperJPN>());
    ret.emplace_back(make_SingleSwitchProgram<DaySkipperEU>());
    ret.emplace_back(make_SingleSwitchProgram<DaySkipperUS>());
    ret.emplace_back(make_SingleSwitchProgram<DaySkipperJPN7p8k>());

    ret.emplace_back("---- Hosting ----");
    ret.emplace_back(make_SingleSwitchProgram<DenRoller>());
    ret.emplace_back(make_SingleSwitchProgram<AutoHostRolling>());
    ret.emplace_back(make_SingleSwitchProgram<AutoHostMultiGame>());

    ret.emplace_back("---- Eggs ----");
    ret.emplace_back(make_SingleSwitchProgram<EggFetcher2>());
    ret.emplace_back(make_SingleSwitchProgram<EggFetcherMultiple>());
    ret.emplace_back(make_SingleSwitchProgram<EggHatcher>());
    ret.emplace_back(make_SingleSwitchProgram<EggAutonomous>());
    ret.emplace_back(make_SingleSwitchProgram<GodEggItemDupe>());
    if (STATIC_GLOBALS.NAUGHTY_MODE || STATIC_GLOBALS.DEVELOPER_MODE){
        ret.emplace_back(make_SingleSwitchProgram<GodEggDuplication>());
    }

    ret.emplace_back("---- Non-Shiny Hunting ----");
    ret.emplace_back(make_SingleSwitchProgram<StatsReset>());
    ret.emplace_back(make_SingleSwitchProgram<StatsResetCalyrex>());
    ret.emplace_back(make_SingleSwitchProgram<StatsResetMoltres>());
    ret.emplace_back(make_SingleSwitchProgram<StatsResetRegi>());

    ret.emplace_back("---- Shiny Hunting ----");
    ret.emplace_back(make_SingleSwitchProgram<MultiGameFossil>());
    ret.emplace_back(make_SingleSwitchProgram<CurryHunter>());
    ret.emplace_back(make_SingleSwitchProgram<ShinyHuntAutonomousRegi>());
    ret.emplace_back(make_SingleSwitchProgram<ShinyHuntAutonomousSwordsOfJustice>());
    ret.emplace_back(make_SingleSwitchProgram<ShinyHuntAutonomousStrongSpawn>());
    ret.emplace_back(make_SingleSwitchProgram<ShinyHuntAutonomousRegigigas2>());
    ret.emplace_back(make_SingleSwitchProgram<ShinyHuntAutonomousIoATrade>());
    ret.emplace_back(make_SingleSwitchProgram<ShinyHuntAutonomousBerryTree>());
    ret.emplace_back(make_SingleSwitchProgram<ShinyHuntAutonomousWhistling>());
    ret.emplace_back(make_SingleSwitchProgram<ShinyHuntAutonomousFishing>());
    ret.emplace_back(make_SingleSwitchProgram<ShinyHuntAutonomousOverworld>());

    ret.emplace_back("---- RNG ----");
    ret.emplace_back(make_SingleSwitchProgram<SeedFinder>());
    ret.emplace_back(make_SingleSwitchProgram<CramomaticRNG>());

    ret.emplace_back("---- Multi-Switch Programs ----");
    ret.emplace_back(make_MultiSwitchProgram<SynchronizedSpinning>());
    ret.emplace_back(make_MultiSwitchProgram<RaidItemFarmerOHKO>());

    ret.emplace_back("---- Auto Max Lair 2.0 ----");
    ret.emplace_back(make_MultiSwitchProgram<MaxLairStandard>());
    ret.emplace_back(make_MultiSwitchProgram<MaxLairStrongBoss>());
    ret.emplace_back(make_MultiSwitchProgram<MaxLairBossFinder>());

    ret.emplace_back("---- Public Betas ----");
    ret.emplace_back(make_SingleSwitchProgram<DailyHighlightRNG>());

#if 0
    ret.emplace_back("---- Deprecated Programs ----");
#endif

    if (STATIC_GLOBALS.DEVELOPER_MODE){
        ret.emplace_back("---- Untested/Beta/WIP ----");
    }
    if (STATIC_GLOBALS.DEVELOPER_MODE){
        ret.emplace_back("---- Developer Tools ----");
        ret.emplace_back(make_SingleSwitchProgram<ShinyEncounterTester>());
        ret.emplace_back(make_SingleSwitchProgram<GenerateIVCheckerOCR>());
        ret.emplace_back(make_SingleSwitchProgram<GenerateNameOCRDataPokedex>());
        ret.emplace_back(make_SingleSwitchProgram<GeneratePokedexSprites>());
    }

    return ret;
}






}
}
}
