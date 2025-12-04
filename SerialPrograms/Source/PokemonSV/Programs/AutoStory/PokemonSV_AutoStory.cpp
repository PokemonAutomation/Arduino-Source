/*  AutoStory
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */


#include "ML/Inference/ML_YOLOv5Detector.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/Exceptions/UnexpectedBattleException.h"


#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonTools/Async/InferenceRoutines.h"


#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonTools/StartupChecks/StartProgramChecks.h"
#include "CommonTools/StartupChecks/VideoResolutionCheck.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Programs/DateManip/NintendoSwitch_DateManip.h"
#include "NintendoSwitch/Programs/NintendoSwitch_GameEntry.h"
#include "PokemonSV/PokemonSV_Settings.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSwSh/Inference/PokemonSwSh_IvJudgeReader.h"
#include "PokemonSV/Inference/PokemonSV_MainMenuDetector.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_DirectionDetector.h"
#include "PokemonSV/Programs/PokemonSV_GameEntry.h"
#include "PokemonSV/Programs/PokemonSV_MenuNavigation.h"
#include "PokemonSV/Programs/PokemonSV_WorldNavigation.h"
#include "PokemonSV_AutoStory_Segment_00.h"
#include "PokemonSV_AutoStory_Segment_01.h"
#include "PokemonSV_AutoStory_Segment_02.h"
#include "PokemonSV_AutoStory_Segment_03.h"
#include "PokemonSV_AutoStory_Segment_04.h"
#include "PokemonSV_AutoStory_Segment_05.h"
#include "PokemonSV_AutoStory_Segment_06.h"
#include "PokemonSV_AutoStory_Segment_07.h"
#include "PokemonSV_AutoStory_Segment_08.h"
#include "PokemonSV_AutoStory_Segment_09.h"
#include "PokemonSV_AutoStory_Segment_10.h"
#include "PokemonSV_AutoStory_Segment_11.h"
#include "PokemonSV_AutoStory_Segment_12.h"
#include "PokemonSV_AutoStory_Segment_13.h"
#include "PokemonSV_AutoStory_Segment_14.h"
#include "PokemonSV_AutoStory_Segment_15.h"
#include "PokemonSV_AutoStory_Segment_16.h"
#include "PokemonSV_AutoStory_Segment_17.h"
#include "PokemonSV_AutoStory_Segment_18.h"
#include "PokemonSV_AutoStory_Segment_19.h"
#include "PokemonSV_AutoStory_Segment_20.h"
#include "PokemonSV_AutoStory_Segment_21.h"
#include "PokemonSV_AutoStory_Segment_22.h"
#include "PokemonSV_AutoStory_Segment_23.h"
#include "PokemonSV_AutoStory_Segment_24.h"
#include "PokemonSV_AutoStory_Segment_25.h"
#include "PokemonSV_AutoStory_Segment_26.h"
#include "PokemonSV_AutoStory_Segment_27.h"
#include "PokemonSV_AutoStory_Segment_28.h"
#include "PokemonSV_AutoStory_Segment_29.h"
#include "PokemonSV_AutoStory_Segment_30.h"
#include "PokemonSV_AutoStory_Segment_31.h"
#include "PokemonSV_AutoStory_Segment_32.h"
#include "PokemonSV_AutoStory_Segment_33.h"
#include "PokemonSV_AutoStory_Segment_34.h"
#include "PokemonSV_AutoStory_Segment_35.h"
#include "PokemonSV_AutoStory_Segment_36.h"
#include "PokemonSV_AutoStory_Segment_37.h"
#include "PokemonSV_AutoStory_Segment_38.h"
#include "PokemonSV_AutoStory_Segment_39.h"
#include "PokemonSV_AutoStory_Segment_40.h"
#include "PokemonSV_AutoStory.h"

#include <iostream>
using std::cout;
using std::endl;
//#include <unordered_map>
//#include <algorithm>

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

using namespace Pokemon;
using namespace ML;

static constexpr size_t INDEX_OF_LAST_TUTORIAL_SEGMENT = 9;
static constexpr size_t INDEX_OF_LAST_TUTORIAL_CHECKPOINT = 20;


std::vector<std::unique_ptr<AutoStory_Segment>> make_autoStory_segment_list(){
    std::vector<std::unique_ptr<AutoStory_Segment>> segment_list;
    segment_list.emplace_back(std::make_unique<AutoStory_Segment_00>());
    segment_list.emplace_back(std::make_unique<AutoStory_Segment_01>());
    segment_list.emplace_back(std::make_unique<AutoStory_Segment_02>());
    segment_list.emplace_back(std::make_unique<AutoStory_Segment_03>());
    segment_list.emplace_back(std::make_unique<AutoStory_Segment_04>());
    segment_list.emplace_back(std::make_unique<AutoStory_Segment_05>());
    segment_list.emplace_back(std::make_unique<AutoStory_Segment_06>());
    segment_list.emplace_back(std::make_unique<AutoStory_Segment_07>());
    segment_list.emplace_back(std::make_unique<AutoStory_Segment_08>());
    segment_list.emplace_back(std::make_unique<AutoStory_Segment_09>());
    segment_list.emplace_back(std::make_unique<AutoStory_Segment_10>());
    segment_list.emplace_back(std::make_unique<AutoStory_Segment_11>());
    segment_list.emplace_back(std::make_unique<AutoStory_Segment_12>());
    segment_list.emplace_back(std::make_unique<AutoStory_Segment_13>());
    segment_list.emplace_back(std::make_unique<AutoStory_Segment_14>());
    segment_list.emplace_back(std::make_unique<AutoStory_Segment_15>());
    segment_list.emplace_back(std::make_unique<AutoStory_Segment_16>());
    segment_list.emplace_back(std::make_unique<AutoStory_Segment_17>());
    segment_list.emplace_back(std::make_unique<AutoStory_Segment_18>());
    segment_list.emplace_back(std::make_unique<AutoStory_Segment_19>());
    segment_list.emplace_back(std::make_unique<AutoStory_Segment_20>());
    segment_list.emplace_back(std::make_unique<AutoStory_Segment_21>());
    segment_list.emplace_back(std::make_unique<AutoStory_Segment_22>());
    segment_list.emplace_back(std::make_unique<AutoStory_Segment_23>());
    segment_list.emplace_back(std::make_unique<AutoStory_Segment_24>());
    segment_list.emplace_back(std::make_unique<AutoStory_Segment_25>());
    segment_list.emplace_back(std::make_unique<AutoStory_Segment_26>());
    segment_list.emplace_back(std::make_unique<AutoStory_Segment_27>());
    segment_list.emplace_back(std::make_unique<AutoStory_Segment_28>());
    segment_list.emplace_back(std::make_unique<AutoStory_Segment_29>());
    segment_list.emplace_back(std::make_unique<AutoStory_Segment_30>());
    segment_list.emplace_back(std::make_unique<AutoStory_Segment_31>());
    segment_list.emplace_back(std::make_unique<AutoStory_Segment_32>());
    segment_list.emplace_back(std::make_unique<AutoStory_Segment_33>());
    segment_list.emplace_back(std::make_unique<AutoStory_Segment_34>());

    if (PreloadSettings::instance().DEVELOPER_MODE){
    segment_list.emplace_back(std::make_unique<AutoStory_Segment_35>());
    segment_list.emplace_back(std::make_unique<AutoStory_Segment_36>());
    segment_list.emplace_back(std::make_unique<AutoStory_Segment_37>());
    segment_list.emplace_back(std::make_unique<AutoStory_Segment_38>());
    // segment_list.emplace_back(std::make_unique<AutoStory_Segment_39>());
    // segment_list.emplace_back(std::make_unique<AutoStory_Segment_40>());
    }
    return segment_list;
};

const std::vector<std::unique_ptr<AutoStory_Segment>>& ALL_AUTO_STORY_SEGMENT_LIST(){
    static std::vector<std::unique_ptr<AutoStory_Segment>> segment_list = make_autoStory_segment_list();
    return segment_list;
}


