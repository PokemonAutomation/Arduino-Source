/*  Egg Hatcher
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonFramework/Tools/StatsTracking.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSV/Inference/PokemonSV_BoxDetection.h"
#include "PokemonSV/Inference/PokemonSV_DialogDetector.h"
#include "PokemonSV/Inference/PokemonSV_EggDetector.h"
#include "PokemonSV/Programs/PokemonSV_Navigation.h"
#include "PokemonSV/Programs/Eggs/PokemonSV_EggRoutines.h"
#include "PokemonSV/Programs/Boxes/PokemonSV_BoxRoutines.h"
#include "PokemonSV_EggHatcher.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

using namespace Pokemon;


EggHatcher_Descriptor::EggHatcher_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSV:EggHatcher",
        STRING_POKEMON + " SV", "Egg Hatcher",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSV/EggHatcher.md",
        "Automatically hatch eggs from boxes.",
        FeedbackType::REQUIRED, false,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}
struct EggHatcher_Descriptor::Stats : public StatsTracker{
    Stats()
        : m_hatched(m_stats["Hatched"])
        , m_errors(m_stats["Errors"])
    {
        m_display_order.emplace_back("Hatched");
        m_display_order.emplace_back("Errors", true);
    }
    std::atomic<uint64_t>& m_hatched;
    std::atomic<uint64_t>& m_errors;
};
std::unique_ptr<StatsTracker> EggHatcher_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}



EggHatcher::EggHatcher()
    : GO_HOME_WHEN_DONE(false)
    , BOXES(
        "<b>How many boxes of eggs to hatch:</b>",
        LockWhileRunning::LOCKED,
        1, 1, 32
    )
    , HAS_CLONE_RIDE_POKEMON(
        "<b>Have cloned ride " + STRING_POKEMON + " as second in party:</b>"
        "Ride legendary cannot be cloned after patch 1.1.0. To preserve the existing clone while hatching eggs, "
        "place it as second in party before starting the program.</b>"
        "The program will skip the first row of eggs in the box as a result.",
        LockWhileRunning::LOCKED,
        false)
    , NOTIFICATION_STATUS_UPDATE("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS_UPDATE,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(BOXES);
    PA_ADD_OPTION(HAS_CLONE_RIDE_POKEMON);
    PA_ADD_OPTION(NOTIFICATIONS);
}

void EggHatcher::hatch_one_box(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    EggHatcher_Descriptor::Stats& stats = env.current_stats<EggHatcher_Descriptor::Stats>();

    for(uint8_t column_index = 0; column_index < 6; column_index++){
        uint8_t num_eggs = 0, num_non_egg_pokemon = 0;
        {
            const uint8_t expected_empty_slots_in_party = HAS_CLONE_RIDE_POKEMON ? 4 : 5;
            if (check_empty_slots_in_party(env.program_info(), env.console, context) != expected_empty_slots_in_party){
                dump_image(env.logger(), env.program_info(), "PartyNotEmpty", env.console.video().snapshot());
                throw FatalProgramException(env.logger(), "party not empty when loading one column for hatching.");
            }
        }

        load_one_column_to_party(env.program_info(), env.console, context, column_index, HAS_CLONE_RIDE_POKEMON);

        std::tie(num_eggs, num_non_egg_pokemon) = check_egg_party_column(env.program_info(), env.console, context);
        if (num_eggs == 0){
            if (num_non_egg_pokemon == 0){
                // nothing in this column
                env.log("Nothing in column " + std::to_string(column_index+1) + ".");
                env.console.overlay().add_log("Empty column", COLOR_WHITE);
                continue;
            }

            // we have only non-egg pokemon in the column
            // Move them back
            env.log("Only non-egg pokemon in column, move them back.");
            env.console.overlay().add_log("No egg in column", COLOR_WHITE);
            unload_one_column_from_party(env.program_info(), env.console, context, column_index, HAS_CLONE_RIDE_POKEMON);
            continue;
        }
        
        env.log("Loaded " + std::to_string(num_eggs) + " eggs to party.");
        env.console.overlay().add_log("Load " + std::to_string(num_eggs) + " eggs", COLOR_WHITE);
        leave_box_system_to_overworld(env.program_info(), env.console, context);

        auto hatched_callback = [&](uint8_t){  
            stats.m_hatched++;
            env.update_stats();
        };
        
        hatch_eggs_at_zero_gate(env.program_info(), env.console, context, num_eggs, hatched_callback);

        reset_position_at_zero_gate(env.program_info(), env.console, context);

        enter_box_system_from_overworld(env.program_info(), env.console, context);

        std::tie(num_eggs, num_non_egg_pokemon) = check_egg_party_column(env.program_info(), env.console, context);
        if (num_eggs > 0){
            dump_image(env.logger(), env.program_info(), "EggInPartyAfterHatching", env.console.video().snapshot());
            throw FatalProgramException(env.logger(), "detected egg in party after hatching.");
        }

        unload_one_column_from_party(env.program_info(), env.console, context, column_index, HAS_CLONE_RIDE_POKEMON);
    }

    context.wait_for_all_requests();
}

void EggHatcher::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    EggHatcher_Descriptor::Stats& stats = env.current_stats<EggHatcher_Descriptor::Stats>();
    //  Connect the controller.
    pbf_press_button(context, BUTTON_LCLICK, 10, 0);

    try{
        enter_box_system_from_overworld(env.program_info(), env.console, context);
        // // Wait one second to let game load box UI
        // context.wait_for(std::chrono::seconds(1));

        for (uint8_t i = 0; i < BOXES; i++){
            if (i > 0){
                env.log("Go to next box.");
                env.console.overlay().add_log("Next box", COLOR_WHITE);
                pbf_press_button(context, BUTTON_R, 40, 80);
            }
            context.wait_for_all_requests();

            send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE);

            hatch_one_box(env, context);
        }
    } catch(OperationFailedException& e){
        stats.m_errors++;
        env.update_stats();
        throw e;
    }

    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}






}
}
}
