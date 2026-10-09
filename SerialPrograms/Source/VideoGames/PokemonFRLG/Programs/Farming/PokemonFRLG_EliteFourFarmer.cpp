/*  Elite Four Farmer
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Loop:
 *    1. From the overworld (outdoors), Fly to Indigo Plateau.
 *    2. Walk through the lobby into Lorelei's room, check the room by its
 *       floor color, and battle her. Then walk room to room through Bruno,
 *       Agatha, Lance and the Champion.
 *    3. After the Champion, wait for the Hall of Fame save message to come
 *       and go, then soft reset. The save loads outside the player's house
 *       in Pallet Town, ready for the next loop.
 *
 *  Battles: every opponent is one-hit KO'd by a fixed move (see the move
 *  tables below). Each turn the opponent's species is read from the screen
 *  and the move for it is used, so an opponent that survives a low damage
 *  roll is simply attacked again. If the name can't be read, the move is
 *  picked from the opponents' known send-out order instead.
 *
 */

#include <algorithm>
#include <array>
#include <cmath>
#include <map>
#include <set>
#include <string>
#include <vector>
#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/Exceptions/OperationFailedExceptionWithScreenshot.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "VideoGames/PokemonFRLG/Inference/Dialogs/PokemonFRLG_BattleDialogs.h"
#include "VideoGames/PokemonFRLG/Inference/Dialogs/PokemonFRLG_DialogDetector.h"
#include "VideoGames/PokemonFRLG/Inference/Map/PokemonFRLG_PokemonLeagueDetectors.h"
#include "VideoGames/PokemonFRLG/Inference/Menus/PokemonFRLG_PartyMenuDetector.h"
#include "VideoGames/PokemonFRLG/Inference/PokemonFRLG_BattlePokemonDetector.h"
#include "VideoGames/PokemonFRLG/Inference/PokemonFRLG_WildEncounterReader.h"
#include "VideoGames/PokemonFRLG/Programs/PokemonFRLG_BattleMenuNavigation.h"
#include "VideoGames/PokemonFRLG/PokemonFRLG_Navigation.h"
#include "PokemonFRLG_EliteFourFarmer.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{


EliteFourFarmer_Descriptor::EliteFourFarmer_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonFRLG:EliteFourFarmer",
        Pokemon::STRING_POKEMON + " FRLG", "Elite Four Farmer",
        "Programs/PokemonFRLG/EliteFourFarmer.html",
        "Repeatedly beat the rematch Elite Four and Champion with a single Pokemon.",
        ProgramControllerClass::StandardController_RequiresPrecision,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}

struct EliteFourFarmer_Descriptor::Stats : public StatsTracker{
    Stats()
        : wins(m_stats["Wins"])
        , battles(m_stats["Battles Won"])
        , losses(m_stats["Losses"])
        , resets(m_stats["Resets"])
        , errors(m_stats["Errors"])
    {
        m_display_order.emplace_back("Wins");
        m_display_order.emplace_back("Battles Won");
        m_display_order.emplace_back("Losses", HIDDEN_IF_ZERO);
        m_display_order.emplace_back("Resets", HIDDEN_IF_ZERO);
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);
    }

    std::atomic<uint64_t>& wins;
    std::atomic<uint64_t>& battles;
    std::atomic<uint64_t>& losses;
    std::atomic<uint64_t>& resets;
    std::atomic<uint64_t>& errors;
};

std::unique_ptr<StatsTracker> EliteFourFarmer_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}


EliteFourFarmer::EliteFourFarmer()
    : STOP_AFTER_CURRENT("Win")
    , LANGUAGE(
        "<b>Game Language:</b><br>"
        "Used to read the opposing Pokemon's name.",
        {
            Language::English,
            Language::Japanese,
            Language::Spanish,
            Language::French,
            Language::German,
            Language::Italian,
        },
        LockMode::LOCK_WHILE_RUNNING,
        true
    )
    , ATTACKER(
        "<b>Attacker:</b><br>"
        "The Pokemon in slot 1 that battles. It must be level 100 with 252 EVs in Sp. Atk and Speed, "
        "a +Sp. Atk nature, and exactly these moves in this order in the battle menu (top-left, top-right, bottom-left, bottom-right).",
        {
            {Attacker::STARMIE, "starmie", "Starmie: Surf / Psychic / Ice Beam / Thunderbolt (Sp. Atk 302+)"},
            {Attacker::MEWTWO,  "mewtwo",  "Mewtwo: Psychic / Ice Beam / Thunderbolt / Water Pulse (Sp. Atk 445+, or 405+ holding Mystic Water)"},
            {Attacker::LAPRAS,  "lapras",  "Lapras: Surf / Ice Beam / Psychic / Thunderbolt, holding NeverMeltIce (Sp. Atk 277+)"},
        },
        LockMode::LOCK_WHILE_RUNNING,
        Attacker::STARMIE
    )
    , STARTER(
        "<b>Your Starter:</b><br>"
        "The starter you chose at the start of the game. This decides your rival's team.",
        {
            {Starter::BULBASAUR,  "bulbasaur",  "Bulbasaur"},
            {Starter::CHARMANDER, "charmander", "Charmander"},
            {Starter::SQUIRTLE,   "squirtle",   "Squirtle"},
        },
        LockMode::LOCK_WHILE_RUNNING,
        Starter::CHARMANDER
    )
    , NUM_WINS(
        "<b>Number of Wins:</b><br>"
        "Stop after this many League wins. Zero runs until stopped.",
        LockMode::UNLOCK_WHILE_RUNNING,
        200, 0
    )
    , GO_HOME_WHEN_DONE(false)
    , NOTIFICATION_STATUS_UPDATE("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS_UPDATE,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_OPTION(STOP_AFTER_CURRENT);
    PA_ADD_OPTION(LANGUAGE);
    PA_ADD_OPTION(ATTACKER);
    PA_ADD_OPTION(STARTER);
    PA_ADD_OPTION(NUM_WINS);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(NOTIFICATIONS);
}



