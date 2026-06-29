/*  Shiny Hunt - Overworld Reset
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLZA_ShinyHunt_OverworldReset_H
#define PokemonAutomation_PokemonLZA_ShinyHunt_OverworldReset_H

#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/TimeDurationOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "VideoGames/PokemonLA/Options/PokemonLA_ShinyDetectedAction.h"
#include "VideoGames/PokemonLZA/Options/PokemonLZA_ShinyDetectedAction.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{




class ShinyHunt_OverworldReset_Descriptor : public SingleSwitchProgramDescriptor{
public:
    ShinyHunt_OverworldReset_Descriptor();

    class Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};


class ShinyHunt_OverworldReset : public SingleSwitchProgramInstance, ConfigOption::Listener{
public:
    using Descriptor = ShinyHunt_OverworldReset_Descriptor;
    ShinyHunt_OverworldReset();
    virtual ~ShinyHunt_OverworldReset();

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    //  How the player moves after entering the game to close any
    //  gap to target spawns.
    enum class Movement{
        NONE,
        RUN,
        ROLL,
    };

    //  Direction to run, relative to the camera.
    enum class RunDirection{
        FORWARD,
        BACKWARD,
        LEFT,
        RIGHT,
    };

    //  Show only the movement options that apply to the selected `MOVEMENT`.
    //  Controls the visibility of RUN_DIRECTION, RUN_DURATION and ROLLS.
    virtual void on_config_value_changed(void* object) override;

    //  Perform the movement selected by `MOVEMENT` after the game loads.
    void move_after_loading(ProControllerContext& context);

    PokemonLA::ShinyRequiresAudioText SHINY_REQUIRES_AUDIO;

    MillisecondsOption RESET_DELAY;
    BooleanCheckBoxOption ROTATE_CAMERA;

    EnumDropdownOption<Movement> MOVEMENT;
    EnumDropdownOption<RunDirection> RUN_DIRECTION;
    MillisecondsOption RUN_DURATION;
    SimpleIntegerOption<uint8_t> ROLLS;

    ShinySoundDetectedActionOption SHINY_DETECTED;

    EventNotificationOption NOTIFICATION_STATUS;
    EventNotificationsOption NOTIFICATIONS;
};





}
}
}
#endif
