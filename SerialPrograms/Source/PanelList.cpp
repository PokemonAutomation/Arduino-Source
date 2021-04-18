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
#include "PokemonSwSh/Programs/QoLMacros/PokemonSwSh_FastCodeEntry.h"
#include "PokemonSwSh/Programs/QoLMacros/PokemonSwSh_FriendSearchDisconnect.h"

#include "PokemonSwSh/Programs/BasicPrograms/PokemonSwSh_TurboA.h"
#include "PokemonSwSh/Programs/BasicPrograms/PokemonSwSh_MassRelease.h"
#include "PokemonSwSh/Programs/BasicPrograms/PokemonSwSh_SurpriseTrade.h"
#include "PokemonSwSh/Programs/BasicPrograms/PokemonSwSh_TradeBot.h"
#include "PokemonSwSh/Programs/BasicPrograms/PokemonSwSh_ClothingBuyer.h"
#include "PokemonSwSh/Programs/BasicPrograms/PokemonSwSh_BallThrower.h"

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
using namespace PokemonSwSh;

using std::cout;
using std::endl;

const std::vector<std::unique_ptr<RightPanel>>& SETTINGS_LIST(){
    static std::vector<std::unique_ptr<RightPanel>> list;
    if (!list.empty()){
        return list;
    }

    list.emplace_back(new FrameworkSettings(settings.settings));
    list.emplace_back(new PokemonSettings(settings.settings));

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

    list.emplace_back(new VirtualConsole(settings.programs));
    list.emplace_back(new SwitchViewer(settings.programs));

    if (settings.developer_mode){
        list.emplace_back(new SingleSwitchProgramWrapper<TestProgram>(settings.programs));
    }

    list.emplace_back(new SingleSwitchProgramWrapper<PreventSleep>(settings.programs));
    list.emplace_back(new SingleSwitchProgramWrapper<FriendCodeAdder>(settings.programs));
    list.emplace_back(new SingleSwitchProgramWrapper<FriendDelete>(settings.programs));
    list.emplace_back(new SingleSwitchProgramWrapper<FastCodeEntry>(settings.programs));
    list.emplace_back(new SingleSwitchProgramWrapper<FriendSearchDisconnect>(settings.programs));

    list.emplace_back(new SingleSwitchProgramWrapper<TurboA>(settings.programs));
    list.emplace_back(new SingleSwitchProgramWrapper<MassRelease>(settings.programs));
    list.emplace_back(new SingleSwitchProgramWrapper<SurpriseTrade>(settings.programs));
    list.emplace_back(new SingleSwitchProgramWrapper<TradeBot>(settings.programs));
    list.emplace_back(new SingleSwitchProgramWrapper<ClothingBuyer>(settings.programs));
    list.emplace_back(new SingleSwitchProgramWrapper<BallThrower>(settings.programs));

    list.emplace_back(new SingleSwitchProgramWrapper<WattFarmer>(settings.programs));
    list.emplace_back(new SingleSwitchProgramWrapper<BerryFarmer>(settings.programs));
    list.emplace_back(new SingleSwitchProgramWrapper<LotoFarmer>(settings.programs));
    list.emplace_back(new SingleSwitchProgramWrapper<StowOnSideFarmer>(settings.programs));
    list.emplace_back(new SingleSwitchProgramWrapper<DailyHighlightFarmer>(settings.programs));

    list.emplace_back(new SingleSwitchProgramWrapper<BeamReset>(settings.programs));
    list.emplace_back(new SingleSwitchProgramWrapper<PurpleBeamFinder>(settings.programs));
    list.emplace_back(new SingleSwitchProgramWrapper<EventBeamFinder>(settings.programs));
    list.emplace_back(new SingleSwitchProgramWrapper<DaySkipperJPN>(settings.programs));
    list.emplace_back(new SingleSwitchProgramWrapper<DaySkipperEU>(settings.programs));
    list.emplace_back(new SingleSwitchProgramWrapper<DaySkipperUS>(settings.programs));
    list.emplace_back(new SingleSwitchProgramWrapper<DaySkipperJPN7p8k>(settings.programs));

    list.emplace_back(new SingleSwitchProgramWrapper<DenRoller>(settings.programs));
    list.emplace_back(new SingleSwitchProgramWrapper<AutoHostRolling>(settings.programs));
    list.emplace_back(new SingleSwitchProgramWrapper<AutoHostMultiGame>(settings.programs));

    list.emplace_back(new SingleSwitchProgramWrapper<MultiGameFossil>(settings.programs));
    list.emplace_back(new SingleSwitchProgramWrapper<ShinyHuntUnattendedRegi>(settings.programs));
    list.emplace_back(new SingleSwitchProgramWrapper<ShinyHuntUnattendedSwordsOfJustice>(settings.programs));
    list.emplace_back(new SingleSwitchProgramWrapper<ShinyHuntUnattendedStrongSpawn>(settings.programs));
    list.emplace_back(new SingleSwitchProgramWrapper<ShinyHuntUnattendedRegigigas2>(settings.programs));
    list.emplace_back(new SingleSwitchProgramWrapper<ShinyHuntUnattendedIoATrade>(settings.programs));
    
    list.emplace_back(new SingleSwitchProgramWrapper<ShinyHuntAutonomousRegi>(settings.programs));
    list.emplace_back(new SingleSwitchProgramWrapper<ShinyHuntAutonomousSwordsOfJustice>(settings.programs));
    list.emplace_back(new SingleSwitchProgramWrapper<ShinyHuntAutonomousStrongSpawn>(settings.programs));
    list.emplace_back(new SingleSwitchProgramWrapper<ShinyHuntAutonomousRegigigas2>(settings.programs));
    list.emplace_back(new SingleSwitchProgramWrapper<ShinyHuntAutonomousIoATrade>(settings.programs));
    list.emplace_back(new SingleSwitchProgramWrapper<ShinyHuntAutonomousBerryTree>(settings.programs));
    list.emplace_back(new SingleSwitchProgramWrapper<ShinyHuntAutonomousWhistling>(settings.programs));
    list.emplace_back(new SingleSwitchProgramWrapper<ShinyHuntAutonomousFishing>(settings.programs));
    if (settings.developer_mode){
        list.emplace_back(new SingleSwitchProgramWrapper<ShinyHuntAutonomousOverworld>(settings.programs));
    }

    list.emplace_back(new SingleSwitchProgramWrapper<EggFetcher2>(settings.programs));
    list.emplace_back(new SingleSwitchProgramWrapper<EggHatcher>(settings.programs));
    list.emplace_back(new SingleSwitchProgramWrapper<EggCombined2>(settings.programs));
    list.emplace_back(new SingleSwitchProgramWrapper<EggSuperCombined2>(settings.programs));
    list.emplace_back(new SingleSwitchProgramWrapper<GodEggItemDupe>(settings.programs));
    if (settings.naughty_mode){
        list.emplace_back(new SingleSwitchProgramWrapper<GodEggDuplication>(settings.programs));
    }

    list.emplace_back(new MultiSwitchProgramWrapper<SynchronizedSpinning>(settings.programs));
    list.emplace_back(new MultiSwitchProgramWrapper<RaidItemFarmerOHKO>(settings.programs));

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