StringSelectDatabase make_all_segments_database(){
    StringSelectDatabase ret;
    int index_num = 0;
    for (const auto& segment : ALL_AUTO_STORY_SEGMENT_LIST()){
        ret.add_entry(StringSelectEntry(std::to_string(index_num), segment->name()));
        index_num++;
    }
    return ret;
}
const StringSelectDatabase& ALL_SEGMENTS_SELECT_DATABASE(){
    static StringSelectDatabase database = make_all_segments_database();
    return database;
}

StringSelectDatabase make_tutorial_segments_database(){
    StringSelectDatabase ret;
    const StringSelectDatabase& all_segments = ALL_SEGMENTS_SELECT_DATABASE();
    size_t start = 0;
    size_t end = INDEX_OF_LAST_TUTORIAL_SEGMENT + 1;
    for (size_t i = start; i < end; i++){
        const auto& segment = all_segments[i];
        ret.add_entry(segment);
    }
    return ret;
}

const StringSelectDatabase& TUTORIAL_SEGMENTS_SELECT_DATABASE(){
    static StringSelectDatabase database = make_tutorial_segments_database();
    return database;
}

StringSelectDatabase make_mainstory_segments_database(){
    StringSelectDatabase ret;
    const StringSelectDatabase& all_segments = ALL_SEGMENTS_SELECT_DATABASE();
    size_t start = INDEX_OF_LAST_TUTORIAL_SEGMENT + 1;
    size_t end = all_segments.case_list().size();
    for (size_t i = start; i < end; i++){
        const auto& segment = all_segments[i];
        ret.add_entry(segment);
    }
    return ret;
}

const StringSelectDatabase& MAINSTORY_SEGMENTS_SELECT_DATABASE(){
    static StringSelectDatabase database = make_mainstory_segments_database();
    return database;
}

std::vector<std::unique_ptr<AutoStory_Checkpoint>> make_autoStory_checkpoint_list(){
    std::vector<std::unique_ptr<AutoStory_Checkpoint>> checkpoint_list;
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_00>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_01>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_02>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_03>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_04>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_05>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_06>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_07>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_08>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_09>());

    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_10>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_11>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_12>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_13>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_14>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_15>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_16>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_17>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_18>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_19>());

    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_20>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_21>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_22>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_23>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_24>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_25>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_26>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_27>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_28>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_29>());

    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_30>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_31>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_32>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_33>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_34>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_35>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_36>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_37>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_38>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_39>());

    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_40>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_41>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_42>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_43>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_44>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_45>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_46>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_47>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_48>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_49>());

    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_50>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_51>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_52>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_53>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_54>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_55>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_56>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_57>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_58>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_59>());

    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_60>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_61>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_62>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_63>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_64>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_65>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_66>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_67>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_68>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_69>());

    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_70>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_71>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_72>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_73>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_74>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_75>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_76>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_77>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_78>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_79>());

    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_80>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_81>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_82>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_83>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_84>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_85>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_86>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_87>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_88>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_89>());

    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_90>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_91>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_92>());

    if (PreloadSettings::instance().DEVELOPER_MODE){
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_93>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_94>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_95>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_96>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_97>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_98>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_99>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_100>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_101>());
    // checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_102>());
    // checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_103>());
    // checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_104>());
    // checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_105>());

    }

    return checkpoint_list;
};

const std::vector<std::unique_ptr<AutoStory_Checkpoint>>& ALL_AUTO_STORY_CHECKPOINT_LIST(){
    static std::vector<std::unique_ptr<AutoStory_Checkpoint>> checkpoint_list = make_autoStory_checkpoint_list();
    return checkpoint_list;
}

StringSelectDatabase make_all_checkpoints_database(){
    StringSelectDatabase ret;
    int index_num = 0;
    for (const auto& segment : ALL_AUTO_STORY_CHECKPOINT_LIST()){
        ret.add_entry(StringSelectEntry(std::to_string(index_num), segment->name()));
        index_num++;
    }
    return ret;
}
const StringSelectDatabase& ALL_CHECKPOINTS_SELECT_DATABASE(){
    static StringSelectDatabase database = make_all_checkpoints_database();
    return database;
}

StringSelectDatabase make_tutorial_checkpoints_database(){
    StringSelectDatabase ret;
    const StringSelectDatabase& all_checkpoints = ALL_CHECKPOINTS_SELECT_DATABASE();
    size_t start = 0;
    size_t end = INDEX_OF_LAST_TUTORIAL_CHECKPOINT + 1;
    for (size_t i = start; i < end; i++){
        const auto& segment = all_checkpoints[i];
        ret.add_entry(segment);
    }
    return ret;
}

const StringSelectDatabase& TUTORIAL_CHECKPOINTS_SELECT_DATABASE(){
    static StringSelectDatabase database = make_tutorial_checkpoints_database();
    return database;
}

StringSelectDatabase make_mainstory_checkpoints_database(){
    StringSelectDatabase ret;
    const StringSelectDatabase& all_checkpoints = ALL_CHECKPOINTS_SELECT_DATABASE();
    size_t start = INDEX_OF_LAST_TUTORIAL_CHECKPOINT + 1;
    size_t end = all_checkpoints.case_list().size();
    for (size_t i = start; i < end; i++){
        const auto& segment = all_checkpoints[i];
        ret.add_entry(segment);
    }
    return ret;
}

const StringSelectDatabase& MAINSTORY_CHECKPOINTS_SELECT_DATABASE(){
    static StringSelectDatabase database = make_mainstory_checkpoints_database();
    return database;
}

AutoStory_Descriptor::AutoStory_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSV:AutoStory",
        STRING_POKEMON + " SV", "Auto Story",
        "Programs/PokemonSV/AutoStory.html",
        "Progress through the mainstory of SV.",
        ProgramControllerClass::StandardController_RequiresPrecision,
        FeedbackType::VIDEO_AUDIO,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        {}
    )
{}

std::unique_ptr<StatsTracker> AutoStory_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new AutoStoryStats());
}



AutoStory::~AutoStory(){
    STORY_SECTION.remove_listener(*this);
    STARTPOINT_TUTORIAL.remove_listener(*this);
    ENDPOINT_TUTORIAL.remove_listener(*this);
    STARTPOINT_MAINSTORY.remove_listener(*this);
    ENDPOINT_MAINSTORY.remove_listener(*this);    
    ENABLE_TEST_CHECKPOINTS.remove_listener(*this);
    ENABLE_TEST_REALIGN.remove_listener(*this);
    ENABLE_MISC_TEST.remove_listener(*this);
    TEST_PBF_LEFT_JOYSTICK.remove_listener(*this);
    TEST_PBF_JOYSTICK2.remove_listener(*this);
    TEST_CURRENT_DIRECTION.remove_listener(*this);
    TEST_CHANGE_DIRECTION.remove_listener(*this);    
}

