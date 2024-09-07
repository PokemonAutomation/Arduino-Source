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

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

enum class PlayerRealignMode{
    REALIGN_NEW_MARKER,
    REALIGN_OLD_MARKER,
    REALIGN_NO_MARKER,
};

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

enum class NavigationStopCondition{
    STOP_DIALOG,
};

enum class NavigationMovementMode{
    DIRECTIONAL_ONLY,
    DIRECTIONAL_SPAM_A,
};

enum class ClearDialogCallback{
    ADVANCE_DIALOG,
    OVERWORLD,
    PROMPT_DIALOG,
    WHITE_A_BUTTON,
    DIALOG_ARROW,
    BATTLE,
    TUTORIAL,
};

class AutoStory_Descriptor : public SingleSwitchProgramDescriptor{
public:
    AutoStory_Descriptor();

    struct Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};

// align player orientation based on the alignment mode
// The direction is specified by (x, y):
// x = 0 : left
// x = 128 : neutral
// x = 255 : right
// y = 0 : up
// y = 128 : neutral
// y = 255 : down
// - REALIGN_NEW_MARKER: place down a map marker, which will align the player towards the marker
// location of the marker is set with move_x, move_y, move_duration
// - REALIGN_OLD_MARKER: assuming a marker is already set, open and close the map, 
// which will align the player towards the marker
// - REALIGN_NO_MARKER: move player towards in the direction set by move_x, move_y, move_duration
// then re-align the camera
void realign_player(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context,
    PlayerRealignMode realign_mode,
    uint8_t move_x = 0, uint8_t move_y = 0, uint8_t move_duration = 0
);

// spam A button to choose the first move
// throw exception if wipeout.
void run_battle(ConsoleHandle& console, BotBaseContext& context,
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
    uint16_t seconds_timeout = 60, uint16_t seconds_realign = 60
);

void walk_forward_until_dialog(
    const ProgramInfo& info, 
    ConsoleHandle& console, 
    BotBaseContext& context,
    NavigationMovementMode movement_mode,
    uint16_t seconds_timeout = 10,
    uint8_t y = 0
);

// walk forward while using lets go to clear the path
// forward_ticks: number of ticks to walk forward
// y = 0: walks forward. y = 128: stand in place. y = 255: walk backwards (towards camera)
// ticks_between_lets_go: number of ticks between firing off Let's go to clear the path from wild pokemon
// delay_after_lets_go: number of ticks to wait after firing off Let's go.
void walk_forward_while_clear_front_path(
    const ProgramInfo& info, 
    ConsoleHandle& console, 
    BotBaseContext& context,
    uint16_t forward_ticks,
    uint8_t y = 0,
    uint16_t ticks_between_lets_go = 125,
    uint16_t delay_after_lets_go = 250
);

// mashes A button by default
void mash_button_till_overworld(
    ConsoleHandle& console, 
    BotBaseContext& context, 
    uint16_t button = BUTTON_A, uint16_t seconds_run = 360
);

// fly to the pokecenter that overlaps with the player on the map, and return true.
// if no overlapping pokecenter, return false.
bool fly_to_overlapping_pokecenter(
    const ProgramInfo& info, 
    ConsoleHandle& console, 
    BotBaseContext& context
);

void config_option(BotBaseContext& context, int change_option_value);

// enter menu and swap the first and third moves for your starter
void swap_starter_moves(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context, Language language);

// enter menu and move the cursor the given side, and index. then press the A button
void enter_menu_from_overworld(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context,
    int menu_index,
    MenuSide side = MenuSide::RIGHT,
    bool has_minimap = true
);

// run the given `action`. if detect a battle, stop the action, and throw exception
void do_action_and_monitor_for_battles(
    ProgramEnvironment& env, 
    ConsoleHandle& console,
    BotBaseContext& context,
    std::function<
        void(ProgramEnvironment& env,
        ConsoleHandle& console,
        BotBaseContext& context)
    >&& action
);

