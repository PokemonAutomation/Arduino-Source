/*  Pokemon Sword/Shield Panels
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
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
#include "Programs/DateSpamFarmers/PokemonSwSh_DateSpam-LotoFarmer.h"
#include "Programs/DateSpamFarmers/PokemonSwSh_DateSpam-StowOnSideFarmer.h"
#include "Programs/DateSpamFarmers/PokemonSwSh_DateSpam-DailyHighlightFarmer.h"
#include "Programs/DateSpamFarmers/PokemonSwSh_DateSpam-PokeJobsFarmer.h"

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

#include "Programs/EggPrograms/PokemonSwSh_EggFetcher2.h"
#include "Programs/EggPrograms/PokemonSwSh_EggFetcherMultiple.h"
#include "Programs/EggPrograms/PokemonSwSh_EggHatcher.h"
#include "Programs/EggPrograms/PokemonSwSh_EggCombined2.h"
#include "Programs/EggPrograms/PokemonSwSh_EggSuperCombined2.h"
#include "Programs/EggPrograms/PokemonSwSh_GodEggDuplication.h"
#include "Programs/EggPrograms/PokemonSwSh_GodEggItemDupe.h"

#include "Programs/PokemonSwSh_SynchronizedSpinning.h"
#include "Programs/PokemonSwSh_RaidItemFarmerOKHO.h"

#include "MaxLair/PokemonSwSh_MaxLair_Standard.h"
#include "MaxLair/PokemonSwSh_MaxLair_StrongBoss.h"
#include "MaxLair/PokemonSwSh_MaxLair_BossFinder.h"

#include "Programs/TestPrograms/PokemonSwSh_ShinyEncounterTester.h"
#include "InferenceTraining/PokemonSwSh_GenerateIVCheckerOCR.h"
#include "InferenceTraining/PokemonSwSh_GenerateNameOCRPokedex.h"


namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


Panels::Panels(QTabWidget& parent, PanelHolder& holder)
    : PanelList(parent, "Sword/Shield", holder)
{
    add_divider("---- Settings ----");
    add_settings<GameSettings_Descriptor, GameSettingsPanel>();

    add_divider("---- QoL Macros ----");
    add_panel(make_single_switch_program<FastCodeEntry_Descriptor, FastCodeEntry>());
    add_panel(make_single_switch_program<FriendSearchDisconnect_Descriptor, FriendSearchDisconnect>());

    add_divider("---- General ----");
    add_panel(make_single_switch_program<MassRelease_Descriptor, MassRelease>());
    add_panel(make_single_switch_program<SurpriseTrade_Descriptor, SurpriseTrade>());
    add_panel(make_single_switch_program<TradeBot_Descriptor, TradeBot>());
    add_panel(make_single_switch_program<ClothingBuyer_Descriptor, ClothingBuyer>());
    add_panel(make_single_switch_program<BallThrower_Descriptor, BallThrower>());
    add_panel(make_single_switch_program<AutonomousBallThrower_Descriptor, AutonomousBallThrower>());
    add_panel(make_single_switch_program<DexRecFinder_Descriptor, DexRecFinder>());
    add_panel(make_single_switch_program<BoxReorderNationalDex_Descriptor, BoxReorderNationalDex>());

    add_divider("---- Date-Spam Farmers ----");
    add_panel(make_single_switch_program<WattFarmer_Descriptor, WattFarmer>());
    add_panel(make_single_switch_program<BerryFarmer_Descriptor, BerryFarmer>());
    add_panel(make_single_switch_program<LotoFarmer_Descriptor, LotoFarmer>());
    add_panel(make_single_switch_program<StowOnSideFarmer_Descriptor, StowOnSideFarmer>());
    add_panel(make_single_switch_program<DailyHighlightFarmer_Descriptor, DailyHighlightFarmer>());
    add_panel(make_single_switch_program<PokeJobsFarmer_Descriptor, PokeJobsFarmer>());

    add_divider("---- Den Hunting ----");
    add_panel(make_single_switch_program<BeamReset_Descriptor, BeamReset>());
    add_panel(make_single_switch_program<PurpleBeamFinder_Descriptor, PurpleBeamFinder>());
    add_panel(make_single_switch_program<EventBeamFinder_Descriptor, EventBeamFinder>());
    add_panel(make_single_switch_program<DaySkipperJPN_Descriptor, DaySkipperJPN>());
    add_panel(make_single_switch_program<DaySkipperEU_Descriptor, DaySkipperEU>());
    add_panel(make_single_switch_program<DaySkipperUS_Descriptor, DaySkipperUS>());
    add_panel(make_single_switch_program<DaySkipperJPN7p8k_Descriptor, DaySkipperJPN7p8k>());

    add_divider("---- Hosting ----");
    add_panel(make_single_switch_program<DenRoller_Descriptor, DenRoller>());
    add_panel(make_single_switch_program<AutoHostRolling_Descriptor, AutoHostRolling>());
    add_panel(make_single_switch_program<AutoHostMultiGame_Descriptor, AutoHostMultiGame>());

    add_divider("---- Non-Shiny Hunting ----");
    add_panel(make_single_switch_program<StatsReset_Descriptor, StatsReset>());
    add_panel(make_single_switch_program<StatsResetCalyrex_Descriptor, StatsResetCalyrex>());
    add_panel(make_single_switch_program<StatsResetMoltres_Descriptor, StatsResetMoltres>());
    add_panel(make_single_switch_program<StatsResetRegi_Descriptor, StatsResetRegi>());

    add_divider("---- Unattended Shiny Hunting ----");
    add_panel(make_single_switch_program<MultiGameFossil_Descriptor, MultiGameFossil>());
    add_panel(make_single_switch_program<ShinyHuntUnattendedRegi_Descriptor, ShinyHuntUnattendedRegi>());
    add_panel(make_single_switch_program<ShinyHuntUnattendedSwordsOfJustice_Descriptor, ShinyHuntUnattendedSwordsOfJustice>());
    add_panel(make_single_switch_program<ShinyHuntUnattendedStrongSpawn_Descriptor, ShinyHuntUnattendedStrongSpawn>());
    add_panel(make_single_switch_program<ShinyHuntUnattendedRegigigas2_Descriptor, ShinyHuntUnattendedRegigigas2>());
    add_panel(make_single_switch_program<ShinyHuntUnattendedIoATrade_Descriptor, ShinyHuntUnattendedIoATrade>());
    add_panel(make_single_switch_program<CurryHunter_Descriptor, CurryHunter>());

    add_divider("---- Autonomous Shiny Hunting ----");
    add_panel(make_single_switch_program<ShinyHuntAutonomousRegi_Descriptor, ShinyHuntAutonomousRegi>());
    add_panel(make_single_switch_program<ShinyHuntAutonomousSwordsOfJustice_Descriptor, ShinyHuntAutonomousSwordsOfJustice>());
    add_panel(make_single_switch_program<ShinyHuntAutonomousStrongSpawn_Descriptor, ShinyHuntAutonomousStrongSpawn>());
    add_panel(make_single_switch_program<ShinyHuntAutonomousRegigigas2_Descriptor, ShinyHuntAutonomousRegigigas2>());
    add_panel(make_single_switch_program<ShinyHuntAutonomousIoATrade_Descriptor, ShinyHuntAutonomousIoATrade>());
    add_panel(make_single_switch_program<ShinyHuntAutonomousBerryTree_Descriptor, ShinyHuntAutonomousBerryTree>());
    add_panel(make_single_switch_program<ShinyHuntAutonomousWhistling_Descriptor, ShinyHuntAutonomousWhistling>());
    add_panel(make_single_switch_program<ShinyHuntAutonomousFishing_Descriptor, ShinyHuntAutonomousFishing>());
    add_panel(make_single_switch_program<ShinyHuntAutonomousOverworld_Descriptor, ShinyHuntAutonomousOverworld>());

    add_divider("---- Eggs ----");
    add_panel(make_single_switch_program<EggFetcher2_Descriptor, EggFetcher2>());
    add_panel(make_single_switch_program<EggFetcherMultiple_Descriptor, EggFetcherMultiple>());
    add_panel(make_single_switch_program<EggHatcher_Descriptor, EggHatcher>());
    add_panel(make_single_switch_program<EggCombined2_Descriptor, EggCombined2>());
    add_panel(make_single_switch_program<EggSuperCombined2_Descriptor, EggSuperCombined2>());
    add_panel(make_single_switch_program<GodEggItemDupe_Descriptor, GodEggItemDupe>());
    if (PreloadSettings::instance().NAUGHTY_MODE){
        add_panel(make_single_switch_program<GodEggDuplication_Descriptor, GodEggDuplication>());
    }

    add_divider("---- Multi-Switch Programs ----");
    add_panel(make_multi_switch_program<SynchronizedSpinning_Descriptor, SynchronizedSpinning>());
    add_program<RaidItemFarmerOHKO_Descriptor, RaidItemFarmerOHKO>();

    add_divider("---- Auto Max Lair 2.0 ----");
    add_panel(make_multi_switch_program<MaxLairStandard_Descriptor, MaxLairStandard>());
    add_panel(make_multi_switch_program<MaxLairStrongBoss_Descriptor, MaxLairStrongBoss>());
    add_panel(make_multi_switch_program<MaxLairBossFinder_Descriptor, MaxLairBossFinder>());

    if (PreloadSettings::instance().DEVELOPER_MODE){
        add_divider("---- Developer Tools ----");
        add_panel(make_single_switch_program<ShinyEncounterTester_Descriptor, ShinyEncounterTester>());
        add_panel(make_single_switch_program<GenerateIVCheckerOCR_Descriptor, GenerateIVCheckerOCR>());
        add_panel(make_single_switch_program<GenerateNameOCRDataPokedex_Descriptor, GenerateNameOCRDataPokedex>());
    }


    finish_panel_setup();
}




}
}
}