AutoStory::AutoStory()
    : LANGUAGE(
        "<b>Game Language:</b>",
        PokemonSwSh::IV_READER().languages(),
        LockMode::UNLOCK_WHILE_RUNNING,
        true
    )
    , STORY_SECTION(
        "<b>Story Section:",
        {
            {StorySection::TUTORIAL,         "tutorial",           "Tutorial"},
            {StorySection::MAIN_STORY,            "main-story",              "Main Story"},
        },
        LockMode::LOCK_WHILE_RUNNING,
        StorySection::TUTORIAL
    )    
    , STARTPOINT_TUTORIAL(
        "<b>Start Segment:</b>", //<br>Program will start with this segment.
        TUTORIAL_SEGMENTS_SELECT_DATABASE(),
        LockMode::LOCK_WHILE_RUNNING,
        "0"
    )
    , ENDPOINT_TUTORIAL(
        "<b>End Segment:</b>", //<br>Program will stop after completing this segment.
        TUTORIAL_SEGMENTS_SELECT_DATABASE(),
        LockMode::UNLOCK_WHILE_RUNNING,
        std::to_string(INDEX_OF_LAST_TUTORIAL_SEGMENT)
    )   
    , STARTPOINT_MAINSTORY(
        "<b>Start Segment:</b>", //<br>Program will start with this segment.
        MAINSTORY_SEGMENTS_SELECT_DATABASE(),
        LockMode::UNLOCK_WHILE_RUNNING,
        std::to_string(INDEX_OF_LAST_TUTORIAL_SEGMENT+1)
    )
    , ENDPOINT_MAINSTORY(
        "<b>End Segment:</b>", //<br>Program will stop after completing this segment.
        MAINSTORY_SEGMENTS_SELECT_DATABASE(),
        LockMode::UNLOCK_WHILE_RUNNING,
        std::to_string(ALL_SEGMENTS_SELECT_DATABASE().case_list().size()-1)
    )       
    , START_CHECKPOINT_TUTORIAL(
        "<b>Start Checkpoint:</b>", //<br>Program will start with this checkpoint.
        TUTORIAL_CHECKPOINTS_SELECT_DATABASE(),
        LockMode::LOCK_WHILE_RUNNING,
        "0"
    )
    , END_CHECKPOINT_TUTORIAL(
        "<b>End Checkpoint:</b>", //<br>Program will stop after completing this checkpoint.
        TUTORIAL_CHECKPOINTS_SELECT_DATABASE(),
        LockMode::UNLOCK_WHILE_RUNNING,
        std::to_string(INDEX_OF_LAST_TUTORIAL_CHECKPOINT)
    ) 
    , START_CHECKPOINT_MAINSTORY(
        "<b>Start Checkpoint:</b>", //<br>Program will start with this checkpoint.
        MAINSTORY_CHECKPOINTS_SELECT_DATABASE(),
        LockMode::LOCK_WHILE_RUNNING,
        std::to_string(INDEX_OF_LAST_TUTORIAL_CHECKPOINT+1)
    )
    , END_CHECKPOINT_MAINSTORY(
        "<b>End Checkpoint:</b>", //<br>Program will stop after completing this checkpoint.
        MAINSTORY_CHECKPOINTS_SELECT_DATABASE(),
        LockMode::UNLOCK_WHILE_RUNNING,
        std::to_string(ALL_CHECKPOINTS_SELECT_DATABASE().case_list().size()-1)
    ) 
    , SETUP_NOTE{
        "NOTE: Make sure you have selected the correct Start Point. "
        "Make sure your player character is in the exact correct start position for that Start Point, "
        "especially if your start point is NOT at the beginning of the Tutorial/Main Story. "
        "Read the Start Point's description to help with finding the correct start position. "
        "For Start Points that are at Pokecenters, ensure that you fly there so that your character is in the exactly correct start position."
    }    
    , MAINSTORY_NOTE{
        "Ensure you have a level 100 Gardevoir with the moves in the following order: Moonblast, Mystical Fire, Psychic, Misty Terrain.<br>"
        "Also, make sure you have two other strong pokemon (e.g. level 100 Talonflames)<br>"
        "Refer to the documentation on github for more details."
    }
    , START_DESCRIPTION(
        ""
    )
    , END_DESCRIPTION(
        ""
    ) 
    , START_CHECKPOINT_DESCRIPTION(
        ""
    )
    , END_CHECKPOINT_DESCRIPTION(
        ""
    )    
    , STARTERCHOICE(
        "<b>Starter " + STRING_POKEMON + ":",
        {
            {StarterChoice::SPRIGATITO,         "sprigatito",           "Sprigatito (Grass)"},
            {StarterChoice::FUECOCO,            "fuecoco",              "Fuecoco (Fire)"},
            {StarterChoice::QUAXLY,             "quaxly",               "Quaxly (Water)"},
        },
        LockMode::LOCK_WHILE_RUNNING,
        StarterChoice::FUECOCO
    )
    , ENABLE_ADVANCED_MODE(
        "<b>Advanced mode:</b><br>"
        "Select the start/end checkpoints instead of segments. i.e. finer control over start/end points.<br>"
        "Also, this enables the option to toggle 'Change settings at Program Start'.",
        LockMode::UNLOCK_WHILE_RUNNING,
        false
    ) 
    , CHANGE_SETTINGS(
        "<b>Pre-check: Update game settings:</b><br>"
        "This is to ensure the game has the correct settings, particularly with Autosave turned off, and Camera Support off.<br>"
        "WARNING: if you disable this, make sure you manually set the in-game settings as laid out in the wiki.",
        LockMode::UNLOCK_WHILE_RUNNING,
        true
    )
    , ENSURE_TIME_UNSYNCED(
        "<b>Pre-check: Ensure time unsynced:</b><br>"
        "This is to ensure the Switch has time unsynced from the internet, so it can be changed. This is run prior to the main story.",
        LockMode::UNLOCK_WHILE_RUNNING,
        true
    )
    , ENSURE_CORRECT_MOVES(
        "<b>Pre-check: Ensure correct moves:</b><br>"
        "This is to ensure the lead Gardevoir has the correct moves in the correct order: Moonblast, Mystical Fire, Psychic, Misty Terrain. This is run prior to the main story.",
        LockMode::UNLOCK_WHILE_RUNNING,
        true
    )
    , GO_HOME_WHEN_DONE(true)
    , NOTIFICATION_STATUS_UPDATE("Status Update", true, false, std::chrono::seconds(30))
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS_UPDATE,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_RECOVERABLE,
        &NOTIFICATION_ERROR_FATAL,
    })
    , m_advanced_options(
        "<font size=4><b>Advanced Options: (developer only)</b></font>"
    )
    , m_advanced_options_end(
        ""
    )    
    , ENABLE_TEST_CHECKPOINTS(
        "<b>TEST: test_checkpoints():</b>",
        LockMode::UNLOCK_WHILE_RUNNING,
        false
    )   
    , START_CHECKPOINT(
        "--Start checkpoint:<br>Start testing with this checkpoint number.",
        LockMode::UNLOCK_WHILE_RUNNING,
        0
    )    
    , END_CHECKPOINT(
        "--End checkpoint:<br>Stop testing when done this checkpoint number.",
        LockMode::UNLOCK_WHILE_RUNNING,
        11
    )     
    , LOOP_CHECKPOINT(
        "--Loop checkpoints:<br>Loop the checkpoints from \"Start loop\" to \"End loop\", inclusive. Loop these checkpoints this number of times.",
        LockMode::UNLOCK_WHILE_RUNNING,
        1, 1
    )
    , START_LOOP(
        "--Start loop:<br>Start looping with this checkpoint number.",
        LockMode::UNLOCK_WHILE_RUNNING,
        0
    )    
    , END_LOOP(
        "--End loop:<br>Stop looping when done this checkpoint number.",
        LockMode::UNLOCK_WHILE_RUNNING,
        11
    )      
    , ENABLE_TEST_REALIGN(
        "<b>TEST: realign_player():</b>",
        LockMode::UNLOCK_WHILE_RUNNING,
        false
    )   
    , REALIGN_MODE(
        "--REALIGN_MODE:",
        {
            {PlayerRealignMode::REALIGN_NEW_MARKER,            "realign_new",       "Realign New Marker"},
            {PlayerRealignMode::REALIGN_NO_MARKER,     "realign_no",     "Realign No Marker"},
            {PlayerRealignMode::REALIGN_OLD_MARKER,          "realign_old",     "Realign Old Marker"},
        },
        LockMode::UNLOCK_WHILE_RUNNING,
        PlayerRealignMode::REALIGN_NEW_MARKER
    )    
    , X_REALIGN(
        "--X_REALIGN:<br>x = 0 : left, x = 128 : neutral, x = 255 : right.",
        LockMode::UNLOCK_WHILE_RUNNING,
        128
    )    
    , Y_REALIGN(
        "--Y_REALIGN:<br>y = 0 : up, y = 128 : neutral, y = 255 : down.",
        LockMode::UNLOCK_WHILE_RUNNING,
        128
    )     
    , REALIGN_DURATION(
        "--REALIGN_DURATION",
        LockMode::UNLOCK_WHILE_RUNNING,
        0
    )
    , ENABLE_MISC_TEST(
        "<b>TEST: Miscellaneous test code:</b>",
        LockMode::UNLOCK_WHILE_RUNNING,
        false
    )       
    , FORWARD_TICKS(
        "--FORWARD_TICKS:",
        LockMode::UNLOCK_WHILE_RUNNING,
        0
    )   
    , TEST_PBF_LEFT_JOYSTICK(
        "<b>TEST: pbf_move_left_joystick():</b>",
        LockMode::UNLOCK_WHILE_RUNNING,
        false
    )     
    , X_MOVE(
        "--X_MOVE:<br>x = 0 : left, x = 128 : neutral, x = 255 : right.",
        LockMode::UNLOCK_WHILE_RUNNING,
        128
    )     
    , Y_MOVE(
        "--Y_MOVE:<br>y = 0 : up, y = 128 : neutral, y = 255 : down.",
        LockMode::UNLOCK_WHILE_RUNNING,
        128
    )   
    , HOLD_TICKS(
        "--HOLD_TICKS:",
        LockMode::UNLOCK_WHILE_RUNNING,
        0
    )    
    , RELEASE_TICKS(
        "--RELEASE_TICKS:",
        LockMode::UNLOCK_WHILE_RUNNING,
        0
    )
    , TEST_PBF_JOYSTICK2(
        "<b>TEST2: pbf_move_right_joystick():</b>",
        LockMode::UNLOCK_WHILE_RUNNING,
        false
    )     
    , X_MOVE2(
        "--X_MOVE:<br>x = 0 : left, x = 128 : neutral, x = 255 : right.",
        LockMode::UNLOCK_WHILE_RUNNING,
        128
    )     
    , Y_MOVE2(
        "--Y_MOVE:<br>y = 0 : up, y = 128 : neutral, y = 255 : down.",
        LockMode::UNLOCK_WHILE_RUNNING,
        128
    )   
    , HOLD_TICKS2(
        "--HOLD_TICKS:",
        LockMode::UNLOCK_WHILE_RUNNING,
        0
    )    
    , RELEASE_TICKS2(
        "--RELEASE_TICKS:",
        LockMode::UNLOCK_WHILE_RUNNING,
        0
    )    
    , TEST_CURRENT_DIRECTION(
        "<b>TEST: get_current_direction():</b>",
        LockMode::UNLOCK_WHILE_RUNNING,
        false
    ) 
    , TEST_CHANGE_DIRECTION(
        "<b>TEST: change_direction():</b>",
        LockMode::UNLOCK_WHILE_RUNNING,
        false
    )         
    , DIR_RADIANS(
        "direction in radians",
        LockMode::UNLOCK_WHILE_RUNNING,
        0
    )   
    , FLYPOINT_TYPE(
        "<b>Flypoint type:</b><br>"
        "For print_flypoint_location() and move_cursor_to_position_offset_from_flypoint()",
        {
            {FlyPoint::POKECENTER,         "pokecenter",           "Pokecenter"},
            {FlyPoint::FAST_TRAVEL,            "fast-travel",              "Fast Travel"},
        },
        LockMode::UNLOCK_WHILE_RUNNING,
        FlyPoint::POKECENTER
    )   
    , TEST_FLYPOINT_LOCATIONS(
        "<b>TEST: print_flypoint_location():</b>",
        LockMode::UNLOCK_WHILE_RUNNING,
        false
    ) 
    , TEST_MOVE_CURSOR_OFFSET_FROM_FLYPOINT(
        "<b>TEST: move_cursor_to_position_offset_from_flypoint():</b>",
        LockMode::UNLOCK_WHILE_RUNNING,
        false
    )
    , X_OFFSET(
        "X offset from flypoint",
        LockMode::UNLOCK_WHILE_RUNNING,
        0
    )
    , Y_OFFSET(
        "Y offset from flypoint",
        LockMode::UNLOCK_WHILE_RUNNING,
        0
    )
    , TEST_YOLO_BOX(
        "<b>TEST: get_yolo_box():</b>",
        LockMode::UNLOCK_WHILE_RUNNING,
        false
    )
    , YOLO_PATH(
        false,
        "<b>YOLO Path:</b>", 
        LockMode::LOCK_WHILE_RUNNING, 
        "PokemonSV/YOLO/A0-station-2.onnx",
        "<.onnx file>"
    )
    , TARGET_LABEL(
        false,
        "<b>YOLO Object Label:</b>", 
        LockMode::LOCK_WHILE_RUNNING, 
        "rock-1",
        "<target label>"
    )
{

    if (PreloadSettings::instance().DEVELOPER_MODE){
        PA_ADD_OPTION(m_advanced_options);

        PA_ADD_OPTION(TEST_YOLO_BOX);
        PA_ADD_OPTION(YOLO_PATH);
        PA_ADD_OPTION(TARGET_LABEL);

        PA_ADD_OPTION(FLYPOINT_TYPE);
        PA_ADD_OPTION(TEST_FLYPOINT_LOCATIONS);
        PA_ADD_OPTION(TEST_MOVE_CURSOR_OFFSET_FROM_FLYPOINT);
        PA_ADD_OPTION(X_OFFSET);
        PA_ADD_OPTION(Y_OFFSET);

        PA_ADD_OPTION(TEST_CURRENT_DIRECTION);
        PA_ADD_OPTION(TEST_CHANGE_DIRECTION);
        PA_ADD_OPTION(DIR_RADIANS);    

        PA_ADD_OPTION(TEST_PBF_LEFT_JOYSTICK);
        PA_ADD_OPTION(X_MOVE);
        PA_ADD_OPTION(Y_MOVE);
        PA_ADD_OPTION(HOLD_TICKS);
        PA_ADD_OPTION(RELEASE_TICKS);  

        PA_ADD_OPTION(TEST_PBF_JOYSTICK2);
        PA_ADD_OPTION(X_MOVE2);
        PA_ADD_OPTION(Y_MOVE2);
        PA_ADD_OPTION(HOLD_TICKS2);
        PA_ADD_OPTION(RELEASE_TICKS2);              

        PA_ADD_OPTION(ENABLE_TEST_CHECKPOINTS);
        PA_ADD_OPTION(START_CHECKPOINT);
        PA_ADD_OPTION(END_CHECKPOINT);
        PA_ADD_OPTION(LOOP_CHECKPOINT);
        PA_ADD_OPTION(START_LOOP);
        PA_ADD_OPTION(END_LOOP);

        // PA_ADD_OPTION(ENABLE_TEST_REALIGN);
        // PA_ADD_OPTION(REALIGN_MODE);
        // PA_ADD_OPTION(X_REALIGN);
        // PA_ADD_OPTION(Y_REALIGN);
        // PA_ADD_OPTION(REALIGN_DURATION);

        PA_ADD_OPTION(ENABLE_MISC_TEST);
        // PA_ADD_OPTION(FORWARD_TICKS);  
        PA_ADD_OPTION(m_advanced_options_end);
    }


    PA_ADD_OPTION(LANGUAGE);
    PA_ADD_OPTION(SETUP_NOTE);
    PA_ADD_OPTION(STORY_SECTION);

    PA_ADD_OPTION(STARTPOINT_TUTORIAL);
    PA_ADD_OPTION(MAINSTORY_NOTE);
    PA_ADD_OPTION(STARTPOINT_MAINSTORY);
    PA_ADD_OPTION(START_DESCRIPTION);
    PA_ADD_OPTION(ENDPOINT_TUTORIAL);
    PA_ADD_OPTION(ENDPOINT_MAINSTORY);
    PA_ADD_OPTION(END_DESCRIPTION);    

    PA_ADD_OPTION(START_CHECKPOINT_TUTORIAL);
    PA_ADD_OPTION(START_CHECKPOINT_MAINSTORY);
    PA_ADD_OPTION(START_CHECKPOINT_DESCRIPTION);
    PA_ADD_OPTION(END_CHECKPOINT_TUTORIAL);
    PA_ADD_OPTION(END_CHECKPOINT_MAINSTORY);
    PA_ADD_OPTION(END_CHECKPOINT_DESCRIPTION);

    PA_ADD_OPTION(STARTERCHOICE);

    PA_ADD_OPTION(GO_HOME_WHEN_DONE);

    PA_ADD_OPTION(ENABLE_ADVANCED_MODE);
    PA_ADD_OPTION(CHANGE_SETTINGS);
    PA_ADD_OPTION(ENSURE_TIME_UNSYNCED);
    PA_ADD_OPTION(ENSURE_CORRECT_MOVES);
    
    PA_ADD_OPTION(NOTIFICATIONS);


    AutoStory::on_config_value_changed(this);

    STORY_SECTION.add_listener(*this);
    STARTPOINT_TUTORIAL.add_listener(*this);
    ENDPOINT_TUTORIAL.add_listener(*this);
    STARTPOINT_MAINSTORY.add_listener(*this);
    ENDPOINT_MAINSTORY.add_listener(*this);    

    START_CHECKPOINT_TUTORIAL.add_listener(*this);
    END_CHECKPOINT_TUTORIAL.add_listener(*this);
    START_CHECKPOINT_MAINSTORY.add_listener(*this);
    END_CHECKPOINT_MAINSTORY.add_listener(*this);  

    ENABLE_ADVANCED_MODE.add_listener(*this); 

    ENABLE_TEST_CHECKPOINTS.add_listener(*this);
    ENABLE_TEST_REALIGN.add_listener(*this);
    ENABLE_MISC_TEST.add_listener(*this);
    TEST_PBF_LEFT_JOYSTICK.add_listener(*this);
    TEST_PBF_JOYSTICK2.add_listener(*this);
    TEST_CURRENT_DIRECTION.add_listener(*this);
    TEST_CHANGE_DIRECTION.add_listener(*this);
}