namespace{


enum class Move{
    SURF,
    PSYCHIC,
    ICE_BEAM,
    THUNDERBOLT,
    WATER_PULSE,
};

const char* move_name(Move move){
    switch (move){
    case Move::SURF:        return "Surf";
    case Move::PSYCHIC:     return "Psychic";
    case Move::ICE_BEAM:    return "Ice Beam";
    case Move::THUNDERBOLT: return "Thunderbolt";
    case Move::WATER_PULSE: return "Water Pulse";
    }
    return "?";
}

enum class Trainer{
    LORELEI,
    BRUNO,
    AGATHA,
    LANCE,
    CHAMPION,
};

const char* trainer_name(Trainer trainer){
    switch (trainer){
    case Trainer::LORELEI:  return "Lorelei";
    case Trainer::BRUNO:    return "Bruno";
    case Trainer::AGATHA:   return "Agatha";
    case Trainer::LANCE:    return "Lance";
    case Trainer::CHAMPION: return "Champion";
    }
    return "?";
}


//  Everything the program needs to know about one attacker.
struct AttackerPlan{
    //  Moves in battle-menu order: top-left, top-right, bottom-left, bottom-right.
    std::array<Move, 4> slots;

    //  Which move one-hit KOs each rematch opponent (min damage roll, no crit)
    //  at the Sp. Atk listed in the option text. Some picks are not the
    //  cheapest move, to keep every move within its base PP per run.
    std::map<std::string, Move> move_for;

    //  Observed send-out order, Lorelei / Bruno / Agatha / Lance.
    std::array<std::vector<std::string>, 4> elite_four;

