/*  BDSP AutoStory - Battle Utilities
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <array>
#include <map>
#include <set>
#include <vector>
#include <string>
#include <algorithm>
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/Images/SolidColorTest.h"
#include "CommonTools/OCR/OCR_RawOCR.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_StatsCalculation.h"
#include "Pokemon/Inference/Pokemon_NameReader.h"
#include "PokemonSwSh/Resources/PokemonSwSh_TypeMatchup.h"
#include "PokemonBDSP/Inference/Battles/PokemonBDSP_BattleMenuDetector.h"
#include "PokemonBDSP/Inference/Battles/PokemonBDSP_EndBattleDetector.h"
#include "PokemonBDSP/Inference/Battles/PokemonBDSP_StartBattleDetector.h"
#include "PokemonBDSP/Inference/Battles/PokemonBDSP_ExperienceGainDetector.h"
#include "PokemonBDSP/Inference/Battles/PokemonBDSP_BattleOpponentNameReader.h"
#include "PokemonBDSP/Inference/PokemonBDSP_ShopItemNameReader.h"
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


// ---------------------------------------------------------------------------
//  Internal helpers for handle_expert_battle
// ---------------------------------------------------------------------------

//  ⚠️ Estimated player-side HP bar region — calibrate against a real capture.
static const ImageFloatBox PLAYER_HP_BAR    {0.057, 0.652, 0.168, 0.008};

//  ⚠️ Estimated player-side status badge region — calibrate against a real capture.
static const ImageFloatBox PLAYER_STATUS_BOX{0.140, 0.710, 0.080, 0.018};

//  ⚠️ Estimated battle-bag item name region (highlighted row) — calibrate.
static const ImageFloatBox BATTLE_BAG_ITEM_NAME_BOX{0.470, 0.150, 0.350, 0.050};

//  Returns the fraction [0.0, 1.0] of the player's HP bar that is filled.
//  Scans each pixel column; a column with high colour saturation counts as "HP present".
static double read_player_hp_ratio(const ImageViewRGB32& frame){
    ImageViewRGB32 strip = extract_box_reference(frame, PLAYER_HP_BAR);
    int total   = (int)strip.width();
    int mid_row = (int)(strip.height() / 2);
    if (total <= 0) return 1.0;

    int colored_cols = 0;
    for (int x = 0; x < total; x++){
        uint32_t px = strip.pixel(x, mid_row);
        uint8_t r = (px >> 16) & 0xFF;
        uint8_t g = (px >>  8) & 0xFF;
        uint8_t b =  px        & 0xFF;
        int mx = std::max({(int)r, (int)g, (int)b});
        int mn = std::min({(int)r, (int)g, (int)b});
        if (mx - mn > 30) colored_cols++;
    }
    return (double)colored_cols / total;
}

//  Returns true if a coloured status badge is visible on the player's active pokemon.
static bool detect_player_status(const ImageViewRGB32& frame){
    return !is_white(extract_box_reference(frame, PLAYER_STATUS_BOX));
}

//  Cycle through battle-bag pockets with BUTTON_R until the highlighted item
//  matches "full-restore", then use it on the first pokemon.
//  Called while the battle-bag is already open (navigate to Bag from battle menu first).
static void use_full_restore_in_open_bag(
    VideoStream& stream,
    ProControllerContext& context,
    Language language
){
    Logger& logger = stream.logger();
    static constexpr size_t MAX_POCKETS = 9;
    static constexpr size_t MAX_SCROLL  = 25;

    // Cycle pockets until we see Full Restore in the highlighted row
    bool pocket_found = false;
    for (size_t tab = 0; tab < MAX_POCKETS; tab++){
        context.wait_for(500ms);
        VideoSnapshot frame = stream.video().snapshot();
        ImageViewRGB32 region = extract_box_reference(frame, BATTLE_BAG_ITEM_NAME_BOX);
        OCR::StringMatchResult result = ShopItemNameReader::instance().read_substring(
            logger, language, region
        );
        for (const auto& [score, match] : result.results){
            if (match.token == "full-restore" && score <= ShopItemNameReader::MAX_LOG10P){
                pocket_found = true;
                break;
            }
        }
        if (pocket_found) break;
        pbf_press_button(context, BUTTON_R, 10ms, 400ms);
    }

    if (!pocket_found){
        logger.log("Full Restore pocket not found — scanning list", COLOR_YELLOW);
    }

    // Scroll down within the pocket until the highlighted row is Full Restore
    for (size_t i = 0; i < MAX_SCROLL; i++){
        context.wait_for(400ms);
        VideoSnapshot frame = stream.video().snapshot();
        ImageViewRGB32 region = extract_box_reference(frame, BATTLE_BAG_ITEM_NAME_BOX);
        OCR::StringMatchResult result = ShopItemNameReader::instance().read_substring(
            logger, language, region
        );
        bool found = false;
        for (const auto& [score, match] : result.results){
            if (match.token == "full-restore" && score <= ShopItemNameReader::MAX_LOG10P){
                found = true;
                break;
            }
        }
        if (found){
            pbf_press_button(context, BUTTON_A, 10ms, 600ms); // select item
            pbf_press_button(context, BUTTON_A, 10ms, 600ms); // "Use"
            pbf_press_button(context, BUTTON_A, 10ms, 800ms); // confirm on first pokemon
            context.wait_for(500ms);
            logger.log("Full Restore used", COLOR_WHITE);
            pbf_mash_button(context, BUTTON_B, 1500ms);
            return;
        }
        pbf_press_dpad(context, DPAD_DOWN, 10ms, 200ms);
    }

    logger.log("Full Restore not found in bag — skipping", COLOR_YELLOW);
    pbf_mash_button(context, BUTTON_B, 1500ms);
}

//  From the battle menu (Fight/Pokemon/Bag/Run), navigate to Bag and open it.
//  In BDSP the battle menu is a 2×2 grid: Fight(top-left) Bag(top-right)
//  Pokemon(bottom-left) Run(bottom-right). Cursor starts at Fight.
static void open_battle_bag(ProControllerContext& context){
    pbf_press_dpad(context, DPAD_RIGHT, 10ms, 200ms);
    pbf_press_button(context, BUTTON_A,  10ms, 800ms);
    context.wait_for(500ms);
}

//  Use a Full Restore on the first party pokemon from the battle menu.
//  Returns immediately after item use; the caller is responsible for resuming the turn.
static void use_full_restore_in_battle(
    VideoStream& stream,
    ProControllerContext& context,
    Language language
){
    open_battle_bag(context);
    use_full_restore_in_open_bag(stream, context, language);
    // Mash B to ensure we're back at the battle menu
    context.wait_for(300ms);
}

//  OCR the opponent name box and restrict matches to `candidates`.
//  Returns the best matching slug in candidates, or the best unrestricted match
//  if no candidate qualifies.
static std::string ocr_opponent_restricted(
    Logger& logger,
    Language language,
    const ImageViewRGB32& frame,
    const ImageFloatBox& box,
    const std::vector<std::string>& candidates
){
    using namespace Pokemon;
    ImageViewRGB32 cropped = extract_box_reference(frame, box);
    OCR::StringMatchResult result = PokemonNameReader::instance().read_substring(
        logger, language, cropped, OCR::BLACK_OR_WHITE_TEXT_FILTERS()
    );
    if (result.results.empty()) return "";

    // Prefer the best candidate-restricted match
    for (const auto& [score, match] : result.results){
        if (score > PokemonNameReader::MAX_LOG10P) break;
        for (const std::string& c : candidates){
            if (match.token == c) return c;
        }
    }
    // Fallback: best unrestricted match (if it meets threshold)
    const auto& best = result.results.begin();
    if (best->first <= PokemonNameReader::MAX_LOG10P) return best->second.token;
    return "";
}


// ---------------------------------------------------------------------------
//  Pre-plan computation
// ---------------------------------------------------------------------------

struct BattlePlan {
    // species slug → move slot to use against that pokemon (for current trainer)
    std::map<std::string, int> move_for_species;
    // Estimated remaining PP after the full trainer sequence is played out
    std::array<int, 4> estimated_pp_after{};
};

//  Returns the Intimidate attack-stage multiplier for `count` Intimidate activations.
//  Stage formula: 2 / (2 + count), capped at 6 drops.
static double intimidate_atk_mult(int count){
    if (count <= 0) return 1.0;
    int capped = std::min(count, 6);
    return 2.0 / (2.0 + capped);
}

//  Score move `slot` (0-3) against a single opponent pokemon.
//  Returns {lower_damage, upper_damage, is_ohko}.
static std::tuple<int,int,bool> score_move_expert(
    const BDSPMoveData& mdata,
    const BDSPTrainerPokemon& tp,
    int effective_atk,
    int effective_spatk,
    bool doubles
){
    if (mdata.category == MoveCategory::STATUS) return {0, 0, false};

    const BDSPPokemonData& pdata = bdsp_pokemon(tp.species);
    uint16_t hp  = calc_stats_hp(pdata.hp,  tp.level, tp.ivs.hp,  tp.evs.hp);

    int atk_stat = (mdata.category == MoveCategory::PHYSICAL) ? effective_atk : effective_spatk;
    uint16_t def = (mdata.category == MoveCategory::PHYSICAL)
        ? calc_stats_nonhp(pdata.defense, tp.level, tp.ivs.defense, tp.evs.defense, NatureAdjustment::NEUTRAL)
        : calc_stats_nonhp(pdata.spdef,   tp.level, tp.ivs.spdef,   tp.evs.spdef,   NatureAdjustment::NEUTRAL);

    double mult = type_multiplier(mdata.type, pdata.type[0], pdata.type[1]);
    if (mdata.spread && doubles) mult *= 0.75;

    // Attacker level is always 100 (helper Pokemon); defender level is tp.level
    auto [lo, hi] = damage_range(100, (int)mdata.base_power, atk_stat, (int)def, mult);
    bool ohko = (lo >= (int)hp);
    return {lo, hi, ohko};
}

static BattlePlan build_expert_plan(
    Logger& logger,
    const std::array<std::string, 4>& move_slugs,
    const std::array<int, 4>& current_pp,
    const std::vector<Trainer>& all_trainers,
    Trainer current_trainer,
    int base_atk,
    int base_spatk
){
    // Running PP budget across the full sequence
    std::array<int, 4> pp = current_pp;

    BattlePlan plan;
    plan.estimated_pp_after = pp;

    int intimidate_count = 0;

    // Iterate through all listed trainers in sequence order.
    // For current_trainer, populate move_for_species.
    for (Trainer t : all_trainers){
        const BDSPTrainerData& tdata = bdsp_trainer(trainer_slug(t));
        bool is_current = (t == current_trainer);
        bool doubles    = (tdata.battle_type == BDSPBattleType::DOUBLES);

        for (const BDSPTrainer& side : tdata.trainers){
            // Check first-out pokemon for Intimidate (activates before we attack)
            if (!side.pokemon.empty() && side.pokemon[0].has_intimidate){
                intimidate_count++;
            }
        }

        int eff_atk   = (int)(base_atk   * intimidate_atk_mult(intimidate_count));
        int eff_spatk = base_spatk;  // Intimidate doesn't affect SpAtk

        for (const BDSPTrainer& side : tdata.trainers){
            for (const BDSPTrainerPokemon& tp : side.pokemon){
                // Find best move for this pokemon
                int best_slot  = -1;
                int best_hi    = -1;
                bool best_ohko = false;

                for (int i = 0; i < 4; i++){
                    if (pp[i] <= 0) continue;
                    const BDSPMoveData& mdata = bdsp_move(move_slugs[i]);
                    if (mdata.category == MoveCategory::STATUS) continue;

                    auto [lo, hi, ohko] = score_move_expert(mdata, tp, eff_atk, eff_spatk, doubles);
                    (void)lo;

                    // Log adjusted accuracy (informational only, not used in OHKO decisions)
                    uint8_t acc = mdata.accuracy;
                    if (acc > 0 && acc < 100){
                        int adj = 100 - 2 * (100 - acc);
                        logger.log(
                            move_slugs[i] + " vs " + tp.species + ": accuracy=" +
                            std::to_string(acc) + "% (adjusted=" + std::to_string(adj) + "%)",
                            COLOR_YELLOW
                        );
                    }

                    bool prefer = (best_slot < 0)
                        || (ohko  && !best_ohko)                    // new OHKO > non-OHKO
                        || (ohko  && best_ohko  && hi < best_hi)    // both OHKO: lower overkill
                        || (!ohko && !best_ohko && hi > best_hi);   // no OHKO: higher damage

                    if (prefer){
                        best_slot = i;
                        best_hi   = hi;
                        best_ohko = ohko;
                    }
                }

                if (best_slot < 0){
                    // All PP at 0 — Struggle will be used in practice
                    logger.log("PP exhausted for " + tp.species + " — Struggle expected", COLOR_YELLOW);
                } else {
                    pp[best_slot]--;
                    logger.log(
                        std::string(is_current ? "[plan] " : "[pre-plan] ") +
                        tp.species + " → slot " + std::to_string(best_slot) +
                        " (" + move_slugs[best_slot] + ")" +
                        (best_ohko ? " [OHKO]" : " [best effort, hi=" + std::to_string(best_hi) + "]"),
                        COLOR_WHITE
                    );
                    if (is_current){
                        // Only override if this species hasn't been planned yet
                        // (in case the same species appears twice — first occurrence wins)
                        if (plan.move_for_species.find(tp.species) == plan.move_for_species.end()){
                            plan.move_for_species[tp.species] = best_slot;
                        }
                    }
                }

                // Track Intimidate for subsequent pokemon (after the initial send-out)
                if (tp.has_intimidate){
                    intimidate_count++;
                    eff_atk = (int)(base_atk * intimidate_atk_mult(intimidate_count));
                }
            }
        }
    }

    plan.estimated_pp_after = pp;
    return plan;
}


// ---------------------------------------------------------------------------
//  handle_expert_battle
// ---------------------------------------------------------------------------

//  Snapshot the current frame, OCR on-field names, and update `on_field`.
static void update_on_field(
    Logger& logger,
    Language language,
    VideoStream& stream,
    BDSPBattleType battle_type,
    const std::vector<std::string>& candidates,
    std::vector<std::string>& on_field
){
    VideoSnapshot frame = stream.video().snapshot();
    if (battle_type == BDSPBattleType::SINGLES){
        static const ImageFloatBox BOX(0.505, 0.010, 0.420, 0.085);
        std::string name = ocr_opponent_restricted(logger, language, frame, BOX, candidates);
        if (!name.empty() && !on_field.empty()) on_field[0] = name;
    } else {
        static const ImageFloatBox BOX_L(0.505, 0.010, 0.200, 0.085);
        static const ImageFloatBox BOX_R(0.710, 0.010, 0.260, 0.085);
        std::string l = ocr_opponent_restricted(logger, language, frame, BOX_L, candidates);
        std::string r = ocr_opponent_restricted(logger, language, frame, BOX_R, candidates);
        if (!l.empty() && on_field.size() > 0) on_field[0] = l;
        if (!r.empty() && on_field.size() > 1) on_field[1] = r;
    }
}


void handle_expert_battle(
    const ProgramInfo& /*info*/,
    VideoStream& stream,
    ProControllerContext& context,
    const std::vector<Trainer>& all_trainers,
    Trainer current_trainer,
    const AutoStoryOptions& options
){
    using namespace Pokemon;

    Logger& logger = stream.logger();
    const BDSPTrainerData& trainer_data = bdsp_trainer(trainer_slug(current_trainer));
    const OutsideHelpConfig& help = options.outside_help;
    bool doubles = (trainer_data.battle_type == BDSPBattleType::DOUBLES);

    // Move slugs for the helper's 4 moves
    std::array<std::string, 4> move_slugs;
    for (int i = 0; i < 4; i++) move_slugs[i] = torterra_move_slug(help.attacks[i]);

    // Helper stats — 0IV/0EV at level 100, neutral nature
    const BDSPPokemonData& helper_base = bdsp_pokemon("torterra");
    int base_atk   = (int)calc_stats_nonhp(helper_base.attack, 100, 0, 0, NatureAdjustment::NEUTRAL);
    int base_spatk = (int)calc_stats_nonhp(helper_base.spatk,  100, 0, 0, NatureAdjustment::NEUTRAL);

    // Candidate species slugs for restricted OCR (all pokemon in current trainer's party)
    std::vector<std::string> candidates;
    for (const BDSPTrainer& side : trainer_data.trainers){
        for (const BDSPTrainerPokemon& tp : side.pokemon){
            candidates.push_back(tp.species);
        }
    }

    // PP tracking — initialised to max PP from JSON; OCR updates each turn
    std::array<int, 4> pp_remaining;
    for (int i = 0; i < 4; i++) pp_remaining[i] = (int)bdsp_move(move_slugs[i]).pp;

    // Intimidate tracking across turns
    int intimidate_count = 0;
    std::set<std::string> intimidate_seen;

    // Current on-field opponents (initialised to first pokemon of each side)
    std::vector<std::string> on_field;
    for (const BDSPTrainer& side : trainer_data.trainers){
        on_field.push_back(side.pokemon.empty() ? "" : side.pokemon[0].species);
    }

    // plan is built on the first turn after PP are OCR'd
    BattlePlan plan;
    bool plan_built = false;

    // ── Wait for the initial battle menu ─────────────────────────────────────
    bool battle_active = true;
    {
        BattleMenuWatcher menu(BattleType::TRAINER);
        EndBattleWatcher  end;
        int r = wait_until(stream, context, 60s, {{menu}, {end}});
        if (r != 0 || r < 0) battle_active = false;
    }

    // ── Main battle loop ──────────────────────────────────────────────────────
    //  Each iteration starts with the battle menu (Fight/Pokemon/Bag/Run) visible.
    //  We check HP/status, open the fight menu, OCR PP, identify the opponent,
    //  select a move, then wait for the next event.
    while (battle_active){

        // 1. Check HP / status → use Full Restore if needed
        {
            VideoSnapshot frame = stream.video().snapshot();
            double hp         = read_player_hp_ratio(frame);
            bool   has_status = detect_player_status(frame);
            if (hp < 0.33 || has_status){
                logger.log(
                    "Full Restore needed (HP=" + std::to_string((int)(hp * 100)) +
                    "%, status=" + (has_status ? "yes" : "no") + ")",
                    COLOR_WHITE
                );
                use_full_restore_in_battle(stream, context, options.language);
            }
        }

        // 2. Open fight menu and re-read PP
        pbf_press_button(context, BUTTON_A, 10ms, 60ms);
        context.wait_for_all_requests();
        context.wait_for(300ms);
        {
            auto ocr_pp = read_all_pp(logger, stream);
            for (int i = 0; i < 4; i++){
                if (ocr_pp[i] >= 0) pp_remaining[i] = ocr_pp[i];
                else logger.log("PP OCR failed slot " + std::to_string(i), COLOR_YELLOW);
            }
        }

        // 3. Build plan on first turn (after we have real PP values from OCR)
        if (!plan_built){
            plan = build_expert_plan(
                logger, move_slugs, pp_remaining,
                all_trainers, current_trainer,
                base_atk, base_spatk
            );
            plan_built = true;
        }

        // 4. Identify current on-field opponent(s) via restricted OCR
        //    (Fight menu is open at bottom; opponent info boxes are at the top — still readable)
        update_on_field(logger, options.language, stream, trainer_data.battle_type, candidates, on_field);

        // 5. Track Intimidate from newly-observed opponents
        {
            int eff_atk = base_atk;
            for (const std::string& slug : on_field){
                if (slug.empty() || intimidate_seen.count(slug)) continue;
                for (const BDSPTrainer& side : trainer_data.trainers){
                    for (const BDSPTrainerPokemon& tp : side.pokemon){
                        if (tp.species == slug && tp.has_intimidate){
                            intimidate_count++;
                            intimidate_seen.insert(slug);
                            eff_atk = (int)(base_atk * intimidate_atk_mult(intimidate_count));
                            logger.log(
                                slug + " has Intimidate — ATK stage -" +
                                std::to_string(intimidate_count) +
                                " (effective ATK=" + std::to_string(eff_atk) + ")",
                                COLOR_YELLOW
                            );
                        }
                    }
                }
            }
        }

        // 6. Select move — prefer pre-plan; fall back to live scoring if plan misses
        int best_slot = -1;
        if (!on_field.empty() && !on_field[0].empty()){
            auto it = plan.move_for_species.find(on_field[0]);
            if (it != plan.move_for_species.end() && pp_remaining[it->second] > 0)
                best_slot = it->second;
        }

        if (best_slot < 0){
            logger.log("Plan miss for " + (on_field.empty() ? "?" : on_field[0]) +
                       " — live scoring", COLOR_YELLOW);
            int eff_atk   = (int)(base_atk   * intimidate_atk_mult(intimidate_count));
            int eff_spatk = base_spatk;
            int best_hi   = -1;
            bool best_ohko = false;

            for (int i = 0; i < 4; i++){
                if (pp_remaining[i] <= 0) continue;
                const BDSPMoveData& mdata = bdsp_move(move_slugs[i]);
                if (mdata.category == MoveCategory::STATUS) continue;

                int hi = 0; bool ohko = false;
                if (!on_field.empty() && !on_field[0].empty()){
                    for (const BDSPTrainer& side : trainer_data.trainers){
                        for (const BDSPTrainerPokemon& tp : side.pokemon){
                            if (tp.species != on_field[0]) continue;
                            auto [lo, h, ok] = score_move_expert(mdata, tp, eff_atk, eff_spatk, doubles);
                            hi = h; ohko = ok;
                        }
                    }
                }

                bool prefer = (best_slot < 0)
                    || (ohko  && !best_ohko)
                    || (ohko  && best_ohko  && hi < best_hi)
                    || (!ohko && !best_ohko && hi > best_hi);

                if (prefer){ best_slot = i; best_hi = hi; best_ohko = ohko; }
            }
            // Last resort: first move with any PP remaining
            if (best_slot < 0){
                for (int i = 0; i < 4; i++){
                    if (pp_remaining[i] > 0){ best_slot = i; break; }
                }
            }
        }

        // 7. Execute selected move (or Struggle if all PP are gone)
        if (best_slot < 0){
            logger.log("All PP at 0 — Struggle", COLOR_YELLOW);
            pbf_press_button(context, BUTTON_B, 10ms, 60ms); // close fight menu
            context.wait_for_all_requests();
        } else {
            logger.log(
                "Move slot " + std::to_string(best_slot) +
                " (" + move_slugs[best_slot] + ") vs " +
                (on_field.empty() ? "?" : on_field[0]),
                COLOR_WHITE
            );
            navigate_to_slot(context, best_slot);
            pbf_mash_button(context, BUTTON_A, 500ms);
            context.wait_for_all_requests();
            pp_remaining[best_slot]--;

            // Single-target move in doubles: confirm default target
            if (!bdsp_move(move_slugs[best_slot]).spread && doubles){
                pbf_press_button(context, BUTTON_A, 10ms, 60ms);
                context.wait_for_all_requests();
            }
            // Mash through target confirm and partner's move selection
            pbf_mash_button(context, BUTTON_A, 3000ms);
            context.wait_for_all_requests();
        }

        // 8. Wait for next event: next battle menu, XP gain, or battle end
        {
            BattleMenuWatcher     menu2(BattleType::TRAINER);
            ExperienceGainWatcher exp;
            EndBattleWatcher      end;

            int event = wait_until(stream, context, 60s, {{menu2}, {exp}, {end}});

            if (event == 2 || event < 0){
                // Battle over
                break;
            } else if (event == 1){
                // XP gain — mash until the NEXT battle menu appears (or battle ends)
                BattleMenuWatcher menu3(BattleType::TRAINER);
                EndBattleWatcher  end2;
                int after = run_until<ProControllerContext>(
                    stream, context,
                    [](ProControllerContext& ctx){ pbf_mash_button(ctx, BUTTON_A, 120000ms); },
                    {{menu3}, {end2}}
                );
                if (after != 0){
                    // Battle ended during XP screens
                    break;
                }
                // Update on-field now that the next pokemon is visible
                update_on_field(logger, options.language, stream,
                                trainer_data.battle_type, candidates, on_field);
                for (size_t i = 0; i < on_field.size(); i++){
                    logger.log("On-field[" + std::to_string(i) + "] = " + on_field[i], COLOR_WHITE);
                }
                // Continue loop — we're now at the next battle menu
            }
            // event == 0: next battle menu appeared (mid-turn switch or next turn)
            // Loop continues from the top, which is the battle menu state
        }
    }

    // Mash through any remaining XP/dialog until we're back in the overworld
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