void AutoStory::on_config_value_changed(void* object){
    // ConfigOptionState state = (STARTPOINT_TUTORIAL.index() <= 1 && STORY_SECTION == StorySection::TUTORIAL)
    //     ? ConfigOptionState::ENABLED
    //     : ConfigOptionState::HIDDEN;
    // STARTERCHOICE.set_visibility(state);

    bool tutorial_segments = STORY_SECTION == StorySection::TUTORIAL && !ENABLE_ADVANCED_MODE;
    bool tutorial_checkpoints = STORY_SECTION == StorySection::TUTORIAL && ENABLE_ADVANCED_MODE;
    bool mainstory_segments = STORY_SECTION == StorySection::MAIN_STORY && !ENABLE_ADVANCED_MODE;
    bool mainstory_checkpoints = STORY_SECTION == StorySection::MAIN_STORY && ENABLE_ADVANCED_MODE;

    STARTPOINT_TUTORIAL.set_visibility(tutorial_segments ? ConfigOptionState::ENABLED : ConfigOptionState::HIDDEN);
    ENDPOINT_TUTORIAL.set_visibility(tutorial_segments ? ConfigOptionState::ENABLED : ConfigOptionState::HIDDEN);
    STARTPOINT_MAINSTORY.set_visibility(mainstory_segments ? ConfigOptionState::ENABLED : ConfigOptionState::HIDDEN);
    ENDPOINT_MAINSTORY.set_visibility(mainstory_segments ? ConfigOptionState::ENABLED : ConfigOptionState::HIDDEN);

    START_CHECKPOINT_TUTORIAL.set_visibility(tutorial_checkpoints ? ConfigOptionState::ENABLED : ConfigOptionState::HIDDEN);
    END_CHECKPOINT_TUTORIAL.set_visibility(tutorial_checkpoints ? ConfigOptionState::ENABLED : ConfigOptionState::HIDDEN);
    START_CHECKPOINT_MAINSTORY.set_visibility(mainstory_checkpoints ? ConfigOptionState::ENABLED : ConfigOptionState::HIDDEN);
    END_CHECKPOINT_MAINSTORY.set_visibility(mainstory_checkpoints ? ConfigOptionState::ENABLED : ConfigOptionState::HIDDEN);

    MAINSTORY_NOTE.set_visibility(STORY_SECTION == StorySection::TUTORIAL ? ConfigOptionState::HIDDEN : ConfigOptionState::ENABLED);
    START_DESCRIPTION.set_text(start_segment_description());
    END_DESCRIPTION.set_text(end_segment_description());
    START_CHECKPOINT_DESCRIPTION.set_text(start_checkpoint_description());
    END_CHECKPOINT_DESCRIPTION.set_text(end_checkpoint_description());

    START_DESCRIPTION.set_visibility(!ENABLE_ADVANCED_MODE ? ConfigOptionState::ENABLED : ConfigOptionState::HIDDEN);
    END_DESCRIPTION.set_visibility(!ENABLE_ADVANCED_MODE ? ConfigOptionState::ENABLED : ConfigOptionState::HIDDEN);
    START_CHECKPOINT_DESCRIPTION.set_visibility(ENABLE_ADVANCED_MODE ? ConfigOptionState::ENABLED : ConfigOptionState::HIDDEN);
    END_CHECKPOINT_DESCRIPTION.set_visibility(ENABLE_ADVANCED_MODE ? ConfigOptionState::ENABLED : ConfigOptionState::HIDDEN);
    CHANGE_SETTINGS.set_visibility(ENABLE_ADVANCED_MODE ? ConfigOptionState::ENABLED : ConfigOptionState::HIDDEN);

    if (ENABLE_TEST_CHECKPOINTS){
        START_CHECKPOINT.set_visibility(ConfigOptionState::ENABLED);
        END_CHECKPOINT.set_visibility(ConfigOptionState::ENABLED);
        LOOP_CHECKPOINT.set_visibility(ConfigOptionState::ENABLED);
        START_LOOP.set_visibility(ConfigOptionState::ENABLED);
        END_LOOP.set_visibility(ConfigOptionState::ENABLED);
    }else{
        START_CHECKPOINT.set_visibility(ConfigOptionState::DISABLED);
        END_CHECKPOINT.set_visibility(ConfigOptionState::DISABLED);
        LOOP_CHECKPOINT.set_visibility(ConfigOptionState::DISABLED);
        START_LOOP.set_visibility(ConfigOptionState::DISABLED);
        END_LOOP.set_visibility(ConfigOptionState::DISABLED);
    }

    if (ENABLE_TEST_REALIGN){
        REALIGN_MODE.set_visibility(ConfigOptionState::ENABLED);
        X_REALIGN.set_visibility(ConfigOptionState::ENABLED);
        Y_REALIGN.set_visibility(ConfigOptionState::ENABLED);
        REALIGN_DURATION.set_visibility(ConfigOptionState::ENABLED);
    }else{
        REALIGN_MODE.set_visibility(ConfigOptionState::DISABLED);
        X_REALIGN.set_visibility(ConfigOptionState::DISABLED);
        Y_REALIGN.set_visibility(ConfigOptionState::DISABLED);
        REALIGN_DURATION.set_visibility(ConfigOptionState::DISABLED);        
    }

    if (ENABLE_MISC_TEST){
        FORWARD_TICKS.set_visibility(ConfigOptionState::ENABLED);
    }else{
        FORWARD_TICKS.set_visibility(ConfigOptionState::DISABLED);
    }

    if (TEST_PBF_LEFT_JOYSTICK){
        X_MOVE.set_visibility(ConfigOptionState::ENABLED);
        Y_MOVE.set_visibility(ConfigOptionState::ENABLED);
        HOLD_TICKS.set_visibility(ConfigOptionState::ENABLED);
        RELEASE_TICKS.set_visibility(ConfigOptionState::ENABLED);
    }else{
        X_MOVE.set_visibility(ConfigOptionState::DISABLED);
        Y_MOVE.set_visibility(ConfigOptionState::DISABLED);
        HOLD_TICKS.set_visibility(ConfigOptionState::DISABLED);
        RELEASE_TICKS.set_visibility(ConfigOptionState::DISABLED);      
    }   

    if (TEST_PBF_JOYSTICK2){
        X_MOVE2.set_visibility(ConfigOptionState::ENABLED);
        Y_MOVE2.set_visibility(ConfigOptionState::ENABLED);
        HOLD_TICKS2.set_visibility(ConfigOptionState::ENABLED);
        RELEASE_TICKS2.set_visibility(ConfigOptionState::ENABLED);
    }else{
        X_MOVE2.set_visibility(ConfigOptionState::DISABLED);
        Y_MOVE2.set_visibility(ConfigOptionState::DISABLED);
        HOLD_TICKS2.set_visibility(ConfigOptionState::DISABLED);
        RELEASE_TICKS2.set_visibility(ConfigOptionState::DISABLED);      
    }     


    if (TEST_CHANGE_DIRECTION){
        DIR_RADIANS.set_visibility(ConfigOptionState::ENABLED);
    }else{
        DIR_RADIANS.set_visibility(ConfigOptionState::DISABLED);  
    }        


}