// heal at the pokecenter, that your character is currently at.
// if not currently at the pokecenter, throws error.
void heal_at_pokecenter(
    const ProgramInfo& info, 
    ConsoleHandle& console, 
    BotBaseContext& context
);



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
        int start, int end, int loop = 1
    );

    void change_settings_prior_to_autostory(SingleSwitchProgramEnvironment& env, BotBaseContext& context);

    void enter_options_menu_from_overworld(SingleSwitchProgramEnvironment& env, BotBaseContext& context);

    // from within the Settings/Options menu, change the settings
    void change_settings(SingleSwitchProgramEnvironment& env, BotBaseContext& context, bool use_inference = true);

    void run_autostory(SingleSwitchProgramEnvironment& env, BotBaseContext& context);

    void checkpoint_save(SingleSwitchProgramEnvironment& env, BotBaseContext& context);

    // start: after selecting character name, style and the cutscene has started
    // end: stood up from chair. Walked to left side of room.
    void checkpoint_00(SingleSwitchProgramEnvironment& env, BotBaseContext& context);

    // start: stood up from chair. Walked to left side of room.
    // end: standing in room. updated settings
    void checkpoint_01(SingleSwitchProgramEnvironment& env, BotBaseContext& context);
    
    // start: standing in room. updated settings
    // end: standing in front of power of science NPC. Cleared map tutorial.
    void checkpoint_02(SingleSwitchProgramEnvironment& env, BotBaseContext& context);

    // start: standing in front of power of science NPC. Cleared map tutorial.
    // end: received starter, changed move order
    void checkpoint_03(SingleSwitchProgramEnvironment& env, BotBaseContext& context);

    // start: Received starter pokemon and changed move order. Cleared autoheal tutorial.
    // end: Battled Nemona on the beach.
    void checkpoint_04(SingleSwitchProgramEnvironment& env, BotBaseContext& context);

    // start: Battled Nemona on the beach.
    // end: Met mom at gate. Received mom's sandwich.
    void checkpoint_05(SingleSwitchProgramEnvironment& env, BotBaseContext& context);

    // start: Met mom at gate. Received mom's sandwich.
    // end: Cleared catch tutorial.
    void checkpoint_06(SingleSwitchProgramEnvironment& env, BotBaseContext& context);

    // start: Cleared catch tutorial.
    // end: Moved to cliff. Heard mystery cry.
    void checkpoint_07(SingleSwitchProgramEnvironment& env, BotBaseContext& context);

    // start: Moved to cliff. Heard mystery cry.
    // end: Rescued Koraidon/Miraidon and escaped from the Houndoom Cave.
    void checkpoint_08(SingleSwitchProgramEnvironment& env, BotBaseContext& context);

    // start: Rescued Koraidon/Miraidon and escaped from the Houndoom Cave.
    // end: Battled Arven and received Legendary's Pokeball.
    void checkpoint_09(SingleSwitchProgramEnvironment& env, BotBaseContext& context);

    // start: Battled Arven and received Legendary's Pokeball.
    // end: Talked to Nemona at the Lighthouse.
    void checkpoint_10(SingleSwitchProgramEnvironment& env, BotBaseContext& context);

    // start: Talked to Nemona at the Lighthouse.
    // end: Arrived at Los Platos pokecenter. Cleared Let's go tutorial.
    void checkpoint_11(SingleSwitchProgramEnvironment& env, BotBaseContext& context);

    // start: Arrived at Los Platos pokecenter. Cleared Let's go tutorial.
    // end: Arrived at Mesagoza (South) Pokecenter
    void checkpoint_12(SingleSwitchProgramEnvironment& env, BotBaseContext& context);

    // start: Arrived at Mesagoza (South) Pokecenter
    // end: Battled Nemona at Mesagoza gate. Entered Mesagoza.
    void checkpoint_13(SingleSwitchProgramEnvironment& env, BotBaseContext& context);

    // start: Battled Nemona at Mesagoza gate. Entered Mesagoza.
    // end: 
    void checkpoint_14(SingleSwitchProgramEnvironment& env, BotBaseContext& context);

    // start: 
    // end: 
    void checkpoint_15(SingleSwitchProgramEnvironment& env, BotBaseContext& context);

    // start: 
    // end: 
    void checkpoint_16(SingleSwitchProgramEnvironment& env, BotBaseContext& context);

private:
    virtual void value_changed(void* object) override;

    std::string start_segment_description();
    std::string end_segment_description();

    // return the index for `Options` in the Main Menu
    int8_t option_index();

private:
    OCR::LanguageOCROption LANGUAGE;
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

    enum class StarterChoice{
        SPRIGATITO,
        FUECOCO,
        QUAXLY,
    };
    EnumDropdownOption<StarterChoice> STARTERCHOICE;

private:
    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;

    EventNotificationOption NOTIFICATION_STATUS_UPDATE;
    EventNotificationsOption NOTIFICATIONS;
    
    SectionDividerOption m_advanced_options;
    BooleanCheckBoxOption CHANGE_SETTINGS;    

    BooleanCheckBoxOption ENABLE_TEST_CHECKPOINTS;    
    SimpleIntegerOption<uint16_t> START_CHECKPOINT;
    SimpleIntegerOption<uint16_t> END_CHECKPOINT;
    SimpleIntegerOption<uint16_t> LOOP_CHECKPOINT;

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