    //  Observed send-out order of the Champion, indexed by the player's
    //  starter: Bulbasaur / Charmander / Squirtle.
    std::array<std::vector<std::string>, 3> champion;
};


const AttackerPlan& STARMIE_PLAN(){
    static const AttackerPlan plan{
        {Move::SURF, Move::PSYCHIC, Move::ICE_BEAM, Move::THUNDERBOLT},
        {
            {"dewgong",    Move::THUNDERBOLT},
            {"cloyster",   Move::THUNDERBOLT},
            {"piloswine",  Move::SURF},
            {"jynx",       Move::SURF},
            {"lapras",     Move::THUNDERBOLT},
            {"steelix",    Move::SURF},
            {"hitmonchan", Move::PSYCHIC},
            {"hitmonlee",  Move::PSYCHIC},
            {"machamp",    Move::PSYCHIC},
            {"gengar",     Move::PSYCHIC},
            {"crobat",     Move::THUNDERBOLT},  //  Psychic also works; Thunderbolt saves Psychic PP.
            {"misdreavus", Move::SURF},
            {"arbok",      Move::PSYCHIC},
            {"gyarados",   Move::THUNDERBOLT},
            {"dragonite",  Move::ICE_BEAM},
            {"kingdra",    Move::PSYCHIC},      //  Sets Starmie's 302 Sp. Atk requirement.
            {"aerodactyl", Move::SURF},
            {"heracross",  Move::PSYCHIC},
            {"alakazam",   Move::SURF},
            {"tyranitar",  Move::SURF},
            {"arcanine",   Move::SURF},
            {"exeggutor",  Move::ICE_BEAM},
            {"blastoise",  Move::THUNDERBOLT},
            {"venusaur",   Move::PSYCHIC},
            {"charizard",  Move::THUNDERBOLT},
        },
        {{
            {"dewgong", "cloyster", "lapras", "piloswine", "jynx"},
            {"steelix", "steelix", "hitmonlee", "hitmonchan", "machamp"},
            {"gengar", "crobat", "arbok", "gengar", "misdreavus"},
            {"gyarados", "dragonite", "dragonite", "kingdra", "aerodactyl"},
        }},
        {{
            {"heracross", "alakazam", "exeggutor", "tyranitar", "gyarados", "charizard"},
            {"heracross", "alakazam", "exeggutor", "tyranitar", "arcanine", "blastoise"},
            {"heracross", "venusaur", "alakazam", "tyranitar", "gyarados", "arcanine"},
        }},
    };
    return plan;
}

const AttackerPlan& MEWTWO_PLAN(){
    static const AttackerPlan plan{
        {Move::PSYCHIC, Move::ICE_BEAM, Move::THUNDERBOLT, Move::WATER_PULSE},
        {
            {"dewgong",    Move::THUNDERBOLT},
            {"cloyster",   Move::THUNDERBOLT},
            {"piloswine",  Move::WATER_PULSE},  //  Psychic also works; Water Pulse saves Psychic PP.
            {"jynx",       Move::THUNDERBOLT},
            {"lapras",     Move::THUNDERBOLT},
            {"steelix",    Move::WATER_PULSE},
            {"hitmonchan", Move::PSYCHIC},
            {"hitmonlee",  Move::PSYCHIC},
            {"machamp",    Move::PSYCHIC},
            {"gengar",     Move::THUNDERBOLT},  //  Psychic also works; Thunderbolt saves Psychic PP.
            {"crobat",     Move::THUNDERBOLT},  //  Same as above.
            {"misdreavus", Move::PSYCHIC},
            {"arbok",      Move::PSYCHIC},
            {"gyarados",   Move::THUNDERBOLT},
            {"dragonite",  Move::ICE_BEAM},
            {"kingdra",    Move::PSYCHIC},
            {"aerodactyl", Move::ICE_BEAM},
            {"heracross",  Move::PSYCHIC},
            {"alakazam",   Move::ICE_BEAM},
            {"tyranitar",  Move::WATER_PULSE},  //  Sets Mewtwo's 445 (405 with Mystic Water) requirement.
            {"arcanine",   Move::WATER_PULSE},  //  Psychic also works; Water Pulse saves Psychic PP.
            {"exeggutor",  Move::ICE_BEAM},
            {"blastoise",  Move::THUNDERBOLT},
            {"venusaur",   Move::PSYCHIC},
            {"charizard",  Move::THUNDERBOLT},
        },
        {{
            {"dewgong", "cloyster", "piloswine", "jynx", "lapras"},
            {"steelix", "steelix", "hitmonlee", "hitmonchan", "machamp"},
            {"gengar", "crobat", "gengar", "misdreavus", "arbok"},
            {"gyarados", "kingdra", "dragonite", "aerodactyl", "dragonite"},
        }},
        {{
            {"heracross", "alakazam", "tyranitar", "exeggutor", "gyarados", "charizard"},
            {"heracross", "alakazam", "tyranitar", "arcanine", "exeggutor", "blastoise"},
            {"heracross", "alakazam", "tyranitar", "gyarados", "arcanine", "venusaur"},
        }},
    };
    return plan;
}

const AttackerPlan& LAPRAS_PLAN(){
    static const AttackerPlan plan{
        {Move::SURF, Move::ICE_BEAM, Move::PSYCHIC, Move::THUNDERBOLT},
        {
            {"dewgong",    Move::THUNDERBOLT},
            {"cloyster",   Move::THUNDERBOLT},
            {"piloswine",  Move::SURF},
            {"jynx",       Move::SURF},
            {"lapras",     Move::THUNDERBOLT},  //  Water Absorb: never Surf.
            {"steelix",    Move::SURF},
            {"hitmonchan", Move::PSYCHIC},
            {"hitmonlee",  Move::PSYCHIC},
            {"machamp",    Move::PSYCHIC},
            {"gengar",     Move::PSYCHIC},
            {"crobat",     Move::THUNDERBOLT},
            {"misdreavus", Move::SURF},
            {"arbok",      Move::PSYCHIC},
            {"gyarados",   Move::THUNDERBOLT},
            {"dragonite",  Move::ICE_BEAM},
            {"kingdra",    Move::ICE_BEAM},
            {"aerodactyl", Move::THUNDERBOLT},
            {"heracross",  Move::PSYCHIC},
            {"alakazam",   Move::ICE_BEAM},
            {"tyranitar",  Move::SURF},
            {"arcanine",   Move::SURF},
            {"exeggutor",  Move::ICE_BEAM},
            {"blastoise",  Move::THUNDERBOLT},  //  Sets Lapras's 277 Sp. Atk requirement.
            {"venusaur",   Move::ICE_BEAM},
            {"charizard",  Move::THUNDERBOLT},
        },
        {{
            {"dewgong", "piloswine", "lapras", "cloyster", "jynx"},
            {"steelix", "hitmonchan", "hitmonlee", "machamp", "steelix"},
            {"gengar", "misdreavus", "arbok", "gengar", "crobat"},
            {"gyarados", "aerodactyl", "dragonite", "dragonite", "kingdra"},
        }},
        {{
            {"heracross", "tyranitar", "exeggutor", "alakazam", "gyarados", "charizard"},
            {"heracross", "tyranitar", "exeggutor", "alakazam", "arcanine", "blastoise"},
            {"heracross", "tyranitar", "venusaur", "alakazam", "gyarados", "arcanine"},
        }},
    };
    return plan;
}

const AttackerPlan& get_plan(EliteFourFarmer::Attacker attacker){
    switch (attacker){
    case EliteFourFarmer::Attacker::STARMIE: return STARMIE_PLAN();
    case EliteFourFarmer::Attacker::MEWTWO:  return MEWTWO_PLAN();
    case EliteFourFarmer::Attacker::LAPRAS:  return LAPRAS_PLAN();
    }
    return STARMIE_PLAN();
}

size_t starter_index(EliteFourFarmer::Starter starter){
    switch (starter){
    case EliteFourFarmer::Starter::BULBASAUR:  return 0;
    case EliteFourFarmer::Starter::CHARMANDER: return 1;
    case EliteFourFarmer::Starter::SQUIRTLE:   return 2;
    }
    return 1;
}

const std::vector<std::string>& expected_order(
    const AttackerPlan& plan, EliteFourFarmer::Starter starter, Trainer trainer
){
    switch (trainer){
    case Trainer::LORELEI:  return plan.elite_four[0];
    case Trainer::BRUNO:    return plan.elite_four[1];
    case Trainer::AGATHA:   return plan.elite_four[2];
    case Trainer::LANCE:    return plan.elite_four[3];
    case Trainer::CHAMPION: return plan.champion[starter_index(starter)];
    }
    return plan.elite_four[0];
}

int slot_of(const AttackerPlan& plan, Move move){
    for (int i = 0; i < 4; i++){
        if (plan.slots[i] == move){
            return i;
        }
    }
    return -1;
}



//  Overworld walking.
//
//  Gen 3 movement at walking speed (from the pokefirered decomp):
//    - Pressing a direction the player is not facing first turns them in
//      place, which takes 8 frames, then they start walking.
//    - Each step takes 16 frames.
//    - The game reads the d-pad only when a step finishes. If a new direction
//      is held then, the player walks that way right away (no turn delay).
//  So a path is held as one continuous input, switching direction halfway
//  through the last step of each segment. That gives +/- 8 frames (~130 ms)
//  of tolerance on every switch, and the player never stops between steps.
//  (Running is not possible inside the Pokemon League.)
constexpr double FRAME_MS = 16.74;
constexpr double TURN_FRAMES = 8;
constexpr double STEP_FRAMES = 16;

Milliseconds frames_to_ms(double frames){
    return Milliseconds((int64_t)std::lround(frames * FRAME_MS));
}

struct WalkSegment{
    DpadPosition direction;
    uint16_t steps;
};

//  Walk the path without stopping. "facing" is the direction the player
//  faces before starting. The last segment is held for "extra" longer than
//  it needs, so a path that ends in a door keeps walking until the warp.
void walk_path(
    ProControllerContext& context,
    DpadPosition facing,
    const std::vector<WalkSegment>& path,
    Milliseconds extra
){
    const double start = path[0].direction == facing ? 0 : TURN_FRAMES;
    double previous = 0;
    uint32_t steps = 0;
    for (size_t i = 0; i < path.size(); i++){
        steps += path[i].steps;
        if (i + 1 < path.size()){
            //  Switch halfway through the last step of this segment.
            double end = start + STEP_FRAMES * steps - STEP_FRAMES / 2;
            pbf_press_dpad(context, path[i].direction, frames_to_ms(end - previous), 0ms);
            previous = end;
        }else{
            double end = start + STEP_FRAMES * steps;
            pbf_press_dpad(context, path[i].direction, frames_to_ms(end - previous) + extra, 0ms);
        }
    }
}

//  Walk the path into a door and wait until the next map has faded in.
//
//  One watcher sees the whole warp (screen goes black, then comes back), so
//  a short fade can't slip by between two separate checks. The last segment
//  is held only slightly past the door: by then the fade-out has started and
//  input is ignored, so the player can't take an extra step on arrival.
void walk_through_door(
    ConsoleHandle& console, ProControllerContext& context,
    DpadPosition facing, const std::vector<WalkSegment>& path,
    const std::string& destination
){
    console.log("Walking to " + destination + "...");
    BlackScreenOverWatcher warp_finished(COLOR_RED);
    context.wait_for_all_requests();
    int ret = run_until<ProControllerContext>(
        console, context,
        [&](ProControllerContext& context){
            walk_path(context, facing, path, 300ms);
            pbf_wait(context, 10000ms);
        },
        { warp_finished }
    );
    if (ret < 0){
        OperationFailedExceptionWithScreenshot::fire(
            ErrorReportMode::SEND_ERROR_REPORT,
            "walk_through_door(): Did not go through the door to " + destination + ".",
            console
        );
    }
}

EliteFourRoom room_of(Trainer trainer){
    switch (trainer){
    case Trainer::LORELEI:  return EliteFourRoom::LORELEI;
    case Trainer::BRUNO:    return EliteFourRoom::BRUNO;
    case Trainer::AGATHA:   return EliteFourRoom::AGATHA;
    case Trainer::LANCE:    return EliteFourRoom::LANCE;
    case Trainer::CHAMPION: return EliteFourRoom::NONE;
    }
    return EliteFourRoom::NONE;
}

//  Confirm the player walked into the expected Elite Four room.
//  The game walks the player in by itself; the floor shows up in the
//  sample boxes during that walk.
//
//  Being in a DIFFERENT Elite Four room is an error (the program would use
//  the wrong moves). A floor that matches no room only logs a warning with
//  the measured colors and carries on: talking to the trainer still has to
//  work, so a real problem gets caught there.
void check_room(ConsoleHandle& console, ProControllerContext& context, Trainer trainer){
    const EliteFourRoom expected = room_of(trainer);
    EliteFourRoomWatcher in_room(COLOR_RED, expected);
    int ret = wait_until(console, context, 3000ms, { in_room });
    if (ret == 0){
        console.log(std::string("Entered ") + elite_four_room_name(expected) + ".");
        return;
    }

    VideoSnapshot screen = console.video().snapshot();
    const EliteFourRoom seen = read_elite_four_room(screen);
    if (seen == EliteFourRoom::NONE || seen == expected){
        console.log(
            std::string("Could not confirm ") + elite_four_room_name(expected) +
            " from the floor color. Continuing anyway. Measured " + describe_elite_four_floor(screen),
            COLOR_RED
        );
        return;
    }
    OperationFailedExceptionWithScreenshot::fire(
        ErrorReportMode::SEND_ERROR_REPORT,
        std::string("check_room(): Expected to be in ") + elite_four_room_name(expected) +
        ", but the screen shows " + elite_four_room_name(seen) + ".",
        console,
        screen
    );
}

//  Mash B through dialogue until the battle menu ("What will X do?") appears.
void mash_until_battle_menu(
    ConsoleHandle& console, ProControllerContext& context,
    Trainer trainer, Milliseconds timeout
){
    BattleMenuWatcher battle_menu(COLOR_RED);
    context.wait_for_all_requests();
    int ret = run_until<ProControllerContext>(
        console, context,
        [&](ProControllerContext& context){
            pbf_mash_button(context, BUTTON_B, timeout);
        },
        { battle_menu }
    );
    if (ret < 0){
        OperationFailedExceptionWithScreenshot::fire(
            ErrorReportMode::SEND_ERROR_REPORT,
            std::string("mash_until_battle_menu(): The battle with ") + trainer_name(trainer) + " did not start.",
            console
        );
    }
    console.log("Battle menu detected.");
}

//  Called once the room check passes, while the game may still be walking
//  the player in. Step up to the trainer (one tile) and press A, repeating
//  until their dialogue opens. Inputs during the automatic walk are ignored,
//  and extra presses of up just bump into the trainer.
void talk_to_trainer(ConsoleHandle& console, ProControllerContext& context, Trainer trainer){
    console.log(std::string("Talking to ") + trainer_name(trainer) + "...");
    WhiteDialogWatcher dialog(COLOR_RED);
    BattleMenuWatcher battle_menu(COLOR_RED);
    context.wait_for_all_requests();
    int ret = run_until<ProControllerContext>(
        console, context,
        [](ProControllerContext& context){
            for (int i = 0; i < 20; i++){
                pbf_press_dpad(context, DPAD_UP, 100ms, 150ms);
                pbf_press_button(context, BUTTON_A, 100ms, 400ms);
            }
        },
        { dialog, battle_menu }
    );
    switch (ret){
    case 0:
        mash_until_battle_menu(console, context, trainer, 30000ms);
        return;
    case 1:
        console.log("Battle menu detected.");
        return;
    default:
        OperationFailedExceptionWithScreenshot::fire(
            ErrorReportMode::SEND_ERROR_REPORT,
            std::string("talk_to_trainer(): Unable to talk to ") + trainer_name(trainer) + ".",
            console
        );
    }
}

//  After a battle, the trainer says one more thing in the overworld (the
//  next door is already open by then). Mash B until that dialogue closes.
void clear_post_battle_dialog(ConsoleHandle& console, ProControllerContext& context, Trainer trainer){
    WhiteDialogWatcher dialog(COLOR_RED);
    int ret = wait_until(console, context, 20000ms, { dialog });
    if (ret < 0){
        OperationFailedExceptionWithScreenshot::fire(
            ErrorReportMode::SEND_ERROR_REPORT,
            std::string("clear_post_battle_dialog(): No dialogue from ") + trainer_name(trainer) + " after the battle.",
            console
        );
    }

    WhiteDialogOverWatcher dialog_closed(COLOR_RED);
    ret = run_until<ProControllerContext>(
        console, context,
        [](ProControllerContext& context){
            pbf_mash_button(context, BUTTON_B, 20000ms);
        },
        { dialog_closed }
    );
    if (ret < 0){
        OperationFailedExceptionWithScreenshot::fire(
            ErrorReportMode::SEND_ERROR_REPORT,
            std::string("clear_post_battle_dialog(): The dialogue from ") + trainer_name(trainer) + " did not close.",
            console
        );
    }
}

//  From the overworld (outdoors), Fly to Indigo Plateau and walk through the
//  lobby into Lorelei's room.
//  Assumes the Pokemon in the LAST party slot knows Fly as its first field move.
void travel_to_lorelei(ConsoleHandle& console, ProControllerContext& context){
    console.log("Flying to Indigo Plateau...");
    open_fly_map_from_overworld(console, context);
    fly_from_kanto_map(console, context, KantoFlyLocation::indigoplateau);

    //  Fly lands right below the entrance.
    walk_through_door(console, context, DPAD_NONE, {{DPAD_UP, 1}}, "the Indigo Plateau lobby");

    //  The lobby door puts the player at (11, 16) facing up. Lorelei's door
    //  is at (4, 1): 7 steps left, then 15 up.
    walk_through_door(console, context, DPAD_UP, {{DPAD_LEFT, 7}, {DPAD_UP, 15}}, "Lorelei's room");
}

//  After a win, walk from below the defeated trainer, around them, and up
//  through the door that opened behind them. The same path works in every
//  Elite Four room.
void walk_to_next_room(ConsoleHandle& console, ProControllerContext& context, Trainer next){
    walk_through_door(
        console, context, DPAD_UP,
        {{DPAD_RIGHT, 1}, {DPAD_UP, 2}, {DPAD_LEFT, 1}, {DPAD_UP, 2}},
        next == Trainer::CHAMPION ? std::string("the Champion's room") : std::string(elite_four_room_name(room_of(next)))
    );
}

//  Enter a trainer's room and get to the battle menu.
void start_battle(ConsoleHandle& console, ProControllerContext& context, Trainer trainer){
    if (trainer == Trainer::CHAMPION){
        //  The game walks the player up to the rival and the battle starts
        //  on its own.
        console.log("Entered the Champion's room.");
        mash_until_battle_menu(console, context, trainer, 60000ms);
        return;
    }
    check_room(console, context, trainer);
    talk_to_trainer(console, context, trainer);
}

//  After the Champion, mash B through the dialogue and the Hall of Fame
//  until "Saving... Don't turn off the power." appears, then wait for it to
//  go away. The save is complete by then.
void wait_for_hall_of_fame_save(ConsoleHandle& console, ProControllerContext& context){
    console.log("Champion defeated. Waiting for the Hall of Fame to save...");
    HallOfFameSavingWatcher saving(COLOR_RED);
    context.wait_for_all_requests();
    int ret = run_until<ProControllerContext>(
        console, context,
        [](ProControllerContext& context){
            pbf_mash_button(context, BUTTON_B, 180000ms);
        },
        { saving }
    );
    if (ret < 0){
        OperationFailedExceptionWithScreenshot::fire(
            ErrorReportMode::SEND_ERROR_REPORT,
            "wait_for_hall_of_fame_save(): The Hall of Fame save message did not appear.",
            console
        );
    }
    console.log("Saving...");

    HallOfFameSavingOverWatcher saved(COLOR_RED);
    ret = wait_until(console, context, 30000ms, { saved });
    if (ret < 0){
        OperationFailedExceptionWithScreenshot::fire(
            ErrorReportMode::SEND_ERROR_REPORT,
            "wait_for_hall_of_fame_save(): The save message did not go away.",
            console
        );
    }
    console.log("Saved.");
}

//  From the battle menu, choose FIGHT and then the move in the given slot.
//  The arrow is read from the screen, so only the presses needed to reach
//  the move are made (none if the arrow is already on it).
void use_move_in_slot(ConsoleHandle& console, ProControllerContext& context, int slot){
    if (!move_cursor_to_option(console, context, BattleMenuOption::FIGHT)){
        console.log("Could not confirm the cursor is on FIGHT. Pressing A anyway.", COLOR_RED);
    }
    pbf_press_button(context, BUTTON_A, 200ms, 300ms);       //  open the move list
    context.wait_for_all_requests();

    if (!move_cursor_to_option(console, context, static_cast<BattleMoveOption>(slot))){
        //  Could not see the arrow. Fall back to pressing left and up, which
        //  always lands on the top-left move, then moving to the slot.
        console.log("Could not find the move cursor. Moving to the move without it.", COLOR_RED);
        pbf_press_dpad(context, DPAD_LEFT, 160ms, 240ms);
        pbf_press_dpad(context, DPAD_UP, 160ms, 240ms);
        if (slot & 1){
            pbf_press_dpad(context, DPAD_RIGHT, 160ms, 240ms);
        }
        if (slot & 2){
            pbf_press_dpad(context, DPAD_DOWN, 160ms, 240ms);
        }
        context.wait_for_all_requests();
    }

    //  The PP text turns red when the highlighted move is out of PP.
    BattleOutOfPpWatcher out_of_pp(COLOR_RED);
    int ret = wait_until(console, context, 500ms, { out_of_pp });
    if (ret == 0){
        OperationFailedExceptionWithScreenshot::fire(
            ErrorReportMode::SEND_ERROR_REPORT,
            "use_move_in_slot(): The selected move is out of PP.",
            console
        );
    }

    pbf_press_button(context, BUTTON_A, 200ms, 300ms);       //  use the move
    context.wait_for_all_requests();
}

enum class TrainerBattleResult{
    WON,
    ATTACKER_FAINTED,
};

//  Read the opposing Pokemon's species from its name. It is matched against
//  every species in the attacker's move table, not just this trainer's, so an
//  unexpected opponent is reported by name instead of being misread.
//  Returns an empty string if it can't be read.
std::string read_opponent(
    ConsoleHandle& console, Language language,
    const std::set<std::string>& candidates
){
    WildEncounterReader reader(COLOR_RED);
    VideoOverlaySet overlays(console.overlay());
    reader.make_overlays(overlays);
    VideoSnapshot screen = console.video().snapshot();
    return reader.read_encounter(console.logger(), language, screen, candidates).name;
}

//  Fight one trainer from the battle menu until the battle ends.
TrainerBattleResult run_battle(
    ConsoleHandle& console, ProControllerContext& context, Language language,
    const AttackerPlan& plan, const std::vector<std::string>& order, Trainer trainer
){
    //  Times the opponent's Pokemon left the field. This counts switch-outs
    //  as well as faints, since both look the same on screen.
    size_t left_field = 0;
    size_t turns = 0;
    std::string last_species;       //  the opponent we attacked last turn
    bool same_opponent = false;     //  true if it did not leave the field

    std::set<std::string> candidates;
    for (const auto& item : plan.move_for){
        candidates.insert(item.first);
    }

    while (true){
        if (turns > 3 * order.size()){
            OperationFailedExceptionWithScreenshot::fire(
                ErrorReportMode::SEND_ERROR_REPORT,
                std::string("run_battle(): Too many turns against ") + trainer_name(trainer) + ".",
                console
            );
        }

        //  We are at the battle menu. Read who we are facing. If that fails,
        //  assume the last opponent if it never left the field, otherwise
        //  fall back to the expected send-out order.
        std::string species = read_opponent(console, language, candidates);
        if (species.empty()){
            if (same_opponent){
                species = last_species;
                console.log("Could not read the opponent's name. Assuming " + species + " is still out.", COLOR_RED);
            }else{
                species = order[std::min(left_field, order.size() - 1)];
                console.log("Could not read the opponent's name. Assuming " + species + " from the send-out order.", COLOR_RED);
            }
        }
        last_species = species;
        auto iter = plan.move_for.find(species);
        if (iter == plan.move_for.end()){
            OperationFailedExceptionWithScreenshot::fire(
                ErrorReportMode::NO_ERROR_REPORT,
                "run_battle(): No move set for opponent: " + species,
                console
            );
        }
        Move move = iter->second;
        int slot = slot_of(plan, move);
        console.log(
            std::string(trainer_name(trainer)) + ", turn " + std::to_string(turns + 1) +
            " (" + species + "): using " + move_name(move) + " (slot " + std::to_string(slot + 1) + ")"
        );
        use_move_in_slot(console, context, slot);
        turns++;

        //  Mash B through the battle text until the battle menu comes back
        //  (next opponent, or the same one if it survived) or the battle ends
        //  (fade to black). The next turn reads the name again, so a survivor
        //  is attacked again.
        bool faint_seen = false;
        auto report_missed_faint = [&]{
            if (!faint_seen){
                console.log(
                    "Did not see the opponent faint. It may have survived: "
                    "check the attacker's Sp. Atk, nature and moves.",
                    COLOR_RED
                );
            }
        };

        bool waiting = true;
        while (waiting){
            BattleMenuWatcher battle_menu(COLOR_RED);
            BlackScreenWatcher battle_ended(COLOR_RED);
            BattleFaintWatcher attacker_fainted(COLOR_RED);
            BattleOpponentFaintWatcher opponent_fainted(COLOR_RED);

            std::vector<PeriodicInferenceCallback> callbacks{battle_menu, battle_ended, attacker_fainted};
            if (!faint_seen){
                callbacks.emplace_back(opponent_fainted);
            }

            int ret = run_until<ProControllerContext>(
                console, context,
                [](ProControllerContext& context){
                    pbf_mash_button(context, BUTTON_B, 60000ms);
                },
                callbacks
            );

            switch (ret){
            case 0:     //  Battle menu: our next turn.
                report_missed_faint();
                same_opponent = !faint_seen;
                waiting = false;
                break;
            case 1:{    //  Fade to black.
                //  A won battle ends with the last opponent fainting, then a
                //  fade to the overworld. If no faint was seen this turn, the
                //  fade may instead be the game opening the party screen
                //  because our attacker fainted (this can happen before the
                //  attacker faint watcher triggers).
                if (!faint_seen){
                    PartyMenuWatcher party_menu(COLOR_RED);
                    if (wait_until(console, context, 5000ms, { party_menu }) == 0){
                        console.log(
                            std::string("The attacker fainted against ") + trainer_name(trainer) +
                            " (the party screen opened). Check its Sp. Atk, Speed, nature and moves against the requirements.",
                            COLOR_RED
                        );
                        return TrainerBattleResult::ATTACKER_FAINTED;
                    }
                }
                if (turns != order.size()){
                    console.log(
                        "Battle took " + std::to_string(turns) + " attacks (" +
                        std::to_string(order.size()) + " if every attack is a one-hit KO).",
                        COLOR_RED
                    );
                }
                console.log(std::string("Defeated ") + trainer_name(trainer) + ".");
                return TrainerBattleResult::WON;
            }
            case 2:     //  Our attacker fainted. The run is lost; no need to wait it out.
                console.log(
                    std::string("The attacker fainted against ") + trainer_name(trainer) +
                    ". Check its Sp. Atk, nature and moves against the requirements.",
                    COLOR_RED
                );
                return TrainerBattleResult::ATTACKER_FAINTED;
            case 3:     //  Opponent fainted (or switched out). Keep waiting for the menu or the end.
                console.log("Opponent left the field (fainted or switched out).");
                faint_seen = true;
                left_field++;
                break;
            default:
                OperationFailedExceptionWithScreenshot::fire(
                    ErrorReportMode::SEND_ERROR_REPORT,
                    "run_battle(): Nothing detected for 60 seconds after attacking.",
                    console
                );
            }
        }
    }
}


}   //  namespace