void AutoStory::test_checkpoints(
    SingleSwitchProgramEnvironment& env,
    VideoStream& stream,
    ProControllerContext& context,
    int start, int end, 
    int loop, int start_loop, int end_loop
){
    AutoStoryStats& stats = env.current_stats<AutoStoryStats>();
    EventNotificationOption& notif_status_update = NOTIFICATION_STATUS_UPDATE;
    Language language = LANGUAGE;
    StarterChoice starter_choice = STARTERCHOICE;
    std::vector<std::function<void()>> checkpoint_list;
    checkpoint_list.push_back([&](){checkpoint_00(env, context);});
    checkpoint_list.push_back([&](){checkpoint_01(env, context, notif_status_update, stats, language);});
    checkpoint_list.push_back([&](){checkpoint_02(env, context, notif_status_update, stats);});
    checkpoint_list.push_back([&](){checkpoint_03(env, context, notif_status_update, stats, language, starter_choice);});
    checkpoint_list.push_back([&](){checkpoint_04(env, context, notif_status_update, stats);});
    checkpoint_list.push_back([&](){checkpoint_05(env, context, notif_status_update, stats);});
    checkpoint_list.push_back([&](){checkpoint_06(env, context, notif_status_update, stats);});
    checkpoint_list.push_back([&](){checkpoint_07(env, context, notif_status_update, stats);});
    checkpoint_list.push_back([&](){checkpoint_08(env, context, notif_status_update, stats);});
    checkpoint_list.push_back([&](){checkpoint_09(env, context, notif_status_update, stats);});
    checkpoint_list.push_back([&](){checkpoint_10(env, context, notif_status_update, stats);});
    checkpoint_list.push_back([&](){checkpoint_11(env, context, notif_status_update, stats);});
    checkpoint_list.push_back([&](){checkpoint_12(env, context, notif_status_update, stats);});
    checkpoint_list.push_back([&](){checkpoint_13(env, context, notif_status_update, stats);});
    checkpoint_list.push_back([&](){checkpoint_14(env, context, notif_status_update, stats);});
    checkpoint_list.push_back([&](){checkpoint_15(env, context, notif_status_update, stats);});
    checkpoint_list.push_back([&](){checkpoint_16(env, context, notif_status_update, stats);});
    checkpoint_list.push_back([&](){checkpoint_17(env, context, notif_status_update, stats);});
    checkpoint_list.push_back([&](){checkpoint_18(env, context, notif_status_update, stats);});
    checkpoint_list.push_back([&](){checkpoint_19(env, context, notif_status_update, stats);});
    checkpoint_list.push_back([&](){checkpoint_20(env, context, notif_status_update, stats);});
    checkpoint_list.push_back([&](){checkpoint_21(env, context, notif_status_update, stats);});
    checkpoint_list.push_back([&](){checkpoint_22(env, context, notif_status_update, stats);});
    checkpoint_list.push_back([&](){checkpoint_23(env, context, notif_status_update, stats);});
    checkpoint_list.push_back([&](){checkpoint_24(env, context, notif_status_update, stats);});
    checkpoint_list.push_back([&](){checkpoint_25(env, context, notif_status_update, stats);});
    checkpoint_list.push_back([&](){checkpoint_26(env, context, notif_status_update, stats);});
    checkpoint_list.push_back([&](){checkpoint_27(env, context, notif_status_update, stats);});
    checkpoint_list.push_back([&](){checkpoint_28(env, context, notif_status_update, stats);});
    checkpoint_list.push_back([&](){checkpoint_29(env, context, notif_status_update, stats);});
    checkpoint_list.push_back([&](){checkpoint_30(env, context, notif_status_update, stats);});
    checkpoint_list.push_back([&](){checkpoint_31(env, context, notif_status_update, stats);});
    checkpoint_list.push_back([&](){checkpoint_32(env, context, notif_status_update, stats);});
    checkpoint_list.push_back([&](){checkpoint_33(env, context, notif_status_update, stats);});
    checkpoint_list.push_back([&](){checkpoint_34(env, context, notif_status_update, stats);});
    checkpoint_list.push_back([&](){checkpoint_35(env, context, notif_status_update, stats);});
    checkpoint_list.push_back([&](){checkpoint_36(env, context, notif_status_update, stats);});
    checkpoint_list.push_back([&](){checkpoint_37(env, context, notif_status_update, stats);});
    checkpoint_list.push_back([&](){checkpoint_38(env, context, notif_status_update, stats);});
    checkpoint_list.push_back([&](){checkpoint_39(env, context, notif_status_update, stats);});
    checkpoint_list.push_back([&](){checkpoint_40(env, context, notif_status_update, stats);});
    checkpoint_list.push_back([&](){checkpoint_41(env, context, notif_status_update, stats);});
    checkpoint_list.push_back([&](){checkpoint_42(env, context, notif_status_update, stats);});
    checkpoint_list.push_back([&](){checkpoint_43(env, context, notif_status_update, stats);});
    checkpoint_list.push_back([&](){checkpoint_44(env, context, notif_status_update, stats);});
    checkpoint_list.push_back([&](){checkpoint_45(env, context, notif_status_update, stats);});
    checkpoint_list.push_back([&](){checkpoint_46(env, context, notif_status_update, stats);});
    checkpoint_list.push_back([&](){checkpoint_47(env, context, notif_status_update, stats);});
    checkpoint_list.push_back([&](){checkpoint_48(env, context, notif_status_update, stats);});
    checkpoint_list.push_back([&](){checkpoint_49(env, context, notif_status_update, stats);});
    checkpoint_list.push_back([&](){checkpoint_50(env, context, notif_status_update, stats);});
    checkpoint_list.push_back([&](){checkpoint_51(env, context, notif_status_update, stats);});
    checkpoint_list.push_back([&](){checkpoint_52(env, context, notif_status_update, stats);});
    checkpoint_list.push_back([&](){checkpoint_53(env, context, notif_status_update, stats);});
    checkpoint_list.push_back([&](){checkpoint_54(env, context, notif_status_update, stats);});
    checkpoint_list.push_back([&](){checkpoint_55(env, context, notif_status_update, stats);});
    checkpoint_list.push_back([&](){checkpoint_56(env, context, notif_status_update, stats);});
    checkpoint_list.push_back([&](){checkpoint_57(env, context, notif_status_update, stats);});
    checkpoint_list.push_back([&](){checkpoint_58(env, context, notif_status_update, stats);});
    checkpoint_list.push_back([&](){checkpoint_59(env, context, notif_status_update, stats);});
    checkpoint_list.push_back([&](){checkpoint_60(env, context, notif_status_update, stats);});
    checkpoint_list.push_back([&](){checkpoint_61(env, context, notif_status_update, stats);});
    checkpoint_list.push_back([&](){checkpoint_62(env, context, notif_status_update, stats);});
    checkpoint_list.push_back([&](){checkpoint_63(env, context, notif_status_update, stats);});
    checkpoint_list.push_back([&](){checkpoint_64(env, context, notif_status_update, stats);});
    checkpoint_list.push_back([&](){checkpoint_65(env, context, notif_status_update, stats);});
    checkpoint_list.push_back([&](){checkpoint_66(env, context, notif_status_update, stats);});
    checkpoint_list.push_back([&](){checkpoint_67(env, context, notif_status_update, stats);});
    checkpoint_list.push_back([&](){checkpoint_68(env, context, notif_status_update, stats);});
    checkpoint_list.push_back([&](){checkpoint_69(env, context, notif_status_update, stats);});
    checkpoint_list.push_back([&](){checkpoint_70(env, context, notif_status_update, stats);});
    checkpoint_list.push_back([&](){checkpoint_71(env, context, notif_status_update, stats);});
    checkpoint_list.push_back([&](){checkpoint_72(env, context, notif_status_update, stats);});
    checkpoint_list.push_back([&](){checkpoint_73(env, context, notif_status_update, stats);});
    checkpoint_list.push_back([&](){checkpoint_74(env, context, notif_status_update, stats);});
    checkpoint_list.push_back([&](){checkpoint_75(env, context, notif_status_update, stats);});
    checkpoint_list.push_back([&](){checkpoint_76(env, context, notif_status_update, stats);});
    checkpoint_list.push_back([&](){checkpoint_77(env, context, notif_status_update, stats);});
    checkpoint_list.push_back([&](){checkpoint_78(env, context, notif_status_update, stats);});
    checkpoint_list.push_back([&](){checkpoint_79(env, context, notif_status_update, stats);});
    checkpoint_list.push_back([&](){checkpoint_80(env, context, notif_status_update, stats);});
    checkpoint_list.push_back([&](){checkpoint_81(env, context, notif_status_update, stats);});
    checkpoint_list.push_back([&](){checkpoint_82(env, context, notif_status_update, stats);});
    checkpoint_list.push_back([&](){checkpoint_83(env, context, notif_status_update, stats);});
    checkpoint_list.push_back([&](){checkpoint_84(env, context, notif_status_update, stats);});
    checkpoint_list.push_back([&](){checkpoint_85(env, context, notif_status_update, stats);});
    checkpoint_list.push_back([&](){checkpoint_86(env, context, notif_status_update, stats);});
    checkpoint_list.push_back([&](){checkpoint_87(env, context, notif_status_update, stats, language, starter_choice);});
    checkpoint_list.push_back([&](){checkpoint_88(env, context, notif_status_update, stats);});
    checkpoint_list.push_back([&](){checkpoint_89(env, context, notif_status_update, stats);});
    checkpoint_list.push_back([&](){checkpoint_90(env, context, notif_status_update, stats);});
    checkpoint_list.push_back([&](){checkpoint_91(env, context, notif_status_update, stats);});
    checkpoint_list.push_back([&](){checkpoint_92(env, context, notif_status_update, stats);});
    checkpoint_list.push_back([&](){checkpoint_93(env, context, notif_status_update, stats);});
    checkpoint_list.push_back([&](){checkpoint_94(env, context, notif_status_update, stats);});
    checkpoint_list.push_back([&](){checkpoint_95(env, context, notif_status_update, stats);});
    checkpoint_list.push_back([&](){checkpoint_96(env, context, notif_status_update, stats);});
    checkpoint_list.push_back([&](){checkpoint_97(env, context, notif_status_update, stats);});
    checkpoint_list.push_back([&](){checkpoint_98(env, context, notif_status_update, stats);});
    checkpoint_list.push_back([&](){checkpoint_99(env, context, notif_status_update, stats);});
    checkpoint_list.push_back([&](){checkpoint_100(env, context, notif_status_update, stats);});
    checkpoint_list.push_back([&](){checkpoint_101(env, context, notif_status_update, stats);});
    // checkpoint_list.push_back([&](){checkpoint_102(env, context, notif_status_update, stats);});
    // checkpoint_list.push_back([&](){checkpoint_103(env, context, notif_status_update, stats);});
    // checkpoint_list.push_back([&](){checkpoint_104(env, context, notif_status_update, stats);});
    // checkpoint_list.push_back([&](){checkpoint_105(env, context, notif_status_update, stats);});
    
    
    if (end == 0){
        end = start;
    }
    stream.log("test_checkpoints: start: " + std::to_string(start) + ", end:" + std::to_string(end));
    for (int checkpoint = start; checkpoint <= end; checkpoint++){
        if (checkpoint == 0){
            stream.log("checkpoint_0");
            checkpoint_list[checkpoint]();
            continue;
        }
        bool has_minimap = true;
        if (checkpoint < 3){
            has_minimap = false;
        }
        
        const size_t DIGITS = 3;
        std::string number = std::to_string(checkpoint);
        if (number.size() < DIGITS){
            number = std::string(DIGITS - number.size(), '0') + number;
        }        
        if (checkpoint >= start_loop && checkpoint <= end_loop){
            for (int i = 0; i < loop; i++){
                if (i > 0){
                    try {
                        reset_game(env.program_info(), env.console, context);
                        enter_menu_from_overworld(env.program_info(), env.console, context, -1, MenuSide::NONE, has_minimap);
                        // we wait 5 seconds then save, so that the initial conditions are slightly different on each reset.
                        env.log("Wait 5 seconds.");
                        context.wait_for(Milliseconds(5 * 1000));
                    }catch(...){
                        // try one more time
                        reset_game(env.program_info(), env.console, context);
                        enter_menu_from_overworld(env.program_info(), env.console, context, -1, MenuSide::NONE, has_minimap);
                        // we wait 5 seconds then save, so that the initial conditions are slightly different on each reset.
                        env.log("Wait 5 seconds.");
                        context.wait_for(Milliseconds(5 * 1000));                        

                    }
                }
                stream.log("checkpoint_" + number + ": loop " + std::to_string(i));
                checkpoint_list[checkpoint]();
            } 
        }else{
            stream.log("checkpoint_" + number + ".");
            checkpoint_list[checkpoint]();            
        }
       
    }
    
}

