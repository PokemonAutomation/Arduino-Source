/*  AutoStory
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/Exceptions/FatalProgramException.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/Tools/StatsTracking.h"
#include "CommonFramework/Tools/VideoResolutionCheck.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSwSh/Inference/PokemonSwSh_IvJudgeReader.h"
#include "PokemonSV/Programs/PokemonSV_GameEntry.h"
#include "PokemonSV_AutoStory_Segment_00.h"
// #include "PokemonSV_AutoStory_Segment_01.h"
// #include "PokemonSV_AutoStory_Segment_02.h"
// #include "PokemonSV_AutoStory_Segment_03.h"
// #include "PokemonSV_AutoStory_Segment_04.h"
// #include "PokemonSV_AutoStory_Segment_05.h"
// #include "PokemonSV_AutoStory_Segment_06.h"
// #include "PokemonSV_AutoStory_Segment_07.h"
// #include "PokemonSV_AutoStory_Segment_08.h"
// #include "PokemonSV_AutoStory_Segment_09.h"
// #include "PokemonSV_AutoStory_Segment_10.h"
// #include "PokemonSV_AutoStory_Segment_11.h"
// #include "PokemonSV_AutoStory_Segment_12.h"
// #include "PokemonSV_AutoStory_Segment_13.h"
// #include "PokemonSV_AutoStory_Segment_14.h"
// #include "PokemonSV_AutoStory_Segment_15.h"
#include "PokemonSV_AutoStory.h"

//#include <iostream>
//using std::cout;
//using std::endl;
//#include <unordered_map>
//#include <algorithm>

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

using namespace Pokemon;


std::vector<std::unique_ptr<AutoStory_Segment>> make_autoStory_segment_list(){
    std::vector<std::unique_ptr<AutoStory_Segment>> segment_list;
    segment_list.emplace_back(std::make_unique<AutoStory_Segment_00>());
    // segment_list.emplace_back(std::make_unique<AutoStory_Segment_01>());
    // segment_list.emplace_back(std::make_unique<AutoStory_Segment_02>());
    // segment_list.emplace_back(std::make_unique<AutoStory_Segment_03>());
    // segment_list.emplace_back(std::make_unique<AutoStory_Segment_04>());
    // segment_list.emplace_back(std::make_unique<AutoStory_Segment_05>());
    // segment_list.emplace_back(std::make_unique<AutoStory_Segment_06>());
    // segment_list.emplace_back(std::make_unique<AutoStory_Segment_07>());
    // segment_list.emplace_back(std::make_unique<AutoStory_Segment_08>());
    // segment_list.emplace_back(std::make_unique<AutoStory_Segment_09>());
    // segment_list.emplace_back(std::make_unique<AutoStory_Segment_10>());
    // segment_list.emplace_back(std::make_unique<AutoStory_Segment_11>());
    // segment_list.emplace_back(std::make_unique<AutoStory_Segment_12>());
    // segment_list.emplace_back(std::make_unique<AutoStory_Segment_13>());
    // segment_list.emplace_back(std::make_unique<AutoStory_Segment_14>());

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
    size_t end = all_segments.case_list().size(); // 10. size() is the placeholder value. will be 10 when rest of segments merged.
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
    size_t start = 0; // 10. 0 is the placeholder value. will be 10 when rest of segments merged.
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


AutoStory_Descriptor::AutoStory_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSV:AutoStory",
        STRING_POKEMON + " SV", "Auto Story",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSV/AutoStory.md",
        "Progress through the mainstory of SV.",
        FeedbackType::VIDEO_AUDIO,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}

std::unique_ptr<StatsTracker> AutoStory_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new AutoStoryStats());
}



AutoStory::~AutoStory(){
    STARTPOINT_TUTORIAL.remove_listener(*this);
    ENDPOINT_TUTORIAL.remove_listener(*this);
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
        "<b>Start Point:</b><br>Program will start with this segment.",
        TUTORIAL_SEGMENTS_SELECT_DATABASE(),
        LockMode::UNLOCK_WHILE_RUNNING,
        "0"
    )
    , ENDPOINT_TUTORIAL(
        "<b>End Point:</b><br>Program will stop after completing this segment.",
        TUTORIAL_SEGMENTS_SELECT_DATABASE(),
        LockMode::UNLOCK_WHILE_RUNNING,
        "0"  //"9"
    )   
    // , STARTPOINT_MAINSTORY(
    //     "<b>Start Point:</b><br>Program will start with this segment.",
    //     MAINSTORY_SEGMENTS_SELECT_DATABASE(),
    //     LockMode::UNLOCK_WHILE_RUNNING,
    //     "10"
    // )
    // , ENDPOINT_MAINSTORY(
    //     "<b>End Point:</b><br>Program will stop after completing this segment.",
    //     MAINSTORY_SEGMENTS_SELECT_DATABASE(),
    //     LockMode::UNLOCK_WHILE_RUNNING,
    //     "10"
    // )       
    , MAINSTORY_NOTE{
        "Ensure you have a level 100 Gardevoir with the moves in the following order: Moonblast, Dazzling Gleam, Psychic, Mystical Fire.<br>"
        "Refer to the documentation on github for more details."
    }
    , START_DESCRIPTION(
        ""
    )
    , END_DESCRIPTION(
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
    , CHANGE_SETTINGS(
        "<b>Change settings at Program Start:</b><br>"
        "This is to ensure the program has the correct settings, particularly with Autosave turned off.",
        LockMode::UNLOCK_WHILE_RUNNING,
        true
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
    , ENABLE_TEST_OVERWORLD_MOVE(
        "<b>TEST: walk_forward_while_clear_front_path():</b>",
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
{
    PA_ADD_OPTION(LANGUAGE);
    PA_ADD_OPTION(STORY_SECTION);
    PA_ADD_OPTION(STARTPOINT_TUTORIAL);
    PA_ADD_OPTION(MAINSTORY_NOTE);
    // PA_ADD_OPTION(STARTPOINT_MAINSTORY);
    PA_ADD_OPTION(START_DESCRIPTION);
    PA_ADD_OPTION(ENDPOINT_TUTORIAL);
    // PA_ADD_OPTION(ENDPOINT_MAINSTORY);
    PA_ADD_OPTION(END_DESCRIPTION);
    PA_ADD_OPTION(STARTERCHOICE);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(NOTIFICATIONS);

    if (PreloadSettings::instance().DEVELOPER_MODE){
        PA_ADD_OPTION(m_advanced_options);
        PA_ADD_OPTION(CHANGE_SETTINGS);
        PA_ADD_OPTION(ENABLE_TEST_CHECKPOINTS);
        PA_ADD_OPTION(START_CHECKPOINT);
        PA_ADD_OPTION(END_CHECKPOINT);
        PA_ADD_OPTION(LOOP_CHECKPOINT);
        PA_ADD_OPTION(START_LOOP);
        PA_ADD_OPTION(END_LOOP);

        PA_ADD_OPTION(ENABLE_TEST_REALIGN);
        PA_ADD_OPTION(REALIGN_MODE);
        PA_ADD_OPTION(X_REALIGN);
        PA_ADD_OPTION(Y_REALIGN);
        PA_ADD_OPTION(REALIGN_DURATION);

        PA_ADD_OPTION(ENABLE_TEST_OVERWORLD_MOVE);
        PA_ADD_OPTION(FORWARD_TICKS);

        PA_ADD_OPTION(TEST_PBF_LEFT_JOYSTICK);
        PA_ADD_OPTION(X_MOVE);
        PA_ADD_OPTION(Y_MOVE);
        PA_ADD_OPTION(HOLD_TICKS);
        PA_ADD_OPTION(RELEASE_TICKS);
    }

    AutoStory::value_changed(this);

    STORY_SECTION.add_listener(*this);
    STARTPOINT_TUTORIAL.add_listener(*this);
    ENDPOINT_TUTORIAL.add_listener(*this);
    // STARTPOINT_MAINSTORY.add_listener(*this);
    // ENDPOINT_MAINSTORY.add_listener(*this);    
    ENABLE_TEST_CHECKPOINTS.add_listener(*this);
    ENABLE_TEST_REALIGN.add_listener(*this);
    ENABLE_TEST_OVERWORLD_MOVE.add_listener(*this);
    TEST_PBF_LEFT_JOYSTICK.add_listener(*this);
}

void AutoStory::value_changed(void* object){
    ConfigOptionState state = (STARTPOINT_TUTORIAL.index() <= 1)
        ? ConfigOptionState::ENABLED
        : ConfigOptionState::HIDDEN;
    STARTERCHOICE.set_visibility(state);

    if (STORY_SECTION == StorySection::TUTORIAL){
        STARTPOINT_TUTORIAL.set_visibility(ConfigOptionState::ENABLED);
        ENDPOINT_TUTORIAL.set_visibility(ConfigOptionState::ENABLED);

        // STARTPOINT_MAINSTORY.set_visibility(ConfigOptionState::HIDDEN);
        // ENDPOINT_MAINSTORY.set_visibility(ConfigOptionState::HIDDEN);
    }else if (STORY_SECTION == StorySection::MAIN_STORY){
        STARTPOINT_TUTORIAL.set_visibility(ConfigOptionState::HIDDEN);
        ENDPOINT_TUTORIAL.set_visibility(ConfigOptionState::HIDDEN);

        // STARTPOINT_MAINSTORY.set_visibility(ConfigOptionState::ENABLED);
        // ENDPOINT_MAINSTORY.set_visibility(ConfigOptionState::ENABLED);        
    }

    MAINSTORY_NOTE.set_visibility(STORY_SECTION == StorySection::TUTORIAL ? ConfigOptionState::HIDDEN : ConfigOptionState::ENABLED);
    START_DESCRIPTION.set_text(start_segment_description());
    END_DESCRIPTION.set_text(end_segment_description());

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

    if (ENABLE_TEST_OVERWORLD_MOVE){
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


}






void AutoStory::test_checkpoints(
    SingleSwitchProgramEnvironment& env,
    ConsoleHandle& console, 
    BotBaseContext& context,
    int start, int end, 
    int loop, int start_loop, int end_loop
){
    // EventNotificationOption& notif_status_update = NOTIFICATION_STATUS_UPDATE;
    // Language language = LANGUAGE;
    // StarterChoice starter_choice = STARTERCHOICE;
    std::vector<std::function<void()>> checkpoint_list;
    checkpoint_list.push_back([&](){checkpoint_00(env, context);});
    // checkpoint_list.push_back([&](){checkpoint_01(env, context, notif_status_update, language);});
    // checkpoint_list.push_back([&](){checkpoint_02(env, context, notif_status_update);});
    // checkpoint_list.push_back([&](){checkpoint_03(env, context, notif_status_update, language, starter_choice);});
    // checkpoint_list.push_back([&](){checkpoint_04(env, context, notif_status_update);});
    // checkpoint_list.push_back([&](){checkpoint_05(env, context, notif_status_update);});
    // checkpoint_list.push_back([&](){checkpoint_06(env, context, notif_status_update);});
    // checkpoint_list.push_back([&](){checkpoint_07(env, context, notif_status_update);});
    // checkpoint_list.push_back([&](){checkpoint_08(env, context, notif_status_update);});
    // checkpoint_list.push_back([&](){checkpoint_09(env, context, notif_status_update);});
    // checkpoint_list.push_back([&](){checkpoint_10(env, context, notif_status_update);});
    // checkpoint_list.push_back([&](){checkpoint_11(env, context, notif_status_update);});
    // checkpoint_list.push_back([&](){checkpoint_12(env, context, notif_status_update);});
    // checkpoint_list.push_back([&](){checkpoint_13(env, context, notif_status_update);});
    // checkpoint_list.push_back([&](){checkpoint_14(env, context, notif_status_update);});
    // checkpoint_list.push_back([&](){checkpoint_15(env, context, notif_status_update);});
    // checkpoint_list.push_back([&](){checkpoint_16(env, context, notif_status_update);});
    // checkpoint_list.push_back([&](){checkpoint_17(env, context, notif_status_update);});
    // checkpoint_list.push_back([&](){checkpoint_18(env, context, notif_status_update);});
    // checkpoint_list.push_back([&](){checkpoint_19(env, context, notif_status_update);});
    // checkpoint_list.push_back([&](){checkpoint_20(env, context, notif_status_update);});
    // checkpoint_list.push_back([&](){checkpoint_21(env, context, notif_status_update);});
    // checkpoint_list.push_back([&](){checkpoint_22(env, context, notif_status_update);});
    // checkpoint_list.push_back([&](){checkpoint_23(env, context, notif_status_update);});
    // checkpoint_list.push_back([&](){checkpoint_24(env, context, notif_status_update);});
    // checkpoint_list.push_back([&](){checkpoint_25(env, context, notif_status_update);});
    // checkpoint_list.push_back([&](){checkpoint_26(env, context, notif_status_update);});
    // checkpoint_list.push_back([&](){checkpoint_27(env, context, notif_status_update);});
    // checkpoint_list.push_back([&](){checkpoint_28(env, context, notif_status_update);});
    // checkpoint_list.push_back([&](){checkpoint_29(env, context, notif_status_update);});
    // checkpoint_list.push_back([&](){checkpoint_30(env, context, notif_status_update);});
    // checkpoint_list.push_back([&](){checkpoint_31(env, context, notif_status_update);});
    // checkpoint_list.push_back([&](){checkpoint_32(env, context, notif_status_update);});
    // checkpoint_list.push_back([&](){checkpoint_33(env, context, notif_status_update);});
    // checkpoint_list.push_back([&](){checkpoint_34(env, context, notif_status_update);});

    for (int checkpoint = start; checkpoint <= end; checkpoint++){
        if (checkpoint == 0){
            console.log("checkpoint_0");
            checkpoint_list[checkpoint]();
            continue;
        }
        
        const size_t DIGITS = 3;
        std::string number = std::to_string(checkpoint);
        if (number.size() < DIGITS){
            number = std::string(DIGITS - number.size(), '0') + number;
        }        
        if (checkpoint >= start_loop && checkpoint <= end_loop){
            for (int i = 0; i < loop; i++){
                if (i > 0){
                    reset_game(env.program_info(), console, context);
                }
                console.log("checkpoint_" + number + ": loop " + std::to_string(i));
                checkpoint_list[checkpoint]();
            } 
        }else{
            console.log("checkpoint_" + number + ".");
            checkpoint_list[checkpoint]();            
        }
       
    }
    
}

std::string AutoStory::start_segment_description(){
    size_t segment_index = 0;
    if (STORY_SECTION == StorySection::TUTORIAL){
        segment_index = STARTPOINT_TUTORIAL.index();
    }else if (STORY_SECTION == StorySection::MAIN_STORY){
        // segment_index = STARTPOINT_MAINSTORY.index() + 10;
    }
    return ALL_AUTO_STORY_SEGMENT_LIST()[segment_index]->start_text();
}

std::string AutoStory::end_segment_description(){
    size_t segment_index = 0;
    if (STORY_SECTION == StorySection::TUTORIAL){
        segment_index = ENDPOINT_TUTORIAL.index();
    }else if (STORY_SECTION == StorySection::MAIN_STORY){
        // segment_index = ENDPOINT_MAINSTORY.index() + 10;
    }    
    return ALL_AUTO_STORY_SEGMENT_LIST()[segment_index]->end_text();
}


void AutoStory::run_autostory(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    AutoStoryOptions options{
        LANGUAGE,
        STARTERCHOICE,
        NOTIFICATION_STATUS_UPDATE
    };    

    size_t start = STARTPOINT_TUTORIAL.index();
    size_t end = ENDPOINT_TUTORIAL.index();
    for (size_t segment_index = start; segment_index <= end; segment_index++){
        ALL_AUTO_STORY_SEGMENT_LIST()[segment_index]->run_segment(env, context, options);
    }
}

void AutoStory::test_code(SingleSwitchProgramEnvironment& env, BotBaseContext& context){

    if (ENABLE_TEST_CHECKPOINTS){
        // test individual checkpoints
        test_checkpoints(env, env.console, context, START_CHECKPOINT, END_CHECKPOINT, LOOP_CHECKPOINT, START_LOOP, END_LOOP);
        return;
    }
    

    if (ENABLE_TEST_REALIGN){
        // clear realign marker
        // realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 128, 128, 0);
        realign_player(env.program_info(), env.console, context, REALIGN_MODE, X_REALIGN, Y_REALIGN, REALIGN_DURATION);
        return;
    }

    if (ENABLE_TEST_OVERWORLD_MOVE){
        // walk_forward_while_clear_front_path(env.program_info(), env.console, context, FORWARD_TICKS);

        overworld_navigation(env.program_info(), env.console, context, 
            NavigationStopCondition::STOP_MARKER, NavigationMovementMode::CLEAR_WITH_LETS_GO, 
            128, 0, 60, 10, false);

        // overworld_navigation(env.program_info(), env.console, context, 
        //     NavigationStopCondition::STOP_TIME, NavigationMovementMode::CLEAR_WITH_LETS_GO, 
        //     128, 0, 25, 10, false);         
        return;
    }

    if (TEST_PBF_LEFT_JOYSTICK){
        pbf_move_left_joystick(context, X_MOVE, Y_MOVE, HOLD_TICKS, RELEASE_TICKS);
        return;
    }        

    // context.wait_for(Milliseconds(1000000));


}

void AutoStory::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    assert_16_9_720p_min(env.logger(), env.console);
    // AutoStoryStats& stats = env.current_stats<AutoStoryStats>();

    // Connect controller
    pbf_press_button(context, BUTTON_L, 20, 20);

    // test code
    if (ENABLE_TEST_CHECKPOINTS || ENABLE_TEST_REALIGN || ENABLE_TEST_OVERWORLD_MOVE || TEST_PBF_LEFT_JOYSTICK){
        test_code(env, context);
        return;
    }

    // Set settings. to ensure autosave is off.
    if (CHANGE_SETTINGS){
        change_settings_prior_to_autostory(env, context, STARTPOINT_TUTORIAL.index(), LANGUAGE);
    }

    run_autostory(env, context);
    
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
    GO_HOME_WHEN_DONE.run_end_of_program(context);
}






}
}
}
