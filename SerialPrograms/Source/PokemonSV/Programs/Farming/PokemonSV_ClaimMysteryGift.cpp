/*  ClaimMysteryGift
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonTools/StartupChecks/VideoResolutionCheck.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSwSh/Inference/PokemonSwSh_IvJudgeReader.h"
#include "PokemonSV/Inference/PokemonSV_MainMenuDetector.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_DirectionDetector.h"
#include "PokemonSV/Programs/PokemonSV_GameEntry.h"
#include "PokemonSV/Programs/PokemonSV_MenuNavigation.h"
#include "PokemonSV/Programs/PokemonSV_WorldNavigation.h"
#include "PokemonSV_ClaimMysteryGift.h"

#include <iostream>
using std::cout;
using std::endl;
//#include <unordered_map>
//#include <algorithm>

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

using namespace Pokemon;


ClaimMysteryGift_Descriptor::ClaimMysteryGift_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSV:ClaimMysteryGift",
        STRING_POKEMON + " SV", "Claim Mystery Gift",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSV/ClaimMysteryGift.md",
        "Claim the Mystery Gift in SV.",
        FeedbackType::VIDEO_AUDIO,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        {SerialPABotBase::OLD_NINTENDO_SWITCH_DEFAULT_REQUIREMENTS}
    )
{}

// std::unique_ptr<StatsTracker> ClaimMysteryGift_Descriptor::make_stats() const{
//     return std::unique_ptr<StatsTracker>(new ClaimMysteryGiftStats());
// }



ClaimMysteryGift::~ClaimMysteryGift(){
    STARTING_POINT.remove_listener(*this);
}

ClaimMysteryGift::ClaimMysteryGift()
    : LANGUAGE(
        "<b>Game Language:</b>",
        PokemonSwSh::IV_READER().languages(),
        LockMode::UNLOCK_WHILE_RUNNING,
        true
    )
    , STARTING_POINT(
        "<b>Starting point:",
        {
            {StartingPoint::NEW_GAME,                "new-game",           "New Game"},
            {StartingPoint::DONE_TUTORIAL,           "done-tutorial",      "Done Tutorial"},
        },
        LockMode::LOCK_WHILE_RUNNING,
        StartingPoint::NEW_GAME
    )    
    , GO_HOME_WHEN_DONE(true)
    , NOTIFICATION_STATUS_UPDATE("Status Update", true, false, std::chrono::seconds(30))
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS_UPDATE,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_RECOVERABLE,
        &NOTIFICATION_ERROR_FATAL,
    })
{

    PA_ADD_OPTION(LANGUAGE);
    PA_ADD_OPTION(STARTING_POINT);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(NOTIFICATIONS);


    ClaimMysteryGift::on_config_value_changed(this);

    STARTING_POINT.add_listener(*this);
}

void ClaimMysteryGift::on_config_value_changed(void* object){


}







void ClaimMysteryGift::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    assert_16_9_720p_min(env.logger(), env.console);
    // ClaimMysteryGiftStats& stats = env.current_stats<ClaimMysteryGiftStats>();


    // Connect controller
    pbf_press_button(context, BUTTON_L, 20, 20);

    // env.console.log("Start Segment " + ALL_AUTO_STORY_SEGMENT_LIST()[get_start_segment_index()]->name(), COLOR_ORANGE);



    // run_autostory(env, context);
    
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
    GO_HOME_WHEN_DONE.run_end_of_program(context);
}






}
}
}
