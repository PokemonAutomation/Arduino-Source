/*  Max Lair
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exception.h"
#include "Common/SwitchFramework/Switch_PushButtons.h"
#include "Common/SwitchRoutines/SwitchDigitEntry.h"
#include "CommonFramework/Tools/InterruptableCommands.h"
#include "CommonFramework/Inference/VisualInferenceSession.h"
#include "Pokemon/Inference/Pokemon_NameReader.h"
#include "PokemonSwSh/Inference/Dens/PokemonSwSh_DenMonReader.h"
#include "PokemonSwSh_MaxLair_State.h"
#include "PokemonSwSh/Programs/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_ItemSelectMenu.h"
#include "PokemonSwSh/Programs/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_BattleMenu.h"
#include "PokemonSwSh/Programs/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_PathSelect.h"
#include "PokemonSwSh/Programs/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_EndBattle.h"
#include "Program/PokemonSwSh_MaxLair_Run_Start.h"
#include "Program/PokemonSwSh_MaxLair_Run_PokemonSelect.h"
#include "Program/PokemonSwSh_MaxLair_Run_PathSelect.h"
#include "Program/PokemonSwSh_MaxLair_Run_Battle.h"
#include "Program/PokemonSwSh_MaxLair_Run_PokemonSwap.h"
#include "PokemonSwSh_MaxLair_StateMachine.h"
#include "PokemonSwSh_MaxLair_SingleRun.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
using namespace MaxLairInternal;



MaxLairSingleRun_Descriptor::MaxLairSingleRun_Descriptor()
    : MultiSwitchProgramDescriptor(
        "PokemonSwSh:MaxLair",
//        "Auto Max Lair 2.0",
        "Max Lair: Single Playthrough (WIP)",
        "SwSh-Arduino/wiki/Advanced:-MaxLair",
        "Auto Max Lair 2.0 - Work in progress...",
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
            "Slot 1",
            "Slot 2",
            "Slot 3",
        },
        0
    )
    , RAID_CODE("<b>Raid Code:</b><br>This is not used if there is only one Switch.", 8, "9107 3091")
    , m_languages(Pokemon::PokemonNameReader::instance().languages())
    , PLAYER0("Switch 0 (Top Left)", m_languages)
    , PLAYER1("Switch 1 (Top Right)", m_languages)
    , PLAYER2("Switch 2 (Bottom Left)", m_languages)
    , PLAYER3("Switch 3 (Bottom Right)", m_languages)
//    , LANGUAGE0("<b>Switch 0 Language:</b>", m_languages, true)
//    , LANGUAGE1("<b>Switch 1 Language:</b><br>This is ignored if there is no Switch 1.", m_languages, true)
//    , LANGUAGE2("<b>Switch 2 Language:</b><br>This is ignored if there is no Switch 2.", m_languages, true)
//    , LANGUAGE3("<b>Switch 3 Language:</b><br>This is ignored if there is no Switch 3.", m_languages, true)
{
    PA_ADD_OPTION(HOST_SWITCH);
    PA_ADD_OPTION(BOSS_SLOT);
    PA_ADD_OPTION(RAID_CODE);

    PA_ADD_OPTION(PLAYER0);
    PA_ADD_OPTION(PLAYER1);
    PA_ADD_OPTION(PLAYER2);
    PA_ADD_OPTION(PLAYER3);
//    PA_ADD_OPTION(LANGUAGE0);
//    PA_ADD_OPTION(LANGUAGE1);
//    PA_ADD_OPTION(LANGUAGE2);
//    PA_ADD_OPTION(LANGUAGE3);
}

bool MaxLairSingleRun::is_valid() const{
    return RunnablePanelInstance::is_valid() && HOST_SWITCH < count();
}




void MaxLairSingleRun::program(MultiSwitchProgramEnvironment& env){
    if (HOST_SWITCH >= env.consoles.size()){
        PA_THROW_StringException("Invalid Host Switch");
    }
    ConsoleHandle& host = env.consoles[HOST_SWITCH];

    uint8_t code[8];
    RAID_CODE.to_str(code);

    const MaxLairPlayerOptions* player_settings[4]{
        &PLAYER0,
        &PLAYER1,
        &PLAYER2,
        &PLAYER3,
    };


    GlobalStateTracker state_tracker(env.consoles.size());

    start_raid(env, state_tracker, host, BOSS_SLOT, code);

    env.run_in_parallel([&](ConsoleHandle& console){
        Action action = Action::CONTINUE;
        do{
            env.log("State Tracker\n" + state_tracker.dump());

            action = run_state_iteration(
                *player_settings[console.index()],
                env, console,
                state_tracker
            );
        }while (action == Action::CONTINUE);

        env.log("End of adventure.");
    });


}



}
}
}




















