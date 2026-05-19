/*  BDSP AutoStory - Battle Utilities
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <array>
#include <vector>
#include <string>
#include <algorithm>
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/OCR/OCR_RawOCR.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_StatsCalculation.h"
#include "PokemonSwSh/Resources/PokemonSwSh_TypeMatchup.h"
#include "PokemonBDSP/Inference/Battles/PokemonBDSP_BattleMenuDetector.h"
#include "PokemonBDSP/Inference/Battles/PokemonBDSP_EndBattleDetector.h"
#include "PokemonBDSP/Inference/Battles/PokemonBDSP_StartBattleDetector.h"
#include "PokemonBDSP/Inference/Battles/PokemonBDSP_ExperienceGainDetector.h"
#include "PokemonBDSP/Inference/Battles/PokemonBDSP_BattleOpponentNameReader.h"
#include "PokemonBDSP/Programs/AutoStory/Data/PokemonBDSP_AutoStory_BDSPData.h"
#include "PokemonBDSP/Programs/AutoStory/Detect/PokemonBDSP_AutoStory_OverworldDetector.h"
#include "PokemonBDSP_AutoStory_Battle.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{

using namespace Pokemon;
using namespace PokemonSwSh;
using namespace std::chrono_literals;


// ---------------------------------------------------------------------------
//  run_battle_press_A
// ---------------------------------------------------------------------------

void run_battle_press_A(
    const ProgramInfo& /*info*/,
    VideoStream& stream,
    ProControllerContext& context
){
    EndBattleWatcher end_battle;
    run_until<ProControllerContext>(
        stream, context,
        [](ProControllerContext& ctx){ pbf_mash_button(ctx, BUTTON_A, 120000ms); },
        {{end_battle}}
    );

    OverworldWatcher overworld;
    run_until<ProControllerContext>(
        stream, context,
        [](ProControllerContext& ctx){ pbf_mash_button(ctx, BUTTON_A, 120000ms); },
        {{overworld}}
    );
}


// ---------------------------------------------------------------------------
//  handle_unexpected_battle
// ---------------------------------------------------------------------------

void handle_unexpected_battle(
    const ProgramInfo& info,
    VideoStream& stream,
    ProControllerContext& context,
    std::function<void(const ProgramInfo&, VideoStream&, ProControllerContext&)>&& action,
    std::function<void(const ProgramInfo&, VideoStream&, ProControllerContext&)>&& recovery_action,
    size_t max_attempts
){
    for (size_t attempt = 0; attempt < max_attempts; attempt++){
        StartBattleDetector start_battle(stream.overlay());
        int result = run_until<ProControllerContext>(
            stream, context,
            [&](ProControllerContext& ctx){ action(info, stream, ctx); },
            {{start_battle}}
        );
        if (result < 0){
            return;
        }
        run_battle_press_A(info, stream, context);
        recovery_action(info, stream, context);
    }
    OperationFailedException::fire(
        ErrorReport::SEND_ERROR_REPORT,
        "handle_unexpected_battle: exceeded max attempts",
        stream
    );
}


// ---------------------------------------------------------------------------
//  Internal helpers for handle_battle_with_outside_help
// ---------------------------------------------------------------------------

static constexpr int TORTERRA_ATK   = 238;
static constexpr int TORTERRA_SPATK = 170;

//  ⚠️ Estimated PP regions (fight menu, 2×2 move grid); require calibration.
static const ImageFloatBox PP_BOX[4] = {
    {0.36, 0.590, 0.10, 0.060},
    {0.75, 0.590, 0.10, 0.060},
    {0.36, 0.730, 0.10, 0.060},
    {0.75, 0.730, 0.10, 0.060},
};

static int ocr_pp_slot(Logger& logger, const ImageViewRGB32& frame, const ImageFloatBox& box){
    ImageViewRGB32 region = extract_box_reference(frame, box);
    std::string text = OCR::ocr_read(Language::English, region, OCR::PageSegMode::SINGLE_LINE);
    text.erase(0, text.find_first_not_of(" \t\r\n"));
    if (!text.empty()) text.erase(text.find_last_not_of(" \t\r\n") + 1);

    size_t slash = text.find('/');
    std::string num_str = (slash != std::string::npos) ? text.substr(0, slash) : text;
    size_t digit_pos = num_str.find_first_of("0123456789");
    if (digit_pos == std::string::npos){
        logger.log("PP OCR: no digit in \"" + text + "\"", COLOR_YELLOW);
        return -1;
    }
    try {
        return std::stoi(num_str.substr(digit_pos));
    } catch (...) {
        logger.log("PP OCR: parse failed \"" + num_str + "\"", COLOR_YELLOW);
        return -1;
    }
}

