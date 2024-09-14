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
// #include "PokemonSV_AutoStory_Checkpoint_00_04.h"
// #include "PokemonSV_AutoStory_Checkpoint_05_10.h"
// #include "PokemonSV_AutoStory_Checkpoint_11_15.h"
// #include "PokemonSV_AutoStory_Checkpoint_16_20.h"
// #include "PokemonSV_AutoStory_Checkpoint_21_25.h"
// #include "PokemonSV_AutoStory_Checkpoint_26_30.h"
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
    STARTPOINT.remove_listener(*this);
    ENDPOINT.remove_listener(*this);
}

AutoStory::AutoStory()
    : LANGUAGE(
        "<b>Game Language:</b>",
        PokemonSwSh::IV_READER().languages(),
        LockMode::UNLOCK_WHILE_RUNNING,
        true
    )
    , STARTPOINT(
        "<b>Start Point:</b><br>Program will start with this segment.",
        {
            {StartPoint::INTRO_CUTSCENE,        "00_gameintro",         "00: Intro Cutscene"},
            {StartPoint::PICK_STARTER,          "01_pickstarter",       "01: Pick Starter"},
            {StartPoint::NEMONA_FIRST_BATTLE,   "02_nemonabattle1",     "02: First Nemona Battle"},
            {StartPoint::CATCH_TUTORIAL,        "03_catchtutorial",     "03: Catch Tutorial"},
            {StartPoint::LEGENDARY_RESCUE,      "04_legendaryrescue",   "04: Rescue Legendary"},
            {StartPoint::ARVEN_FIRST_BATTLE,    "05_arvenbattle1",      "05: First Arven Battle"},
            {StartPoint::LOS_PLATOS,            "06_losplatos",         "06: Go to Los Platos"},
            {StartPoint::MESAGOZA_SOUTH,        "07_mesagozasouth",     "07: Go to Mesagoza South"},
        },
        LockMode::LOCK_WHILE_RUNNING,
        StartPoint::INTRO_CUTSCENE
    )
    , ENDPOINT(
        "<b>End Point:</b><br>Program will stop after completing this segment.",
        {
            {EndPoint::PICK_STARTER,            "01_pickstarter",       "01: Pick Starter"},
            {EndPoint::NEMONA_FIRST_BATTLE,     "02_nemonabattle1",     "02: First Nemona Battle"},
            {EndPoint::CATCH_TUTORIAL,          "03_catchtutorial",     "03: Catch Tutorial"},
            {EndPoint::LEGENDARY_RESCUE,        "04_legendaryrescue",   "04: Rescue Legendary"},
            {EndPoint::ARVEN_FIRST_BATTLE,      "05_arvenbattle1",      "05: First Arven Battle"},
            {EndPoint::LOS_PLATOS,              "06_losplatos",         "06: Go to Los Platos"},
            {EndPoint::MESAGOZA_SOUTH,          "07_mesagozasouth",     "07: Go to Mesagoza South"},
        },
        LockMode::LOCK_WHILE_RUNNING,
        EndPoint::PICK_STARTER
    )
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
    PA_ADD_OPTION(STARTPOINT);
    PA_ADD_OPTION(START_DESCRIPTION);
    PA_ADD_OPTION(ENDPOINT);
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

    STARTPOINT.add_listener(*this);
    ENDPOINT.add_listener(*this);
    ENABLE_TEST_CHECKPOINTS.add_listener(*this);
    ENABLE_TEST_REALIGN.add_listener(*this);
    ENABLE_TEST_OVERWORLD_MOVE.add_listener(*this);
    TEST_PBF_LEFT_JOYSTICK.add_listener(*this);
}

