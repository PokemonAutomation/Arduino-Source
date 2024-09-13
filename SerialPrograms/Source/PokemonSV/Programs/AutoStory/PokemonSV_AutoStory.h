/*  Autostory
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_AutoStory_H
#define PokemonAutomation_PokemonSV_AutoStory_H

#include <functional>
//#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
//#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "CommonFramework/Options/LanguageOCROption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"
#include "Common/NintendoSwitch/NintendoSwitch_ControllerDefs.h"
#include "PokemonSV/Inference/PokemonSV_MainMenuDetector.h"
#include "PokemonSV/Programs/PokemonSV_Navigation.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


enum class BattleStopCondition{
    STOP_OVERWORLD,
    STOP_DIALOG,
};

enum class ClearDialogMode{
    STOP_OVERWORLD,
    STOP_PROMPT,
    STOP_WHITEBUTTON,
    STOP_TIMEOUT,
    STOP_BATTLE,
};


enum class ClearDialogCallback{
    ADVANCE_DIALOG,
    OVERWORLD,
    PROMPT_DIALOG,
    WHITE_A_BUTTON,
    DIALOG_ARROW,
    BATTLE,
    TUTORIAL,
    BLACK_DIALOG_BOX
};

enum class StartPoint{
    INTRO_CUTSCENE,
    PICK_STARTER,
    NEMONA_FIRST_BATTLE,
    CATCH_TUTORIAL,
    LEGENDARY_RESCUE,
    ARVEN_FIRST_BATTLE,
    LOS_PLATOS,
    MESAGOZA_SOUTH,
};

enum class StarterChoice{
    SPRIGATITO,
    FUECOCO,
    QUAXLY,
};

class AutoStory_Descriptor : public SingleSwitchProgramDescriptor{
public:
    AutoStory_Descriptor();

    struct Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};

// spam A button to choose the first move
// throw exception if wipeout.
void run_battle_press_A(
    ConsoleHandle& console, 
    BotBaseContext& context,
    BattleStopCondition stop_condition
);

// press A to clear tutorial screens
// throw exception if tutorial screen never detected
void clear_tutorial(ConsoleHandle& console, BotBaseContext& context, uint16_t seconds_timeout = 5);

// spam the A button to clear dialog.
// stop depending on ClearDialogMode: stop when detect overworld, or dialog prompt, or A button prompt. Or if times out
// throw exception if times out, unless this is the intended stop condition.
// also throw exception if dialog is never detected.
void clear_dialog(ConsoleHandle& console, BotBaseContext& context,
    ClearDialogMode mode, uint16_t seconds_timeout = 60,
    std::vector<ClearDialogCallback> optional_callbacks = {}
);

// move character with ssf left joystick, as per given x, y, until 
// stop_condition is met (e.g. Dialog detected). 
// throw exception if reaches timeout before detecting stop condition
void overworld_navigation(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context,
    NavigationStopCondition stop_condition,
    NavigationMovementMode movement_mode,
    uint8_t x, uint8_t y,
    uint16_t seconds_timeout = 60, uint16_t seconds_realign = 60,
    bool auto_heal = true
);

void config_option(BotBaseContext& context, int change_option_value);

// enter menu and swap the first and third moves for your starter
void swap_starter_moves(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context, Language language);

// run the given `action`. if detect a battle, stop the action, and throw exception
void do_action_and_monitor_for_battles(
    SingleSwitchProgramEnvironment& env, 
    ConsoleHandle& console,
    BotBaseContext& context,
    std::function<
        void(SingleSwitchProgramEnvironment& env,
        ConsoleHandle& console,
        BotBaseContext& context)
    >&& action
);

void wait_for_gradient_arrow(
    const ProgramInfo& info, 
    ConsoleHandle& console, 
    BotBaseContext& context, 
    ImageFloatBox box_area_to_check,
    uint16_t seconds_timeout
);

void press_A_until_dialog(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context, uint16_t seconds_between_button_presses);

bool check_ride_active(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context);

void get_on_ride(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context);


// change the settings prior to Autostory
// Assumes that `current_segment` represents where we currently are in the story.
void change_settings_prior_to_autostory(SingleSwitchProgramEnvironment& env, BotBaseContext& context, StartPoint current_segment, Language language);

// from within the Settings/Options menu, change the settings
void change_settings(SingleSwitchProgramEnvironment& env, BotBaseContext& context, Language language, bool use_inference = true);


void checkpoint_save(SingleSwitchProgramEnvironment& env, BotBaseContext& context, EventNotificationOption& notif_status_update);


// start: after selecting character name, style and the cutscene has started
// end: stood up from chair. Walked to left side of room.
void checkpoint_00(SingleSwitchProgramEnvironment& env, BotBaseContext& context);

// start: stood up from chair. Walked to left side of room.
// end: standing in room. updated settings
void checkpoint_01(SingleSwitchProgramEnvironment& env, BotBaseContext& context, EventNotificationOption& notif_status_update, Language language);

// start: standing in room. updated settings
// end: standing in front of power of science NPC. Cleared map tutorial.
void checkpoint_02(SingleSwitchProgramEnvironment& env, BotBaseContext& context, EventNotificationOption& notif_status_update);

// start: standing in front of power of science NPC. Cleared map tutorial.
// end: received starter, changed move order
void checkpoint_03(SingleSwitchProgramEnvironment& env, BotBaseContext& context, EventNotificationOption& notif_status_update, Language language, StarterChoice starter_choice);

// start: Received starter pokemon and changed move order. Cleared autoheal tutorial.
// end: Battled Nemona on the beach.
void checkpoint_04(SingleSwitchProgramEnvironment& env, BotBaseContext& context, EventNotificationOption& notif_status_update);

// start: Battled Nemona on the beach.
// end: Met mom at gate. Received mom's sandwich.
void checkpoint_05(SingleSwitchProgramEnvironment& env, BotBaseContext& context, EventNotificationOption& notif_status_update);

// start: Met mom at gate. Received mom's sandwich.
// end: Cleared catch tutorial.
void checkpoint_06(SingleSwitchProgramEnvironment& env, BotBaseContext& context, EventNotificationOption& notif_status_update);

// start: Cleared catch tutorial.
// end: Moved to cliff. Heard mystery cry.
void checkpoint_07(SingleSwitchProgramEnvironment& env, BotBaseContext& context, EventNotificationOption& notif_status_update);

// start: Moved to cliff. Heard mystery cry.
// end: Rescued Koraidon/Miraidon and escaped from the Houndoom Cave.
void checkpoint_08(SingleSwitchProgramEnvironment& env, BotBaseContext& context, EventNotificationOption& notif_status_update);

// start: Rescued Koraidon/Miraidon and escaped from the Houndoom Cave.
// end: Battled Arven and received Legendary's Pokeball.
void checkpoint_09(SingleSwitchProgramEnvironment& env, BotBaseContext& context, EventNotificationOption& notif_status_update);

// start: Battled Arven and received Legendary's Pokeball.
// end: Talked to Nemona at the Lighthouse.
void checkpoint_10(SingleSwitchProgramEnvironment& env, BotBaseContext& context, EventNotificationOption& notif_status_update);

// start: Talked to Nemona at the Lighthouse.
// end: Arrived at Los Platos pokecenter. Cleared Let's go tutorial.
void checkpoint_11(SingleSwitchProgramEnvironment& env, BotBaseContext& context, EventNotificationOption& notif_status_update);

// start: Arrived at Los Platos pokecenter. Cleared Let's go tutorial.
// end: Arrived at Mesagoza (South) Pokecenter
void checkpoint_12(SingleSwitchProgramEnvironment& env, BotBaseContext& context, EventNotificationOption& notif_status_update);

// start: Arrived at Mesagoza (South) Pokecenter
// end: Battled Nemona at Mesagoza gate. Entered Mesagoza.
void checkpoint_13(SingleSwitchProgramEnvironment& env, BotBaseContext& context, EventNotificationOption& notif_status_update);

// start: Battled Nemona at Mesagoza gate. Entered Mesagoza.
// end: Battled Team Star at school entrance.
void checkpoint_14(SingleSwitchProgramEnvironment& env, BotBaseContext& context, EventNotificationOption& notif_status_update);

// start: Battled Team Star at school entrance.
// end: Talked to Jacq in classroom. Standing in classroom.
void checkpoint_15(SingleSwitchProgramEnvironment& env, BotBaseContext& context, EventNotificationOption& notif_status_update);

// start: Talked to Jacq in classroom. Standing in classroom.
// end: Talked to Arven. Received Titan questline (Path of Legends). Talked to Cassiopeia. Standing in main hall.
void checkpoint_16(SingleSwitchProgramEnvironment& env, BotBaseContext& context, EventNotificationOption& notif_status_update);

// start: Talked to Arven. Received Titan questline (Path of Legends).
// end: Talked to Cassiopeia. Saw Geeta. Talked to Nemona. Received Gym/Elite Four questline (Victory Road). Standing in staff room.
void checkpoint_17(SingleSwitchProgramEnvironment& env, BotBaseContext& context, EventNotificationOption& notif_status_update); 

// start: Talked to Cassiopeia. Saw Geeta. Talked to Nemona. Received Gym/Elite Four questline (Victory Road). Standing in staff room.
// end: Talked to Clavell and the professor.
void checkpoint_18(SingleSwitchProgramEnvironment& env, BotBaseContext& context, EventNotificationOption& notif_status_update);          

// start: Talked to Clavell and the professor.
// end: Talked to Nemona, visited dorm, time passed.
void checkpoint_19(SingleSwitchProgramEnvironment& env, BotBaseContext& context, EventNotificationOption& notif_status_update);      

// start: Talked to Nemona, visited dorm, time passed.
// end: Get on ride for first time.
void checkpoint_20(SingleSwitchProgramEnvironment& env, BotBaseContext& context, EventNotificationOption& notif_status_update);   

// start: (After the break, with level 100 Gardevoir. At Mesagoza West pokecenter.)
// end: 
void checkpoint_21(SingleSwitchProgramEnvironment& env, BotBaseContext& context, EventNotificationOption& notif_status_update);   

// start: (At South Province Area Two Pokecenter. )
// end: 
void checkpoint_22(SingleSwitchProgramEnvironment& env, BotBaseContext& context, EventNotificationOption& notif_status_update);   

// start: (At Cortondo East Pokecenter)
// end: 
void checkpoint_23(SingleSwitchProgramEnvironment& env, BotBaseContext& context, EventNotificationOption& notif_status_update);   

// start: (Done Cortondo Gym challenge)
// end: 
void checkpoint_24(SingleSwitchProgramEnvironment& env, BotBaseContext& context, EventNotificationOption& notif_status_update);   

// start: (Beat Cortondo Gym leader (Bug))
// end: 
void checkpoint_25(SingleSwitchProgramEnvironment& env, BotBaseContext& context, EventNotificationOption& notif_status_update);  

// start: At Cortondo West Pokecenter.
// end: 
void checkpoint_26(SingleSwitchProgramEnvironment& env, BotBaseContext& context, EventNotificationOption& notif_status_update);   

// start: 
// end: 
void checkpoint_27(SingleSwitchProgramEnvironment& env, BotBaseContext& context, EventNotificationOption& notif_status_update);    

// start: 
// end: 
void checkpoint_28(SingleSwitchProgramEnvironment& env, BotBaseContext& context, EventNotificationOption& notif_status_update);   

// start: 
// end: 
void checkpoint_29(SingleSwitchProgramEnvironment& env, BotBaseContext& context, EventNotificationOption& notif_status_update);   

// start: 
// end: 
void checkpoint_30(SingleSwitchProgramEnvironment& env, BotBaseContext& context, EventNotificationOption& notif_status_update);   

// start: 
// end: 
void checkpoint_31(SingleSwitchProgramEnvironment& env, BotBaseContext& context, EventNotificationOption& notif_status_update);  

// start: 
// end: 
void checkpoint_32(SingleSwitchProgramEnvironment& env, BotBaseContext& context, EventNotificationOption& notif_status_update);   

// start: 
// end: 
void checkpoint_33(SingleSwitchProgramEnvironment& env, BotBaseContext& context, EventNotificationOption& notif_status_update);             

// start: 
// end: 
void checkpoint_34(SingleSwitchProgramEnvironment& env, BotBaseContext& context, EventNotificationOption& notif_status_update);  


class AutoStory : public SingleSwitchProgramInstance, public ConfigOption::Listener{
public:
    ~AutoStory();
    AutoStory();

    virtual void program(SingleSwitchProgramEnvironment& env, BotBaseContext& context) override;

    // test the checkpoints from start to end, inclusive
    // test each checkpoints "loop" number of times
    void test_checkpoints(
        SingleSwitchProgramEnvironment& env,
        ConsoleHandle& console, 
        BotBaseContext& context,
        int start, int end, 
        int loop, int start_loop, int end_loop
    );

    void run_autostory(SingleSwitchProgramEnvironment& env, BotBaseContext& context);

private:
    virtual void value_changed(void* object) override;

    std::string start_segment_description();
    std::string end_segment_description();

private:
    OCR::LanguageOCROption LANGUAGE;

    EnumDropdownOption<StartPoint> STARTPOINT;

    enum class EndPoint{
        INTRO_CUTSCENE,
        PICK_STARTER,
        NEMONA_FIRST_BATTLE,
        CATCH_TUTORIAL,
        LEGENDARY_RESCUE,
        ARVEN_FIRST_BATTLE,
        LOS_PLATOS,
        MESAGOZA_SOUTH,
    };
    EnumDropdownOption<EndPoint> ENDPOINT;

    StaticTextOption START_DESCRIPTION;
    StaticTextOption END_DESCRIPTION;


    EnumDropdownOption<StarterChoice> STARTERCHOICE;

    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;

    EventNotificationOption NOTIFICATION_STATUS_UPDATE;
    EventNotificationsOption NOTIFICATIONS;
    
    SectionDividerOption m_advanced_options;
    BooleanCheckBoxOption CHANGE_SETTINGS;    

    BooleanCheckBoxOption ENABLE_TEST_CHECKPOINTS;    
    SimpleIntegerOption<uint16_t> START_CHECKPOINT;
    SimpleIntegerOption<uint16_t> END_CHECKPOINT;
    SimpleIntegerOption<uint16_t> LOOP_CHECKPOINT;
    SimpleIntegerOption<uint16_t> START_LOOP;
    SimpleIntegerOption<uint16_t> END_LOOP;

    BooleanCheckBoxOption ENABLE_TEST_REALIGN;    
    EnumDropdownOption<PlayerRealignMode> REALIGN_MODE;
    SimpleIntegerOption<uint16_t> X_REALIGN;
    SimpleIntegerOption<uint16_t> Y_REALIGN;
    SimpleIntegerOption<uint16_t> REALIGN_DURATION;

    BooleanCheckBoxOption ENABLE_TEST_OVERWORLD_MOVE;    
    SimpleIntegerOption<uint16_t> FORWARD_TICKS;

    BooleanCheckBoxOption TEST_PBF_LEFT_JOYSTICK;
    SimpleIntegerOption<uint16_t> X_MOVE;
    SimpleIntegerOption<uint16_t> Y_MOVE;
    SimpleIntegerOption<uint16_t> HOLD_TICKS;
    SimpleIntegerOption<uint16_t> RELEASE_TICKS;
};





}
}
}
#endif
