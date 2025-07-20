/*  Pokemon Sword/Shield Panels
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/GlobalSettingsPanel.h"
#include "PokemonSwSh_Panels.h"

#include "PokemonSwSh_Settings.h"

#include "Programs/QoLMacros/PokemonSwSh_FastCodeEntry.h"
#include "Programs/QoLMacros/PokemonSwSh_FriendSearchDisconnect.h"

#include "Programs/General/PokemonSwSh_MassRelease.h"
#include "Programs/General/PokemonSwSh_SurpriseTrade.h"
#include "Programs/General/PokemonSwSh_TradeBot.h"
#include "Programs/General/PokemonSwSh_ClothingBuyer.h"
#include "Programs/General/PokemonSwSh_BallThrower.h"
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

#include "Programs/DenHunting/PokemonSwSh_BeamReset.h"
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
#include "Programs/EggPrograms/PokemonSwSh_EggCombined2.h"
#include "Programs/EggPrograms/PokemonSwSh_EggSuperCombined2.h"
#include "Programs/EggPrograms/PokemonSwSh_GodEggDuplication.h"
#include "Programs/EggPrograms/PokemonSwSh_GodEggItemDupe.h"

#include "Programs/ShinyHuntUnattended/PokemonSwSh_MultiGameFossil.h"
#include "Programs/ShinyHuntUnattended/PokemonSwSh_ShinyHuntUnattended-Regi.h"
#include "Programs/ShinyHuntUnattended/PokemonSwSh_ShinyHuntUnattended-SwordsOfJustice.h"
#include "Programs/ShinyHuntUnattended/PokemonSwSh_ShinyHuntUnattended-StrongSpawn.h"
#include "Programs/ShinyHuntUnattended/PokemonSwSh_ShinyHuntUnattended-Regigigas2.h"
#include "Programs/ShinyHuntUnattended/PokemonSwSh_ShinyHuntUnattended-IoATrade.h"
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
    : PanelListDescriptor(Pokemon::STRING_POKEMON + " Sword and Shield")
{}

std::vector<PanelEntry> PanelListFactory::make_panels() const{
    std::vector<PanelEntry> ret;

    ret.emplace_back("---- Settings ----");
    ret.emplace_back(make_settings<GameSettings_Descriptor, GameSettingsPanel>());

    ret.emplace_back("---- QoL Macros ----");
    ret.emplace_back(make_single_switch_program<FastCodeEntry_Descriptor, FastCodeEntry>());
    ret.emplace_back(make_single_switch_program<FriendSearchDisconnect_Descriptor, FriendSearchDisconnect>());

    ret.emplace_back("---- General ----");
    ret.emplace_back(make_single_switch_program<MassRelease_Descriptor, MassRelease>());
    ret.emplace_back(make_single_switch_program<SurpriseTrade_Descriptor, SurpriseTrade>());
    ret.emplace_back(make_single_switch_program<TradeBot_Descriptor, TradeBot>());
    ret.emplace_back(make_single_switch_program<ClothingBuyer_Descriptor, ClothingBuyer>());
    ret.emplace_back(make_single_switch_program<AutonomousBallThrower_Descriptor, AutonomousBallThrower>());
    ret.emplace_back(make_single_switch_program<DexRecFinder_Descriptor, DexRecFinder>());
    ret.emplace_back(make_single_switch_program<BoxReorderNationalDex_Descriptor, BoxReorderNationalDex>());

    ret.emplace_back("---- Date-Spam Farmers ----");
    ret.emplace_back(make_single_switch_program<WattFarmer_Descriptor, WattFarmer>());
    ret.emplace_back(make_single_switch_program<BerryFarmer_Descriptor, BerryFarmer>());
    ret.emplace_back(make_single_switch_program<BerryFarmer2_Descriptor, BerryFarmer2>());
    ret.emplace_back(make_single_switch_program<LotoFarmer_Descriptor, LotoFarmer>());
    ret.emplace_back(make_single_switch_program<StowOnSideFarmer_Descriptor, StowOnSideFarmer>());
    ret.emplace_back(make_single_switch_program<DailyHighlightFarmer_Descriptor, DailyHighlightFarmer>());
    ret.emplace_back(make_single_switch_program<PokeJobsFarmer_Descriptor, PokeJobsFarmer>());
    if (PreloadSettings::instance().DEVELOPER_MODE){
        ret.emplace_back(make_single_switch_program<WattTraderFarmer_Descriptor, WattTraderFarmer>());
    }

    ret.emplace_back("---- Den Hunting ----");
    ret.emplace_back(make_single_switch_program<PurpleBeamFinder_Descriptor, PurpleBeamFinder>());
    ret.emplace_back(make_single_switch_program<EventBeamFinder_Descriptor, EventBeamFinder>());
#ifdef PA_OFFICIAL
    ret.emplace_back(make_single_switch_program<DaySkipperJPN_Descriptor, DaySkipperJPN>());
    ret.emplace_back(make_single_switch_program<DaySkipperEU_Descriptor, DaySkipperEU>());
    ret.emplace_back(make_single_switch_program<DaySkipperUS_Descriptor, DaySkipperUS>());
    ret.emplace_back(make_single_switch_program<DaySkipperJPN7p8k_Descriptor, DaySkipperJPN7p8k>());
#endif

    ret.emplace_back("---- Hosting ----");
    ret.emplace_back(make_single_switch_program<DenRoller_Descriptor, DenRoller>());
    ret.emplace_back(make_single_switch_program<AutoHostRolling_Descriptor, AutoHostRolling>());
    ret.emplace_back(make_single_switch_program<AutoHostMultiGame_Descriptor, AutoHostMultiGame>());

    ret.emplace_back("---- Eggs ----");
    ret.emplace_back(make_single_switch_program<EggFetcher2_Descriptor, EggFetcher2>());
    ret.emplace_back(make_single_switch_program<EggFetcherMultiple_Descriptor, EggFetcherMultiple>());
    ret.emplace_back(make_single_switch_program<EggHatcher_Descriptor, EggHatcher>());
    ret.emplace_back(make_single_switch_program<EggAutonomous_Descriptor, EggAutonomous>());
    ret.emplace_back(make_single_switch_program<GodEggItemDupe_Descriptor, GodEggItemDupe>());
    if (PreloadSettings::instance().NAUGHTY_MODE || PreloadSettings::instance().DEVELOPER_MODE){
        ret.emplace_back(make_single_switch_program<GodEggDuplication_Descriptor, GodEggDuplication>());
    }

    ret.emplace_back("---- Non-Shiny Hunting ----");
    ret.emplace_back(make_single_switch_program<StatsReset_Descriptor, StatsReset>());
    ret.emplace_back(make_single_switch_program<StatsResetCalyrex_Descriptor, StatsResetCalyrex>());
    ret.emplace_back(make_single_switch_program<StatsResetMoltres_Descriptor, StatsResetMoltres>());
    ret.emplace_back(make_single_switch_program<StatsResetRegi_Descriptor, StatsResetRegi>());

    ret.emplace_back("---- Shiny Hunting ----");
    ret.emplace_back(make_single_switch_program<MultiGameFossil_Descriptor, MultiGameFossil>());
    ret.emplace_back(make_single_switch_program<CurryHunter_Descriptor, CurryHunter>());
    ret.emplace_back(make_single_switch_program<ShinyHuntAutonomousRegi_Descriptor, ShinyHuntAutonomousRegi>());
    ret.emplace_back(make_single_switch_program<ShinyHuntAutonomousSwordsOfJustice_Descriptor, ShinyHuntAutonomousSwordsOfJustice>());
    ret.emplace_back(make_single_switch_program<ShinyHuntAutonomousStrongSpawn_Descriptor, ShinyHuntAutonomousStrongSpawn>());
    ret.emplace_back(make_single_switch_program<ShinyHuntAutonomousRegigigas2_Descriptor, ShinyHuntAutonomousRegigigas2>());
    ret.emplace_back(make_single_switch_program<ShinyHuntAutonomousIoATrade_Descriptor, ShinyHuntAutonomousIoATrade>());
    ret.emplace_back(make_single_switch_program<ShinyHuntAutonomousBerryTree_Descriptor, ShinyHuntAutonomousBerryTree>());
    ret.emplace_back(make_single_switch_program<ShinyHuntAutonomousWhistling_Descriptor, ShinyHuntAutonomousWhistling>());
    ret.emplace_back(make_single_switch_program<ShinyHuntAutonomousFishing_Descriptor, ShinyHuntAutonomousFishing>());
    ret.emplace_back(make_single_switch_program<ShinyHuntAutonomousOverworld_Descriptor, ShinyHuntAutonomousOverworld>());

    ret.emplace_back("---- RNG ----");
    ret.emplace_back(make_single_switch_program<SeedFinder_Descriptor, SeedFinder>());
    ret.emplace_back(make_single_switch_program<CramomaticRNG_Descriptor, CramomaticRNG>());

    ret.emplace_back("---- Multi-Switch Programs ----");
    ret.emplace_back(make_multi_switch_program<SynchronizedSpinning_Descriptor, SynchronizedSpinning>());
    ret.emplace_back(make_multi_switch_program<RaidItemFarmerOHKO_Descriptor, RaidItemFarmerOHKO>());

    ret.emplace_back("---- Auto Max Lair 2.0 ----");
    ret.emplace_back(make_multi_switch_program<MaxLairStandard_Descriptor, MaxLairStandard>());
    ret.emplace_back(make_multi_switch_program<MaxLairStrongBoss_Descriptor, MaxLairStrongBoss>());
    ret.emplace_back(make_multi_switch_program<MaxLairBossFinder_Descriptor, MaxLairBossFinder>());

    ret.emplace_back("---- Deprecated Programs ----");
    ret.emplace_back(make_single_switch_program<BallThrower_Descriptor, BallThrower>());
    ret.emplace_back(make_single_switch_program<BeamReset_Descriptor, BeamReset>());
    ret.emplace_back(make_single_switch_program<EggCombined2_Descriptor, EggCombined2>());
    ret.emplace_back(make_single_switch_program<EggSuperCombined2_Descriptor, EggSuperCombined2>());
    ret.emplace_back(make_single_switch_program<ShinyHuntUnattendedRegi_Descriptor, ShinyHuntUnattendedRegi>());
    ret.emplace_back(make_single_switch_program<ShinyHuntUnattendedSwordsOfJustice_Descriptor, ShinyHuntUnattendedSwordsOfJustice>());
    ret.emplace_back(make_single_switch_program<ShinyHuntUnattendedStrongSpawn_Descriptor, ShinyHuntUnattendedStrongSpawn>());
    ret.emplace_back(make_single_switch_program<ShinyHuntUnattendedRegigigas2_Descriptor, ShinyHuntUnattendedRegigigas2>());
    ret.emplace_back(make_single_switch_program<ShinyHuntUnattendedIoATrade_Descriptor, ShinyHuntUnattendedIoATrade>());

    if (PreloadSettings::instance().DEVELOPER_MODE){
        ret.emplace_back("---- Untested/Beta/WIP ----");
        ret.emplace_back(make_single_switch_program<DailyHighlightRNG_Descriptor, DailyHighlightRNG>());
    }
    if (PreloadSettings::instance().DEVELOPER_MODE){
        ret.emplace_back("---- Developer Tools ----");
        ret.emplace_back(make_single_switch_program<ShinyEncounterTester_Descriptor, ShinyEncounterTester>());
        ret.emplace_back(make_single_switch_program<GenerateIVCheckerOCR_Descriptor, GenerateIVCheckerOCR>());
        ret.emplace_back(make_single_switch_program<GenerateNameOCRDataPokedex_Descriptor, GenerateNameOCRDataPokedex>());
        ret.emplace_back(make_single_switch_program<GeneratePokedexSprites_Descriptor, GeneratePokedexSprites>());
    }

    return ret;
}






}
}
}