static std::array<int, 4> read_all_pp(Logger& logger, VideoStream& stream){
    VideoSnapshot frame = stream.video().snapshot();
    std::array<int, 4> result;
    for (int i = 0; i < 4; i++) result[i] = ocr_pp_slot(logger, frame, PP_BOX[i]);
    return result;
}

static std::pair<int,int> damage_range(int level, int base_power, int atk, int def, double type_mult){
    if (base_power == 0) return {0, 0};
    int base = (((2 * level) / 5 + 2) * base_power * atk / def) / 50 + 2;
    return {(int)(base * 85.0 / 100.0 * type_mult), (int)(base * type_mult)};
}

struct MoveScore {
    int  lower_sum = 0;
    int  upper_sum = 0;
    bool ohko_all  = false;
};

static MoveScore score_move(
    const BDSPMoveData& mdata,
    const std::vector<std::string>& on_field,
    const BDSPTrainerData& trainer_data
){
    if (mdata.category == MoveCategory::STATUS) return {};

    int atk_stat = (mdata.category == MoveCategory::PHYSICAL) ? TORTERRA_ATK : TORTERRA_SPATK;

    MoveScore score;
    score.ohko_all = !on_field.empty();

    for (const std::string& slug : on_field){
        if (slug.empty()){ score.ohko_all = false; continue; }

        const BDSPTrainerPokemon* tp = nullptr;
        for (const BDSPTrainer& trainer : trainer_data.trainers){
            for (const BDSPTrainerPokemon& mon : trainer.pokemon){
                if (mon.species == slug){ tp = &mon; break; }
            }
            if (tp) break;
        }
        if (!tp){ score.ohko_all = false; continue; }

        const BDSPPokemonData& pdata = bdsp_pokemon(slug);
        uint16_t hp  = calc_stats_hp(pdata.hp, tp->level, tp->ivs.hp, tp->evs.hp);
        uint16_t def = (mdata.category == MoveCategory::PHYSICAL)
            ? calc_stats_nonhp(pdata.defense, tp->level, tp->ivs.defense, tp->evs.defense, NatureAdjustment::NEUTRAL)
            : calc_stats_nonhp(pdata.spdef,   tp->level, tp->ivs.spdef,   tp->evs.spdef,   NatureAdjustment::NEUTRAL);

        double mult = type_multiplier(mdata.type, pdata.type[0], pdata.type[1]);
        if (mdata.spread && trainer_data.battle_type == BDSPBattleType::DOUBLES) mult *= 0.75;

        auto [lo, hi] = damage_range(100, (int)mdata.base_power, atk_stat, (int)def, mult);
        score.lower_sum += lo;
        score.upper_sum += hi;
        if (lo < (int)hp) score.ohko_all = false;
    }
    return score;
}

static void navigate_to_slot(ProControllerContext& context, int slot){
    if (slot == 1 || slot == 3) pbf_press_dpad(context, DPAD_RIGHT, 10ms, 40ms);
    if (slot == 2 || slot == 3) pbf_press_dpad(context, DPAD_DOWN,  10ms, 40ms);
}


// ---------------------------------------------------------------------------
//  handle_battle_with_outside_help
// ---------------------------------------------------------------------------