std::string AutoStory::start_segment_description(){
    size_t segment_index = 0;
    if (STORY_SECTION == StorySection::TUTORIAL){
        segment_index = STARTPOINT_TUTORIAL.index();
    }else if (STORY_SECTION == StorySection::MAIN_STORY){
        segment_index = STARTPOINT_MAINSTORY.index() + (INDEX_OF_LAST_TUTORIAL_SEGMENT + 1);
    }
    return "    " + ALL_AUTO_STORY_SEGMENT_LIST()[segment_index]->start_text();
}

std::string AutoStory::end_segment_description(){
    size_t segment_index = 0;
    if (STORY_SECTION == StorySection::TUTORIAL){
        segment_index = ENDPOINT_TUTORIAL.index();
    }else if (STORY_SECTION == StorySection::MAIN_STORY){
        segment_index = ENDPOINT_MAINSTORY.index() + (INDEX_OF_LAST_TUTORIAL_SEGMENT + 1);
    }    
    return "    " + ALL_AUTO_STORY_SEGMENT_LIST()[segment_index]->end_text();
}

size_t AutoStory::get_start_segment_index(){
    size_t start = 0;

    if (STORY_SECTION == StorySection::TUTORIAL){
        start = STARTPOINT_TUTORIAL.index();
    }else if (STORY_SECTION == StorySection::MAIN_STORY){
        start = (INDEX_OF_LAST_TUTORIAL_SEGMENT + 1) + STARTPOINT_MAINSTORY.index();
    }
    
    return start;
}

