/*  Battles
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <cmath>
#include "CommonFramework/Exceptions/ProgramFinishedException.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Exceptions/FatalProgramException.h"
#include "CommonFramework/Tools/ProgramEnvironment.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_DataTypes.h"
#include "Pokemon/Pokemon_Strings.h"
#include "Pokemon/Pokemon_Notification.h"
#include "PokemonSV/Options/PokemonSV_EncounterBotCommon.h"
#include "PokemonSV/Inference/PokemonSV_MainMenuDetector.h"
#include "PokemonSV/Inference/Dialogs/PokemonSV_DialogDetector.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_OverworldDetector.h"
#include "PokemonSV/Inference/Battles/PokemonSV_NormalBattleMenus.h"
#include "PokemonSV_BasicCatcher.h"
#include "PokemonSV_Battles.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

using namespace Pokemon;


void auto_heal_from_menu_or_overworld(
    const ProgramInfo& info, VideoStream& stream, ProControllerContext& context,
    uint8_t party_slot,
    bool return_to_overworld
){
    stream.log("Auto-healing...");
    WallClock start = current_time();
    bool healed = false;
    while (true){
        if (current_time() - start > std::chrono::minutes(5)){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "auto_heal_from_menu(): Failed auto-heal after 5 minutes.",
                stream
            );
        }

        OverworldWatcher overworld(stream.logger(), COLOR_RED);
        MainMenuWatcher main_menu(COLOR_CYAN);
        AdvanceDialogWatcher dialog(COLOR_YELLOW);
        context.wait_for_all_requests();
        int ret = wait_until(
            stream, context, std::chrono::seconds(60),
            {overworld, main_menu, dialog}
        );
        switch (ret){
        case 0:
            stream.log("Detected overworld.");
            if (healed && return_to_overworld){
                return;
            }
            pbf_press_button(context, BUTTON_X, 20, 230);
            continue;
        case 1:
            stream.log("Detected main menu.");
            if (!healed){
                main_menu.move_cursor(info, stream, context, MenuSide::LEFT, party_slot, false);
                pbf_press_button(context, BUTTON_MINUS, 20, 230);
                healed = true;
                continue;
            }
            if (!return_to_overworld){
                return;
            }
            pbf_press_button(context, BUTTON_B, 20, 105);
            continue;
        case 2:
            stream.log("Detected dialog.");
            healed = true;
            pbf_press_button(context, BUTTON_B, 20, 105);
            continue;
        default:
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "auto_heal_from_menu(): No state detected after 60 seconds.",
                stream
            );
        }
    }
}




int run_from_battle(
    VideoStream& stream, ProControllerContext& context
){
    stream.log("Attempting to run away...");

    int attempts = 0;
    for (size_t c = 0; c < 10; c++){
        OverworldWatcher overworld(stream.logger(), COLOR_RED);
        NormalBattleMenuWatcher battle_menu(COLOR_YELLOW);
        GradientArrowWatcher next_pokemon(COLOR_GREEN, GradientArrowType::RIGHT, {0.50, 0.51, 0.30, 0.10});
        context.wait_for_all_requests();
        int ret = wait_until(
            stream, context, std::chrono::seconds(60),
            {
                overworld,
                battle_menu,
                next_pokemon,
            }
        );

        switch (ret){
        case 0:
            stream.log("Detected overworld...");
            return attempts;
        case 1:
            stream.log("Detected battle menu...");
            battle_menu.move_to_slot(stream, context, 3);
//            pbf_press_dpad(context, DPAD_DOWN, 250, 0);
            pbf_press_button(context, BUTTON_A, 20, 105);
            pbf_press_button(context, BUTTON_B, 20, 1 * TICKS_PER_SECOND);
            attempts++;
            continue;
        case 2:
            stream.log("Detected own " + STRING_POKEMON + " fainted...");
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Your " + STRING_POKEMON + " fainted while attempting to run away.",
                stream
            );
        default:
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "run_from_battle(): No state detected after 60 seconds.",
                stream
            );
        }
    }

    OperationFailedException::fire(
        ErrorReport::SEND_ERROR_REPORT,
        "Failed to run away after 10 attempts.",
        stream
    );
}


int run_from_battle(
    VideoStream& stream, ProControllerContext& context,
    OverworldBattleTracker& tracker
){
    stream.log("Attempting to run away...");

    int attempts = 0;
    for (size_t c = 0; c < 10; c++){
        OverworldWatcher overworld(stream.logger(), COLOR_RED);
        NormalBattleMenuWatcher battle_menu(COLOR_YELLOW);
        GradientArrowWatcher next_pokemon(COLOR_GREEN, GradientArrowType::RIGHT, {0.50, 0.51, 0.30, 0.10});
        context.wait_for_all_requests();
        int ret = wait_until(
            stream, context, std::chrono::seconds(60),
            {
                overworld,
                battle_menu,
                next_pokemon,
            }
        );

        switch (ret){
        case 0:
            stream.log("Detected overworld...");
            tracker.report_out_of_battle();
            return attempts;
        case 1:{
            stream.log("Detected battle menu...");
            OverworldBattleTracker::Detection result = tracker.report_battle_menu_detected();
            if (result != OverworldBattleTracker::Detection::CONTINUE_BATTLE){
                stream.log("Successfully ran away, but were chain attacked.", COLOR_ORANGE);
                return attempts;
            }

            battle_menu.move_to_slot(stream, context, 3);
//            pbf_press_dpad(context, DPAD_DOWN, 250, 0);
            pbf_press_button(context, BUTTON_A, 20, 105);
            pbf_press_button(context, BUTTON_B, 20, 1 * TICKS_PER_SECOND);
            attempts++;
            continue;
        }
        case 2:
            stream.log("Detected own " + STRING_POKEMON + " fainted...");
            tracker.report_in_battle();
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Your " + STRING_POKEMON + " fainted while attempting to run away.",
                stream
            );
        default:
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "run_from_battle(): No state detected after 60 seconds.",
                stream
            );
        }
    }

    OperationFailedException::fire(
        ErrorReport::SEND_ERROR_REPORT,
        "Failed to run away after 10 attempts.",
        stream
    );
}






void process_battle(
    bool& caught, bool& should_save,
    ProgramEnvironment& env,
    EncounterBotCommonOptions& settings,
    VideoStream& stream, ProControllerContext& context,
    OverworldBattleTracker& tracker,
    EncounterFrequencies& encounter_frequencies,
    std::atomic<uint64_t>& shiny_counter,
    Language language
){
    std::set<std::string> slugs;
    if (language != Language::None){
        slugs = read_singles_opponent(env.program_info(), stream, context, language);
        encounter_frequencies += slugs;
        stream.log(encounter_frequencies.dump_sorted_map("Encounter Stats:\n"));
    }

    VideoSnapshot shiny_screenshot;
    float shiny_error;
    OverworldBattleTracker::Detection type = tracker.report_battle_menu_detected(shiny_screenshot, shiny_error);
    bool is_shiny = type == OverworldBattleTracker::Detection::NEW_SHINY_BATTLE;
    if (is_shiny){
        shiny_counter++;
        if (settings.VIDEO_ON_SHINY){
            context.wait_for(std::chrono::seconds(3));
            pbf_press_button(context, BUTTON_CAPTURE, 2 * TICKS_PER_SECOND, 0);
        }
    }
    env.update_stats();

    send_encounter_notification(
        env,
        settings.NOTIFICATION_NONSHINY,
        settings.NOTIFICATION_SHINY,
        language != Language::None,
        is_shiny,
        {{slugs, is_shiny ? ShinyType::UNKNOWN_SHINY : ShinyType::NOT_SHINY}},
        is_shiny ? shiny_error : std::nan(""),
        shiny_screenshot,
        &encounter_frequencies
    );

    tracker.report_in_battle();

    //  Set default action: stop program if shiny, otherwise run away.
    EncounterActionsEntry action;
    action.action = is_shiny
        ? EncounterActionsAction::STOP_PROGRAM
        : EncounterActionsAction::RUN_AWAY;

    //  Iterate the actions table. If found an entry matches the pokemon species,
    //  set the action to be what specified in the entry.
    for (EncounterActionsEntry& entry : settings.ACTIONS_TABLE.snapshot()){
        if (language == Language::None){
            throw UserSetupError(stream.logger(), "You must set the game language to use the actions table.");
        }

        //  See if Pokemon name matches.
        auto iter = slugs.find(entry.pokemon);
        if (iter == slugs.end()){
            continue;
        }

        switch (entry.shininess){
        case EncounterActionsShininess::ANYTHING:
            break;
        case EncounterActionsShininess::NOT_SHINY:
            if (is_shiny){
                continue;
            }
            break;
        case EncounterActionsShininess::SHINY:
            if (!is_shiny){
                continue;
            }
            break;
        }

        action = std::move(entry);
    }

    //  Run the chosen action.
    switch (action.action){
    case EncounterActionsAction::RUN_AWAY:
        try{
            run_from_battle(stream, context, tracker);
        }catch (OperationFailedException& e){
            throw FatalProgramException(std::move(e));
        }
        caught = false;
        should_save = false;
        return;
    case EncounterActionsAction::STOP_PROGRAM:
        throw ProgramFinishedException();
//        throw ProgramFinishedException(m_stream, "", true);
    case EncounterActionsAction::THROW_BALLS:
    case EncounterActionsAction::THROW_BALLS_AND_SAVE:
        if (language == Language::None){
            throw InternalProgramError(&stream.logger(), PA_CURRENT_FUNCTION, "Language is not set.");
        }

        CatchResults result = basic_catcher(
            stream, context,
            language,
            action.ball, action.ball_limit,
            settings.USE_FIRST_MOVE_IF_CANNOT_THROW_BALL
        );
        send_catch_notification(
            env,
            settings.NOTIFICATION_CATCH_SUCCESS,
            settings.NOTIFICATION_CATCH_FAILED,
            &slugs,
            action.ball,
            result.balls_used,
            result.result
        );

        switch (result.result){
        case CatchResult::POKEMON_CAUGHT:
        case CatchResult::POKEMON_FAINTED:
            break;
        default:
            throw_and_log<FatalProgramException>(
                stream.logger(), ErrorReport::NO_ERROR_REPORT,
                "Unable to recover from failed catch.",
                stream
            );
        }

        caught = result.result == CatchResult::POKEMON_CAUGHT;
        should_save = caught && action.action == EncounterActionsAction::THROW_BALLS_AND_SAVE;
        return;
    }

    caught = false;
    should_save = false;
}



















}
}
}