void handle_battle_with_outside_help(
    const ProgramInfo& /*info*/,
    VideoStream& stream,
    ProControllerContext& context,
    const std::string& trainer_id,
    const AutoStoryOptions& options
){
    Logger& logger = stream.logger();
    const BDSPTrainerData& trainer_data = bdsp_trainer(trainer_id);
    const OutsideHelpConfig& help = options.outside_help;

    std::array<std::string, 4> move_slugs;
    for (int i = 0; i < 4; i++) move_slugs[i] = torterra_move_slug(help.attacks[i]);

    std::array<int, 4> pp_remaining;
    for (int i = 0; i < 4; i++) pp_remaining[i] = (int)bdsp_move(move_slugs[i]).pp;

    std::vector<std::string> on_field;
    for (const BDSPTrainer& trainer : trainer_data.trainers){
        if (!trainer.pokemon.empty()) on_field.push_back(trainer.pokemon[0].species);
    }

    bool battle_active = true;
    while (battle_active){

        // Wait for Torterra's battle menu
        {
            BattleMenuWatcher menu(BattleType::TRAINER);
            EndBattleWatcher  end;
            int r = wait_until(stream, context, 120s, {{menu}, {end}});
            if (r == 1 || r < 0){ battle_active = false; break; }
        }

        // Open Fight menu
        pbf_press_button(context, BUTTON_A, 10ms, 60ms);
        context.wait_for_all_requests();
        context.wait_for(300ms);

        // OCR current PP
        {
            auto ocr_pp = read_all_pp(logger, stream);
            for (int i = 0; i < 4; i++){
                if (ocr_pp[i] >= 0) pp_remaining[i] = ocr_pp[i];
                else logger.log("PP OCR failed slot " + std::to_string(i) + "; using tracked", COLOR_YELLOW);
            }
        }

        // Check for all-zero PP (Struggle)
        bool all_zero = true;
        for (int pp : pp_remaining) if (pp > 0){ all_zero = false; break; }

        if (all_zero){
            logger.log("All PP at 0 — using Struggle", COLOR_YELLOW);
            pbf_press_button(context, BUTTON_B, 10ms, 60ms);
            context.wait_for_all_requests();
        } else {
            // Score all usable moves
            int  best_slot = -1, best_hi = -1;
            bool best_ohko = false;

            for (int i = 0; i < 4; i++){
                if (pp_remaining[i] <= 0) continue;
                MoveScore s = score_move(bdsp_move(move_slugs[i]), on_field, trainer_data);
                if (bdsp_move(move_slugs[i]).category == MoveCategory::STATUS) continue;

                bool prefer = (best_slot < 0)
                    || (s.ohko_all && !best_ohko)
                    || (s.ohko_all && best_ohko  && s.upper_sum < best_hi)
                    || (!s.ohko_all && !best_ohko && s.upper_sum > best_hi);

                if (prefer){ best_slot = i; best_hi = s.upper_sum; best_ohko = s.ohko_all; }
            }
            if (best_slot < 0){
                for (int i = 0; i < 4; i++) if (pp_remaining[i] > 0){ best_slot = i; break; }
            }

            logger.log(
                "Move slot " + std::to_string(best_slot) + " (" + move_slugs[best_slot] + ")"
                + (best_ohko ? " [OHKO]" : " [best effort]"),
                COLOR_WHITE
            );

            navigate_to_slot(context, best_slot);
            pbf_mash_button(context, BUTTON_A, 500ms);
            context.wait_for_all_requests();
            pp_remaining[best_slot]--;

            // Single-target in doubles: confirm default target
            if (!bdsp_move(move_slugs[best_slot]).spread
                && trainer_data.battle_type == BDSPBattleType::DOUBLES)
            {
                pbf_press_button(context, BUTTON_A, 10ms, 60ms);
                context.wait_for_all_requests();
            }

            // Mash through target confirmation and partner's move selection
            pbf_mash_button(context, BUTTON_A, 3000ms);
            context.wait_for_all_requests();
        }

        // Wait for next event
        {
            BattleMenuWatcher menu2(BattleType::TRAINER);
            ExperienceGainWatcher exp;
            EndBattleWatcher      end;

            int event = wait_until(stream, context, 60s, {{menu2}, {exp}, {end}});

            if (event == 2 || event < 0){
                battle_active = false;
            } else if (event == 1){
                // XP gain — mash through, then update on_field via OCR
                BattleMenuWatcher menu3(BattleType::TRAINER);
                EndBattleWatcher  end2;
                int after = run_until<ProControllerContext>(
                    stream, context,
                    [](ProControllerContext& ctx){ pbf_mash_button(ctx, BUTTON_A, 120000ms); },
                    {{menu3}, {end2}}
                );
                if (after == 1 || after < 0){
                    battle_active = false;
                } else {
                    // Update on_field via OCR
                    VideoSnapshot frame = stream.video().snapshot();
                    std::vector<std::string> names = read_battle_opponent_names(
                        logger, options.language, frame, trainer_data.battle_type
                    );
                    for (size_t i = 0; i < names.size() && i < on_field.size(); i++){
                        if (!names[i].empty() && names[i] != on_field[i]){
                            logger.log("Slot " + std::to_string(i) + ": " + on_field[i] + " → " + names[i], COLOR_WHITE);
                            on_field[i] = names[i];
                        }
                    }
                }
            }
            // event == 0: next turn, loop continues
        }
    }

    // Mash through XP/dialog until overworld
    OverworldWatcher overworld;
    run_until<ProControllerContext>(
        stream, context,
        [](ProControllerContext& ctx){ pbf_mash_button(ctx, BUTTON_A, 120000ms); },
        {{overworld}}
    );
}


}
}
}