void AutoStory::value_changed(void* object){
    ConfigOptionState state = (STARTPOINT == StartPoint::INTRO_CUTSCENE) || (STARTPOINT == StartPoint::PICK_STARTER)
        ? ConfigOptionState::ENABLED
        : ConfigOptionState::HIDDEN;
    STARTERCHOICE.set_visibility(state);

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

std::string AutoStory::start_segment_description(){
    switch(STARTPOINT){
    case StartPoint::INTRO_CUTSCENE:
        return "Start: Intro cutscene.";
    case StartPoint::PICK_STARTER:
        return "Start: Finished cutscene. Adjusted settings. Standing in left side of room.";
    case StartPoint::NEMONA_FIRST_BATTLE:
        return "Start: Picked the starter.";
    case StartPoint::CATCH_TUTORIAL:
        return "Start: Battled Nemona on the beach.";
    case StartPoint::LEGENDARY_RESCUE:
        return "Start: Finished catch tutorial. Walked to the cliff and heard mystery cry.";
    case StartPoint::ARVEN_FIRST_BATTLE:
        return "Start: Saved the Legendary. Escaped from the Houndoom cave.";
    case StartPoint::LOS_PLATOS:
        return "Start: Battled Arven, received Legendary's Pokeball. Talked to Nemona at Lighthouse.";
    case StartPoint::MESAGOZA_SOUTH:
        return "Start: At Los Platos Pokecenter.";
    default:
        return "";        
    }
}

std::string AutoStory::end_segment_description(){
    switch(ENDPOINT){
    case EndPoint::INTRO_CUTSCENE:
        return "End: Finished cutscene. Adjusted settings. Standing in left side of room.";
    case EndPoint::PICK_STARTER:
        return "End: Picked the starter.";
    case EndPoint::NEMONA_FIRST_BATTLE:
        return "End: Battled Nemona on the beach.";
    case EndPoint::CATCH_TUTORIAL:
        return "End: Finished catch tutorial. Walked to the cliff and heard mystery cry.";
    case EndPoint::LEGENDARY_RESCUE:
        return "End: Saved the Legendary. Escaped from the Houndoom cave.";
    case EndPoint::ARVEN_FIRST_BATTLE:
        return "End: Battled Arven, received Legendary's Pokeball. Talked to Nemona at Lighthouse.";
    case EndPoint::LOS_PLATOS:
        return "End: At Los Platos Pokecenter.";
    case EndPoint::MESAGOZA_SOUTH:
        return "End: ";
    default:
        return "";
    }
}


void AutoStory::test_checkpoints(
    SingleSwitchProgramEnvironment& env,
    ConsoleHandle& console, 
    BotBaseContext& context,
    int start, int end, 
    int loop, int start_loop, int end_loop
){
    EventNotificationOption& notif_status_update = NOTIFICATION_STATUS_UPDATE;
    Language language = LANGUAGE;
    StarterChoice starter_choice = STARTERCHOICE;
    std::vector<std::function<void()>> checkpoint_list;
    // checkpoint_list.push_back([&](){checkpoint_00(env, context);});
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
        
        std::string leading_zero = checkpoint < 10 ? "0" : "";
        if (checkpoint >= start_loop && checkpoint <= end_loop){
            for (int i = 0; i < loop; i++){
                if (i > 0){
                    reset_game(env.program_info(), console, context);
                }
                console.log("checkpoint_" + leading_zero + std::to_string(checkpoint) + ": loop " + std::to_string(i));
                checkpoint_list[checkpoint]();
            } 
        }else{
            console.log("checkpoint_" + leading_zero + std::to_string(checkpoint) + ".");
            checkpoint_list[checkpoint]();            
        }
       
    }
    
}



