/*  Pokemon Sword/Shield Panels
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Qt/QtJsonTools.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/Windows/MainWindow.h"
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

#include "Programs/DateSpamFarmers/PokemonSwSh_DateSpam-WattFarmer.h"
#include "Programs/DateSpamFarmers/PokemonSwSh_DateSpam-BerryFarmer.h"
#include "Programs/DateSpamFarmers/PokemonSwSh_DateSpam-LotoFarmer.h"
#include "Programs/DateSpamFarmers/PokemonSwSh_DateSpam-StowOnSideFarmer.h"
#include "Programs/DateSpamFarmers/PokemonSwSh_DateSpam-DailyHighlightFarmer.h"

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
#include "Programs/EggPrograms/PokemonSwSh_EggHatcher.h"
#include "Programs/EggPrograms/PokemonSwSh_EggCombined2.h"
#include "Programs/EggPrograms/PokemonSwSh_EggSuperCombined2.h"
#include "Programs/EggPrograms/PokemonSwSh_GodEggDuplication.h"
#include "Programs/EggPrograms/PokemonSwSh_GodEggItemDupe.h"

#include "Programs/PokemonSwSh_SynchronizedSpinning.h"
#include "Programs/PokemonSwSh_RaidItemFarmerOKHO.h"

#include "MaxLair/PokemonSwSh_MaxLair_SingleRun.h"
#include "MaxLair/PokemonSwSh_MaxLair_Standard.h"
#include "MaxLair/PokemonSwSh_MaxLair_BossFinder.h"

#include "InferenceTraining/PokemonSwSh_GenerateIVCheckerOCR.h"
#include "InferenceTraining/PokemonSwSh_GenerateNameOCRPokedex.h"


namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


Panels::Panels(QTabWidget& parent, PanelListener& listener)
    : PanelList(parent, "Sword/Shield", listener)
{
    add_divider("---- Settings ----");
    add_settings<GameSettings_Descriptor, GameSettingsPanel>();

    add_divider("---- QoL Macros ----");
    add_program<FastCodeEntry_Descriptor, FastCodeEntry>();
    add_program<FriendSearchDisconnect_Descriptor, FriendSearchDisconnect>();

    add_divider("---- General ----");
    add_program<MassRelease_Descriptor, MassRelease>();
    add_program<SurpriseTrade_Descriptor, SurpriseTrade>();
    add_program<TradeBot_Descriptor, TradeBot>();
    add_program<ClothingBuyer_Descriptor, ClothingBuyer>();
    add_program<BallThrower_Descriptor, BallThrower>();
    add_program<AutonomousBallThrower_Descriptor, AutonomousBallThrower>();
    add_program<DexRecFinder_Descriptor, DexRecFinder>();

    add_divider("---- Date-Spam Farmers ----");
    add_program<WattFarmer_Descriptor, WattFarmer>();
    add_program<BerryFarmer_Descriptor, BerryFarmer>();
    add_program<LotoFarmer_Descriptor, LotoFarmer>();
    add_program<StowOnSideFarmer_Descriptor, StowOnSideFarmer>();
    add_program<DailyHighlightFarmer_Descriptor, DailyHighlightFarmer>();

    add_divider("---- Den Hunting ----");
    add_program<BeamReset_Descriptor, BeamReset>();
    add_program<PurpleBeamFinder_Descriptor, PurpleBeamFinder>();
    add_program<EventBeamFinder_Descriptor, EventBeamFinder>();
    add_program<DaySkipperJPN_Descriptor, DaySkipperJPN>();
    add_program<DaySkipperEU_Descriptor, DaySkipperEU>();
    add_program<DaySkipperUS_Descriptor, DaySkipperUS>();
    add_program<DaySkipperJPN7p8k_Descriptor, DaySkipperJPN7p8k>();

    add_divider("---- Hosting ----");
    add_program<DenRoller_Descriptor, DenRoller>();
    add_program<AutoHostRolling_Descriptor, AutoHostRolling>();
    add_program<AutoHostMultiGame_Descriptor, AutoHostMultiGame>();

    add_divider("---- Non-Shiny Hunting ----");
    add_program<StatsReset_Descriptor, StatsReset>();
    add_program<StatsResetCalyrex_Descriptor, StatsResetCalyrex>();
    add_program<StatsResetMoltres_Descriptor, StatsResetMoltres>();
    add_program<StatsResetRegi_Descriptor, StatsResetRegi>();

    add_divider("---- Unattended Shiny Hunting ----");
    add_program<MultiGameFossil_Descriptor, MultiGameFossil>();
    add_program<ShinyHuntUnattendedRegi_Descriptor, ShinyHuntUnattendedRegi>();
    add_program<ShinyHuntUnattendedSwordsOfJustice_Descriptor, ShinyHuntUnattendedSwordsOfJustice>();
    add_program<ShinyHuntUnattendedStrongSpawn_Descriptor, ShinyHuntUnattendedStrongSpawn>();
    add_program<ShinyHuntUnattendedRegigigas2_Descriptor, ShinyHuntUnattendedRegigigas2>();
    add_program<ShinyHuntUnattendedIoATrade_Descriptor, ShinyHuntUnattendedIoATrade>();
    add_program<CurryHunter_Descriptor, CurryHunter>();

    add_divider("---- Autonomous Shiny Hunting ----");
    add_program<ShinyHuntAutonomousRegi_Descriptor, ShinyHuntAutonomousRegi>();
    add_program<ShinyHuntAutonomousSwordsOfJustice_Descriptor, ShinyHuntAutonomousSwordsOfJustice>();
    add_program<ShinyHuntAutonomousStrongSpawn_Descriptor, ShinyHuntAutonomousStrongSpawn>();
    add_program<ShinyHuntAutonomousRegigigas2_Descriptor, ShinyHuntAutonomousRegigigas2>();
    add_program<ShinyHuntAutonomousIoATrade_Descriptor, ShinyHuntAutonomousIoATrade>();
    add_program<ShinyHuntAutonomousBerryTree_Descriptor, ShinyHuntAutonomousBerryTree>();
    add_program<ShinyHuntAutonomousWhistling_Descriptor, ShinyHuntAutonomousWhistling>();
    add_program<ShinyHuntAutonomousFishing_Descriptor, ShinyHuntAutonomousFishing>();
    add_program<ShinyHuntAutonomousOverworld_Descriptor, ShinyHuntAutonomousOverworld>();

    add_divider("---- Eggs ----");
    add_program<EggFetcher2_Descriptor, EggFetcher2>();
    add_program<EggHatcher_Descriptor, EggHatcher>();
    add_program<EggCombined2_Descriptor, EggCombined2>();
    add_program<EggSuperCombined2_Descriptor, EggSuperCombined2>();
    add_program<GodEggItemDupe_Descriptor, GodEggItemDupe>();
    if (GlobalSettings::instance().NAUGHTY_MODE){
        add_program<GodEggDuplication_Descriptor, GodEggDuplication>();
    }

    add_divider("---- Multi-Switch Programs ----");
    add_program<SynchronizedSpinning_Descriptor, SynchronizedSpinning>();
    add_program<RaidItemFarmerOHKO_Descriptor, RaidItemFarmerOHKO>();

    if (GlobalSettings::instance().DEVELOPER_MODE){
        add_divider("---- Auto Max Lair 2.0 ----");
        add_program<MaxLairSingleRun_Descriptor, MaxLairSingleRun>();
        add_program<MaxLairStandard_Descriptor, MaxLairStandard>();
        add_program<MaxLairBossFinder_Descriptor, MaxLairBossFinder>();
    }

    if (GlobalSettings::instance().DEVELOPER_MODE){
        add_divider("---- Developer Tools ----");
        add_program<GenerateIVCheckerOCR_Descriptor, GenerateIVCheckerOCR>();
        add_program<GenerateNameOCRDataPokedex_Descriptor, GenerateNameOCRDataPokedex>();
    }


    finish_panel_setup();
}




}
}
}
