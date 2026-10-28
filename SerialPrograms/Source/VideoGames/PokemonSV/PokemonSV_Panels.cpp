/*  Pokemon SV Panels
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/StaticGlobals.h"
#include "CommonFramework/GlobalAutoPaths.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSV_Panels.h"

#include "PokemonSV_Settings.h"

#include "Programs/General/PokemonSV_MassPurchase.h"
#include "Programs/General/PokemonSV_ClothingBuyer.h"
#include "Programs/General/PokemonSV_AutonomousBallThrower.h"
#include "Programs/General/PokemonSV_SizeChecker.h"

#include "Programs/Boxes/PokemonSV_MassRelease.h"
#include "Programs/Boxes/PokemonSV_MassAttachItems.h"

#include "Programs/Trading/PokemonSV_SelfBoxTrade.h"
#include "Programs/Sandwiches/PokemonSV_SandwichMaker.h"

#include "Programs/Farming/PokemonSV_LPFarmer.h"
#include "Programs/Farming/PokemonSV_GimmighoulChestFarmer.h"
#include "Programs/Farming/PokemonSV_GimmighoulRoamingFarmer.h"
#include "Programs/Farming/PokemonSV_AuctionFarmer.h"
#include "Programs/Farming/PokemonSV_ESPTraining.h"
#include "Programs/Farming/PokemonSV_TournamentFarmer.h"
#include "Programs/Farming/PokemonSV_TournamentFarmer2.h"
#include "Programs/Farming/PokemonSV_FlyingTrialFarmer.h"
#include "Programs/Farming/PokemonSV_BBQSoloFarmer.h"
#include "Programs/Farming/PokemonSV_MaterialFarmer.h"
#include "Programs/ItemPrinter/PokemonSV_ItemPrinterRNG.h"

#include "Programs/Eggs/PokemonSV_EggFetcher.h"
#include "Programs/Eggs/PokemonSV_EggHatcher.h"
#include "Programs/Eggs/PokemonSV_EggAutonomous.h"

#include "Programs/TeraRaids/PokemonSV_AutoHost.h"
#include "Programs/TeraRaids/PokemonSV_TeraRoller.h"
#include "Programs/TeraRaids/PokemonSV_TeraSelfFarmer.h"
#include "Programs/TeraRaids/PokemonSV_TeraMultiFarmer.h"

#include "Programs/FastCodeEntry/PokemonSV_FastCodeEntry.h"
#include "Programs/FastCodeEntry/PokemonSV_ClipboardFastCodeEntry.h"
#include "Programs/FastCodeEntry/PokemonSV_VideoFastCodeEntry.h"

#include "Programs/General/PokemonSV_StatsReset.h"
#include "Programs/General/PokemonSV_StatsResetEventBattle.h"

#include "Programs/ShinyHunting/PokemonSV_ShinyHunt-AreaZeroPlatform.h"
#include "Programs/ShinyHunting/PokemonSV_ShinyHunt-Scatterbug.h"

#include "Programs/AutoStory/PokemonSV_AutoStory.h"
#include "Programs/Farming/PokemonSV_ClaimMysteryGift.h"

#include "Programs/Glitches/PokemonSV_WildItemFarmer.h"
#include "Programs/Glitches/PokemonSV_RideCloner-1.0.1.h"
#include "Programs/Glitches/PokemonSV_CloneItems-1.0.1.h"

//  Deprecated
#include "Programs/ItemPrinter/PokemonSV_AutoItemPrinter.h"

#include "Programs/TestPrograms/PokemonSV_SoundListener.h"
#include "Programs/FormHunting/PokemonSV_ThreeSegmentDudunsparceFinder.h"

#ifdef PA_OFFICIAL
#include "../../Internal/SerialPrograms/NintendoSwitch_TestPrograms.h"
#endif

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{



PanelListFactory::PanelListFactory()
    : PanelListDescriptor(
        Pokemon::STRING_POKEMON + " Scarlet and Violet",
        RESOURCE_PATH() + "CategoryIcons/PokemonSV.png"
    )
{}

std::vector<PanelEntry> PanelListFactory::make_panels() const{
    std::vector<PanelEntry> ret;

    ret.emplace_back("---- Settings ----");
    ret.emplace_back(make_OptionsPanel<GameSettingsPanel>());

    ret.emplace_back("---- General ----");
    ret.emplace_back(make_SingleSwitchProgram<MassPurchase>());
    ret.emplace_back(make_SingleSwitchProgram<ClothingBuyer>());
    ret.emplace_back(make_SingleSwitchProgram<AutonomousBallThrower>());
    ret.emplace_back(make_SingleSwitchProgram<SizeChecker>());

//    ret.emplace_back("---- Trading ----");
    ret.emplace_back(make_MultiSwitchProgram<SelfBoxTrade>());

//    ret.emplace_back("---- Sandwiches ----");
    ret.emplace_back(make_SingleSwitchProgram<SandwichMaker>());

    ret.emplace_back("---- Boxes ----");
    ret.emplace_back(make_SingleSwitchProgram<MassRelease>());
    ret.emplace_back(make_SingleSwitchProgram<MassAttachItems>());

    ret.emplace_back("---- Farming ----");
    ret.emplace_back(make_SingleSwitchProgram<LPFarmer>());
    ret.emplace_back(make_SingleSwitchProgram<GimmighoulRoamingFarmer>());
    ret.emplace_back(make_SingleSwitchProgram<GimmighoulChestFarmer>());
    ret.emplace_back(make_SingleSwitchProgram<AuctionFarmer>());
    ret.emplace_back(make_SingleSwitchProgram<ESPTraining>());
    ret.emplace_back(make_SingleSwitchProgram<TournamentFarmer>());
    ret.emplace_back(make_SingleSwitchProgram<TournamentFarmer2>());
    ret.emplace_back(make_SingleSwitchProgram<FlyingTrialFarmer>());
    ret.emplace_back(make_SingleSwitchProgram<BBQSoloFarmer>());
    ret.emplace_back(make_SingleSwitchProgram<MaterialFarmer>());
    ret.emplace_back(make_SingleSwitchProgram<ItemPrinterRNG>());

    ret.emplace_back("---- Eggs ----");
    ret.emplace_back(make_SingleSwitchProgram<EggFetcher>());
    ret.emplace_back(make_SingleSwitchProgram<EggHatcher>());
    ret.emplace_back(make_SingleSwitchProgram<EggAutonomous>());

    ret.emplace_back("---- Tera Raids ----");
    ret.emplace_back(make_SingleSwitchProgram<AutoHost>());
    ret.emplace_back(make_SingleSwitchProgram<TeraRoller>());
    ret.emplace_back(make_SingleSwitchProgram<TeraSelfFarmer>());
    ret.emplace_back(make_MultiSwitchProgram<TeraMultiFarmer>());

    ret.emplace_back("---- Fast Code Entry ----");
    ret.emplace_back(make_MultiSwitchProgram<FastCodeEntry>());
    ret.emplace_back(make_MultiSwitchProgram<ClipboardFastCodeEntry>());
    ret.emplace_back(make_MultiSwitchProgram<VideoFastCodeEntry>());

    ret.emplace_back("---- Stats Hunting ----");
    ret.emplace_back(make_SingleSwitchProgram<StatsReset>());
    ret.emplace_back(make_SingleSwitchProgram<StatsResetEventBattle>());

    ret.emplace_back("---- Shiny Hunting ----");
    ret.emplace_back(make_SingleSwitchProgram<ShinyHuntAreaZeroPlatform>());
    ret.emplace_back(make_SingleSwitchProgram< ShinyHuntScatterbug>());

    ret.emplace_back("---- Story Automation ----");
    ret.emplace_back(make_SingleSwitchProgram<AutoStory>());
    ret.emplace_back(make_SingleSwitchProgram<ClaimMysteryGift>());

    ret.emplace_back("---- Glitches (v3.0.0) ----");
    ret.emplace_back(make_SingleSwitchProgram<WildItemFarmer>());

    ret.emplace_back("---- Glitches (v1.0.1) ----");
    ret.emplace_back(make_SingleSwitchProgram<RideCloner101>());
    ret.emplace_back(make_SingleSwitchProgram< CloneItems101>());

//    ret.emplace_back("---- Public Betas ----");

    ret.emplace_back("---- Deprecated Programs ----");
    ret.emplace_back(make_SingleSwitchProgram<AutoItemPrinter>());

//    if (STATIC_GLOBALS.DEVELOPER_MODE || IS_BETA_VERSION){
//        ret.emplace_back("---- Untested/Beta/WIP ----");
//    }
//    if (IS_BETA_VERSION){
//        ret.emplace_back("---- Story Automation ----");
//    }
    if (STATIC_GLOBALS.DEVELOPER_MODE){
        ret.emplace_back("---- Developer Tools ----");
        ret.emplace_back(make_SingleSwitchProgram<SoundListener>());
        ret.emplace_back(make_SingleSwitchProgram<ThreeSegmentDudunsparceFinder>());
    }

#ifdef PA_OFFICIAL
    if (STATIC_GLOBALS.INTERNAL_DEVELOPER_MODE){
        ret.emplace_back("---- Research ----");
        add_panels(ret);
    }
#endif

    return ret;
}




}
}
}