void EliteFourFarmer::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    EliteFourFarmer_Descriptor::Stats& stats = env.current_stats<EliteFourFarmer_Descriptor::Stats>();
    DeferredStopButtonOption::ResetOnExit reset_on_exit(STOP_AFTER_CURRENT);

    const AttackerPlan& plan = get_plan(ATTACKER);
    const Starter starter = STARTER;

    const std::array<Trainer, 5> TRAINERS{
        Trainer::LORELEI, Trainer::BRUNO, Trainer::AGATHA, Trainer::LANCE, Trainer::CHAMPION
    };

    uint32_t consecutive_errors = 0;
    uint32_t consecutive_losses = 0;
    while (true){
        send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE);
        if (NUM_WINS != 0 && stats.wins >= NUM_WINS){
            break;
        }

        try{
            travel_to_lorelei(env.console, context);

            bool lost = false;
            for (size_t i = 0; i < TRAINERS.size(); i++){
                Trainer trainer = TRAINERS[i];
                if (i > 0){
                    walk_to_next_room(env.console, context, trainer);
                }
                start_battle(env.console, context, trainer);
                TrainerBattleResult result = run_battle(env.console, context, LANGUAGE, plan, expected_order(plan, starter, trainer), trainer);
                if (result == TrainerBattleResult::ATTACKER_FAINTED){
                    lost = true;
                    break;
                }
                stats.battles++;
                env.update_stats();

                if (trainer != Trainer::CHAMPION){
                    clear_post_battle_dialog(env.console, context, trainer);
                }
            }

            if (lost){
                //  A lost run is not an error: reset right away and try again.
                stats.losses++;
                env.update_stats();
                consecutive_losses++;
                if (consecutive_losses >= 5){
                    throw UserSetupError(
                        env.console,
                        "The attacker lost 5 runs in a row. Check its moves and move order, "
                        "its Sp. Atk, Speed and nature, and the Attacker and Your Starter options."
                    );
                }
                env.log("Run lost. Soft resetting and trying again.", COLOR_RED);
                soft_reset(env.console, context);
                stats.resets++;
                env.update_stats();
                if (STOP_AFTER_CURRENT.should_stop()){
                    break;
                }
                continue;
            }

            //  The game is saved once the Hall of Fame save message is gone,
            //  so reset right away.
            wait_for_hall_of_fame_save(env.console, context);

            stats.wins++;
            env.update_stats();
            consecutive_errors = 0;
            consecutive_losses = 0;

            env.log("Soft resetting to Pallet Town...");
            soft_reset(env.console, context);
            stats.resets++;
            env.update_stats();
        }catch (OperationFailedException&){
            stats.errors++;
            env.update_stats();
            consecutive_errors++;
            if (consecutive_errors >= 3){
                throw;
            }
            //  The save is outside the house in Pallet Town (from the last
            //  Hall of Fame), so a soft reset puts us back at the start.
            env.log("Error. Soft resetting and trying again.", COLOR_RED);
            soft_reset(env.console, context);
            stats.resets++;
            env.update_stats();
        }

        if (STOP_AFTER_CURRENT.should_stop()){
            break;
        }
    }

    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
    GO_HOME_WHEN_DONE.run_end_of_program(context);
}


}
}
}
