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

void realign_player(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context,
    PlayerRealignMode realign_mode,
    uint8_t move_x = 0, uint8_t move_y = 0, uint8_t move_duration = 0
);

bool run_battle(ConsoleHandle& console, BotBaseContext& context,
    BattleStopCondition stop_condition
);

bool clear_dialog(ConsoleHandle& console, BotBaseContext& context,
    ClearDialogMode mode, uint16_t seconds_timeout = 60
);

bool overworld_navigation(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context,
    NavigationStopCondition stop_condition,
    NavigationMovementMode movement_mode,
    uint8_t x, uint8_t y,
    uint16_t seconds_timeout = 60, uint16_t seconds_realign = 60
);

void mash_button_till_overworld(
    ConsoleHandle& console, 
    BotBaseContext& context, 
    uint16_t button = BUTTON_A, uint16_t seconds_run = 360
);

void reset_game(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context, const std::string& error_msg);

void config_option(BotBaseContext& context, int change_option_value);

void enter_menu_from_overworld(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context,
    int menu_index,
    MenuSide side = MenuSide::RIGHT,
    bool has_minimap = true
);


class AutoStory : public SingleSwitchProgramInstance, public ConfigOption::Listener{
public:
    ~AutoStory();
    AutoStory();

    virtual void program(SingleSwitchProgramEnvironment& env, BotBaseContext& context) override;



private:
    virtual void value_changed(void* object) override;

private:
    enum class StartPoint{
        INTRO_CUTSCENE,
        IN_ROOM,
        NEMONA_FIRST_BATTLE,
        CATCH_TUTORIAL,
        LEGENDARY_RESCUE,
        ARVEN_FIRST_BATTLE,
        LOS_PLATOS,
        MESAGOZA_SOUTH,
    };
    EnumDropdownOption<StartPoint> STARTPOINT;

    enum class EndPoint{
        IN_ROOM,
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