void AutoStory::run_autostory(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    AutoStoryStats& stats = env.current_stats<AutoStoryStats>();
    switch (STARTPOINT){
    // case StartPoint::INTRO_CUTSCENE:
    //     context.wait_for_all_requests();
    //     env.console.log("Start Segment 00: Intro Cutscene", COLOR_ORANGE);
    //     env.console.overlay().add_log("Start Segment 00: Intro Cutscene", COLOR_ORANGE);

    //     checkpoint_00(env, context);

    //     context.wait_for_all_requests();
    //     env.console.log("End Segment 00: Intro Cutscene", COLOR_GREEN);
    //     env.console.overlay().add_log("End Segment 00: Intro Cutscene", COLOR_GREEN);
    //     stats.m_segment++;
    //     env.update_stats();
    //     pbf_wait(context, 1 * TICKS_PER_SECOND);        
    // case StartPoint::PICK_STARTER:
    //     context.wait_for_all_requests();
    //     env.console.log("Start Segment 01: Pick Starter", COLOR_ORANGE);
    //     env.console.overlay().add_log("Start Segment 01: Pick Starter", COLOR_ORANGE);

    //     checkpoint_01(env, context, NOTIFICATION_STATUS_UPDATE, LANGUAGE);
    //     checkpoint_02(env, context, NOTIFICATION_STATUS_UPDATE);
    //     checkpoint_03(env, context, NOTIFICATION_STATUS_UPDATE, LANGUAGE, STARTERCHOICE);

    //     context.wait_for_all_requests();
    //     env.console.log("End Segment 02: Pick Starter", COLOR_GREEN);
    //     env.console.overlay().add_log("End Segment 02: Pick Starter", COLOR_GREEN);
    //     stats.m_segment++;
    //     env.update_stats();
    //     if (ENDPOINT == EndPoint::PICK_STARTER){
    //         break;
    //     }

    // case StartPoint::NEMONA_FIRST_BATTLE:
    //     context.wait_for_all_requests();
    //     env.console.log("Start Segment 02: First Nemona Battle", COLOR_ORANGE);
    //     env.console.overlay().add_log("Start Segment 02: First Nemona Battle", COLOR_ORANGE);

    //     checkpoint_04(env, context, NOTIFICATION_STATUS_UPDATE);

    //     context.wait_for_all_requests();
    //     env.console.log("End Segment 02: First Nemona Battle", COLOR_GREEN);
    //     env.console.overlay().add_log("End Segment 02: First Nemona Battle", COLOR_GREEN);
    //     stats.m_segment++;
    //     env.update_stats();
    //     if (ENDPOINT == EndPoint::NEMONA_FIRST_BATTLE){
    //         break;
    //     }

    // case StartPoint::CATCH_TUTORIAL:
    //     context.wait_for_all_requests();
    //     env.console.log("Start Segment 03: Catch Tutorial", COLOR_ORANGE);
    //     env.console.overlay().add_log("Start Segment 03: Catch Tutorial", COLOR_ORANGE);

    //     checkpoint_05(env, context, NOTIFICATION_STATUS_UPDATE);
    //     checkpoint_06(env, context, NOTIFICATION_STATUS_UPDATE);
    //     checkpoint_07(env, context, NOTIFICATION_STATUS_UPDATE);

    //     context.wait_for_all_requests();
    //     env.console.log("End Segment 03: Catch Tutorial", COLOR_GREEN);
    //     env.console.overlay().add_log("End Segment 03: Catch Tutorial", COLOR_GREEN);
    //     stats.m_segment++;
    //     env.update_stats();
    //     if (ENDPOINT == EndPoint::CATCH_TUTORIAL){
    //         break;
    //     }

    // case StartPoint::LEGENDARY_RESCUE:
    //     context.wait_for_all_requests();
    //     env.console.log("Start Segment 04: Rescue Legendary", COLOR_ORANGE);
    //     env.console.overlay().add_log("Start Segment 04: Rescue Legendary", COLOR_ORANGE);

    //     checkpoint_08(env, context, NOTIFICATION_STATUS_UPDATE);

    //     context.wait_for_all_requests();
    //     env.console.log("End Segment 04: Rescue Legendary", COLOR_GREEN);
    //     env.console.overlay().add_log("End Segment 04: Rescue Legendary", COLOR_GREEN);
    //     stats.m_segment++;
    //     env.update_stats();
    //     if (ENDPOINT == EndPoint::LEGENDARY_RESCUE){
    //         break;
    //     }

    // case StartPoint::ARVEN_FIRST_BATTLE:
    //     context.wait_for_all_requests();
    //     env.console.log("Start Segment 05: First Arven Battle", COLOR_ORANGE);
    //     env.console.overlay().add_log("Start Segment 05: First Arven Battle", COLOR_ORANGE);

    //     checkpoint_09(env, context, NOTIFICATION_STATUS_UPDATE);
    //     checkpoint_10(env, context, NOTIFICATION_STATUS_UPDATE);

    //     context.wait_for_all_requests();
    //     env.console.log("End Segment 05: First Arven Battle", COLOR_GREEN);
    //     env.console.overlay().add_log("End Segment 05: First Arven Battle", COLOR_GREEN);
    //     stats.m_segment++;
    //     env.update_stats();
    //     if (ENDPOINT == EndPoint::ARVEN_FIRST_BATTLE){
    //         break;
    //     }

    // case StartPoint::LOS_PLATOS:
    //     context.wait_for_all_requests();
    //     env.console.log("Start Segment 06: Go to Los Platos", COLOR_ORANGE);
    //     env.console.overlay().add_log("Start Segment 06: Go to Los Platos", COLOR_ORANGE);

    //     checkpoint_11(env, context, NOTIFICATION_STATUS_UPDATE);

    //     context.wait_for_all_requests();
    //     env.console.log("End Segment 06: Go to Los Platos", COLOR_GREEN);
    //     env.console.overlay().add_log("End Segment 06: Go to Los Platos", COLOR_GREEN);
    //     stats.m_segment++;
    //     env.update_stats();
    //     if (ENDPOINT == EndPoint::LOS_PLATOS){
    //         break;
    //     }
    // case StartPoint::MESAGOZA_SOUTH:
    //     context.wait_for_all_requests();
    //     env.console.log("Start Segment 07: Go to Mesagoza South", COLOR_ORANGE);
    //     env.console.overlay().add_log("Start Segment 07: Go to Mesagoza South", COLOR_ORANGE);

    //     // // Mystery Gift, delete later
    //     // enter_menu_from_overworld(env.program_info(), env.console, context, 2);
    //     // pbf_press_button(context, BUTTON_A, 20, 4 * TICKS_PER_SECOND);
    //     // pbf_press_dpad(context, DPAD_UP, 20, 105);
    //     // pbf_press_button(context, BUTTON_A, 20, 4 * TICKS_PER_SECOND);
    //     // pbf_press_dpad(context, DPAD_DOWN, 20, 105);
    //     // pbf_press_button(context, BUTTON_A, 20, 4 * TICKS_PER_SECOND);
    //     // pbf_press_button(context, BUTTON_A, 20, 10 * TICKS_PER_SECOND);
    //     // clear_dialog(env.console, context, ClearDialogMode::STOP_TIMEOUT, 10);

    //     context.wait_for_all_requests();
    //     env.console.log("End Segment 07: Go to Mesagoza South", COLOR_GREEN);
    //     env.console.overlay().add_log("End Segment 07: Go to Mesagoza South", COLOR_GREEN);
    //     stats.m_segment++;
    //     env.update_stats();
    //     if (ENDPOINT == EndPoint::MESAGOZA_SOUTH){
    //         break;
    //     }
    default:
        break;

    }
}

