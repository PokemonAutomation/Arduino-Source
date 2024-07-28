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
