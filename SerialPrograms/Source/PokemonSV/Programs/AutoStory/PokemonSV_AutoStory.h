/*  Shiny Hunt - Area Zero Platform
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
};

enum class NavigationStopCondition{
    STOP_DIALOG,
};

enum class NavigationMovementMode{
    DIRECTIONAL_ONLY,
    DIRECTIONAL_SPAM_A,
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

bool run_battle(ConsoleHandle& console, BotBaseContext& context,
    BattleStopCondition stop_condition
);

// press A to clear tutorial screens
// throw exception if tutorial screen never detected
void clear_tutorial(ConsoleHandle& console, BotBaseContext& context, uint16_t seconds_timeout = 5);

// spam the A button to clear dialog.
// stop depending on ClearDialogMode: stop when detect overworld, or dialog prompt, or A button prompt. Or if times out
// return false if times out, unless this is the intended stop condition.
// also return false if dialog is never detected.
bool clear_dialog(ConsoleHandle& console, BotBaseContext& context,
    ClearDialogMode mode, uint16_t seconds_timeout = 60
);

// move character with ssf left joystick, as per given x, y, until 
// stop_condition is met (e.g. Dialog detected). 
// return false if reaches timeout before detecting stop condition
bool overworld_navigation(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context,
    NavigationStopCondition stop_condition,
    NavigationMovementMode movement_mode,
    uint8_t x, uint8_t y,
    uint16_t seconds_timeout = 60, uint16_t seconds_realign = 60
);

// mashes A button by default
void mash_button_till_overworld(
    ConsoleHandle& console, 
    BotBaseContext& context, 
    uint16_t button = BUTTON_A, uint16_t seconds_run = 360
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



class AutoStory : public SingleSwitchProgramInstance, public ConfigOption::Listener{
public:
    ~AutoStory();
    AutoStory();

    virtual void program(SingleSwitchProgramEnvironment& env, BotBaseContext& context) override;

    // test the segments from start to end, inclusive
    // test each segment "loop" number of times
    void test_segments(
        SingleSwitchProgramEnvironment& env,
        ConsoleHandle& console, 
        BotBaseContext& context,
        int start, int end, int loop = 1
    );

    void run_autostory(SingleSwitchProgramEnvironment& env, BotBaseContext& context);

    void checkpoint_save(SingleSwitchProgramEnvironment& env, BotBaseContext& context);

    // start: after selecting character name, style and the cutscene has started
    // end: stood up from chair. updated settings
    void segment_00(SingleSwitchProgramEnvironment& env, BotBaseContext& context);

    // start: stood up from chair. updated settings
    // end: standing in front of power of science NPC. cleared map tutorial
    void segment_01(SingleSwitchProgramEnvironment& env, BotBaseContext& context);

    // start: standing in front of power of science NPC. cleared map tutorial
    // end: received starter, changed move order
    void segment_02(SingleSwitchProgramEnvironment& env, BotBaseContext& context);

    void segment_03(SingleSwitchProgramEnvironment& env, BotBaseContext& context);

    void segment_04(SingleSwitchProgramEnvironment& env, BotBaseContext& context);

    void segment_05(SingleSwitchProgramEnvironment& env, BotBaseContext& context);

    void segment_06(SingleSwitchProgramEnvironment& env, BotBaseContext& context);

    void segment_07(SingleSwitchProgramEnvironment& env, BotBaseContext& context);

    void segment_08(SingleSwitchProgramEnvironment& env, BotBaseContext& context);

    void segment_09(SingleSwitchProgramEnvironment& env, BotBaseContext& context);

    void segment_10(SingleSwitchProgramEnvironment& env, BotBaseContext& context);

    void segment_11(SingleSwitchProgramEnvironment& env, BotBaseContext& context);

private:
    virtual void value_changed(void* object) override;

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
        PICK_STARTER,
        NEMONA_FIRST_BATTLE,
        CATCH_TUTORIAL,
        LEGENDARY_RESCUE,
        ARVEN_FIRST_BATTLE,
        LOS_PLATOS,
        MESAGOZA_SOUTH,
    };
    EnumDropdownOption<EndPoint> ENDPOINT;

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
};





}
}
}
#endif
