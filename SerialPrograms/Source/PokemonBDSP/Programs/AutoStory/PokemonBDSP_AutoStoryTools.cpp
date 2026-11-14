


/*  BDSP AutoStory Tools
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <cctype>
#include <cmath>
#include <cstdlib>
#include <unordered_set>

#include "Common/Cpp/Filesystem/Filesystem.h"

#include "CommonFramework/Exceptions/FatalProgramException.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Exceptions/OperationFailedExceptionWithScreenshot.h"
#include "CommonFramework/GlobalAutoPaths.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/ImageTools/ImageDiff.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/Tools/VideoStream.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"

#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/Images/ImageFilter.h"
#include "CommonTools/Images/SolidColorTest.h"
#include "CommonTools/OCR/OCR_Routines.h"
#include "CommonTools/VisualDetectors/BlackScreenDetector.h"
#include "CommonTools/VisualDetectors/ImageMatchDetector.h"

#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"

#include "PokemonBDSP/Inference/PokemonBDSP_DialogDetector.h"
#include "PokemonBDSP/Inference/PokemonBDSP_MenuDetector.h"
#include "PokemonBDSP/Inference/PokemonBDSP_SelectionArrow.h"
#include "PokemonBDSP/Inference/PokemonBDSP_ShopItemNameReader.h"
#include "PokemonBDSP/Inference/PokemonBDSP_TransitionWatchers.h"
#include "PokemonBDSP/Programs/PokemonBDSP_GameEntry.h"
#include "PokemonBDSP/Programs/PokemonBDSP_GameNavigation.h"

#include "Detect/PokemonBDSP_AutoStory_OverworldDetector.h"

#include "PokemonBDSP_AutoStoryTools.h"

namespace PokemonAutomation {
namespace NintendoSwitch {
namespace PokemonBDSP {


const char* torterra_move_slug(TorterraMove move){
    switch (move){
    case TorterraMove::EARTHQUAKE:
        return "earthquake";
    case TorterraMove::WOOD_HAMMER:
        return "wood-hammer";
    case TorterraMove::RAZOR_LEAF:
        return "razor-leaf";
    case TorterraMove::CRUNCH:
        return "crunch";
    case TorterraMove::GIGA_DRAIN:
        return "giga-drain";
    case TorterraMove::IRON_TAIL:
        return "iron-tail";
    case TorterraMove::ROCK_TOMB:
        return "rock-tomb";
    case TorterraMove::FACADE:
        return "facade";
    case TorterraMove::STONE_EDGE:
        return "stone-edge";
    case TorterraMove::ROCK_SLIDE:
        return "rock-slide";
    case TorterraMove::BULLDOZE:
        return "bulldoze";
    case TorterraMove::STRENGTH:
        return "strength";
    case TorterraMove::ROCK_SMASH:
        return "rock-smash";
    case TorterraMove::ROCK_CLIMB:
        return "rock-climb";
    }
    return "unknown";
}

const char* trainer_slug(Trainer t){
    switch (t){
    case Trainer::Veilstone_GalacticGrunts_2v2:
        return "Veilstone_GalacticGrunts_2v2";
    }
    return "unknown";
}

bool fake_save_game(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    pbf_mash_button(context, BUTTON_L, 1000ms);

    context.wait_for_all_requests();
    pbf_press_button(context, BUTTON_X, 80ms, 1000ms);
    pbf_press_button(context, BUTTON_R, 80ms, 1000ms);
    pbf_mash_button(context, BUTTON_B, 2000ms);
    context.wait_for_all_requests();

    return true;
}

bool save_game(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    static constexpr size_t MAX_RETRIES = 3;

    for (size_t attempt = 0; attempt < MAX_RETRIES; attempt++){
        try {
            env.console.log("Checkpoint save attempt: " + std::to_string(attempt + 1));

            // Only mash B on retries - no need to on first attempt
            // if already cleanly in the overworld.
            if (attempt > 0){
                pbf_mash_button(context, BUTTON_B, 1000ms);
            }

            // BlackScreenOverWatcher can finish while the destination is still
            // fading in. Drain movement and wait before sending the menu input.
            context.wait_for_all_requests();
            BlackScreenWatcher destination_visible(COLOR_RED,
                {0.1, 0.1, 0.8, 0.8},
                100,
                10,
                BlackScreenWatcher::FinderType::GONE,
                1000ms);
            if (wait_until(env.console, context, 15s, {{destination_visible}}) != 0){
                throw OperationFailedExceptionWithScreenshot(ErrorReportMode::SEND_ERROR_REPORT,
                    "Checkpoint destination did not become visible before saving.",
                    env.console);
            }
            pbf_wait(context, 1000ms);
            context.wait_for_all_requests();

            OverworldWatcher ready_to_save;
            if (wait_until(env.console, context, 15s, {{ready_to_save}}) != 0){
                throw OperationFailedExceptionWithScreenshot(ErrorReportMode::SEND_ERROR_REPORT,
                    "Checkpoint destination did not return to the overworld before saving.",
                    env.console);
            }

            // Reuse existing feedback-based save: opens menu with MenuWatcher
            // + respects GameSettings::OVERWORLD_TO_MENU_DELAY0 internally.
            save_game(env.console, context);

            // Return to overworld cleanly.
            pbf_mash_button(context, BUTTON_B, 1000ms);

            // Drain R/confirm/B before detection; an earlier overworld frame
            // must not report success while saving commands are still queued.
            context.wait_for_all_requests();

            // Verify overworld. Generous timeout: save animation + fade can be slow.
            OverworldWatcher overworld;
            int ret = wait_until(env.console, context, std::chrono::seconds(15), {overworld});
            if (ret < 0){
                throw OperationFailedExceptionWithScreenshot(ErrorReportMode::SEND_ERROR_REPORT,
                    "Failed to detect overworld after save.",
                    env.console);
            }

            env.console.log("Checkpoint save verified.");
            return true;

        } catch (OperationFailedException& e){
            env.console.log("Checkpoint save failed: " + e.message(), COLOR_RED);
            context.wait_for(1000ms);
        }
    }

    env.console.log("All checkpoint save attempts failed; route movement will not continue.", COLOR_RED);

    return false;
}


void checkpoint_save(SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    EventNotificationOption& notif_status_update,
    AutoStoryStats& stats,
    CheckpointSaveMode save_mode){
    const bool fake = save_mode == CheckpointSaveMode::FAKE;
    env.console.log(
        fake ? "[Checkpoint save] DIAGNOSTIC FAKE mode: save is not verified." : "[Checkpoint save] REAL mode.");
    bool success = fake ? fake_save_game(env, context) : save_game(env, context);
    if (!success){
        throw FatalProgramException(ErrorReportMode::NO_ERROR_REPORT,
            "Checkpoint save could not be verified. Stopped before route movement; the previous save was not reloaded.",
            env.console);
    }

    stats.m_checkpoint++;
    env.update_stats();

    send_program_status_notification(env,
        notif_status_update,
        fake ? "Diagnostic fake checkpoint save (not verified)." : "Saved at checkpoint (verified).");
}


void reload_backup_save(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    // TODO: implement backup save reload.
    // This is called when a checkpoint with save=false fails.
    // Load the game's automatic backup save rather than resetting from HOME.
    (void)env;
    (void)context;
}


void checkpoint_reattempt_loop(SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    EventNotificationOption& notif_status_update,
    AutoStoryStats& stats,
    std::function<void(size_t attempt_number)>&& action,
    bool save,
    CheckpointSaveMode save_mode,
    CheckpointRetryScope retry_scope){
    const auto pp_at_checkpoint = stats.move_pp_remaining;
    static constexpr size_t MAX_ATTEMPTS = 20;

    for (size_t i = 0;; i++){
        try {
            if (i == 0 && retry_scope != CheckpointRetryScope::SELECTED_RANGE){
                if (save || stats.m_first_run){
                    checkpoint_save(env, context, notif_status_update, stats, save_mode);
                }
                stats.m_first_run = false;
            }

            action(i);
            return; // success

        } catch (OperationFailedException& e){
            if (retry_scope == CheckpointRetryScope::DEFER_TO_SELECTED_RANGE){
                throw;
            }
            env.console.log("checkpoint_reattempt_loop: attempt " + std::to_string(i) + " failed: " + e.message(),
                COLOR_RED);
            stats.m_reset++;
            env.update_stats();

            if (i + 1 >= MAX_ATTEMPTS){
                throw;
            }

            if (save || retry_scope == CheckpointRetryScope::SELECTED_RANGE){
                env.console.log("Resetting game from HOME before retry...", COLOR_ORANGE);
                bool ok = reset_game_from_home(env, env.console, context, true, 1000ms);
                if (!ok){
                    throw OperationFailedExceptionWithScreenshot(ErrorReportMode::SEND_ERROR_REPORT,
                        "Failed to reset game from HOME during checkpoint retry.",
                        env.console);
                }
                // Only a real game reload restores the corresponding PP balance.
                stats.move_pp_remaining = pp_at_checkpoint;
                stats.game_reset_generation++;
            }else{
                env.console.log("Reloading from backup save before retry...", COLOR_ORANGE);
                reload_backup_save(env, context);
            }
        }
    }
}


void repeat(std::initializer_list<std::function<void()>> actions, size_t times){
    for (size_t i = 0; i < times; i++){
        for (const auto& action : actions){
            action();
        }
    }
}


void mash_until_dialogue_ends(VideoStream& stream, ProControllerContext& context, Button button, Milliseconds timeout){
    OverworldWatcher overworld;
    int ret = run_until<ProControllerContext>(stream,
        context,
        [button, timeout](ProControllerContext& context){
            pbf_mash_button(context, button, timeout);
        },
        {{overworld}});
    if (ret < 0){
        OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
            "mash_until_dialogue_ends: overworld not reached within timeout.",
            stream);
    }
}


void repeat_dpad(ProControllerContext& context,
    DpadState& state,
    DpadPosition dir,
    Milliseconds press,
    Milliseconds hold,
    size_t times,
    bool include_last_dir){
    if (include_last_dir && state.last_dir != DPAD_NONE && state.last_dir != dir){
        times += 1;
    }
    state.last_dir = dir;
    for (size_t i = 0; i < times; i++){
        pbf_press_dpad(context, dir, press, hold);
    }
}

bool heal_pokemon(VideoStream& stream,
    ProControllerContext& context,
    const std::string& label,
    AutoStoryStats* stats,
    bool exit_center){
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);

    pbf_move_left_joystick(context, {0, +1}, 1800ms, 100ms); // 7+
    context.wait_for_all_requests();

    pbf_mash_button(context, BUTTON_A, 5000ms);
    context.wait_for_all_requests();
    pbf_wait(context, 3000ms);
    wait_for_dialog(stream, context, label + " - Pokemon healing");
    // Nurse dialogue can pause before the final message. Require a continuous
    // quiet overworld before exiting or restoring the tracked PP budget.
    clear_dialog(stream, context, ClearDialogMode::STOP_OVERWORLD, 60, BUTTON_B, 6000ms);
    context.wait_for_all_requests();
    pbf_wait(context, 1000ms);

    if (!exit_center){
        if (stats){
            stats->move_pp_remaining = stats->move_pp_capacity;
        }
        return true;
    }

    // Exiting the Pokemon Center
    BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
    int ret = run_until<ProControllerContext>(stream,
        context,
        [](ProControllerContext& context){
            pbf_move_left_joystick(context, {0, -1}, 15000ms, 100ms);
        },
        {{black_screen}});
    if (ret < 0){
        stream.log("heal_pokemon(" + label + "): black screen not detected", COLOR_RED);
        return false;
    }

    context.wait_for_all_requests();
    if (stats){
        stats->move_pp_remaining = stats->move_pp_capacity;
        stream.log("[Move PP] " + label + ": restored the selected starter's PP budgets.");
    }
    return true;
}

std::string fly_point_name(FlyPoint place){
    switch (place){
    case FlyPoint::CanalavCity:
        return "canalave_city";
    case FlyPoint::CelesticTown:
        return "celestic_town";
    case FlyPoint::EternaCity:
        return "eterna_city";
    case FlyPoint::FloaromaTown:
        return "floaroma_town";
    case FlyPoint::HearthomeCity:
        return "hearthome_city";
    case FlyPoint::JubilifeCity:
        return "jubilife_city";
    case FlyPoint::OreburghCity:
        return "oreburgh_city";
    case FlyPoint::PastoriaCity:
        return "pastoria_city";
    case FlyPoint::PokemonLeagueLower:
        return "pokemon_league_lower";
    case FlyPoint::PokemonLeagueUpper:
        return "pokemon_league_upper";
    case FlyPoint::Route221:
        return "route_221";
    case FlyPoint::SandgemTown:
        return "sandgem_town";
    case FlyPoint::SolaceonTown:
        return "solaceon_town";
    case FlyPoint::SnowpointCity:
        return "snowpoint_city";
    case FlyPoint::SunyshoreCity:
        return "sunyshore_city";
    case FlyPoint::TwinleafTown:
        return "twinleaf_town";
    case FlyPoint::VeilstoneCity:
        return "veilstone_city";
    case FlyPoint::CurrentCity:
        return "current_city";
    }
    return "unknown";
}

bool fly_to(VideoStream& stream, ProControllerContext& context, FlyPoint place){
    // Each city branch below should update this to the city icon's position on the fly map.
    // Detection: when the cursor brackets overlap the icon, its normally uniform
    // color becomes non-uniform (stddev rises). Box stays as a rough fallback
    // until each city is calibrated.
    ImageFloatBox city_icon_box{0.700000, 0.550000, 0.100000, 0.120000};

    DpadState dpad;
    context.wait_for_all_requests();
    pbf_wait(context, 1000ms);
    open_menu(stream, context, MenuCursorPosition::MAP, 8);

    if (place == FlyPoint::CurrentCity){
        ImageFloatBox button_a_box{0.540000, 0.960000, 0.070000, 0.035000};
        struct FlyAButtonDetector : VisualInferenceCallback {
            ImageFloatBox m_box;
            FlyAButtonDetector(const ImageFloatBox& box)
                : VisualInferenceCallback("FlyAButtonDetector")
                , m_box(box){
            }
            void make_overlays(VideoOverlaySet&) const override {
            }
            bool process_frame(const ImageViewRGB32& frame, WallClock) override {
                ImageStats s = image_stats(extract_box_reference(frame, m_box));
                return s.stddev.sum() > 30.0;
            }
        };
        FlyAButtonDetector watcher(button_a_box);
        int ret = wait_until(stream, context, 3000ms, {{watcher}});
        if (ret < 0){
            stream.log("CurrentCity: A button not found.", COLOR_RED);
            return false;
        }
        BlackScreenOverWatcher flying_black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
        int fly_ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_mash_button(context, BUTTON_A, 200000ms);
            },
            {{flying_black_screen}});
        if (fly_ret < 0){
            stream.log("CurrentCity: black screen not detected!", COLOR_RED);
            return false;
        }
        stream.log("CurrentCity: black screen detected, arrived.", COLOR_GREEN);
        return true;
    }

    switch (place){
    case FlyPoint::CelesticTown:
        pbf_move_left_joystick(context, {-1, +1}, 4000ms, 500ms);
        break;
    case FlyPoint::OreburghCity:
        pbf_move_left_joystick(context, {-1, -1}, 4000ms, 500ms);
        break;
    case FlyPoint::HearthomeCity:
    case FlyPoint::PastoriaCity:
    case FlyPoint::PokemonLeagueLower:
    case FlyPoint::PokemonLeagueUpper:
    case FlyPoint::SolaceonTown:
    case FlyPoint::SunyshoreCity:
    case FlyPoint::VeilstoneCity:
        pbf_move_left_joystick(context, {+1, -1}, 4000ms, 500ms);
        break;
    case FlyPoint::SnowpointCity:
    case FlyPoint::FloaromaTown:
        pbf_move_left_joystick(context, {-1, +1}, 4000ms, 500ms);
        break;
    case FlyPoint::TwinleafTown:
        pbf_move_left_joystick(context, {-1, -1}, 4000ms, 500ms);
        break;
    default:
        pbf_move_left_joystick(context, {-1, -1}, 4000ms, 500ms);
        break;
    }
    context.wait_for_all_requests();

    ImageFloatBox button_a_box{0.540000, 0.960000, 0.070000, 0.035000};

    struct FlyAButtonDetector : VisualInferenceCallback {
        ImageFloatBox m_box;
        FlyAButtonDetector(const ImageFloatBox& box)
            : VisualInferenceCallback("FlyAButtonDetector")
            , m_box(box){
        }
        void make_overlays(VideoOverlaySet&) const override {
        }
        bool process_frame(const ImageViewRGB32& frame, WallClock) override {
            ImageStats s = image_stats(extract_box_reference(frame, m_box));
            // Blue stripe is uniform when no button shown; A button disrupts it
            return s.stddev.sum() > 30.0;
        }
    };
    FlyAButtonDetector watcher_1(button_a_box);

    switch (place){
    case FlyPoint::CanalavCity:
        city_icon_box = {0.380000, 0.673000, 0.012000, 0.054000};
        repeat_dpad(context, dpad, DPAD_UP, 80ms, 300ms, 6, false);
        break;
    case FlyPoint::CelesticTown:
        city_icon_box = {0.660000, 0.470000, 0.012000, 0.020000};
        repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 15, false);
        repeat_dpad(context, dpad, DPAD_DOWN, 80ms, 300ms, 13, false);
        break;
    case FlyPoint::EternaCity:
        city_icon_box = {0.566000, 0.482000, 0.012000, 0.048000};
        repeat_dpad(context, dpad, DPAD_UP, 80ms, 300ms, 13, false);
        repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 10, false);
        break;
    case FlyPoint::FloaromaTown:
        city_icon_box = {0.475000, 0.573000, 0.012000, 0.054000};
        repeat_dpad(context, dpad, DPAD_DOWN, 80ms, 300ms, 16, false);
        repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 5, false);
        break;
    case FlyPoint::HearthomeCity:
        city_icon_box = {0.660000, 0.637000, 0.029000, 0.052000};
        repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 16, false);
        repeat_dpad(context, dpad, DPAD_UP, 80ms, 300ms, 7, false);
        break;
    case FlyPoint::JubilifeCity:
        city_icon_box = {0.453000, 0.702000, 0.016000, 0.054000};
        repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 4, false);
        repeat_dpad(context, dpad, DPAD_UP, 80ms, 300ms, 5, false);
        break;
    case FlyPoint::OreburghCity:
        city_icon_box = {0.566000, 0.702000, 0.012000, 0.054000};
        repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 10, false);
        repeat_dpad(context, dpad, DPAD_UP, 80ms, 300ms, 5, false);
        break;
    case FlyPoint::PastoriaCity:
        city_icon_box = {0.773000, 0.806000, 0.029000, 0.052000};
        repeat_dpad(context, dpad, DPAD_UP, 80ms, 300ms, 2, false);
        repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 10, false);
        break;
    case FlyPoint::PokemonLeagueLower:
        // Estimated from the marked map; verify without the player/flag overlay.
        city_icon_box = {0.924000, 0.509000, 0.012000, 0.025000};
        repeat_dpad(context, dpad, DPAD_UP, 80ms, 300ms, 12, false);
        repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 3, false);
        break;
    case FlyPoint::PokemonLeagueUpper:
        // Estimated from the marked map; verify without the player/flag overlay.
        city_icon_box = {0.924000, 0.477000, 0.012000, 0.025000};
        repeat_dpad(context, dpad, DPAD_UP, 80ms, 300ms, 13, false);
        repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 3, false);
        break;
    case FlyPoint::Route221:
        city_icon_box = {0.507000, 0.914000, 0.044000, 0.010000};
        break;
    case FlyPoint::SandgemTown:
        city_icon_box = {0.475000, 0.838000, 0.012000, 0.022000};
        repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 5, false);
        repeat_dpad(context, dpad, DPAD_UP, 80ms, 300ms, 2, false);
        break;
    case FlyPoint::SolaceonTown:
        city_icon_box = {0.735000, 0.603000, 0.030000, 0.022000};
        repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 12, false);
        repeat_dpad(context, dpad, DPAD_UP, 80ms, 300ms, 9, false);
        break;
    case FlyPoint::SnowpointCity:
        city_icon_box = {0.602000, 0.070000, 0.013000, 0.054000};
        repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 12, false);
        repeat_dpad(context, dpad, DPAD_DOWN, 80ms, 300ms, 1, false);
        break;
    case FlyPoint::SunyshoreCity:
        city_icon_box = {0.924000, 0.705000, 0.029000, 0.052000};
        repeat_dpad(context, dpad, DPAD_UP, 80ms, 300ms, 5, false);
        repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 2, false);
        break;
    case FlyPoint::TwinleafTown:
        city_icon_box = {0.415000, 0.870000, 0.013000, 0.022000};
        repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 2, false);
        repeat_dpad(context, dpad, DPAD_UP, 80ms, 300ms, 1, false);
        break;
    case FlyPoint::VeilstoneCity:
        city_icon_box = {0.829000, 0.537000, 0.017000, 0.054000};
        repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 8, false);
        repeat_dpad(context, dpad, DPAD_UP, 80ms, 300ms, 10, false);
        break;
    case FlyPoint::CurrentCity:
        break;
    }

    context.wait_for_all_requests();

    struct CityIconDetector : VisualInferenceCallback {
        ImageFloatBox m_box;
        CityIconDetector(const ImageFloatBox& box)
            : VisualInferenceCallback("CityIconDetector")
            , m_box(box){
        }
        void make_overlays(VideoOverlaySet&) const override {
        }
        bool process_frame(const ImageViewRGB32& frame, WallClock) override {
            ImageStats s = image_stats(extract_box_reference(frame, m_box));
            // City icon is normally a uniform color; cursor brackets disrupt it
            return s.stddev.sum() > 30.0;
        }
    };
    CityIconDetector watcher_2(city_icon_box);

    const std::string name = fly_point_name(place);
    int ret_1 = wait_until(stream, context, 3000ms, {{watcher_1}});
    int ret_2 = wait_until(stream, context, 4000ms, {{watcher_2}});
    if (ret_1 < 0){
        stream.log("A button not found, retrying...", COLOR_RED);
    };
    if (ret_2 < 0){
        stream.log(name + " not found, retrying...", COLOR_RED);
    };
    if (ret_1 == 0 and ret_2 == 0){
        stream.log("Found " + name + ", flying towards it.", COLOR_GREEN);
        WhiteScreenOverWatcher flying_white_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
        BlackScreenOverWatcher flying_black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_mash_button(context, BUTTON_A, 200000ms);
            },
            {{flying_white_screen}, {flying_black_screen}});
        if (ret < 0){
            stream.log("Flying to " + name + ": white / black screen not detected!", COLOR_RED);
            return false;
        }else if (ret >= 0){
            stream.log("Flying to " + name + ": white / black screen detected!", COLOR_GREEN);
            return true;
        }
    }
    return false;
}

bool test_fly_locations(VideoStream& stream, ProControllerContext& context){
    // Follow the marked map order, excluding Route 221 and CurrentCity.
    const FlyPoint destinations[] = {
        FlyPoint::CanalavCity,
        FlyPoint::TwinleafTown,
        FlyPoint::SandgemTown,
        FlyPoint::JubilifeCity,
        FlyPoint::FloaromaTown,
        FlyPoint::OreburghCity,
        FlyPoint::EternaCity,
        FlyPoint::SnowpointCity,
        FlyPoint::HearthomeCity,
        FlyPoint::CelesticTown,
        FlyPoint::SolaceonTown,
        FlyPoint::PastoriaCity,
        FlyPoint::VeilstoneCity,
        FlyPoint::SunyshoreCity,
        FlyPoint::PokemonLeagueLower,
        FlyPoint::PokemonLeagueUpper,
    };
    for (FlyPoint place : destinations){
        const std::string name = fly_point_name(place);
        stream.log("Fly test: starting " + name + ".");
        if (!fly_to(stream, context, place)){
            stream.log("Fly test: failed at " + name + ".", COLOR_RED);
            return false;
        }
        // fly_to returns at the flight fade, before the landing animation.
        // Let the animation finish before opening the map for the next flight.
        pbf_wait(context, 10000ms);
        context.wait_for_all_requests();
        stream.log("Fly test: finished " + name + ".");
    }
    stream.log("Fly test: completed all 16 destinations.", COLOR_GREEN);
    return true;
}

// Slot 1's colored HP bar in the Bag. Read fill position rather than OCR.
static double potion_hp_fraction(const ImageViewRGB32& frame){
    if (bag_pocket_position(frame) == 0){
        return -1;
    }
    const ImageViewRGB32 bar = extract_box_reference(frame, ImageFloatBox{0.137, 0.160, 0.202, 0.016});
    size_t filled = 0;
    for (size_t x = 0; x < bar.width(); x++){
        size_t color = 0;
        for (size_t y = 0; y < bar.height(); y++){
            uint32_t p = bar.pixel(x, y);
            int r = (p >> 16) & 255, g = (p >> 8) & 255, b = p & 255;
            const bool green = g > r + 30 && g > b + 20;
            const bool yellow = r > 150 && g > 120 && b < 100;
            const bool red = r > 150 && r > g + 50 && r > b + 50;
            color += green || yellow || red;
        }
        filled += color > bar.height() / 4;
    }
    return bar.width() ? (double)filled / bar.width() : -1;
}

static double confirmed_potion_hp(VideoStream& stream, ProControllerContext& context, double minimum = -1){
    double previous = -1;
    size_t stable = 0;
    for (size_t check = 0; check < 40; check++){
        context.wait_for_all_requests();
        auto frame = stream.video().snapshot();
        double hp = frame ? potion_hp_fraction(frame) : -1;
        if (hp >= 0 && hp > minimum && previous > minimum && std::abs(hp - previous) <= 0.015){
            const size_t required = minimum >= 0 && hp < 0.98 ? 8 : 3;
            if (++stable >= required){
                return hp;
            }
        }else{
            stable = 0;
        }
        previous = hp;
        pbf_wait(context, 150ms);
    }
    OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
        "Potion: Slot 1 HP bar could not be confirmed.",
        stream);
}

void use_potion_first_pokemon(VideoStream& stream, ProControllerContext& context, Language language, uint8_t* level){
    OverlayBoxScope hp_overlay(stream.overlay(), COLOR_GREEN, {0.137, 0.160, 0.202, 0.016}, "");
    BagItemTemplate potion;
    potion.reference_path = "PokemonBDSP/Autostory/Bag_Potion.png";
    potion.label = "potion";
    potion.reference_row = 1;
    open_menu(stream, context, MenuCursorPosition::BAG, 5);
    pbf_wait(context, 1000ms);
    double hp = confirmed_potion_hp(stream, context);
    for (size_t dose = 0; hp < 0.98 && dose < 3; dose++){
        if (hp <= 0.005){
            OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                "Potion: Slot 1 appears fainted; a Potion cannot revive it.", stream);
        }
        stream.log("[Potion] Finding Potion; dose " + std::to_string(dose + 1) + "/3.");
        if (!find_bag_item(stream, context, potion, 1, 5, 64, 45s, true, language)){
            OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                "Potion: Could not verify Potion in the Medicine pocket.", stream);
        }
        pbf_press_button(context, BUTTON_A, 80ms, 500ms);
        pbf_press_button(context, BUTTON_A, 80ms, 500ms);
        pbf_press_button(context, BUTTON_A, 80ms, 1000ms);
        // The party HP bar stays visible while the Potion is applied, even
        // when it was the last Potion. Require an increase before accepting
        // stable HP; an unchanged pre-animation bar is not a successful dose.
        hp = confirmed_potion_hp(stream, context, hp + 0.005);
        stream.log("[Potion] Slot 1 HP after use=" + std::to_string(hp * 100) + "%.");
        if (hp < 0.98){
            // Return from the target/result prompt to the item list for another dose.
            bool item_list = false;
            for (size_t press = 0; press < 5; press++){
                context.wait_for_all_requests();
                const auto frame = stream.video().snapshot();
                if (frame && bag_pocket_position(frame) != 0 && bag_cursor_position(frame) != 0){
                    item_list = true;
                    break;
                }
                pbf_press_button(context, BUTTON_B, 80ms, 300ms);
            }
            if (!item_list){
                OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                    "Potion: Item list not confirmed for another dose.", stream);
            }
        }
    }
    if (hp < 0.98){
        OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
            "Potion: Full HP not confirmed after three uses.", stream);
    }
    if (level){
        // Reuse the visible Bag level for tutorial readiness. Strict numeric
        // reads must agree across filters and three frames before skipping training.
        *level = 0;
        int previous = -1;
        size_t stable = 0;
        for (size_t check = 0; check < 12; check++){
            context.wait_for_all_requests();
            const auto frame = stream.video().snapshot();
            int observed = -1;
            bool conflicting = false;
            if (frame && bag_pocket_position(frame) != 0){
                for (const auto& range : OCR::WHITE_TEXT_FILTERS()){
                    const std::string text = OCR::tesseract_ocr_read(Language::English,
                        to_blackwhite_rgb32_range(
                            extract_box_reference(frame, ImageFloatBox{0.320, 0.180, 0.025, 0.040}),
                            true, range.mins, range.maxs), OCR::PageSegMode::SINGLE_WORD);
                    std::string digits;
                    for (char c : text){
                        if (!std::isspace(static_cast<unsigned char>(c))){
                            digits += c;
                        }
                    }
                    if (digits.empty() || digits.size() > 2 ||
                        digits.find_first_not_of("0123456789") != std::string::npos){
                        continue;
                    }
                    const int value = std::atoi(digits.c_str());
                    // This optional check is used only before tutorial training.
                    if (value >= 5 && value <= 20){
                        conflicting |= observed >= 0 && observed != value;
                        observed = value;
                    }
                }
            }
            if (conflicting){
                observed = -1;
            }
            stable = observed >= 0 && observed == previous ? stable + 1 : 1;
            previous = observed;
            if (observed >= 0 && stable >= 3){
                *level = static_cast<uint8_t>(observed);
                break;
            }
            pbf_wait(context, 150ms);
        }
        if (*level == 0){
            OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                "Tutorial training: Starter level could not be confirmed in the Bag.", stream);
        }
    }
    pbf_mash_button(context, BUTTON_B, 2000ms);
    context.wait_for_all_requests();
    OverworldWatcher overworld;
    if (wait_until(stream, context, 10s, {{overworld}}) != 0){
        OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
            "Potion: Final overworld return not confirmed.", stream);
    }
    stream.log("[Potion] Slot 1 HP bar full and overworld confirmed.");
}


namespace {

struct IconGrid {
    int row, col;
};

IconGrid icon_to_grid(MenuCursorPosition pos, int num_icons){
    switch (pos){
    case MenuCursorPosition::POKEDEX:
        return {0, 0};
    case MenuCursorPosition::POKEMON:
        return {0, 1};
    case MenuCursorPosition::BAG:
        return {0, 2};
    case MenuCursorPosition::CARD:
        return {0, 3};
    case MenuCursorPosition::OPTIONS:
        return {1, num_icons <= 5 ? 0 : 2};
    case MenuCursorPosition::MAP:
        return {1, 0};
    case MenuCursorPosition::CAPSULES:
        return {1, 1};
    case MenuCursorPosition::MYSTERYGIFT:
        return {1, 3};
    default:
        return {-1, -1};
    }
}

} // anonymous namespace


void open_menu(VideoStream& stream, ProControllerContext& context, MenuCursorPosition target, int num_icons){
    using namespace std::chrono_literals;

    // Previous single opening attempt:
    // context.wait_for_all_requests();
    // overworld_to_menu(stream, context);
    // pbf_wait(context, 1000ms);
    // context.wait_for_all_requests();
    context.wait_for_all_requests();

    MenuCursorDetector detector;
    {
        static const char* ICON_NAMES[MENU_CURSOR_NUM_ICONS] = {
            "POKEDEX",
            "POKEMON",
            "BAG",
            "CARD",
            "OPTIONS",
            "MAP",
            "CAPSULES",
            "MYSTERYGIFT",
        };
        std::string ref_dir = RESOURCE_PATH() + "PokemonBDSP/Autostory";
        for (int i = 0; i < MENU_CURSOR_NUM_ICONS; i++){
            std::string path = ref_dir + "/" + ICON_NAMES[i] + ".png";
            // These templates are optional. The dark-pixel cursor detector
            // remains available when a template is not packaged.
            if (!Filesystem::is_regular_file(path)){
                continue;
            }
            ImageRGB32 img(path);
            if (img){
                detector.set_icon_reference(static_cast<MenuCursorPosition>(i), std::move(img));
            }
        }
    }
    // Previous single detection attempt retained for comparison.
    // MenuCursorPosition cur = MenuCursorPosition::UNKNOWN;
    // for (int i = 0; i < 10 && cur == MenuCursorPosition::UNKNOWN; i++){
    //     context.wait_for(50ms);
    //     VideoSnapshot snap = stream.video().snapshot();
    //     if (snap) cur = detector.detect(snap);
    // }
    // if (cur == MenuCursorPosition::UNKNOWN){
    //     OperationFailedExceptionWithScreenshot::fire(
    //         ErrorReportMode::SEND_ERROR_REPORT,
    //         "Menu cursor not detected after opening menu.",
    //         stream
    //     );
    // }

    MenuCursorPosition cur = MenuCursorPosition::UNKNOWN;
    bool menu_confirmed_open = false;
    for (size_t attempt = 1; attempt <= 3; attempt++){
        stream.log("[Open menu] Open and detect attempt " + std::to_string(attempt) + "/3.");
        try {
            // A late-opening menu may already be visible after a timeout.
            // Check before pressing X again, since X would close that menu.
            if (!menu_confirmed_open){
                MenuWatcher already_open;
                if (wait_until(stream, context, 300ms, {{already_open}}) < 0){
                    overworld_to_menu(stream, context);
                }else{
                    stream.log("[Open menu] Menu already visible; skipping X.");
                }
                menu_confirmed_open = true;
            }else{
                // Cursor detection retries send no input that could close the
                // confirmed menu. Keep observing it instead of toggling X.
                stream.log("[Open menu] Retrying cursor detection in the open menu.");
            }
            pbf_wait(context, 1000ms);
            context.wait_for_all_requests();
            for (int frame = 0; frame < 10 && cur == MenuCursorPosition::UNKNOWN; frame++){
                context.wait_for(50ms);
                const VideoSnapshot snapshot = stream.video().snapshot();
                if (snapshot){
                    cur = detector.detect(snapshot);
                }
            }
            if (cur != MenuCursorPosition::UNKNOWN){
                stream.log("[Open menu] Menu and cursor detected on attempt " + std::to_string(attempt) + ".",
                    COLOR_GREEN);
                break;
            }
            stream.log("[Open menu] Menu cursor not detected on attempt " + std::to_string(attempt) + ".",
                COLOR_ORANGE);
        } catch (OperationFailedException& error){
            stream.log("[Open menu] Attempt " + std::to_string(attempt) + " failed: " + error.message(), COLOR_ORANGE);
        }
    }
    if (cur == MenuCursorPosition::UNKNOWN){
        OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
            "Menu or cursor detection failed after 3 opening attempts.",
            stream);
    }

    IconGrid cur_g = icon_to_grid(cur, num_icons);
    IconGrid tgt_g = icon_to_grid(target, num_icons);
    if (tgt_g.row == -1){
        OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
            "Unknown target menu icon.",
            stream);
    }

    if (cur_g.row != tgt_g.row){
        DpadPosition vert = (cur_g.row < tgt_g.row) ? DPAD_DOWN : DPAD_UP;
        pbf_press_dpad(context, vert, 80ms, 300ms);
        context.wait_for_all_requests();
    }

    int col_diff = tgt_g.col - cur_g.col;
    if (col_diff != 0){
        DpadPosition horiz = (col_diff > 0) ? DPAD_RIGHT : DPAD_LEFT;
        for (int i = 0; i < std::abs(col_diff); i++){
            pbf_press_dpad(context, horiz, 80ms, 300ms);
            context.wait_for_all_requests();
        }
    }

    pbf_press_button(context, BUTTON_A, 80ms, 200ms);
}


// Search the selected item's fixed description header rather than guessing
// which list row is selected. This remains in place while the list scrolls.
// A cursor parked at row 2 or 6 does not indicate a list end: only repeated
// unchanged item headers after movement are used to detect an end.
// Previous selected-item-only search, retained for reference.
// bool find_bag_item(
//     VideoStream& stream,
//     ProControllerContext& context,
//     const BagItemTemplate& item,
//     int num_icons,
//     size_t max_scrolls_per_pocket,
//     std::chrono::milliseconds search_timeout
// ){
//     auto reference = std::make_shared<const ImageRGB32>(
//         RESOURCE_PATH() + item.reference_path
//     );
//     ImageMatchDetector icon(reference, item.icon_box, item.max_icon_rmsd, true);
//     ImageMatchDetector name(reference, item.name_box, item.max_name_rmsd, true);
//     ImageMatchDetector bag(reference, ImageFloatBox(0.055, 0.012, 0.044, 0.045), 35.0, true);
//     const ImageFloatBox selected_header(0.428, 0.754, 0.382, 0.046);
//     const auto deadline = std::chrono::steady_clock::now() + search_timeout;
//
//     stream.log("[Bag search] Looking for " + item.label + ".");
//     open_menu(stream, context, MenuCursorPosition::BAG, num_icons);
//     pbf_wait(context, 1000ms);
//     context.wait_for_all_requests();
//
//     for (size_t pocket = 0; pocket < 8; pocket++){
//         ImageRGB32 initial_header;
//         ImageRGB32 previous_header;
//         size_t unchanged = 0;
//         size_t matching_frames = 0;
//         size_t scrolls = 0;
//         bool scanning_up = false;
//
//         while (std::chrono::steady_clock::now() < deadline){
//             context.wait_for_all_requests();
//             const VideoSnapshot frame = stream.video().snapshot();
//             if (!frame || !bag.detect(frame)){
//                 stream.log("[Bag search] Bag screen not detected.", COLOR_ORANGE);
//                 return false;
//             }
//             const double icon_score = icon.rmsd(frame);
//             const double name_score = item.check_name ? name.rmsd(frame) : 0.0;
//             stream.log(
//                 "[Bag search] Pocket " + std::to_string(pocket + 1) +
//                 ", scroll " + std::to_string(scrolls) +
//                 ": icon=" + std::to_string(icon_score) +
//                 ", name=" + std::to_string(name_score)
//             );
//             if (icon_score <= item.max_icon_rmsd &&
//                 name_score <= item.max_name_rmsd){
//                 if (++matching_frames >= 3){
//                     stream.log("[Bag search] " + item.label + " confirmed.", COLOR_GREEN);
//                     return true;
//                 }
//                 pbf_wait(context, 150ms);
//                 continue;
//             }
//             matching_frames = 0;
//
//             const ImageRGB32 header = extract_box_reference(frame, selected_header).scale_to(382, 46);
//             if (!initial_header){
//                 initial_header = header.copy();
//             }
//             else if (scrolls > 0){
//                 const bool same_as_previous =
//                     ImageMatch::pixel_RMSD(previous_header, header) <= 3.0;
//                 const bool returned_to_start =
//                     ImageMatch::pixel_RMSD(initial_header, header) <= 3.0;
//                 if (returned_to_start && !same_as_previous){
//                     break;
//                 }
//                 // Repeated unchanged rows mean an empty pocket or list end.
//                 // Returning to the initial row means the list wrapped.
//                 unchanged = same_as_previous || returned_to_start ? unchanged + 1 : 0;
//                 if (unchanged >= 3){
//                     if (scanning_up){
//                         break;
//                     }
//                     // A non-wrapping list may have opened in the middle.
//                     // Search upward as well before moving to another pocket.
//                     scanning_up = true;
//                     unchanged = 0;
//                     initial_header = header.copy();
//                 }
//             }
//             previous_header = header.copy();
//             if (scrolls >= max_scrolls_per_pocket){
//                 stream.log("[Bag search] Scroll limit reached; search incomplete.", COLOR_ORANGE);
//                 return false;
//             }
//             scrolls++;
//             pbf_press_dpad(context, scanning_up ? DPAD_UP : DPAD_DOWN, 80ms, 350ms);
//         }
//         if (std::chrono::steady_clock::now() >= deadline){
//             stream.log("[Bag search] Search timed out; search incomplete.", COLOR_ORANGE);
//             return false;
//         }
//         if (pocket < 7){
//             pbf_press_dpad(context, DPAD_RIGHT, 80ms, 500ms);
//             context.wait_for_all_requests();
//         }
//     }
//     stream.log("[Bag search] Item not found in any pocket.", COLOR_ORANGE);
//     return false;
// }
//

// One narrow strip on the left side of the red selection border. Divide its
// height into the seven row bands; the item's art and NEW badge are outside it.
int bag_cursor_position(const ImageViewRGB32& frame, bool party_target){
    if (!frame){
        return 0;
    }
    if (party_target){
        // Target selection outlines a party card, not an item-list row.
        // The right edge avoids the Pokemon artwork and changing HP digits.
        double best = 0.0;
        double second = 0.0;
        int position = 0;
        for (int row = 0; row < 6; row++){
            const auto patch = extract_box_reference(frame,
                ImageFloatBox{0.356, 0.137 + row * 0.137, 0.004, 0.040});
            size_t red = 0;
            for (size_t y = 0; y < patch.height(); y++){
                for (size_t x = 0; x < patch.width(); x++){
                    const uint32_t pixel = patch.pixel(x, y);
                    const int r = (pixel >> 16) & 255;
                    const int g = (pixel >> 8) & 255;
                    const int b = pixel & 255;
                    red += r > 150 && r > g + 45 && r > b + 45;
                }
            }
            const size_t total = patch.width() * patch.height();
            const double score = total == 0 ? 0.0 : static_cast<double>(red) / total;
            if (score > best){
                second = best;
                best = score;
                position = row + 1;
            }else if (score > second){
                second = score;
            }
        }
        return best >= 0.15 && best - second >= 0.10 ? position : 0;
    }
    const ImageViewRGB32 strip = extract_box_reference(frame, ImageFloatBox(0.431, 0.229, 0.005, 0.470));
    double best = 0.0;
    double second = 0.0;
    int position = 0;
    for (int row = 0; row < 7; row++){
        const size_t top = static_cast<size_t>(strip.height() * (row * 0.075 / 0.470));
        const size_t bottom = static_cast<size_t>(strip.height() * ((row * 0.075 + 0.020) / 0.470));
        size_t red = 0;
        size_t total = 0;
        for (size_t y = top; y < bottom && y < strip.height(); y++){
            for (size_t x = 0; x < strip.width(); x++){
                const uint32_t pixel = strip.pixel(x, y);
                const unsigned r = (pixel >> 16) & 255;
                const unsigned g = (pixel >> 8) & 255;
                const unsigned b = pixel & 255;
                red += r > 180 && g < 120 && b < 120;
                total++;
            }
        }
        const double score = total == 0 ? 0.0 : static_cast<double>(red) / total;
        if (score > best){
            second = best;
            best = score;
            position = row + 1;
        }else if (score > second){
            second = score;
        }
    }
    return best >= 0.15 && best - second >= 0.10 ? position : 0;
}

static ImageFloatBox bag_list_icon_box(size_t row){
    return ImageFloatBox(0.449, 0.212 + (row - 1) * 0.075, 0.033, 0.050);
}

ImageFloatBox bag_list_name_box(size_t row){
    // Previous height included the faint lower selection border in rows 2/6.
    // return ImageFloatBox(0.492, 0.213 + (row - 1) * 0.075, 0.335, 0.052);
    return ImageFloatBox(0.492, 0.213 + (row - 1) * 0.075, 0.335, 0.050);
}

static double bag_row_patch_score(const ImageRGB32& reference, const ImageViewRGB32& frame, const ImageFloatBox& box){
    const ImageRGB32 live = extract_box_reference(frame, box).scale_to(reference.width(), reference.height());
    return ImageMatch::pixel_RMSD(reference, live);
}

// Previous name-area empty-list check retained for reference.
// static bool bag_list_is_empty(const ImageViewRGB32& frame){
//     const ImageViewRGB32 names = extract_box_reference(
//         frame, ImageFloatBox(0.492, 0.213, 0.335, 0.500)
//     );
//     size_t dark = 0;
//     for (size_t y = 0; y < names.height(); y++){
//         for (size_t x = 0; x < names.width(); x++){
//             const uint32_t pixel = names.pixel(x, y);
//             dark += ((pixel >> 16) & 255) < 100 &&
//                 ((pixel >> 8) & 255) < 100 && (pixel & 255) < 100;
//         }
//     }
//     return dark < names.width() * names.height() * 0.001;
// }

size_t bag_clear_tab_count(const ImageViewRGB32& frame, int pocket){
    if (!frame || bag_pocket_position(frame) == 0){
        return 0;
    }
    static const std::vector<std::shared_ptr<const ImageRGB32>> references = [] {
        std::vector<std::shared_ptr<const ImageRGB32>> refs;
        for (const char* name : {"Normal",
            "NormalMedium",
            "NormalSmall",
            "Alternate",
            "AlternateMedium",
            "AlternateSmall",
            "Selected",
            "SelectedMedium",
            "SelectedSmall",
            "SelectedBlue",
            "SelectedBlueMedium",
            "SelectedBlueSmall"}){
            refs.emplace_back(
                std::make_shared<const ImageRGB32>(RESOURCE_PATH() + "PokemonBDSP/Autostory/BagTabs/" + name + ".png"));
        }
        return refs;
    }();
    size_t clear = 0;
    for (size_t tab = 0; tab < 8; tab++){
        if (pocket != 0 && static_cast<int>(tab + 1) != pocket){
            continue;
        }
        const double x = .570 + tab * .0421;
        // A selected tab has a thin red outline. The NEW marker fills the
        // interior of this upper-left patch, which the outline leaves white.
        auto badge = extract_box_reference(frame, ImageFloatBox(x + .007, .074, .006, .014));
        size_t red = 0;
        for (size_t y = 0; y < badge.height(); y++){
            for (size_t bx = 0; bx < badge.width(); bx++){
                const uint32_t pixel = badge.pixel(bx, y);
                int rr = (pixel >> 16) & 255, gg = (pixel >> 8) & 255, bb = pixel & 255;
                red += rr > 150 && rr > gg + 60 && rr > bb + 60;
            }
        }
        if (badge.width() * badge.height() && (double)red / (badge.width() * badge.height()) > .45){
            continue;
        }
        const ImageFloatBox box{x, .06, .038, .072};
        for (const auto& reference : references){
            ImageMatchDetector detector(reference, box, 50.0);
            if (detector.detect(frame)){
                clear++;
                break;
            }
        }
    }
    return clear;
}

bool clear_new_item_flags(VideoStream& stream, ProControllerContext& context, int num_icons,
    const std::array<std::chrono::milliseconds, 8>& scroll_durations){
    stream.log("[Bag NEW flags] Opening bag to verify all eight tabs.");
    open_menu(stream, context, MenuCursorPosition::BAG, num_icons);
    pbf_wait(context, 1000ms);
    auto all_clear = [&](){
        context.wait_for_all_requests();
        size_t confirmed = 0;
        for (size_t check = 0; check < 3; check++){
            const auto frame = stream.video().snapshot();
            const size_t count = frame ? bag_clear_tab_count(*frame.frame) : 0;
            stream.log("[Bag NEW flags] Clear tabs: " + std::to_string(count) + "/8.");
            if (count != 8){
                return false;
            }
            confirmed++;
            pbf_wait(context, 150ms);
            context.wait_for_all_requests();
        }
        return confirmed == 3;
    };
    bool cleared = all_clear();
    for (size_t pocket = 0; pocket < 32 && !cleared; pocket++){
        if (pocket){
            pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
        }
        // Let the tab settle and its initial selection clear before inspecting it.
        pbf_wait(context, 1000ms);
        context.wait_for_all_requests();
        auto frame = stream.video().snapshot();
        const int selected = frame ? bag_pocket_position(*frame.frame) : 0;
        if (selected == 0){
            stream.log("[Bag NEW flags] Selected tab uncertain; stopping.", COLOR_ORANGE);
            break;
        }
        if (bag_list_is_empty(*frame.frame) || bag_clear_tab_count(*frame.frame, selected) == 1){
            stream.log("[Bag NEW flags] Skip empty or already-clear tab " + std::to_string(selected) + ".");
        }else{
            // The caller budgets each pocket independently. Retain the dwell on
            // each row: a long held Down can skip NEW flags without clearing them.
            const auto budget = scroll_durations[selected - 1];
            stream.log("[Bag NEW flags] Tab " + std::to_string(selected) +
                ": scroll budget " + std::to_string(budget.count()) + "ms.");
            for (auto elapsed = 0ms; elapsed + 1080ms <= budget; elapsed += 1080ms){
                pbf_press_dpad(context, DPAD_DOWN, 80ms, 1000ms);
                context.wait_for_all_requests();
                frame = stream.video().snapshot();
                if (frame && bag_clear_tab_count(*frame.frame, selected) == 1){
                    break;
                }
            }
        }
        cleared = all_clear();
    }
    pbf_press_dpad(context, DPAD_UP, 3000ms, 250ms);
    pbf_mash_button(context, BUTTON_B, 2000ms);
    context.wait_for_all_requests();
    stream.log(cleared ? "[Bag NEW flags] All eight tabs clear; bag closed."
        : "[Bag NEW flags] Full pocket pass did not clear all eight tabs; bag closed.",
        cleared ? COLOR_GREEN : COLOR_ORANGE);
    return cleared;
}

int bag_pocket_position(const ImageViewRGB32& frame){
    if (!frame){
        return 0;
    }
    int selected = 0;
    for (int pocket = 1; pocket <= 8; pocket++){
        // The lower left edge of each tab is below the red NEW badge.
        // This strip includes both the normal outline and enlarged selection.
        const auto edge =
            extract_box_reference(frame, ImageFloatBox(0.570 + (pocket - 1) * 0.0421, 0.098, 0.0045, 0.021));
        size_t colored = 0;
        for (size_t y = 0; y < edge.height(); y++){
            for (size_t x = 0; x < edge.width(); x++){
                const uint32_t pixel = edge.pixel(x, y);
                const int r = (pixel >> 16) & 255;
                const int g = (pixel >> 8) & 255;
                const int b = pixel & 255;
                // Red when focused, blue when the item list has focus.
                colored += (r > 160 && r > g + 60 && r > b + 60) || (b > 160 && b > r + 40 && b > g + 20);
            }
        }
        const double pixels = (double)edge.width() * edge.height();
        if (pixels > 0 && colored / pixels >= 0.18){
            if (selected != 0){
                return 0;
            }
            selected = pocket;
        }
    }
    return selected;
}

bool bag_list_is_empty(const ImageViewRGB32& frame){
    if (!frame || bag_pocket_position(frame) == 0 || bag_cursor_position(frame) != 0){
        return false;
    }
    // Empty pockets have no item icons. The central Empty label is outside
    // this column; counting dark pixels in the name area included that label.
    const auto icons = extract_box_reference(frame, ImageFloatBox(0.449, 0.213, 0.032, 0.500));
    const auto stats = image_stats(icons);
    return stats.average.r > 220 && stats.average.g > 220 && stats.average.b > 220 && stats.stddev.sum() < 20;
}


std::array<BagItemRowMatch, 7>
match_bag_item_rows(const ImageViewRGB32& frame, const ImageViewRGB32& reference, const BagItemTemplate& item){
    std::array<BagItemRowMatch, 7> result;
    if (!frame || !reference || item.reference_row < 1 || item.reference_row > 7){
        return result;
    }
    const ImageRGB32 icon = extract_box_reference(reference, bag_list_icon_box(item.reference_row)).copy();
    const ImageRGB32 name = extract_box_reference(reference, bag_list_name_box(item.reference_row)).copy();
    for (size_t row = 1; row <= 7; row++){
        auto& match = result[row - 1];
        match.icon_rmsd = bag_row_patch_score(icon, frame, bag_list_icon_box(row));
        match.name_rmsd = item.check_name ? bag_row_patch_score(name, frame, bag_list_name_box(row)) : 0.0;
        match.matched = match.icon_rmsd <= item.max_icon_rmsd && match.name_rmsd <= item.max_name_rmsd;
    }
    return result;
}

BagMovePlan plan_bag_item_move(int cursor_row, int item_row){
    if (cursor_row < 1 || cursor_row > 7 || item_row < 0 || item_row > 7){
        return {};
    }
    if (item_row != 0){
        if (item_row == cursor_row){
            return {BagMoveAction::CONFIRM, 0};
        }
        return {item_row > cursor_row ? BagMoveAction::DOWN : BagMoveAction::UP,
            static_cast<size_t>(std::abs(item_row - cursor_row))};
    }
    if (cursor_row == 7){
        return {BagMoveAction::REWIND, 1};
    }
    return {BagMoveAction::PAGE_DOWN, cursor_row == 1 ? 13 : static_cast<size_t>(13 - cursor_row)};
}


static int bag_confirm_pocket(VideoStream& stream, ProControllerContext& context){
    int previous = 0;
    size_t matches = 0;
    for (size_t check = 0; check < 10; check++){
        context.wait_for_all_requests();
        const auto frame = stream.video().snapshot();
        const int pocket = frame ? bag_pocket_position(frame) : 0;
        matches = pocket != 0 && pocket == previous ? matches + 1 : pocket != 0 ? 1 : 0;
        previous = pocket;
        if (matches >= 3){
            return pocket;
        }
        pbf_wait(context, 100ms);
    }
    return 0;
}

// static bool bag_advance_pocket(VideoStream& stream, ProControllerContext& context, int current){
//     const int expected = current % 8 + 1;
//     for (size_t attempt = 0; attempt < 3; attempt++){
//         // Recheck before retrying: a delayed change must not cause a second step.
//         const int observed = bag_confirm_pocket(stream, context);
//         if (observed == expected) return true;
//         if (observed != current){
//             stream.log("[Bag search] Pocket selection uncertain; stopping.", COLOR_ORANGE);
//             return false;
//         }
//         pbf_press_dpad(context, DPAD_RIGHT, 80ms, 500ms);
//         const int result = bag_confirm_pocket(stream, context);
//         stream.log("[Bag search] Pocket change: " + std::to_string(current) + " -> " +
//             std::to_string(result) + ", expected " + std::to_string(expected) + ".");
//         if (result == expected) return true;
//     }
//     stream.log("[Bag search] Could not confirm pocket change after 3 attempts.", COLOR_ORANGE);
//     return false;
// }

BagPocketMovePlan plan_bag_pocket_move(int current, int target){
    if (current < 1 || current > 8 || target < 1 || target > 8){
        return {};
    }
    const int right = (target - current + 8) % 8;
    const int left = (current - target + 8) % 8;
    if (right == 0){
        return {BagPocketAction::CONFIRM, 0};
    }
    return right <= left ? BagPocketMovePlan{BagPocketAction::RIGHT, static_cast<size_t>(right)}
        : BagPocketMovePlan{BagPocketAction::LEFT, static_cast<size_t>(left)};
}

bool bag_navigate_to_pocket(VideoStream& stream, ProControllerContext& context, int target){
    int current = bag_confirm_pocket(stream, context);
    for (size_t attempt = 0; attempt < 3; attempt++){
        const auto plan = plan_bag_pocket_move(current, target);
        if (plan.action == BagPocketAction::CONFIRM){
            return true;
        }
        if (plan.action == BagPocketAction::UNKNOWN){
            stream.log("[Bag search] Selected pocket not detected.", COLOR_ORANGE);
            return false;
        }
        stream.log("[Bag search] Pocket " + std::to_string(current) + " to " + std::to_string(target) + ": " +
            (plan.action == BagPocketAction::RIGHT ? "right " : "left ") + std::to_string(plan.presses) +
            " presses, attempt " + std::to_string(attempt + 1) + "/3.");
        for (size_t press = 0; press < plan.presses; press++){
            pbf_press_dpad(context, plan.action == BagPocketAction::RIGHT ? DPAD_RIGHT : DPAD_LEFT, 80ms, 150ms);
        }
        pbf_wait(context, 350ms);
        current = bag_confirm_pocket(stream, context);
        stream.log(
            "[Bag search] Final pocket=" + std::to_string(current) + ", expected=" + std::to_string(target) + ".");
        if (current == target){
            return true;
        }
    }
    stream.log("[Bag search] Target pocket not reached after 3 movement batches.", COLOR_ORANGE);
    return false;
}


bool find_bag_item(VideoStream& stream,
    ProControllerContext& context,
    const BagItemTemplate& item,
    int target_pocket,
    int num_icons,
    size_t max_scrolls_per_pocket,
    std::chrono::milliseconds search_timeout,
    bool bag_already_open,
    Language ocr_language){
    if (target_pocket < 1 || target_pocket > 8 || item.reference_row < 1 || item.reference_row > 7){
        stream.log("[Bag search] Invalid template reference row.", COLOR_ORANGE);
        return false;
    }
    auto reference = std::make_shared<const ImageRGB32>(RESOURCE_PATH() + item.reference_path);
    //     const ImageRGB32 reference_icon = extract_box_reference(
    //         *reference, bag_list_icon_box(item.reference_row)
    //     ).copy();
    //     const ImageRGB32 reference_name = extract_box_reference(
    //         *reference, bag_list_name_box(item.reference_row)
    //     ).copy();
    ImageMatchDetector bag(reference, ImageFloatBox(0.365, 0.22, 0.055, 0.10), 35.0, true);
    const auto deadline = std::chrono::steady_clock::now() + search_timeout;

    if (!bag_already_open){
        open_menu(stream, context, MenuCursorPosition::BAG, num_icons);
        pbf_wait(context, 1000ms);
    }
    context.wait_for_all_requests();

    if (!bag_navigate_to_pocket(stream, context, target_pocket)){
        return false;
    }
    if (ocr_language != Language::None){
        size_t commands = 0;
        for (DpadPosition direction : {DPAD_DOWN, DPAD_UP}){
            size_t confirmed = 0;
            size_t unknown_cursor = 0;
            int first_cursor = 0;
            ImageRGB32 first_page;
            for (size_t scan = 0; scan < 64 && std::chrono::steady_clock::now() < deadline; scan++){
                context.wait_for_all_requests();
                const VideoSnapshot frame = stream.video().snapshot();
                // Verify the shared panel border and pocket without localized text or gender-specific icons.
                const int pocket = frame ? bag_pocket_position(frame) : 0;
                if (!frame || !bag.detect(frame) || pocket != target_pocket){
                    stream.log("[Bag OCR] Pocket " + std::to_string(pocket) + ", expected " +
                        std::to_string(target_pocket) + "; Bag layout/pocket check failed.", COLOR_ORANGE);
                    return false;
                }
                if (bag_list_is_empty(frame)){
                    return false;
                }
                const int cursor = bag_cursor_position(frame);
                if (cursor == 0){
                    confirmed = 0;
                    if (++unknown_cursor >= 3){
                        stream.log("[Bag OCR] Cursor location uncertain.", COLOR_ORANGE);
                        return false;
                    }
                    pbf_wait(context, 150ms);
                    continue;
                }
                unknown_cursor = 0;
                auto result = ShopItemNameReader::instance().read_substring(
                    stream.logger(), ocr_language, extract_box_reference(frame, bag_list_name_box(cursor)));
                result.clear_beyond_log10p(ShopItemNameReader::MAX_LOG10P);
                const bool matches = result.results.size() == 1 &&
                    result.results.begin()->second.token == item.label;
                if (matches){
                    if (++confirmed >= 3){
                        stream.log("[Bag OCR] " + item.label + " confirmed in row " +
                            std::to_string(cursor) + ".", COLOR_GREEN);
                        return true;
                    }
                    pbf_wait(context, 150ms);
                    continue;
                }
                confirmed = 0;
                const ImageRGB32 page =
                    extract_box_reference(frame, ImageFloatBox(0.449, 0.212, 0.378, 0.500)).scale_to(378, 500);
                if (first_page && cursor == first_cursor && ImageMatch::pixel_RMSD(first_page, page) <= 3.0){
                    stream.log("[Bag OCR] Returned to the starting page; ending this direction.");
                    break;
                }
                if (!first_page){
                    first_cursor = cursor;
                    first_page = page.copy();
                }
                if (commands >= max_scrolls_per_pocket || std::chrono::steady_clock::now() >= deadline){
                    stream.log("[Bag OCR] Limit reached; search incomplete.", COLOR_ORANGE);
                    return false;
                }
                commands++;
                pbf_press_dpad(context, direction, 80ms, 300ms);
            }
        }
        stream.log("[Bag OCR] Could not confirm " + item.label + "; no item selected.", COLOR_ORANGE);
        return false;
    }
    // Previously searched all eight pockets. Only inspect the requested pocket now.
    // for (size_t pocket = 0; pocket < 8; pocket++){
    {
        const int selected_pocket = bag_confirm_pocket(stream, context);
        if (selected_pocket == 0){
            stream.log("[Bag search] Selected pocket not detected.", COLOR_ORANGE);
            return false;
        }
        size_t commands = 0;
        bool empty = false;
        auto move = [&](DpadPosition direction, size_t count, bool check_rows = false){
            for (size_t press = 0; press < count; press++){
                if (commands >= max_scrolls_per_pocket || std::chrono::steady_clock::now() >= deadline){
                    stream.log("[Bag search] Limit reached; search incomplete.", COLOR_ORANGE);
                    return false;
                }
                commands++;
                pbf_press_dpad(context, direction, 80ms, 150ms);
                if (check_rows){
                    context.wait_for_all_requests();
                    const VideoSnapshot page_frame = stream.video().snapshot();
                    if (!page_frame || !bag.detect(page_frame)){
                        return false;
                    }
                    const auto matches = match_bag_item_rows(page_frame, *reference, item);
                    for (const auto& match : matches){
                        if (match.matched){
                            // Stop paging; the main loop verifies and selects the row.
                            return true;
                        }
                    }
                }
            }
            pbf_wait(context, 250ms);
            context.wait_for_all_requests();
            return true;
        };
        for (size_t pass = 0; pass < 3 && !empty; pass++){
            size_t matching_frames = 0;
            size_t missing_cursor = 0;
            size_t unchanged_pages = 0;
            ImageRGB32 previous_page;
            stream.log("[Bag search] Looking for " + item.label + ": pocket " + std::to_string(selected_pocket) +
                ", pass " + std::to_string(pass + 1) + "/3.");
            while (std::chrono::steady_clock::now() < deadline){
                const VideoSnapshot frame = stream.video().snapshot();
                if (!frame || !bag.detect(frame)){
                    stream.log("[Bag search] Bag screen not detected.", COLOR_ORANGE);
                    return false;
                }
                const int cursor = bag_cursor_position(frame);
                int target = 0;
                double best = 1000.0;
                bool ambiguous = false;
                // The seven list ROIs move neither with the cursor nor with
                // scrolling. Each contains an icon patch and a name patch.
                // Previous per-row loop retained for reference.
                // for (size_t row = 1; row <= 7; row++){
                // const double icon = bag_row_patch_score(reference_icon, frame, bag_list_icon_box(row));
                // const double name = item.check_name
                // ? bag_row_patch_score(reference_name, frame, bag_list_name_box(row)) : 0.0;
                // if (icon <= item.max_icon_rmsd && name <= item.max_name_rmsd){
                // if (target != 0){
                // ambiguous = true;
                // }
                // if (icon + name < best){
                // target = static_cast<int>(row);
                // best = icon + name;
                // }
                // }
                // }
                const auto matches = match_bag_item_rows(frame, *reference, item);
                for (size_t row = 1; row <= 7; row++){
                    const auto& match = matches[row - 1];
                    if (match.matched){
                        if (target != 0){
                            ambiguous = true;
                        }
                        if (match.icon_rmsd + match.name_rmsd < best){
                            target = static_cast<int>(row);
                            best = match.icon_rmsd + match.name_rmsd;
                        }
                    }
                }
                stream.log("[Bag search] " + item.label + (target == 0 ? " is not detected" : " is detected") +
                    " on screen; item row=" + std::to_string(target) + ", cursor row=" + std::to_string(cursor) + ".");
                if (ambiguous){
                    stream.log("[Bag search] Multiple rows match; selection is uncertain.", COLOR_ORANGE);
                    return false;
                }
                if (cursor == 0){
                    matching_frames = 0;
                    if (bag_list_is_empty(frame)){
                        empty = true;
                        break;
                    }
                    if (++missing_cursor >= 3){
                        stream.log("[Bag search] Cursor location uncertain.", COLOR_ORANGE);
                        return false;
                    }
                    pbf_wait(context, 200ms);
                    context.wait_for_all_requests();
                    continue;
                }
                missing_cursor = 0;
                if (target != 0){
                    previous_page = ImageRGB32();
                    unchanged_pages = 0;
                    // A unique list sprite/name match with the cursor on that
                    // row is sufficient; do not depend on the detail panel.
                    if (target == cursor){
                        if (++matching_frames >= 3){
                            stream.log("[Bag search] Item and cursor confirmed in row " + std::to_string(cursor) + ".",
                                COLOR_GREEN);
                            return true;
                        }
                        pbf_wait(context, 150ms);
                        context.wait_for_all_requests();
                        continue;
                    }
                    matching_frames = 0;
                    const size_t count = plan_bag_item_move(cursor, target).presses;
                    stream.log("[Bag search] Moving " + std::string(target > cursor ? "down " : "up ") +
                        std::to_string(count) + " time(s), then checking again.");
                    if (!move(target > cursor ? DPAD_DOWN : DPAD_UP, count)){
                        return false;
                    }
                    continue;
                }
                matching_frames = 0;
                const ImageRGB32 page =
                    extract_box_reference(frame, ImageFloatBox(0.449, 0.212, 0.378, 0.500)).scale_to(378, 500);
                if (previous_page && ImageMatch::pixel_RMSD(previous_page, page) <= 3.0){
                    unchanged_pages++;
                }else{
                    unchanged_pages = 0;
                }
                previous_page = page.copy();
                // Row 7 signals the bottom of a long list. Repeated pages
                // also cover shorter lists whose final row is less than 7.
                if (cursor == 7 || unchanged_pages >= 2){
                    stream.log("[Bag search] List end; returning to row 1.");
                    while (true){
                        const VideoSnapshot rewind_frame = stream.video().snapshot();
                        if (!rewind_frame || !bag.detect(rewind_frame)){
                            return false;
                        }
                        const int rewind_cursor = bag_cursor_position(rewind_frame);
                        if (rewind_cursor == 1){
                            break;
                        }
                        if (rewind_cursor == 0 || !move(DPAD_UP, 1)){
                            return false;
                        }
                    }
                    break;
                }
                const size_t count = plan_bag_item_move(cursor, 0).presses;
                stream.log("[Bag search] Advancing " + std::to_string(count) + " presses to another page.");
                if (!move(DPAD_DOWN, count, true)){
                    return false;
                }
            }
            if (std::chrono::steady_clock::now() >= deadline){
                stream.log("[Bag search] Timeout; search incomplete.", COLOR_ORANGE);
                return false;
            }
        }
        //         if (pocket < 7){
        //             // pbf_press_dpad(context, DPAD_RIGHT, 80ms, 500ms);
        //             // context.wait_for_all_requests();
        //             if (!bag_advance_pocket(stream, context, selected_pocket)) return false;
        //         }
    }
    stream.log("[Bag search] Item not found after three passes per nonempty pocket.", COLOR_ORANGE);
    return false;
}

bool use_escape_rope(VideoStream& stream, ProControllerContext& context, Language language, int num_icons){
    const BagItemTemplate rope{// "PokemonBDSP/AutoStory/Bag_EscapeRope.png", "escape-rope"
        "PokemonBDSP/Autostory/Bag_EscapeRope.png",
        "escape-rope"};
    if (!find_bag_item(stream, context, rope, 5, num_icons, 128, 60s, false, language)){
        return false;
    }

    pbf_press_button(context, BUTTON_A, 80ms, 350ms);
    context.wait_for_all_requests();
    SelectionArrowFinder use_menu(stream.overlay(), ImageFloatBox(0.660, 0.100, 0.310, 0.430), COLOR_GREEN);
    if (wait_until(stream, context, 3s, {{use_menu}}) != 0){
        stream.log("[Escape Rope] Item action menu not detected.", COLOR_ORANGE);
        return false;
    }

    // Watch while selecting Use so the transition cannot happen before
    // the watcher starts. Extra A presses handle a confirmation prompt.
    BlackScreenWatcher transition;
    // Escape Rope fades white in the supplied clips; support black transitions too.
    WhiteScreenWatcher white_transition;
    const int result = run_until<ProControllerContext>(stream,
        context,
        [](ProControllerContext& ctx){
            for (size_t press = 0; press < 10; press++){
                pbf_press_button(ctx, BUTTON_A, 80ms, 500ms);
            }
            ctx.wait_for_all_requests();
            pbf_wait(ctx, 10s);
            ctx.wait_for_all_requests();
        },
        {transition, white_transition});
    if (result < 0){
        stream.log("[Escape Rope] No exit transition detected.", COLOR_ORANGE);
        return false;
    }
    BlackScreenWatcher finished(COLOR_RED, {0.1, 0.1, 0.8, 0.8}, 100, 10, BlackScreenWatcher::FinderType::GONE, 500ms);
    WhiteScreenWatcher white_finished(COLOR_RED,
        {0.1, 0.1, 0.8, 0.8},
        500,
        10,
        WhiteScreenWatcher::FinderType::GONE,
        500ms);
    stream.log(std::string("[Escape Rope] ") + (result == 1 ? "White" : "Black") + " exit transition detected.");
    // if (wait_until(stream, context, 15s, {{finished}}) != 0){ return false; }
    const int finish_result = result == 1 ? wait_until(stream, context, 15s, {{white_finished}})
        : wait_until(stream, context, 15s, {{finished}});
    if (finish_result != 0){
        stream.log("[Escape Rope] Exit transition did not finish.", COLOR_ORANGE);
        return false;
    }
    OverworldWatcher overworld;
    if (wait_until(stream, context, 10s, {{overworld}}) != 0){
        stream.log("[Escape Rope] Overworld not detected after transition.", COLOR_ORANGE);
        return false;
    }
    pbf_wait(context, 1000ms);
    context.wait_for_all_requests();
    stream.log("[Escape Rope] Exit transition and overworld confirmed.", COLOR_GREEN);
    return true;
}


bool activate_repel(VideoStream& stream, ProControllerContext& context, Language language, int num_icons, int repel_type){
    static const char* const slugs[] = {"repel", "super-repel", "max-repel"};
    if (repel_type < 0 || repel_type > 2){
        stream.log("activate_repel: type must be 0 (Repel), 1 (Super Repel), or 2 (Max Repel).", COLOR_RED);
        return false;
    }
    const std::string slug = slugs[repel_type];
    stream.log("Activating " + slug);
    BagItemTemplate repel;
    // OCR identifies the item; this reference is used only for the Bag layout.
    repel.reference_path = "PokemonBDSP/Autostory/Bag_EscapeRope.png";
    repel.label = slug;
    if (!find_bag_item(stream, context, repel, 5, num_icons, 128, 60s, false, language)){
        stream.log("activate_repel: requested item not confirmed: " + slug, COLOR_RED);
        pbf_mash_button(context, BUTTON_B, 2000ms);
        context.wait_for_all_requests();
        return false;
    }
    stream.log("[AutoStory] activate_repel: selected item confirmed: " + slug, COLOR_GREEN);

    context.wait_for_all_requests();
    pbf_wait(context, 400ms);
    pbf_press_button(context, BUTTON_A, 80ms, 300ms);
    context.wait_for_all_requests();

    // The Use-item popup follows the selected row, which may be anywhere
    // in a later save's list. Exclude the Bag list cursor on the left.
    const ImageFloatBox repel_item_box{0.700000, 0.100000, 0.280000, 0.650000};
    SelectionArrowFinder arrow_finder(stream.overlay(), repel_item_box, COLOR_GREEN);
    int arrow_ret = wait_until(stream, context, 2000ms, {{arrow_finder}});
    if (arrow_ret != 0){
        stream.log("[AutoStory] activate_repel: selection arrow not found after pressing A.", COLOR_RED);
        return false;
    }
    stream.log("[AutoStory] activate_repel: selection arrow confirmed.", COLOR_GREEN);
    pbf_press_button(context, BUTTON_A, 80ms, 105ms);
    pbf_mash_button(context, BUTTON_B, 2000ms);
    return true;
}


void use_HM(VideoStream& stream, ProControllerContext& context, const std::string& label

){
    stream.log("Using " + label);
    const ImageFloatBox box{0.670000, 0.600000, 0.100000, 0.150000};
    SelectionArrowFinder arrow(stream.overlay(), box, COLOR_GREEN);

    int ret = run_until<ProControllerContext>(stream,
        context,
        [](ProControllerContext& context){
            pbf_mash_button(context, BUTTON_A, 30000ms);
        },
        {{arrow}});
    if (ret != 0){
        OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
            "Using " + label + ": HM confirmation arrow not found; stopping before further movement.",
            stream);
    }

    stream.log("Using " + label + ": selection arrow found, confirming.", COLOR_GREEN);
    pbf_mash_button(context, BUTTON_A, 1000ms);
    pbf_mash_button(context, BUTTON_B, 6000ms);
}


bool get_on_bicycle(VideoStream& stream, ProControllerContext& context, GearLevel /*gear*/){
    static const ImageFloatBox FRONT_BOX{0.440000, 0.470000, 0.035000, 0.054000};
    static const ImageFloatBox BACK_BOX{0.520000, 0.470000, 0.022000, 0.054000};
    // Spark color range: white (R=232,G=255,B=255) down to light green (R=163,G=217,B=181)
    static const uint32_t SPARK_MIN = 0xFF000000u | (163u << 16) | (217u << 8) | 181u;
    static const uint32_t SPARK_MAX = 0xFFFFFFFFu;
    static const size_t SPARK_THRESHOLD = 13;

    stream.log("[AutoStory] get_on_bicycle: mounting bicycle.", COLOR_WHITE);
    pbf_press_button(context, BUTTON_PLUS, 80ms, 800ms);

    // Press B, then poll up to 1 s for Fast-gear sparks.
    // Logs the peak pixel counts in both boxes for threshold tuning.
    // Returns true if sparks are detected (switched TO Fast gear).
    auto toggle_and_check = [&]() -> bool {
        context.wait_for_all_requests();
        pbf_press_button(context, BUTTON_B, 80ms, 800ms);

        size_t max_front = 0, max_back = 0;
        auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(1000);
        while (std::chrono::steady_clock::now() < deadline){
            context.wait_for(50ms);
            VideoSnapshot screen = stream.video().snapshot();
            if (!screen){
                continue;
            }

            size_t front_pixels = 0, back_pixels = 0;
            filter_rgb32_range(front_pixels,
                extract_box_reference(screen, FRONT_BOX),
                SPARK_MIN,
                SPARK_MAX,
                Color(0),
                false);
            filter_rgb32_range(back_pixels,
                extract_box_reference(screen, BACK_BOX),
                SPARK_MIN,
                SPARK_MAX,
                Color(0),
                false);

            if (front_pixels > max_front){
                max_front = front_pixels;
            }
            if (back_pixels > max_back){
                max_back = back_pixels;
            }

            if (front_pixels >= SPARK_THRESHOLD || back_pixels >= SPARK_THRESHOLD){
                stream.log("[AutoStory] get_on_bicycle: sparks detected "
                    "(front=" +
                    std::to_string(front_pixels) + " back=" + std::to_string(back_pixels) +
                    " threshold=" + std::to_string(SPARK_THRESHOLD) + ").",
                    COLOR_GREEN);
                return true;
            }
        }
        stream.log("[AutoStory] get_on_bicycle: no sparks detected "
            "(peak front=" +
            std::to_string(max_front) + " back=" + std::to_string(max_back) +
            " threshold=" + std::to_string(SPARK_THRESHOLD) + ").",
            COLOR_ORANGE);
        return false;
    };

    if (toggle_and_check()){
        stream.log("[AutoStory] get_on_bicycle: Fast gear confirmed.", COLOR_GREEN);
        return true;
    }

    // First toggle produced no sparks -> we switched to Slow gear; toggle back to Fast.
    stream.log("[AutoStory] get_on_bicycle: Slow gear detected, toggling back to Fast.", COLOR_ORANGE);
    if (toggle_and_check()){
        stream.log("[AutoStory] get_on_bicycle: Fast gear confirmed after correction.", COLOR_GREEN);
        return true;
    }

    stream.log("[AutoStory] get_on_bicycle: Failed to confirm Fast gear after two toggles.", COLOR_RED);
    return false;
}


}
}
}