size_t AutoStory::get_end_segment_index(){
    size_t end = 0;

    if (STORY_SECTION == StorySection::TUTORIAL){
        end = ENDPOINT_TUTORIAL.index();
    }else if (STORY_SECTION == StorySection::MAIN_STORY){
        end = (INDEX_OF_LAST_TUTORIAL_SEGMENT + 1) + ENDPOINT_MAINSTORY.index();     
    }
    
    return end;
}

std::string AutoStory::start_checkpoint_description(){
    size_t checkpoint_index = 0;
    if (STORY_SECTION == StorySection::TUTORIAL){
        checkpoint_index = START_CHECKPOINT_TUTORIAL.index();
    }else if (STORY_SECTION == StorySection::MAIN_STORY){
        checkpoint_index = START_CHECKPOINT_MAINSTORY.index() + (INDEX_OF_LAST_TUTORIAL_CHECKPOINT + 1);
    }
    return "    Start: " + ALL_AUTO_STORY_CHECKPOINT_LIST()[checkpoint_index]->start_text();
}

std::string AutoStory::end_checkpoint_description(){
    size_t checkpoint_index = 0;
    if (STORY_SECTION == StorySection::TUTORIAL){
        checkpoint_index = END_CHECKPOINT_TUTORIAL.index();
    }else if (STORY_SECTION == StorySection::MAIN_STORY){
        checkpoint_index = END_CHECKPOINT_MAINSTORY.index() + (INDEX_OF_LAST_TUTORIAL_CHECKPOINT + 1);
    }    
    return "    End: " + ALL_AUTO_STORY_CHECKPOINT_LIST()[checkpoint_index]->end_text();
}

