/*  Autonomous Ball Thrower
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/Exceptions/FatalProgramException.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "CommonFramework/Tools/StatsTracking.h"
#include "CommonFramework/Tools/VideoResolutionCheck.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Inference/Pokemon_NameReader.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSV/Inference/Battles/PokemonSV_BattleBallReader.h"
#include "PokemonSV/Inference/Battles/PokemonSV_BattleMenuDetector.h"
#include "PokemonSV/Programs/PokemonSV_BasicCatcher.h"
#include "PokemonSV_AutonomousBallThrower.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

using namespace Pokemon;


AutonomousBallThrower_Descriptor::AutonomousBallThrower_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSV:AutonomousBallThrower",
        STRING_POKEMON + " SV", "Autonomous Ball Thrower",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSV/AutonomousBallThrower.md",
        "Repeatedly throw a ball until you catch the pokemon.",
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}
struct AutonomousBallThrower_Descriptor::Stats : public StatsTracker{
    Stats()
        : m_balls(m_stats["Balls Thrown"])
    {
        m_display_order.emplace_back("Balls Thrown");
    }
    std::atomic<uint64_t>& m_balls;
};
std::unique_ptr<StatsTracker> AutonomousBallThrower_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}



AutonomousBallThrower::AutonomousBallThrower()
    : LANGUAGE(
        "<b>Game Language:</b>",
        PokemonNameReader::instance().languages(),
        LockWhileRunning::UNLOCKED
    )
    , BALL_SELECT(
        "<b>Ball Select:</b>",
        LockWhileRunning::UNLOCKED,
        "poke-ball"
    )
    , NOTIFICATION_STATUS_UPDATE("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS_UPDATE,
    })
{
    PA_ADD_OPTION(LANGUAGE);
    PA_ADD_OPTION(BALL_SELECT);
    PA_ADD_OPTION(NOTIFICATIONS);
}



void AutonomousBallThrower::throw_ball(ConsoleHandle& console, BotBaseContext& context){

    pbf_press_button(context, BUTTON_X, 20, 100);
    context.wait_for_all_requests();

    BattleBallReader reader(console, LANGUAGE);
    int quantity = move_to_ball(reader, console, context, BALL_SELECT.slug());
    if (quantity == 0) {
        throw FatalProgramException(console, "Unable to find appropriate ball. Did you run out?", true);
    }
    if (quantity < 0) {
        console.log("Unable to read ball quantity.", COLOR_RED);
    }
    pbf_mash_button(context, BUTTON_A, 125);
    context.wait_for_all_requests();
}

void AutonomousBallThrower::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    assert_16_9_720p_min(env.logger(), env.console);

    AutonomousBallThrower_Descriptor::Stats& stats = env.current_stats<AutonomousBallThrower_Descriptor::Stats>();

    //  Connect the controller.
    pbf_press_button(context, BUTTON_LCLICK, 10, 0);

    while (true)
    {
        NormalBattleMenuWatcher battle_menu(COLOR_RED);
        int ret = wait_until(
            env.console, context,
            std::chrono::seconds(120),
            {
                battle_menu,
            }
        );

        context.wait_for(std::chrono::milliseconds(100));
        switch (ret){
        case 0:
            throw_ball(env.console, context);
            stats.m_balls++;
            env.update_stats();
            break;
        default:
            throw FatalProgramException(env.console, "Failed to detect battle menu after 60 seconds, did you catch or fail?", true);
        }

    }

    env.update_stats();
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}



}
}
}
