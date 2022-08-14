/*  Max Lair State Machine
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_MaxLair_StateMachine_H
#define PokemonAutomation_PokemonSwSh_MaxLair_StateMachine_H

#include "Common/Cpp/SpinLock.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "NintendoSwitch/NintendoSwitch_MultiSwitchProgram.h"
#include "PokemonSwSh/Inference/PokemonSwSh_QuantityReader.h"
#include "PokemonSwSh/MaxLair/Options/PokemonSwSh_MaxLair_Options.h"
#include "PokemonSwSh/MaxLair/Options/PokemonSwSh_MaxLair_Options_Consoles.h"
#include "PokemonSwSh/MaxLair/Options/PokemonSwSh_MaxLair_Options_Hosting.h"
#include "PokemonSwSh/MaxLair/Framework/PokemonSwSh_MaxLair_Stats.h"
#include "PokemonSwSh_MaxLair_StateTracker.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


enum class ConsoleState{
    CONSOLE_DEAD,
    POKEMON_SELECT,
    PATH_SELECT,
    ITEM_SELECT,
    MOVE_SELECT,
    CATCH_SELECT,
    POKEMON_SWAP,
    CAUGHT_MENU,
};



enum class StateMachineAction{
    KEEP_GOING,
    DONE_WITH_ADVENTURE,
    STOP_PROGRAM,
    RESET_RECOVER,
};


struct ConsoleRuntime{
    ReadableQuantity999 ore;
    ReadableQuantity999 normal_balls;
    ReadableQuantity999 boss_balls;
};

struct AdventureRuntime{
    AdventureRuntime(
        const size_t p_host_index,
        const Consoles& p_console_settings,
        const EndBattleDecider& p_actions,
        const bool p_go_home_when_done,
        HostingSettings& p_hosting_settings,
        EventNotificationOption& p_notification_status,
        EventNotificationOption& p_notification_shiny,
        Stats& p_session_stats
    )
        : host_index(p_host_index)
        , console_settings(p_console_settings)
        , actions(p_actions)
        , go_home_when_done(p_go_home_when_done)
        , hosting_settings(p_hosting_settings)
        , notification_status(p_notification_status)
        , notification_shiny(p_notification_shiny)
        , session_stats(p_session_stats)
    {}

    const size_t host_index;
    const Consoles& console_settings;
    const EndBattleDecider& actions;
    const bool go_home_when_done;
    HostingSettings& hosting_settings;
    EventNotificationOption& notification_status;
    EventNotificationOption& notification_shiny;
    Stats& session_stats;

    PathStats path_stats;
    std::string last_boss;
    ConsoleRuntime consoles[4];

    //  State lock to protect fields of this class.
    SpinLock m_lock;

    //  A lock to allow timed-serialization of Switches.
    //  For example: Don't let multiple Switches simultaneously choose to swap with Pokemon.
    std::mutex m_delay_lock;
};


//  Return true if done.
StateMachineAction run_state_iteration(
    AdventureRuntime& runtime, size_t console_index,
    ProgramEnvironment& env, ConsoleHandle& console, BotBaseContext& context,
    bool save_path,
    GlobalStateTracker& state_tracker,
    const EndBattleDecider& boss_action,
    const ImageViewRGB32& entrance
);



}
}
}
}
#endif