size_t AutoStory::get_start_checkpoint_index(){
    size_t start = 0;

    if (STORY_SECTION == StorySection::TUTORIAL){
        start = START_CHECKPOINT_TUTORIAL.index();
    }else if (STORY_SECTION == StorySection::MAIN_STORY){
        start = (INDEX_OF_LAST_TUTORIAL_CHECKPOINT + 1) + START_CHECKPOINT_MAINSTORY.index();
    }
    
    return start;
}

size_t AutoStory::get_end_checkpoint_index(){
    size_t end = 0;

    if (STORY_SECTION == StorySection::TUTORIAL){
        end = END_CHECKPOINT_TUTORIAL.index();
    }else if (STORY_SECTION == StorySection::MAIN_STORY){
        end = (INDEX_OF_LAST_TUTORIAL_CHECKPOINT + 1) + END_CHECKPOINT_MAINSTORY.index();     
    }
    
    return end;
}


void AutoStory::run_autostory(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    AutoStoryOptions options{
        LANGUAGE,
        STARTERCHOICE,
        NOTIFICATION_STATUS_UPDATE
    };    

    AutoStoryStats& stats = env.current_stats<AutoStoryStats>();
    context.wait_for_all_requests();

    if (ENABLE_ADVANCED_MODE){
        for (size_t checkpoint_index = get_start_checkpoint_index(); checkpoint_index <= get_end_checkpoint_index(); checkpoint_index++){
            env.console.log("Start Checkpoint " + ALL_AUTO_STORY_CHECKPOINT_LIST()[checkpoint_index]->name(), COLOR_ORANGE);
            ALL_AUTO_STORY_CHECKPOINT_LIST()[checkpoint_index]->run_checkpoint(env, context, options, stats);
        }

    }else{
        for (size_t segment_index = get_start_segment_index(); segment_index <= get_end_segment_index(); segment_index++){
            ALL_AUTO_STORY_SEGMENT_LIST()[segment_index]->run_segment(env, context, options, stats);
        }

    }
}

void AutoStory::test_code(SingleSwitchProgramEnvironment& env, ProControllerContext& context){


    if (TEST_YOLO_BOX){
        VideoOverlaySet overlays(env.console.overlay());
        YOLOv5Detector yolo_detector(RESOURCE_PATH() + std::string(YOLO_PATH));
        
        // ImageFloatBox target_box = 
        get_yolo_box(env, context, overlays, yolo_detector, TARGET_LABEL);

        context.wait_for(Milliseconds(1000));
        return;
    }


    if (TEST_FLYPOINT_LOCATIONS){
        print_flypoint_location(env.program_info(), env.console, context, FLYPOINT_TYPE);
        // print_flypoint_location(env.program_info(), env.console, context, FlyPoint::FAST_TRAVEL);
        return;
    }

    if (TEST_MOVE_CURSOR_OFFSET_FROM_FLYPOINT){
        move_cursor_to_position_offset_from_flypoint(env.program_info(), env.console, context, FLYPOINT_TYPE, {X_OFFSET, Y_OFFSET});

        return;
    }

    if (TEST_CURRENT_DIRECTION){
        DirectionDetector direction;
        // direction.change_direction(env.program_info(), env.console, context, DIR_RADIANS);
        VideoSnapshot snapshot = env.console.video().snapshot();
        env.console.log("current direction: " + std::to_string(direction.get_current_direction(env.console, snapshot)));
        return;
    }

    if (TEST_CHANGE_DIRECTION){
        DirectionDetector direction;
        direction.change_direction(env.program_info(), env.console, context, DIR_RADIANS);
        // VideoSnapshot snapshot = env.console.video().snapshot();
        // env.console.log("current direction: " + std::to_string(direction.get_current_direction(env.console, snapshot)));
        return;
    }    

    if (TEST_PBF_LEFT_JOYSTICK){
        pbf_move_left_joystick(context, X_MOVE, Y_MOVE, HOLD_TICKS, RELEASE_TICKS);
        return;
    } 

    if (TEST_PBF_JOYSTICK2){
        pbf_move_right_joystick(context, X_MOVE2, Y_MOVE2, HOLD_TICKS2, RELEASE_TICKS2);
        return;
    }            

    if (ENABLE_TEST_CHECKPOINTS){
        // test individual checkpoints
        test_checkpoints(env, env.console, context, START_CHECKPOINT, END_CHECKPOINT, LOOP_CHECKPOINT, START_LOOP, END_LOOP);
        GO_HOME_WHEN_DONE.run_end_of_program(context);
        return;
    }
    

    if (ENABLE_TEST_REALIGN){
        // clear realign marker
        // realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 128, 128, 0);
        realign_player(env.program_info(), env.console, context, REALIGN_MODE, X_REALIGN, Y_REALIGN, REALIGN_DURATION);
        return;
    }

    if (ENABLE_MISC_TEST){
        // walk_forward_while_clear_front_path(env.program_info(), env.console, context, FORWARD_TICKS);

        // overworld_navigation(env.program_info(), env.console, context, 
        //     NavigationStopCondition::STOP_MARKER, NavigationMovementMode::CLEAR_WITH_LETS_GO, 
        //     128, 0, 60, 10, false);

        DirectionDetector direction;


        YOLOv5Detector yolo_detector(RESOURCE_PATH() + "PokemonSV/YOLO/A0-station-2.onnx");
        // move_camera_yolo(env, context, CameraAxis::Y, yolo_detector, "tree-tera", 0.294444);
        // move_camera_yolo(env, context, CameraAxis::X, yolo_detector, "tree-tera", 0.604688);



        return;
    }

    // context.wait_for(Milliseconds(1000000));


}

void AutoStory::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    StartProgramChecks::check_performance_class_wired_or_wireless(context);
    assert_16_9_720p_min(env.logger(), env.console);


    // test code
    if (TEST_FLYPOINT_LOCATIONS || TEST_MOVE_CURSOR_OFFSET_FROM_FLYPOINT || ENABLE_TEST_CHECKPOINTS || ENABLE_TEST_REALIGN || ENABLE_MISC_TEST || TEST_PBF_LEFT_JOYSTICK || TEST_PBF_JOYSTICK2 || TEST_CHANGE_DIRECTION || TEST_CURRENT_DIRECTION){
        test_code(env, context);
        return;
    }

    if (ENABLE_ADVANCED_MODE){
        if (get_start_checkpoint_index() > get_end_checkpoint_index()){
            throw UserSetupError(env.logger(), "The start checkpoint cannot be later than the end segment.");
        }

    }else{
        if (get_start_segment_index() > get_end_segment_index()){
            throw UserSetupError(env.logger(), "The start segment cannot be later than the end segment.");
        }
    }

    // Connect controller
    pbf_press_button(context, BUTTON_L, 20, 20);

    if (ENABLE_ADVANCED_MODE){
        env.console.log("Start Checkpoint " + ALL_AUTO_STORY_CHECKPOINT_LIST()[get_start_checkpoint_index()]->name(), COLOR_ORANGE);        
    }else{
        env.console.log("Start Segment " + ALL_AUTO_STORY_SEGMENT_LIST()[get_start_segment_index()]->name(), COLOR_ORANGE);
    }

    // Set settings. to ensure autosave is off.
    if (CHANGE_SETTINGS){
        if (ENABLE_ADVANCED_MODE){
            change_settings_prior_to_autostory_checkpoint_mode(env, context, get_start_checkpoint_index(), LANGUAGE);
        }else{
            change_settings_prior_to_autostory_segment_mode(env, context, get_start_segment_index(), LANGUAGE);
        }
    }

    if (ENSURE_TIME_UNSYNCED && STORY_SECTION == StorySection::MAIN_STORY){
        env.console.log("Ensure time is not synchronized to the internet.");
        pbf_press_button(context, BUTTON_HOME, 160ms, GameSettings::instance().GAME_TO_HOME_DELAY1);
        ensure_time_unsynced(env, context);
        go_home(env.console, context);
        resume_game_from_home(env.console, context, true);
    }

    if (ENSURE_CORRECT_MOVES && STORY_SECTION == StorySection::MAIN_STORY){
        env.console.log("Ensure lead Gardevoir has the correct moves.");
        confirm_lead_pokemon_moves(env, context, LANGUAGE);
    }

    run_autostory(env, context);
    
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
    GO_HOME_WHEN_DONE.run_end_of_program(context);
}






}
}
}

