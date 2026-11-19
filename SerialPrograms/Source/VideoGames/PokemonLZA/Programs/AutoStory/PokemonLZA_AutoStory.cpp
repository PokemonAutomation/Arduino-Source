/*  Auto Story
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonTools/StartupChecks/VideoResolutionCheck.h"
#include "NintendoSwitch/Programs/NintendoSwitch_GameEntry.h"
#include "Pokemon/Pokemon_Strings.h"
#include "Pokemon/AutoStory/Pokemon_AutoStorySelect.h"
#include "PokemonLZA_AutoStory_Segment_01.h"
#include "PokemonLZA_AutoStory.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{

const std::vector<std::unique_ptr<AutoStory_Segment>>& ALL_AUTO_STORY_SEGMENT_LIST(){
    static const std::vector<std::unique_ptr<AutoStory_Segment>> segments = []{
        std::vector<std::unique_ptr<AutoStory_Segment>> ret;
        ret.emplace_back(std::make_unique<AutoStory_Segment_01>());
        return ret;
    }();
    return segments;
}

const std::vector<std::unique_ptr<AutoStory_Checkpoint>>& ALL_AUTO_STORY_CHECKPOINT_LIST(){
    static const std::vector<std::unique_ptr<AutoStory_Checkpoint>> checkpoints = []{
        std::vector<std::unique_ptr<AutoStory_Checkpoint>> ret;
        ret.emplace_back(std::make_unique<AutoStory_Checkpoint_00>());
        ret.emplace_back(std::make_unique<AutoStory_Checkpoint_01>());
        ret.emplace_back(std::make_unique<AutoStory_Checkpoint_02>());
        ret.emplace_back(std::make_unique<AutoStory_Checkpoint_03>());
        ret.emplace_back(std::make_unique<AutoStory_Checkpoint_04>());
        return ret;
    }();
    return checkpoints;
}

const StringSelectDatabase& AUTO_STORY_SEGMENTS_SELECT_DATABASE(){
    static const StringSelectDatabase database = Pokemon::make_story_select_database(
        ALL_AUTO_STORY_SEGMENT_LIST(), 0, ALL_AUTO_STORY_SEGMENT_LIST().size()
    );
    return database;
}

const StringSelectDatabase& AUTO_STORY_CHECKPOINTS_SELECT_DATABASE(){
    static const StringSelectDatabase database = Pokemon::make_story_select_database(
        ALL_AUTO_STORY_CHECKPOINT_LIST(), 0, ALL_AUTO_STORY_CHECKPOINT_LIST().size()
    );
    return database;
}

AutoStory_Descriptor::AutoStory_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonLZA:AutoStory",
        Pokemon::STRING_POKEMON + " LZA", "Auto Story",
        "Programs/PokemonLZA/AutoStory.html",
        "Progress through the main story of LZA (currently only tutorial)",
        PanelQualityStanding::OFFICIAL_BETA,
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}

std::unique_ptr<StatsTracker> AutoStory_Descriptor::make_stats() const{
    return std::make_unique<AutoStoryStats>();
}

AutoStory::AutoStory()
    : GO_HOME_WHEN_DONE(true)
    , STORY_SECTION(
        "<b>Story Section:</b>",
        {{StorySection::TUTORIAL, "tutorial", "Tutorial"}},
        LockMode::LOCK_WHILE_RUNNING,
        StorySection::TUTORIAL
    )
    , ENABLE_ADVANCED_MODE(
        "<b>Advanced mode:</b><br>Select start/end checkpoints instead of a starting segment",
        LockMode::LOCK_WHILE_RUNNING,
        false
    )
    , START_SEGMENT(
        "<b>Start Segment:</b>", AUTO_STORY_SEGMENTS_SELECT_DATABASE(),
        LockMode::LOCK_WHILE_RUNNING, "0"
    )
    , START_CHECKPOINT(
        "<b>Start Checkpoint:</b>", AUTO_STORY_CHECKPOINTS_SELECT_DATABASE(),
        LockMode::LOCK_WHILE_RUNNING, "0"
    )
    , END_CHECKPOINT(
        "<b>End Checkpoint:</b>", AUTO_STORY_CHECKPOINTS_SELECT_DATABASE(),
        LockMode::LOCK_WHILE_RUNNING, "4"
    )
    , START_DESCRIPTION("")
    , END_DESCRIPTION("")
    , SETUP_NOTE(
        "Place the player at the exact position described for the selected start point"
        "Later checkpoints assume the preceding story segments are complete with the Text Speed on High"
    )
    , STARTER(
        "<b>Starter:</b><br>Choose the starter Pokemon to receive",
        {
            {StarterChoice::CHIKORITA, "chikorita", "Chikorita (Grass)"},
            {StarterChoice::TOTODILE, "totodile", "Totodile (Water)"},
            {StarterChoice::TEPIG, "tepig", "Tepig (Fire)"},
        },
        LockMode::LOCK_WHILE_RUNNING,
        StarterChoice::TOTODILE
    )
    , NOTIFICATIONS({
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(STORY_SECTION);
    PA_ADD_OPTION(ENABLE_ADVANCED_MODE);
    PA_ADD_OPTION(START_SEGMENT);
    PA_ADD_OPTION(START_CHECKPOINT);
    PA_ADD_OPTION(END_CHECKPOINT);
    PA_ADD_OPTION(START_DESCRIPTION);
    PA_ADD_OPTION(END_DESCRIPTION);
    PA_ADD_OPTION(SETUP_NOTE);
    PA_ADD_OPTION(STARTER);
    PA_ADD_OPTION(BATTLE_AI);
    PA_ADD_OPTION(NOTIFICATIONS);

    on_config_value_changed(this);
    STORY_SECTION.add_listener(*this);
    ENABLE_ADVANCED_MODE.add_listener(*this);
    START_SEGMENT.add_listener(*this);
    START_CHECKPOINT.add_listener(*this);
    END_CHECKPOINT.add_listener(*this);
}

AutoStory::~AutoStory(){
    STORY_SECTION.remove_listener(*this);
    ENABLE_ADVANCED_MODE.remove_listener(*this);
    START_SEGMENT.remove_listener(*this);
    START_CHECKPOINT.remove_listener(*this);
    END_CHECKPOINT.remove_listener(*this);
}

void AutoStory::on_config_value_changed(void*){
    const bool advanced = ENABLE_ADVANCED_MODE;
    START_SEGMENT.set_visibility(advanced ? ConfigOptionState::HIDDEN : ConfigOptionState::ENABLED);
    START_CHECKPOINT.set_visibility(advanced ? ConfigOptionState::ENABLED : ConfigOptionState::HIDDEN);
    END_CHECKPOINT.set_visibility(advanced ? ConfigOptionState::ENABLED : ConfigOptionState::HIDDEN);
    if (advanced){
        START_DESCRIPTION.set_text("Start: " + ALL_AUTO_STORY_CHECKPOINT_LIST()[START_CHECKPOINT.index()]->start_text());
        END_DESCRIPTION.set_text("End: " + ALL_AUTO_STORY_CHECKPOINT_LIST()[END_CHECKPOINT.index()]->end_text());
    }else{
        START_DESCRIPTION.set_text(ALL_AUTO_STORY_SEGMENT_LIST()[START_SEGMENT.index()]->start_text());
        END_DESCRIPTION.set_text(ALL_AUTO_STORY_SEGMENT_LIST().back()->end_text());
    }
}

std::string AutoStory::check_validity() const{
    std::string error = SingleSwitchProgramInstance::check_validity();
    if (!error.empty()){
        return error;
    }
    if (ENABLE_ADVANCED_MODE && START_CHECKPOINT.index() > END_CHECKPOINT.index()){
        return "The start checkpoint cannot be later than the end checkpoint.";
    }
    return "";
}

void AutoStory::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    const std::string error = check_validity();
    if (!error.empty()){
        throw UserSetupError(env.console, error);
    }
    assert_16_9_720p_min(env.logger(), env.console);
    require_player(env.console, context, BUTTON_L);

    AutoStoryOptions options{
        STARTER,
        BATTLE_AI,
    };
    AutoStoryStats& stats = env.current_stats<AutoStoryStats>();
    if (ENABLE_ADVANCED_MODE){
        const auto& checkpoints = ALL_AUTO_STORY_CHECKPOINT_LIST();
        for (size_t index = START_CHECKPOINT.index(); index <= END_CHECKPOINT.index(); index++){
            context.throw_if_cancelled();
            checkpoints[index]->run_checkpoint(env, context, options, stats);
        }
    }else{
        const auto& segments = ALL_AUTO_STORY_SEGMENT_LIST();
        for (size_t index = START_SEGMENT.index(); index < segments.size(); index++){
            context.throw_if_cancelled();
            segments[index]->run_segment(env, context, options, stats);
        }
    }

    env.log("Story input sequence finished.");
    context.wait_for_all_requests();
    send_program_finished_notification(
        env, NOTIFICATION_PROGRAM_FINISH, "Auto Story finished",
        env.console.video().snapshot()
    );
    GO_HOME_WHEN_DONE.run_end_of_program(context);
}

}
}
}
