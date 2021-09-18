/*  Max Lair
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exception.h"
#include "Common/SwitchFramework/Switch_PushButtons.h"
#include "Common/SwitchRoutines/SwitchDigitEntry.h"
#include "Pokemon/Inference/Pokemon_NameReader.h"
#include "Framework/PokemonSwSh_MaxLair_StateMachine.h"
#include "Program/PokemonSwSh_MaxLair_Run_Start.h"
#include "PokemonSwSh_MaxLair_SingleRun.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
using namespace MaxLairInternal;



MaxLairSingleRun_Descriptor::MaxLairSingleRun_Descriptor()
    : MultiSwitchProgramDescriptor(
        "PokemonSwSh:MaxLair-SingleRun",
        "Max Lair: Single Adventure (WIP)",
        "SwSh-Arduino/wiki/Advanced:-MaxLair-SingleRun",
        "Auto Max Lair 2.0 - Run a single Dynamax Adventure and stop.",
        FeedbackType::REQUIRED,
        PABotBaseLevel::PABOTBASE_12KB,
        1, 4, 1
    )
{}


MaxLairSingleRun::MaxLairSingleRun(const MaxLairSingleRun_Descriptor& descriptor)
    : MultiSwitchProgramInstance(descriptor)
    , HOST_SWITCH(
        "<b>Host Switch:</b><br>This is the Switch that hosts the raid.",
        {
            "Switch 0 (Top Left)",
            "Switch 1 (Top Right)",
            "Switch 2 (Bottom Left)",
            "Switch 3 (Bottom Right)",
        },
        0
    )
    , BOSS_SLOT(
        "<b>Boss Slot:</b>",
        {
            "Random",
            "Slot 1",
            "Slot 2",
            "Slot 3",
        },
        0
    )
    , RAID_CODE("<b>Raid Code:</b><br>This is not used if there is only one Switch.", 8, "9107 3091")
    , SCREENSHOT("<b>Attach Shiny Screenshot:</b><br>Attach screenshot of shiny catches to notification.")
    , m_languages(PokemonNameReader::instance().languages())
    , PLAYER0("Switch 0 (Top Left)", m_languages, false)
    , PLAYER1("Switch 1 (Top Right)", m_languages, false)
    , PLAYER2("Switch 2 (Bottom Left)", m_languages, false)
    , PLAYER3("Switch 3 (Bottom Right)", m_languages, false)
{
    PA_ADD_OPTION(HOST_SWITCH);
    PA_ADD_OPTION(BOSS_SLOT);
    PA_ADD_OPTION(RAID_CODE);
    PA_ADD_OPTION(SCREENSHOT);

    PA_ADD_OPTION(PLAYER0);
    PA_ADD_OPTION(PLAYER1);
    PA_ADD_OPTION(PLAYER2);
    PA_ADD_OPTION(PLAYER3);
}

bool MaxLairSingleRun::is_valid() const{
    return RunnablePanelInstance::is_valid() && HOST_SWITCH < count();
}
std::unique_ptr<StatsTracker> MaxLairSingleRun::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}




void MaxLairSingleRun::program(MultiSwitchProgramEnvironment& env){
    if (HOST_SWITCH >= env.consoles.size()){
        PA_THROW_StringException("Invalid Host Switch");
    }
    ConsoleHandle& host = env.consoles[HOST_SWITCH];

    Stats& stats = env.stats<Stats>();
    env.update_stats();

    uint8_t code[8];
    RAID_CODE.to_str(code);

    MaxLairRuntime settings{
        descriptor().display_name(),
        {
            &PLAYER0,
            &PLAYER1,
            &PLAYER2,
            &PLAYER3,
        },
        CaughtScreenAction::ALWAYS_STOP,
        SCREENSHOT,
        stats
    };


    QImage entrance[4];
    GlobalStateTracker state_tracker(env, env.consoles.size());

    start_raid(env, state_tracker, entrance, host, BOSS_SLOT, code);

    std::atomic<bool> stop(false);

    env.run_in_parallel([&](ConsoleHandle& console){
        StateMachineAction action;
        while (true){
            console.log("State Tracker\n" + state_tracker.dump());
            size_t index = console.index();
            action = run_state_iteration(
                settings, index,
                env, console, console.index() == host.index(),
                state_tracker,
                entrance[index]
            );
            switch (action){
            case StateMachineAction::KEEP_GOING:
                continue;
            case StateMachineAction::DONE_WITH_ADVENTURE:
                env.log("End of adventure.", "purple");
                return;
            case StateMachineAction::STOP_PROGRAM:
                env.log("End of adventure. Stop program requested...", "purple");
                stop.store(true, std::memory_order_release);
                return;
            }
        }
    });

    env.update_stats();
}



}
}
}




















