/*  Overworld Matcher Lighting Test
 *
 *  From: https://github.com/PokemonAutomation/
 */

#include <algorithm>
#include <array>
#include <ctime>
#include "Common/Cpp/Json/JsonObject.h"
#include <sstream>
#include "Common/Cpp/Filesystem/Filesystem.h"
#include "Common/Cpp/Filesystem/FileIO.h"
#include "CommonFramework/GlobalAutoPaths.h"
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
static const StringSelectDatabase& lighting_matcher_database(size_t segment){
    static const std::array<StringSelectDatabase,10> databases=[](){
        // Include shared locations in each segment that visits them.
        const std::array<std::vector<std::string>,10> slugs{{
            {"poketch-landmarks","jubilife-center","route201-potion-exit",
                "sandgem-junction","route202-grass-exit"},
            {"oreburgh-mart"},
            {"oreburgh-mart","jubilife-street-approach","jubilife-plaza-approach",
                "route204-north-approach","teighler-alcove-corner","route204-north-stairs",
                "floaroma-center-entrance","ravaged-path-south-entrance","route204-sign",
                "windworks-sign","route205-bridge","route205-bank-tree","route205-zackary",
                "route205-kelsey","route205-forest","eterna-center","eterna-gym",
                "eterna-gym-statues","eterna-underground-house","eterna-underground-man"},
            {"eterna-center","galactic-hq-jupiter","hearthome-center","hearthome-center-interior",
                "hearthome-fantina","hearthome-gym","route208-stair-top",
                "route208-hannah-departure","route209-repel-point","route215-jogger-corner",
                "route215-bridge-renewal","route215-ace-renewal",
                "route215-derek-stairs-renewal","route215-derek-stair-top-renewal",
                "veilstone-city-passage","veilstone-dawn-stairs"},
            {},
            {"hearthome-gym"},
            {}, {}, {}, {}
        }};
        std::array<StringSelectDatabase,10> result;
        for(size_t i=0;i<result.size();i++){
            for(const auto& matcher:overworld_matchers()){
                if(std::find(slugs[i].begin(),slugs[i].end(),matcher.slug)!=slugs[i].end())
                    result[i].add_entry({matcher.slug,matcher.name});
            }
            if(result[i].size()==0)
                result[i].add_entry({"none","No landmark matchers available"});
        }
        return result;
    }();
    return databases.at(segment);
}
OverworldLightingTest_Descriptor::OverworldLightingTest_Descriptor()
    : SingleSwitchProgramDescriptor("PokemonBDSP:OverworldLightingTest","Pokemon BDSP","Overworld Matcher Lighting Test","",
        "Place the player at the selected matcher's reference location. Tests 24 hourly lighting settings without moving the player. "
        "Changes console date/time; saves screenshots and CSV results under Screenshots/OverworldLightingTest.",
        PanelQualityStanding::DEVELOPER,
        ProgramControllerClass::StandardController_NoRestrictions,FeedbackType::REQUIRED,AllowCommandsWhenRunning::DISABLE_COMMANDS)
{}
OverworldLightingTest::~OverworldLightingTest(){
    SEGMENT.remove_listener(*this);
}
OverworldLightingTest::OverworldLightingTest()
    : SEGMENT("<b>Segment:</b>",{
        {0,"00","00: Intro to Poketch"},
        {1,"01","01: Jubilife City to First Badge"},
        {2,"02","02: First Badge to Second Badge"},
        {3,"03","03: Second Badge to Third Badge"},
        {4,"04","04: Third Badge to Fourth Badge"},
        {5,"05","05: Fourth Badge to Fifth Badge"},
        {6,"06","06: Fifth Badge to Sixth Badge"},
        {7,"07","07: Sixth Badge to Seventh Badge"},
        {8,"08","08: Seventh Badge to Eighth Badge"},
        {9,"09","09: Eighth Badge to Champion"}
    },LockMode::LOCK_WHILE_RUNNING,0)
    , START_HOUR("<b>Starting hour:</b><br>Establishes the initial time, then advances one hour for each of 24 samples.",LockMode::LOCK_WHILE_RUNNING,0,0,23)
    , SETTLE_SECONDS("<b>Wait after returning to game (seconds):</b>",LockMode::LOCK_WHILE_RUNNING,5,1,60)
{
    PA_ADD_OPTION(SEGMENT);
    for(size_t i=0;i<MATCHERS.size();i++){
        const auto& database=lighting_matcher_database(i);
        MATCHERS[i]=std::make_unique<StringSelectOption>("<b>Landmark matcher:</b>",
            database,LockMode::LOCK_WHILE_RUNNING,i==0 ? "poketch-landmarks" : database[0].slug);
        add_option(*MATCHERS[i],"MATCHER_"+std::to_string(i));
    }
    PA_ADD_OPTION(START_HOUR);
    PA_ADD_OPTION(SETTLE_SECONDS);
    SEGMENT.add_listener(*this);
    on_config_value_changed(nullptr);
}
void OverworldLightingTest::on_config_value_changed(void*){
    for(size_t i=0;i<MATCHERS.size();i++){
        MATCHERS[i]->set_visibility(i!=SEGMENT.current_value() ? ConfigOptionState::HIDDEN :
            MATCHERS[i]->slug()=="none" ? ConfigOptionState::DISABLED : ConfigOptionState::ENABLED);
    }
}
std::string OverworldLightingTest::check_validity() const{
    const auto error=SingleSwitchProgramInstance::check_validity();
    if(!error.empty())return error;
    if(MATCHERS[SEGMENT.current_value()]->slug()=="none")
        return "The selected segment has no landmark matchers available.";
    return {};
}
void OverworldLightingTest::load_json(const JsonValue& json){
    SingleSwitchProgramInstance::load_json(json);
    // Preserve the selected matcher from settings saved before segment filtering.
    const auto* object=json.to_object();
    if(!object || object->get_value("SEGMENT"))return;
    const auto* legacy=object->get_string("MATCHER");
    if(!legacy)return;
    for(size_t i=0;i<MATCHERS.size();i++){
        if(lighting_matcher_database(i).search_index_by_slug(*legacy)!=SIZE_MAX){
            MATCHERS[i]->set_by_slug(*legacy);
            SEGMENT.set_value(i);
            return;
        }
    }
}
void OverworldLightingTest::program(SingleSwitchProgramEnvironment& env,ProControllerContext& context){
    const auto& slug=MATCHERS[SEGMENT.current_value()]->slug();
    const auto& matchers=overworld_matchers();
    const auto selected=std::find_if(matchers.begin(),matchers.end(),
        [&slug](const OverworldMatcher& entry){return entry.slug==slug;});
    if(selected==matchers.end())
        OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::NO_ERROR_REPORT,
            "The selected segment has no landmark matcher available.",env.console);
    const auto& matcher=*selected;
    Filesystem::Path output=Filesystem::Path(USER_FILE_PATH())/"Screenshots/OverworldLightingTest"/
        (matcher.slug+"-"+std::to_string(std::time(nullptr)));
    Filesystem::create_directories(output);
    FileIO results(output/"results.csv",FileMode::WRITE);
    std::ostringstream csv;
    if(!results.is_open())OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::NO_ERROR_REPORT,"Cannot create lighting-test results file.",env.console);
    csv<<"sample,hour,frame,landmark,score,margin,accepted,dx_screen,dy_screen,located,at_target,consensus_dx_screen,consensus_dy_screen,coarse\n";
    csv<<std::setprecision(8);
    int base=(int)(uint8_t)START_HOUR;
    set_time_fast(env,context,DateTime{2026,1,1,(int8_t)base,15,0});
    context.wait_for_all_requests();
    const auto baseline=env.console.video().snapshot();
    if(baseline){
        const auto position=matcher.match(*baseline.frame);
        env.log("[Lighting test] Baseline: located="+std::to_string(position.located)+
            "; at_target="+std::to_string(position.at_target)+
            "; screen-relative offset=("+std::to_string(position.dx)+","+std::to_string(position.dy)+
            "). The lighting test does not move the player.");
    }
    size_t located_frames=0, target_frames=0;
    for(int sample=1;sample<=24;sample++){
        int total=base+sample, hour=total%24, day=1+total/24;
        env.log("[Lighting test] Sample "+std::to_string(sample)+"/24; matcher="+matcher.name+"; hour="+std::to_string(hour)+":15.");
        set_time_fast(env,context,DateTime{2026,1,(int8_t)day,(int8_t)hour,15,0});
        pbf_wait(context,std::chrono::seconds((uint16_t)SETTLE_SECONDS));
        context.wait_for_all_requests();
        size_t located_this_hour=0, target_this_hour=0;
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
            if(result.located){located_frames++;located_this_hour++;}
            if(result.at_target){target_frames++;target_this_hour++;}
            std::string name="hour-"+std::to_string(hour)+"-frame-"+std::to_string(capture)+".png";
            if(!snapshot->save((output/name).string()))OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::NO_ERROR_REPORT,"Lighting test: Screenshot save failed.",env.console);
            const std::string rows=csv.str();
            if(results.write(rows)!=rows.size() || !results.flush()){
                OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::NO_ERROR_REPORT,"Lighting test: CSV write failed.",env.console);
            }
            csv.str("");
            csv.clear();
            pbf_wait(context,250ms);context.wait_for_all_requests();
        }
        env.log("[Lighting test] Hour "+std::to_string(hour)+": "+
            std::to_string(located_this_hour)+"/5 located; "+std::to_string(target_this_hour)+
            "/5 at target; "+std::to_string(located_this_hour-target_this_hour)+
            "/5 off target; "+std::to_string(5-located_this_hour)+"/5 missing landmarks.");
    }
    env.log("[Lighting test] Complete: "+std::to_string(located_frames)+"/120 located; "+
        std::to_string(target_frames)+"/120 at target; "+
        std::to_string(located_frames-target_frames)+"/120 off target; "+
        std::to_string(120-located_frames)+"/120 missing landmarks. Results: "+output.string());
}
}}}
