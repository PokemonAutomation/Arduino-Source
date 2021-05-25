/*  List of all Panels
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <iostream>
#include "Common/Qt/StringException.h"
#include "CommonFramework/PersistentSettings.h"
#include "PanelList.h"

#include "NintendoSwitch/FrameworkSettingsPanel.h"
#include "PokemonSwSh/PokemonSwSh_SettingsPanel.h"

#include "NintendoSwitch/Programs/VirtualConsole.h"
#include "NintendoSwitch/Programs/SwitchViewer.h"

#include "PokemonSwSh/Programs/TestProgram.h"

#include "NintendoSwitch/Programs/PreventSleep.h"
#include "NintendoSwitch/Programs/FriendCodeAdder.h"
#include "NintendoSwitch/Programs/FriendDelete.h"
#include "NintendoSwitch/Programs/PokemonHome_PageSwap.h"

#include "PokemonSwSh/Programs/QoLMacros/PokemonSwSh_FastCodeEntry.h"
#include "PokemonSwSh/Programs/QoLMacros/PokemonSwSh_FriendSearchDisconnect.h"

#include "PokemonSwSh/Programs/BasicPrograms/PokemonSwSh_TurboA.h"
#include "PokemonSwSh/Programs/BasicPrograms/PokemonSwSh_MassRelease.h"
#include "PokemonSwSh/Programs/BasicPrograms/PokemonSwSh_SurpriseTrade.h"
#include "PokemonSwSh/Programs/BasicPrograms/PokemonSwSh_TradeBot.h"
#include "PokemonSwSh/Programs/BasicPrograms/PokemonSwSh_ClothingBuyer.h"
#include "PokemonSwSh/Programs/BasicPrograms/PokemonSwSh_BallThrower.h"
#include "PokemonSwSh/Programs/BasicPrograms/PokemonSwSh_DexRecFinder.h"

#include "PokemonSwSh/Programs/DateSpamFarmers/PokemonSwSh_DateSpam-WattFarmer.h"
#include "PokemonSwSh/Programs/DateSpamFarmers/PokemonSwSh_DateSpam-BerryFarmer.h"
#include "PokemonSwSh/Programs/DateSpamFarmers/PokemonSwSh_DateSpam-LotoFarmer.h"
#include "PokemonSwSh/Programs/DateSpamFarmers/PokemonSwSh_DateSpam-StowOnSideFarmer.h"
#include "PokemonSwSh/Programs/DateSpamFarmers/PokemonSwSh_DateSpam-DailyHighlightFarmer.h"

#include "PokemonSwSh/Programs/DenHunting/PokemonSwSh_BeamReset.h"
#include "PokemonSwSh/Programs/DenHunting/PokemonSwSh_PurpleBeamFinder.h"
#include "PokemonSwSh/Programs/DenHunting/PokemonSwSh_EventBeamFinder.h"
#include "PokemonSwSh/Programs/DenHunting/PokemonSwSh_DaySkipperJPN.h"
#include "PokemonSwSh/Programs/DenHunting/PokemonSwSh_DaySkipperEU.h"
#include "PokemonSwSh/Programs/DenHunting/PokemonSwSh_DaySkipperUS.h"
#include "PokemonSwSh/Programs/DenHunting/PokemonSwSh_DaySkipperJPN-7.8k.h"

#include "PokemonSwSh/Programs/Hosting/PokemonSwSh_DenRoller.h"
#include "PokemonSwSh/Programs/Hosting/PokemonSwSh_AutoHost-Rolling.h"
#include "PokemonSwSh/Programs/Hosting/PokemonSwSh_AutoHost-MultiGame.h"

#include "PokemonSwSh/Programs/ShinyHunting/PokemonSwSh_MultiGameFossil.h"
#include "PokemonSwSh/Programs/ShinyHunting/PokemonSwSh_ShinyHuntUnattended-Regi.h"
#include "PokemonSwSh/Programs/ShinyHunting/PokemonSwSh_ShinyHuntUnattended-SwordsOfJustice.h"
#include "PokemonSwSh/Programs/ShinyHunting/PokemonSwSh_ShinyHuntUnattended-StrongSpawn.h"
#include "PokemonSwSh/Programs/ShinyHunting/PokemonSwSh_ShinyHuntUnattended-Regigigas2.h"
#include "PokemonSwSh/Programs/ShinyHunting/PokemonSwSh_ShinyHuntUnattended-IoATrade.h"

#include "PokemonSwSh/Programs/ShinyHunting/PokemonSwSh_ShinyHuntAutonomous-Regi.h"
#include "PokemonSwSh/Programs/ShinyHunting/PokemonSwSh_ShinyHuntAutonomous-SwordsOfJustice.h"
#include "PokemonSwSh/Programs/ShinyHunting/PokemonSwSh_ShinyHuntAutonomous-StrongSpawn.h"
#include "PokemonSwSh/Programs/ShinyHunting/PokemonSwSh_ShinyHuntAutonomous-Regigigas2.h"
#include "PokemonSwSh/Programs/ShinyHunting/PokemonSwSh_ShinyHuntAutonomous-IoATrade.h"
#include "PokemonSwSh/Programs/ShinyHunting/PokemonSwSh_ShinyHuntAutonomous-BerryTree.h"
#include "PokemonSwSh/Programs/ShinyHunting/PokemonSwSh_ShinyHuntAutonomous-Whistling.h"
#include "PokemonSwSh/Programs/ShinyHunting/PokemonSwSh_ShinyHuntAutonomous-Fishing.h"
#include "PokemonSwSh/Programs/ShinyHunting/PokemonSwSh_ShinyHuntAutonomous-Overworld.h"

#include "PokemonSwSh/Programs/EggPrograms/PokemonSwSh_EggFetcher2.h"
#include "PokemonSwSh/Programs/EggPrograms/PokemonSwSh_EggHatcher.h"
#include "PokemonSwSh/Programs/EggPrograms/PokemonSwSh_EggCombined2.h"
#include "PokemonSwSh/Programs/EggPrograms/PokemonSwSh_EggSuperCombined2.h"
#include "PokemonSwSh/Programs/EggPrograms/PokemonSwSh_GodEggDuplication.h"
#include "PokemonSwSh/Programs/EggPrograms/PokemonSwSh_GodEggItemDupe.h"

#include "PokemonSwSh/Programs/PokemonSwSh_SynchronizedSpinning.h"
#include "PokemonSwSh/Programs/PokemonSwSh_RaidItemFarmerOKHO.h"

namespace PokemonAutomation{

using namespace NintendoSwitch;
using namespace PokemonHome;
using namespace PokemonSwSh;

using std::cout;
using std::endl;

const std::vector<std::unique_ptr<RightPanel>>& SETTINGS_LIST(){
    static std::vector<std::unique_ptr<RightPanel>> list;
    if (!list.empty()){
        return list;
    }

    const QJsonObject& json = PERSISTENT_SETTINGS().settings;


    list.emplace_back(new FrameworkSettings(json));
    list.emplace_back(new PokemonSettings(json));

    return list;
}
const std::map<QString, RightPanel*>& SETTINGS_MAP(){
    static std::map<QString, RightPanel*> map;
    if (!map.empty()){
        return map;
    }
    for (const auto& setting : SETTINGS_LIST()){
        auto ret = map.emplace(setting->name(), setting.get());
        if (!ret.second){
            cout << ("Duplicate setting name: " + setting->name()).toUtf8().data() << endl;
            throw StringException("Duplicate setting name: " + setting->name());
        }
    }
    return map;
}

const std::vector<std::unique_ptr<RightPanel>>& PROGRAM_LIST(){
    static std::vector<std::unique_ptr<RightPanel>> list;
    if (!list.empty()){
        return list;
    }

    PersistentSettings& settings = PERSISTENT_SETTINGS();
    const QJsonObject& json = settings.programs;

    list.emplace_back(new VirtualConsole(json));
    list.emplace_back(new SwitchViewer(json));

    if (settings.developer_mode){
        list.emplace_back(new SingleSwitchProgramWrapper<TestProgram>(json));
    }

    list.emplace_back(new SingleSwitchProgramWrapper<PreventSleep>(json));
    list.emplace_back(new SingleSwitchProgramWrapper<FriendCodeAdder>(json));
    list.emplace_back(new SingleSwitchProgramWrapper<FriendDelete>(json));
    list.emplace_back(new SingleSwitchProgramWrapper<PageSwap>(json));

    list.emplace_back(new SingleSwitchProgramWrapper<FastCodeEntry>(json));
    list.emplace_back(new SingleSwitchProgramWrapper<FriendSearchDisconnect>(json));

    list.emplace_back(new SingleSwitchProgramWrapper<TurboA>(json));
    list.emplace_back(new SingleSwitchProgramWrapper<MassRelease>(json));
    list.emplace_back(new SingleSwitchProgramWrapper<SurpriseTrade>(json));
    list.emplace_back(new SingleSwitchProgramWrapper<TradeBot>(json));
    list.emplace_back(new SingleSwitchProgramWrapper<ClothingBuyer>(json));
    list.emplace_back(new SingleSwitchProgramWrapper<BallThrower>(json));
    list.emplace_back(new SingleSwitchProgramWrapper<DexRecFinder>(json));

    list.emplace_back(new SingleSwitchProgramWrapper<WattFarmer>(json));
    list.emplace_back(new SingleSwitchProgramWrapper<BerryFarmer>(json));
    list.emplace_back(new SingleSwitchProgramWrapper<LotoFarmer>(json));
    list.emplace_back(new SingleSwitchProgramWrapper<StowOnSideFarmer>(json));
    list.emplace_back(new SingleSwitchProgramWrapper<DailyHighlightFarmer>(json));

    list.emplace_back(new SingleSwitchProgramWrapper<BeamReset>(json));
    list.emplace_back(new SingleSwitchProgramWrapper<PurpleBeamFinder>(json));
    list.emplace_back(new SingleSwitchProgramWrapper<EventBeamFinder>(json));
    list.emplace_back(new SingleSwitchProgramWrapper<DaySkipperJPN>(json));
    list.emplace_back(new SingleSwitchProgramWrapper<DaySkipperEU>(json));
    list.emplace_back(new SingleSwitchProgramWrapper<DaySkipperUS>(json));
    list.emplace_back(new SingleSwitchProgramWrapper<DaySkipperJPN7p8k>(json));

    list.emplace_back(new SingleSwitchProgramWrapper<DenRoller>(json));
    list.emplace_back(new SingleSwitchProgramWrapper<AutoHostRolling>(json));
    list.emplace_back(new SingleSwitchProgramWrapper<AutoHostMultiGame>(json));

    list.emplace_back(new SingleSwitchProgramWrapper<MultiGameFossil>(json));
    list.emplace_back(new SingleSwitchProgramWrapper<ShinyHuntUnattendedRegi>(json));
    list.emplace_back(new SingleSwitchProgramWrapper<ShinyHuntUnattendedSwordsOfJustice>(json));
    list.emplace_back(new SingleSwitchProgramWrapper<ShinyHuntUnattendedStrongSpawn>(json));
    list.emplace_back(new SingleSwitchProgramWrapper<ShinyHuntUnattendedRegigigas2>(json));
    list.emplace_back(new SingleSwitchProgramWrapper<ShinyHuntUnattendedIoATrade>(json));
    
    list.emplace_back(new SingleSwitchProgramWrapper<ShinyHuntAutonomousRegi>(json));
    list.emplace_back(new SingleSwitchProgramWrapper<ShinyHuntAutonomousSwordsOfJustice>(json));
    list.emplace_back(new SingleSwitchProgramWrapper<ShinyHuntAutonomousStrongSpawn>(json));
    list.emplace_back(new SingleSwitchProgramWrapper<ShinyHuntAutonomousRegigigas2>(json));
    list.emplace_back(new SingleSwitchProgramWrapper<ShinyHuntAutonomousIoATrade>(json));
    list.emplace_back(new SingleSwitchProgramWrapper<ShinyHuntAutonomousBerryTree>(json));
    list.emplace_back(new SingleSwitchProgramWrapper<ShinyHuntAutonomousWhistling>(json));
    list.emplace_back(new SingleSwitchProgramWrapper<ShinyHuntAutonomousFishing>(json));
//    if (settings.developer_mode){
        list.emplace_back(new SingleSwitchProgramWrapper<ShinyHuntAutonomousOverworld>(json));
//    }

    list.emplace_back(new SingleSwitchProgramWrapper<EggFetcher2>(json));
    list.emplace_back(new SingleSwitchProgramWrapper<EggHatcher>(json));
    list.emplace_back(new SingleSwitchProgramWrapper<EggCombined2>(json));
    list.emplace_back(new SingleSwitchProgramWrapper<EggSuperCombined2>(json));
    list.emplace_back(new SingleSwitchProgramWrapper<GodEggItemDupe>(json));
    if (settings.naughty_mode){
        list.emplace_back(new SingleSwitchProgramWrapper<GodEggDuplication>(json));
    }

    list.emplace_back(new MultiSwitchProgramWrapper<SynchronizedSpinning>(json));
    list.emplace_back(new MultiSwitchProgramWrapper<RaidItemFarmerOHKO>(json));

    return list;
}
const std::map<QString, RightPanel*>& PROGRAM_MAP(){
    static std::map<QString, RightPanel*> map;
    if (!map.empty()){
        return map;
    }
    for (const auto& program : PROGRAM_LIST()){
        auto ret = map.emplace(program->name(), program.get());
        if (!ret.second){
            cout << ("Duplicate program name: " + program->name()).toUtf8().data() << endl;
            throw StringException("Duplicate program name: " + program->name());
        }
    }
    return map;
}



}
