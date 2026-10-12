#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Exceptions/OperationFailedExceptionWithScreenshot.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonBDSP_OverworldLightingTest.h"
#include "../AutoStory/Utils/PokemonBDSP_OverworldMatchers.h"
#include "../AutoStory/Utils/PokemonBDSP_AutoStory_Segment_00_tools.h"
namespace PokemonAutomation{ namespace NintendoSwitch{ namespace PokemonBDSP{
using namespace std::chrono_literals;
static const StringSelectDatabase& lighting_matcher_database(){
    static const StringSelectDatabase db=[](){
        StringSelectDatabase result;
        for(const auto& matcher:overworld_matchers())result.add_entry({matcher.slug,matcher.name});
        return result;
    }();return db;
}
OverworldLightingTest_Descriptor::OverworldLightingTest_Descriptor()
    : SingleSwitchProgramDescriptor("PokemonBDSP:OverworldLightingTest","Pokemon BDSP","Overworld Matcher Lighting Test","",
        "Place the player at the selected matcher's reference location. Tests 24 hourly lighting settings without moving the player. "
        "Changes console date/time; saves screenshots and CSV results under Screenshots/OverworldLightingTest.",
        ProgramControllerClass::StandardController_NoRestrictions,FeedbackType::REQUIRED,AllowCommandsWhenRunning::DISABLE_COMMANDS)
{}
OverworldLightingTest::OverworldLightingTest()
    : MATCHER("<b>Matcher:</b>",lighting_matcher_database(),LockMode::LOCK_WHILE_RUNNING,"poketch-landmarks")
    , START_HOUR("<b>Starting hour:</b><br>Establishes the initial time, then advances one hour for each of 24 samples.",LockMode::LOCK_WHILE_RUNNING,0,0,23)
    , SETTLE_SECONDS("<b>Wait after returning to game (seconds):</b>",LockMode::LOCK_WHILE_RUNNING,5,1,60)
{PA_ADD_OPTION(MATCHER);PA_ADD_OPTION(START_HOUR);PA_ADD_OPTION(SETTLE_SECONDS);}
void OverworldLightingTest::program(SingleSwitchProgramEnvironment& env,ProControllerContext& context){
    const auto& matcher=overworld_matchers()[MATCHER.index()];
    std::filesystem::path output=std::filesystem::path("Screenshots/OverworldLightingTest")/
        (matcher.slug+"-"+std::to_string(std::time(nullptr)));
    std::filesystem::create_directories(output);
    std::ofstream csv(output/"results.csv");
    if(!csv)OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::NO_ERROR_REPORT,"Cannot create lighting-test results file.",env.console);
    csv<<"sample,hour,frame,landmark,score,margin,accepted,dx_screen,dy_screen,located,at_target,consensus_dx_screen,consensus_dy_screen,coarse\n";
    csv<<std::setprecision(8);
    int base=(int)(uint8_t)START_HOUR;
    set_time_fast(env,context,DateTime{2026,1,1,(int8_t)base,15,0});
    size_t target_frames=0;
    for(int sample=1;sample<=24;sample++){
        int total=base+sample, hour=total%24, day=1+total/24;
        env.log("[Lighting test] Sample "+std::to_string(sample)+"/24; matcher="+matcher.name+"; hour="+std::to_string(hour)+":15.");
        set_time_fast(env,context,DateTime{2026,1,(int8_t)day,(int8_t)hour,15,0});
        pbf_wait(context,std::chrono::seconds((uint16_t)SETTLE_SECONDS));
        context.wait_for_all_requests();
        for(int capture=1;capture<=5;capture++){
            auto snapshot=env.console.video().snapshot();
            if(!snapshot)OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,"Lighting test: No video frame.",env.console);
            const auto result=matcher.match(*snapshot.frame);
            VideoOverlaySet overlays(env.console.overlay());
            for(const auto& landmark:result.landmarks){
                overlays.add(COLOR_CYAN,landmark.search);
                if(landmark.accepted)overlays.add(COLOR_GREEN,landmark.found);
                env.log("[Lighting test] hour="+std::to_string(hour)+" frame="+std::to_string(capture)+" landmark="+landmark.name+
                    " score="+std::to_string(landmark.score)+" margin="+std::to_string(landmark.margin)+
                    " accepted="+std::to_string(landmark.accepted)+" screen-relative offset=("+std::to_string(landmark.dx)+","+std::to_string(landmark.dy)+
                    ") located="+std::to_string(result.located)+" at_target="+std::to_string(result.at_target)+" coarse="+std::to_string(result.coarse));
                csv<<sample<<','<<hour<<','<<capture<<','<<landmark.name<<','<<landmark.score<<','<<landmark.margin<<','<<landmark.accepted<<','
                    <<landmark.dx<<','<<landmark.dy<<','<<result.located<<','<<result.at_target<<','<<result.dx<<','<<result.dy<<','<<result.coarse<<'\n';
            }
            if(result.at_target)target_frames++;
            std::string name="hour-"+std::to_string(hour)+"-frame-"+std::to_string(capture)+".png";
            if(!snapshot->save((output/name).string()))OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::NO_ERROR_REPORT,"Lighting test: Screenshot save failed.",env.console);
            csv.flush();
            pbf_wait(context,250ms);context.wait_for_all_requests();
        }
    }
    env.log("[Lighting test] Complete: "+std::to_string(target_frames)+"/120 frames at target. Results: "+output.string());
}
}}}