void AutoStory::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    assert_16_9_720p_min(env.logger(), env.console);
    // AutoStoryStats& stats = env.current_stats<AutoStoryStats>();

    // Connect controller
    pbf_press_button(context, BUTTON_L, 20, 20);

    // press_Bs_to_back_to_overworld(env.program_info(), env.console, context, 7);
    // walk_forward_until_dialog(env.program_info(), env.console, context, NavigationMovementMode::DIRECTIONAL_SPAM_A, 10);

    // context.wait_for(Milliseconds(1000000));

    if (ENABLE_TEST_REALIGN){
        // clear realign marker
        // realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 128, 128, 0);
        realign_player(env.program_info(), env.console, context, REALIGN_MODE, X_REALIGN, Y_REALIGN, REALIGN_DURATION);
        context.wait_for(Milliseconds(1000000));
    }

    if (ENABLE_TEST_OVERWORLD_MOVE){
        // walk_forward_while_clear_front_path(env.program_info(), env.console, context, FORWARD_TICKS);
        overworld_navigation(env.program_info(), env.console, context, 
            NavigationStopCondition::STOP_MARKER, NavigationMovementMode::CLEAR_WITH_LETS_GO, 
            128, 0, 60, 10, false);
        context.wait_for(Milliseconds(1000000));
    }

    if (TEST_PBF_LEFT_JOYSTICK){
        pbf_move_left_joystick(context, X_MOVE, Y_MOVE, HOLD_TICKS, RELEASE_TICKS);
        context.wait_for(Milliseconds(1000000));
    }    

    // Set settings. to ensure autosave is off.
    if (CHANGE_SETTINGS){
        change_settings_prior_to_autostory(env, context, STARTPOINT, LANGUAGE);
    }

    if (ENABLE_TEST_CHECKPOINTS){
        // test individual checkpoints
        test_checkpoints(env, env.console, context, START_CHECKPOINT, END_CHECKPOINT, LOOP_CHECKPOINT, START_LOOP, END_LOOP);
    }else{
        run_autostory(env, context);
    }
    
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
    GO_HOME_WHEN_DONE.run_end_of_program(context);
}






}
}
}
