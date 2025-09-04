/*  Egg Fetcher
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonTools/StartupChecks/VideoResolutionCheck.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSV/Inference/Boxes/PokemonSV_IvJudgeReader.h"
#include "PokemonSV/Programs/Eggs/PokemonSV_EggRoutines.h"
#include "PokemonSV/Programs/PokemonSV_WorldNavigation.h"
#include "PokemonSV_EggFetcher.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

using namespace Pokemon;


EggFetcher_Descriptor::EggFetcher_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSV:EggFetcher",
        STRING_POKEMON + " SV", "Egg Fetcher",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSV/EggFetcher.md",
        "Automatically fetch eggs from a picnic.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}
struct EggFetcher_Descriptor::Stats : public StatsTracker{
    Stats()
        : m_sandwiches(m_stats["Sandwiches"])
        , m_attempts(m_stats["Fetch Attempts"])
        , m_eggs(m_stats["Eggs"])
        , m_errors(m_stats["Errors"])
    {
        m_display_order.emplace_back("Sandwiches");
        m_display_order.emplace_back("Fetch Attempts");
        m_display_order.emplace_back("Eggs");
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);
    }
    std::atomic<uint64_t>& m_sandwiches;
    std::atomic<uint64_t>& m_attempts;
    std::atomic<uint64_t>& m_eggs;
    std::atomic<uint64_t>& m_errors;
};
std::unique_ptr<StatsTracker> EggFetcher_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}



EggFetcher::EggFetcher()
    : GO_HOME_WHEN_DONE(false)
    , LANGUAGE(
        "<b>Game Language:</b>",
        IV_READER().languages(),
        LockMode::LOCK_WHILE_RUNNING,
        true
    )
    , EGGS_TO_FETCH(
        "<b>Fetch this many eggs:</b>",
        LockMode::LOCK_WHILE_RUNNING,
        900
    )
    , NOTIFICATION_STATUS_UPDATE("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS_UPDATE,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(LANGUAGE);
    PA_ADD_OPTION(EGGS_TO_FETCH);
    PA_ADD_OPTION(EGG_SANDWICH);
    PA_ADD_OPTION(NOTIFICATIONS);
}


void EggFetcher::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    assert_16_9_720p_min(env.logger(), env.console);

    EggFetcher_Descriptor::Stats& stats = env.current_stats<EggFetcher_Descriptor::Stats>();

    //  Connect the controller.
    pbf_press_button(context, BUTTON_L, 10, 100);

    size_t num_eggs_collected = 0;

    try{
        for(uint16_t i = 0; i < EGG_SANDWICH.MAX_NUM_SANDWICHES; i++){
            send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE);

            picnic_at_zero_gate(env.program_info(), env.console, context);
            // Now we are at picnic. We are at one end of picnic table while the egg basket is at the other end

#if 1
            bool can_make_sandwich = eat_egg_sandwich_at_picnic(env, env.console, context,
                EGG_SANDWICH.EGG_SANDWICH_TYPE, LANGUAGE);
            if (can_make_sandwich == false){
                throw UserSetupError(env.console, "No sandwich recipe or ingredients. Cannot open and select the sandwich recipe.");
            }
#endif
            stats.m_sandwiches++;
            env.update_stats();

            // move past the table and collect eggs
            auto basket_check_callback = [&](size_t new_eggs){
                stats.m_attempts++;
                stats.m_eggs += new_eggs;
                env.update_stats();
            };

            const size_t basket_wait_seconds = (EGG_SANDWICH.EGG_SANDWICH_TYPE == EggSandwichType::GREAT_PEANUT_BUTTER ? 180 : 120);
            collect_eggs_after_sandwich(
                env.program_info(),
                env.console, context,
                basket_wait_seconds,
                EGGS_TO_FETCH,
                num_eggs_collected,
                basket_check_callback
            );

            leave_picnic(env.program_info(), env.console, context);
            
            // Reset position to flying spot:
            reset_position_at_zero_gate(env.program_info(), env.console, context);

            if (num_eggs_collected >= EGGS_TO_FETCH){
                break;
            }
        }
    } catch(OperationFailedException&){
        stats.m_errors++;
        env.update_stats();
        throw;
    }

    env.update_stats();
    GO_HOME_WHEN_DONE.run_end_of_program(context);
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}






}
}
}
