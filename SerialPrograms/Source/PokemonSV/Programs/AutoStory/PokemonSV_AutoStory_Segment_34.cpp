/*  AutoStory
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "Pokemon/Inference/Pokemon_NameReader.h"
#include "CommonFramework/Notifications/ProgramInfo.h"

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonSV/Programs/PokemonSV_GameEntry.h"
#include "PokemonSV/Programs/PokemonSV_SaveGame.h"
#include "PokemonSV/Programs/PokemonSV_MenuNavigation.h"
#include "PokemonSV/Programs/PokemonSV_WorldNavigation.h"
#include "PokemonSV_AutoStoryTools.h"
#include "PokemonSV_AutoStory_Segment_34.h"

//#include <iostream>
//using std::cout;
//using std::endl;
//#include <unordered_map>
//#include <algorithm>

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{




std::string AutoStory_Segment_34::name() const{
    return "34: Elite Four";
}

std::string AutoStory_Segment_34::start_text() const{
    return "Start: Beat Clavell. At Academy fly point.";
}

std::string AutoStory_Segment_34::end_text() const{
    return "End: Beat Elite Four. At Pokemon League Pokecenter.";
}

void AutoStory_Segment_34::run_segment(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    

    stats.m_segment++;
    env.update_stats();
    context.wait_for_all_requests();
    env.console.log("Start Segment " + name(), COLOR_ORANGE);

    checkpoint_86(env, context, options.notif_status_update, stats, options.language, options.starter_choice);

    context.wait_for_all_requests();
    env.console.log("End Segment " + name(), COLOR_GREEN);

}


void checkpoint_86(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats, Language language, StarterChoice starter_choice){
    checkpoint_reattempt_loop(env, context, notif_status_update, stats,
    [&](size_t attempt_number){

        ImageFloatBox box = {0.116223, 0.895000, 0.194915, 0.051724};
        ImageViewRGB32 image = extract_box_reference(env.console.video().snapshot(), box);

        std::set<std::string> ret;

        OCR::StringMatchResult result = Pokemon::PokemonNameReader::instance().read_substring(
            env.console.logger(), language, image,
            OCR::WHITE_TEXT_FILTERS()
        );
        if (result.results.empty()){
            env.console.log("Unable to check whether we are riding Miraidon or Koraidon.");
            OperationFailedException exception(
                ErrorReport::SEND_ERROR_REPORT,
                "Unable to check whether we are riding Miraidon or Koraidon.\n" + language_warning(language),
                env.console
            );
            exception.send_recoverable_notification(env);
        }else{
            for (const auto& item : result.results){
                ret.insert(item.second.token);
            }
        }
        


    });   
}

void checkpoint_87(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats){
}

void checkpoint_88(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats){
}

void checkpoint_89(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats){
}

void checkpoint_90(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats){
}




}
}
}
