/*  EXP Farmer
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Seven Island loop:
 *    1. Fly to Seven Island, heal at the Pokemon Center, and run to the
 *       Chansey dance house near the end of Sevault Canyon, using a Max Repel
 *       at the canyon entrance.
 *    2. Inside the house, pace until the Repel wears off (and put the
 *       switch-in Pokemon first in the party, if used).
 *    3. Each cycle: leave the house, walk to the VS Seeker tile and use the
 *       VS Seeker.
 *         - "Not ready": go back in, pace 100 steps (no heal), try again.
 *         - "Battery not charged": pace the steps it asks for, try again.
 *         - No message: watch which Cooltrainer hops in place (ready), Cut
 *           through the grass to the northern one if needed, and battle the
 *           ready ones.
 *       Then go back in, heal (Chansey dance), and pace until the VS Seeker
 *       is charged again.
 *
 *  One Island loop (Kindle Road):
 *    1. Fly to One Island, heal at the Pokemon Center, walk to the beach,
 *       use a Max Repel, Surf north and walk up Kindle Road to the Ember Spa.
 *    2. In the spa, pace until the Repel wears off.
 *    3. Each cycle: leave the spa, walk to the VS Seeker tile and wait until
 *       Crush Girl Sharon (who walks up and down out of view) is on screen,
 *       so she is in the VS Seeker's range, then use it. Battle the trainers
 *       that show "!!" (Black Belts Hugh and Shea, Crush Kin Mik & Kia - a
 *       double battle - and Sharon), then go back into the spa, heal if
 *       there was a battle, and pace until the VS Seeker is charged.
 *    Kindle Road is wider than the screen and has no landmark like the
 *    Chansey house's roof, so the position there is read by comparing the
 *    screen with screens remembered at known tiles (ScreenOdometry). In the
 *    spa, walls stop every move, so it needs no position reading.
 *
 *  Map coordinates below are tile coordinates from the pokefirered decomp
 *  (data/maps/SevenIsland_SevaultCanyon, OneIsland_KindleRoad,
 *  OneIsland_KindleRoad_EmberSpa, OneIsland_PokemonCenter_1F).
 *
 */

#include <algorithm>
#include <array>
#include <cctype>
#include <cmath>
#include <deque>
#include <functional>
#include <map>
#include <set>
#include <string>
#include <vector>
#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/Exceptions/OperationFailedExceptionWithScreenshot.h"
#include "CommonFramework/Exceptions/ProgramFinishedException.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "VideoGames/PokemonFRLG/Inference/Dialogs/PokemonFRLG_BattleDialogs.h"
#include "VideoGames/PokemonFRLG/Inference/Dialogs/PokemonFRLG_DialogDetector.h"
#include "VideoGames/PokemonFRLG/Inference/Dialogs/PokemonFRLG_DialogTextReader.h"
#include "VideoGames/PokemonFRLG/Inference/Dialogs/PokemonFRLG_PartyDialogs.h"
#include "VideoGames/PokemonFRLG/Inference/Map/PokemonFRLG_OverworldReaders.h"
#include "VideoGames/PokemonFRLG/Inference/Menus/PokemonFRLG_PartyMenuDetector.h"
#include "VideoGames/PokemonFRLG/Inference/PokemonFRLG_BattleMoveReader.h"
#include "VideoGames/PokemonFRLG/Inference/PokemonFRLG_BattleSelectionArrowDetector.h"
#include "VideoGames/PokemonFRLG/Inference/PokemonFRLG_WildEncounterReader.h"
#include "VideoGames/PokemonFRLG/Programs/PokemonFRLG_BattleMenuNavigation.h"
#include "VideoGames/PokemonFRLG/PokemonFRLG_Navigation.h"
#include "PokemonFRLG_BattleMoveData.h"
#include "PokemonFRLG_ExpFarmer.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{


ExpFarmer_Descriptor::ExpFarmer_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonFRLG:ExpFarmer",
        Pokemon::STRING_POKEMON + " FRLG", "EXP Farmer",
        "Programs/PokemonFRLG/ExpFarmer.html",
        "Farm EXP by rebattling trainers with the VS Seeker: the Kindle Road trainers on One Island, or the Sevault Canyon Cooltrainers on Seven Island.",
        PanelQualityStanding::THIRD_PARTY,
        ProgramControllerClass::StandardController_RequiresPrecision,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}

struct ExpFarmer_Descriptor::Stats : public StatsTracker{
    Stats()
        : battles(m_stats["Battles Won"])
        , cycles(m_stats["Cycles"])
        , not_ready(m_stats["Nobody Ready"])
        , errors(m_stats["Errors"])
        , missed(m_stats["Missed Battles"])
    {
        m_display_order.emplace_back("Battles Won");
        m_display_order.emplace_back("Cycles");
        m_display_order.emplace_back("Nobody Ready");
        m_display_order.emplace_back("Missed Battles", HIDDEN_IF_ZERO);
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);
    }

    std::atomic<uint64_t>& battles;
    std::atomic<uint64_t>& cycles;
    std::atomic<uint64_t>& not_ready;
    std::atomic<uint64_t>& errors;
    std::atomic<uint64_t>& missed;
};

std::unique_ptr<StatsTracker> ExpFarmer_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}


ExpFarmer::ExpFarmer()
    : STOP_AFTER_CURRENT("Cycle")
    , LOCATION(
        "<b>Location:</b><br>"
        "<b>One Island:</b> Black Belts Hugh and Shea, Crush Kin Mik & Kia (a double battle) and "
        "Crush Girl Sharon on Kindle Road (levels 50-56 after the Elite Four), healing at the Ember "
        "Spa. A party Pokemon must know Surf.<br>"
        "<b>Seven Island:</b> Cooltrainers Leroy and Michelle in Sevault Canyon (levels 52-56), "
        "healing at the Chansey house. Cut must be known by the second-to-last Pokemon in the party.<br>"
        "Either way, the last Pokemon in the party must know Fly and the VS Seeker must be registered "
        "to SELECT. Fly only reaches places on the same island group, so start outdoors on Six or "
        "Seven Island (or their routes) for Seven Island, and on One, Two or Three Island (or their "
        "routes) for One Island.",
        {
            {Location::ONE_ISLAND,   "one-island",   "One Island (Kindle Road)"},
            {Location::SEVEN_ISLAND, "seven-island", "Seven Island (Sevault Canyon)"},
        },
        LockMode::LOCK_WHILE_RUNNING,
        Location::SEVEN_ISLAND
    )
    , SWITCH_IN(
        "<b>Switch-In:</b><br>"
        "Turn this on to level a weak Pokemon in party slot 2: it leads each battle and is switched "
        "out right away for the strong Pokemon in slot 1, so it shares the EXP. It is switched back "
        "in before every opponent. Requires the Battle Style option to be SHIFT.<br>"
        "Leave this off if the Pokemon you are leveling can win the battles by itself: put it in "
        "slot 1 and it battles alone. Either way, slot 1 must be at least level 55 so the "
        "Max Repel keeps the wild Pokemon away.<br>"
        "In the One Island double battle (Mik & Kia), slots 1 and 2 both battle and nobody "
        "switches.",
        LockMode::LOCK_WHILE_RUNNING,
        false
    )
    , EVOLUTION(
        "<b>Evolution:</b><br>"
        "What to do when any party Pokemon starts to evolve (not only the one being leveled).",
        {
            {EvolutionChoice::CANCEL,         "cancel",     "Cancel evolution"},
            {EvolutionChoice::ALLOW,          "allow",      "Allow evolution"},
            {EvolutionChoice::ALLOW_AND_STOP, "allow-stop", "Allow evolution, then stop the program"},
        },
        LockMode::LOCK_WHILE_RUNNING,
        EvolutionChoice::CANCEL
    )
    , MOVE_LEARN(
        "<b>Learning Moves:</b><br>"
        "What to do when a party Pokemon wants to learn a move but already knows four.",
        {
            {MoveLearnChoice::DECLINE,        "decline",  "Don't learn the move"},
            {MoveLearnChoice::REPLACE_SLOT_4, "replace4", "Learn it, forgetting the 4th move"},
            {MoveLearnChoice::STOP,           "stop",     "Stop the program at the prompt"},
        },
        LockMode::LOCK_WHILE_RUNNING,
        MoveLearnChoice::DECLINE
    )
    , MAX_REPEL_SLOT(
        "<b>Max Repel Position:</b><br>"
        "Where Max Repel is in the Items pocket of the bag (1 = first item).",
        LockMode::LOCK_WHILE_RUNNING,
        1, 1, 42
    )
    , LEVELING(
        "<b>Pokemon Being Leveled:</b><br>"
        "Which Pokemon the Target Level is for.",
        {
            {LevelingTarget::BATTLING,  "battling",  "The battling Pokemon (slot 1, or slot 2 with Switch-In)"},
            {LevelingTarget::EXP_SHARE, "exp-share", "An Exp. Share holder (a Pokemon that doesn't battle)"},
        },
        LockMode::LOCK_WHILE_RUNNING,
        LevelingTarget::BATTLING
    )
    , TARGET_LEVEL(
        "<b>Target Level:</b><br>"
        "Stop after the battle in which the Pokemon chosen above reaches this level. "
        "Level-ups of other Pokemon don't count.<br>"
        "With an Exp. Share holder, only give one Pokemon an Exp. Share (with several, the "
        "first one to reach the level stops the program), and its level must still be below "
        "the target when the program starts.",
        LockMode::UNLOCK_WHILE_RUNNING,
        100, 2, 100
    )
    , GO_HOME_WHEN_DONE(true)
    , NOTIFICATION_STATUS_UPDATE("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS_UPDATE,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_OPTION(STOP_AFTER_CURRENT);
    PA_ADD_OPTION(LOCATION);
    PA_ADD_OPTION(SWITCH_IN);
    PA_ADD_OPTION(EVOLUTION);
    PA_ADD_OPTION(MOVE_LEARN);
    PA_ADD_OPTION(MAX_REPEL_SLOT);
    PA_ADD_OPTION(LEVELING);
    PA_ADD_OPTION(TARGET_LEVEL);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(NOTIFICATIONS);
}



namespace{


////////////////////////////////////////////////////////////////////////////////
//  Movement
//
//  From the pokefirered decomp: a step takes 16 frames walking and 8 frames
//  running (B held, outdoors only). Pressing a direction the player is not
//  facing first turns them in place (8 frames). The game reads the d-pad
//  when a step finishes, so releasing halfway through the last step stops the
//  player after exactly that step, with half a step of timing tolerance.
//
//  Each straight segment is walked from a standstill, so walking into a wall
//  only wastes time: the next segment starts fresh.
////////////////////////////////////////////////////////////////////////////////

constexpr double FRAME_MS = 16.74;
constexpr double TURN_FRAMES = 8;
constexpr double WALK_STEP_FRAMES = 16;
constexpr double RUN_STEP_FRAMES = 8;
//  Rock stairs (metatile behavior MB_ROCK_STAIRS) slow vertical movement:
//  running alternates 1 and 2 pixels per frame, so a tile takes 11 frames.
//  A step up is slow if the player starts on stairs; a step down if the
//  tile below is stairs.
constexpr double RUN_STAIRS_STEP_FRAMES = 11;
constexpr double WALK_STAIRS_STEP_FRAMES = 32;

Milliseconds frames_to_ms(double frames){
    return Milliseconds((int64_t)std::lround(frames * FRAME_MS));
}

enum class Dir{
    UP,
    DOWN,
    LEFT,
    RIGHT,
};

DpadPosition to_dpad(Dir dir){
    switch (dir){
    case Dir::UP:    return DPAD_UP;
    case Dir::DOWN:  return DPAD_DOWN;
    case Dir::LEFT:  return DPAD_LEFT;
    case Dir::RIGHT: return DPAD_RIGHT;
    }
    return DPAD_NONE;
}
int dir_dx(Dir dir){
    return dir == Dir::LEFT ? -1 : dir == Dir::RIGHT ? 1 : 0;
}
int dir_dy(Dir dir){
    return dir == Dir::UP ? -1 : dir == Dir::DOWN ? 1 : 0;
}
const char* dir_name(Dir dir){
    switch (dir){
    case Dir::UP:    return "up";
    case Dir::DOWN:  return "down";
    case Dir::LEFT:  return "left";
    case Dir::RIGHT: return "right";
    }
    return "?";
}

struct Segment{
    Dir dir;
    uint16_t steps;
    uint8_t stair_steps = 0;        //  How many of the steps are on rock stairs.
    bool ends_on_stairs = false;    //  The last step is one of them.
};

struct MapTile{
    int x;
    int y;
    bool operator==(const MapTile& t) const{ return x == t.x && y == t.y; }
    bool operator<(const MapTile& t) const{ return x != t.x ? x < t.x : y < t.y; }
};
std::string tile_str(const MapTile& t){
    return "(" + std::to_string(t.x) + ", " + std::to_string(t.y) + ")";
}


//  Tracks which way the player faces and how many steps were taken (for the
//  VS Seeker's 100-step charge).
class Walker{
public:
    Walker(ConsoleHandle& console, ProControllerContext& context)
        : m_console(console)
        , m_context(context)
    {}

    void set_facing(Dir dir){ m_facing = dir; m_facing_known = true; }
    bool is_facing(Dir dir) const{ return m_facing_known && m_facing == dir; }
    void forget_facing(){ m_facing_known = false; }
    uint32_t steps() const{ return m_steps; }
    void reset_steps(){ m_steps = 0; }
    void add_steps(uint32_t steps){ m_steps += steps; }
    //  For steps that were walked into a wall (no step happened).
    void uncount(uint32_t steps){ m_steps = m_steps > steps ? m_steps - steps : 0; }

    //  Hold the direction (and B when running) for the given time.
    void hold(Dir dir, bool run, Milliseconds duration){
        if (run){
            pbf_controller_state(m_context, BUTTON_B, to_dpad(dir), {0, 0}, {0, 0}, duration);
        }else{
            pbf_press_dpad(m_context, to_dpad(dir), duration, 0ms);
        }
    }

    //  Walk one straight segment from a standstill and stop.
    void walk(Dir dir, uint16_t steps, bool run, uint8_t stair_steps = 0, bool ends_on_stairs = false){
        if (steps == 0){
            return;
        }
        const double step = run ? RUN_STEP_FRAMES : WALK_STEP_FRAMES;
        const double stair_step = run ? RUN_STAIRS_STEP_FRAMES : WALK_STAIRS_STEP_FRAMES;
        const double last_step = ends_on_stairs ? stair_step : step;
        const double total = step * (steps - stair_steps) + stair_step * stair_steps;
        //  The facing must be known: a turn that doesn't happen (or one that
        //  does but wasn't expected) is a whole running step of difference.
        //  Every caller keeps it known; if not, assume a turn.
        double turn = 0;
        if (!m_facing_known || m_facing != dir){
            turn = TURN_FRAMES;
        }
        //  Release in the middle of the last step.
        hold(dir, run, frames_to_ms(turn + total - last_step / 2));
        pbf_wait(m_context, frames_to_ms(last_step) + 80ms);
        m_facing = dir;
        m_facing_known = true;
        m_steps += steps;
    }

    //  Run a path of straight segments in one go, without stopping at the
    //  turns: the next direction is pressed as the last step of a segment
    //  ends (a turn while moving costs no time). Released in the middle of
    //  the last step. No rock stairs.
    void run_path(const std::vector<Segment>& path){
        if (path.empty()){
            return;
        }
        double turn = (!m_facing_known || m_facing != path[0].dir) ? TURN_FRAMES : 0;
        for (size_t i = 0; i < path.size(); i++){
            double frames = RUN_STEP_FRAMES * path[i].steps;
            if (i == 0){
                frames += turn;
            }
            if (i + 1 == path.size()){
                frames -= RUN_STEP_FRAMES / 2;
            }
            hold(path[i].dir, true, frames_to_ms(frames));
            m_steps += path[i].steps;
        }
        pbf_wait(m_context, frames_to_ms(RUN_STEP_FRAMES) + 80ms);
        m_facing = path.back().dir;
        m_facing_known = true;
    }

    void walk_path(const std::vector<Segment>& path, bool run){
        for (const Segment& segment : path){
            walk(segment.dir, segment.steps, run, segment.stair_steps, segment.ends_on_stairs);
        }
        m_context.wait_for_all_requests();
    }

    //  Turn to face a direction without stepping. If the tile is free this
    //  may take a step, so only use it toward a wall or a person.
    void face(Dir dir){
        pbf_press_dpad(m_context, to_dpad(dir), 150ms, 200ms);
        m_facing = dir;
        m_facing_known = true;
    }

    //  Take one step and check on screen that it happened. Someone walking
    //  into the tile first blocks the step; then wait and try again.
    void checked_step(Dir dir, bool run){
        for (int attempt = 0; attempt < 20; attempt++){
            m_context.wait_for_all_requests();
            VideoSnapshot before = m_console.video().snapshot();
            walk(dir, 1, run);
            m_context.wait_for_all_requests();
            VideoSnapshot after = m_console.video().snapshot();
            if (screen_scrolled_one_tile(before, after, dir_dx(dir), dir_dy(dir))){
                return;
            }
            m_steps--;
            m_console.log(std::string("Step ") + dir_name(dir) + " was blocked. Waiting and trying again.", COLOR_ORANGE);
            pbf_wait(m_context, 800ms);
        }
        OperationFailedExceptionWithScreenshot::fire(
            ErrorReportMode::SEND_ERROR_REPORT,
            std::string("checked_step(): Could not step ") + dir_name(dir) + " for 20 tries.",
            m_console
        );
    }

private:
    ConsoleHandle& m_console;
    ProControllerContext& m_context;
    Dir m_facing = Dir::DOWN;
    bool m_facing_known = false;
    uint32_t m_steps = 0;
};


//  Hold a direction into a door until the screen fades out and back in.
bool try_go_through_door(ConsoleHandle& console, ProControllerContext& context, Walker& walker, Dir dir, const std::string& where){
    console.log("Going through the door to " + where + "...");
    BlackScreenOverWatcher warp_finished(COLOR_RED);
    context.wait_for_all_requests();
    int ret = run_until<ProControllerContext>(
        console, context,
        [&](ProControllerContext& context){
            pbf_press_dpad(context, to_dpad(dir), 3000ms, 0ms);
            pbf_wait(context, 4000ms);
        },
        { warp_finished }
    );
    walker.forget_facing();
    if (ret < 0){
        console.log("Did not go through the door to " + where + ".", COLOR_ORANGE);
        return false;
    }
    pbf_wait(context, 500ms);
    context.wait_for_all_requests();
    return true;
}
void go_through_door(ConsoleHandle& console, ProControllerContext& context, Walker& walker, Dir dir, const std::string& where){
    if (!try_go_through_door(console, context, walker, dir, where)){
        OperationFailedExceptionWithScreenshot::fire(
            ErrorReportMode::SEND_ERROR_REPORT,
            "go_through_door(): Did not go through the door to " + where + ".",
            console
        );
    }
}

//  After closing a menu: give the game time to fade back to the overworld
//  before walking, so the first frames of a walk aren't lost.
void settle_in_overworld(ConsoleHandle& console, ProControllerContext& context){
    pbf_wait(context, 800ms);
    context.wait_for_all_requests();
    if (!in_overworld(console, context)){
        console.log("Not back in the overworld yet. Pressing B.", COLOR_ORANGE);
        pbf_mash_button(context, BUTTON_B, 2000ms);
        pbf_wait(context, 800ms);
        context.wait_for_all_requests();
    }
}


////////////////////////////////////////////////////////////////////////////////
//  Sevault Canyon around the Chansey house (map rows 50 to 69).
//    '#' blocked, '.' floor, 'g' tall grass. Generated from the decomp's map
//    blocks and metatile behaviors.
////////////////////////////////////////////////////////////////////////////////

const int CANYON_FIRST_ROW = 50;
const char* const CANYON_ROWS[] = {
    "#########...############",     //  y = 50
    "#########...############",     //  y = 51
    "########.....###########",     //  y = 52
    "########..ggg###########",     //  y = 53
    "########..ggg###########",     //  y = 54
    "#######..gggg###########",     //  y = 55
    "#######..gggg###########",     //  y = 56
    "#######..gggg###########",     //  y = 57
    "########ggggg###########",     //  y = 58
    "########ggg..###########",     //  y = 59
    "#########gg..###########",     //  y = 60
    "#########gg..###########",     //  y = 61
    "#########gg......#######",     //  y = 62
    "#########g.......#######",     //  y = 63
    "###########......#######",     //  y = 64
    "############.....#######",     //  y = 65
    "############.....#######",     //  y = 66
    "############.....#######",     //  y = 67
    "#############..#########",     //  y = 68
    ".############...########",     //  y = 69
};

char canyon_cell(const MapTile& t){
    if (t.x < 0 || t.x >= 24 || t.y < CANYON_FIRST_ROW || t.y >= CANYON_FIRST_ROW + 20){
        return '#';
    }
    return CANYON_ROWS[t.y - CANYON_FIRST_ROW][t.x];
}

//  Where the VS Seeker is used: 2 left and 3 up from the house door's exit.
const MapTile VS_SEEKER_TILE{12, 59};
//  Where the player stands after leaving the house. The door is above it.
const MapTile HOUSE_EXIT_TILE{14, 62};

//  Each Cooltrainer wanders within 1 tile of its spot (MOVEMENT_TYPE_WANDER_AROUND,
//  range 1). These are the floor tiles of those areas.
const std::vector<MapTile>& NORTH_TRAINER_TILES(){   //  Leroy, home (7, 56)
    static const std::vector<MapTile> tiles{{7, 55}, {8, 55}, {7, 56}, {8, 56}, {7, 57}, {8, 57}};
    return tiles;
}
const std::vector<MapTile>& SOUTH_TRAINER_TILES(){   //  Michelle, home (11, 63)
    static const std::vector<MapTile> tiles{
        {10, 62}, {11, 62}, {12, 62},
        {10, 63}, {11, 63}, {12, 63},
                  {11, 64}, {12, 64},
    };
    return tiles;
}

//  Cut clears the tall grass in the 3x3 square around the player (CUT_SIDE in
//  the decomp's fldeff_cut.c). The Cut path (from the VS Seeker tile): left,
//  Cut, left, up, Cut. These are the tiles it is used on. Together they clear
//  x = 10..12 on rows 58..60 and x = 9..11 on rows 57..59, which reaches the
//  sand of the north trainer's area at (8, 57).
const std::vector<MapTile>& CUT_CENTERS(){
    static const std::vector<MapTile> tiles{{11, 59}, {10, 58}};
    return tiles;
}
std::set<MapTile> cut_tiles(){
    std::set<MapTile> ret;
    for (const MapTile& c : CUT_CENTERS()){
        for (int dy = -1; dy <= 1; dy++){
            for (int dx = -1; dx <= 1; dx++){
                ret.insert({c.x + dx, c.y + dy});
            }
        }
    }
    return ret;
}

//  Shortest path from `start` to any tile in `goals`, over floor (and over
//  cut grass if `grass_cut`), avoiding `blocked`. Returns false if none.
bool find_path(
    const MapTile& start, const std::set<MapTile>& goals,
    const std::set<MapTile>& blocked, bool grass_cut,
    std::vector<Dir>& path, MapTile& end
){
    static const std::set<MapTile> CUT = cut_tiles();
    const Dir DIRS[] = {Dir::UP, Dir::DOWN, Dir::LEFT, Dir::RIGHT};

    std::map<MapTile, std::pair<MapTile, Dir>> previous;
    std::deque<MapTile> queue{start};
    previous.emplace(start, std::make_pair(start, Dir::UP));
    while (!queue.empty()){
        MapTile tile = queue.front();
        queue.pop_front();
        if (goals.find(tile) != goals.end()){
            path.clear();
            end = tile;
            while (!(tile == start)){
                const auto& item = previous.at(tile);
                path.emplace_back(item.second);
                tile = item.first;
            }
            std::reverse(path.begin(), path.end());
            return true;
        }
        for (Dir dir : DIRS){
            MapTile next{tile.x + dir_dx(dir), tile.y + dir_dy(dir)};
            if (previous.find(next) != previous.end() || blocked.find(next) != blocked.end()){
                continue;
            }
            char cell = canyon_cell(next);
            if (cell == '#'){
                continue;
            }
            if (cell == 'g' && !(grass_cut && CUT.find(next) != CUT.end())){
                continue;
            }
            previous.emplace(next, std::make_pair(tile, dir));
            queue.emplace_back(next);
        }
    }
    return false;
}

Dir direction_to(const MapTile& from, const MapTile& to){
    if (to.x < from.x){
        return Dir::LEFT;
    }
    if (to.x > from.x){
        return Dir::RIGHT;
    }
    return to.y < from.y ? Dir::UP : Dir::DOWN;
}



////////////////////////////////////////////////////////////////////////////////
//  Menus
////////////////////////////////////////////////////////////////////////////////

//  From the party menu, with the cursor on the given slot, open its action
//  list and return the tokens (see read_party_action_list()).
std::vector<std::string> open_action_list(ConsoleHandle& console, ProControllerContext& context){
    PartySelectionWatcher action_list(COLOR_RED);
    context.wait_for_all_requests();
    int ret = run_until<ProControllerContext>(
        console, context,
        [](ProControllerContext& context){
            pbf_press_button(context, BUTTON_A, 200ms, 1800ms);
        },
        { action_list }
    );
    if (ret < 0){
        OperationFailedExceptionWithScreenshot::fire(
            ErrorReportMode::SEND_ERROR_REPORT,
            "open_action_list(): The Pokemon's action list did not open.",
            console
        );
    }
    pbf_wait(context, 300ms);
    context.wait_for_all_requests();
    VideoSnapshot screen = console.video().snapshot();
    return read_party_action_list(console.logger(), screen);
}

//  From the overworld: open the party menu and put the cursor on the
//  Pokemon `from_last` slots before the last one (0 = last slot).
void select_party_slot_from_last(ConsoleHandle& console, ProControllerContext& context, int from_last){
    open_party_menu_from_overworld(console, context);
    //  From slot 1, up goes to CANCEL, then to the last slot, then upward.
    for (int i = 0; i < 2 + from_last; i++){
        pbf_move_left_joystick(context, {0, +1}, 200ms, 300ms);
    }
    context.wait_for_all_requests();
}

//  With an action list open, move to the action and press A.
//  The list wraps around when it has more than 3 entries (always, here).
void choose_action(ConsoleHandle& console, ProControllerContext& context, const std::vector<std::string>& actions, const std::string& action){
    for (size_t i = 0; i < actions.size(); i++){
        if (actions[i] == action){
            for (size_t c = 0; c < i; c++){
                pbf_move_left_joystick(context, {0, -1}, 200ms, 300ms);
            }
            pbf_press_button(context, BUTTON_A, 200ms, 800ms);
            context.wait_for_all_requests();
            return;
        }
    }
    OperationFailedExceptionWithScreenshot::fire(
        ErrorReportMode::NO_ERROR_REPORT,
        "choose_action(): \"" + action + "\" is not in the Pokemon's action list. "
        "Check that the Pokemon in that party slot knows the move.",
        console
    );
}


//  The Six/Seven Island page of the Fly map is a 22 x 15 grid and the cursor
//  stops at its edges (region_map.c and region_map_layout_sevii_67.h in the
//  decomp). Seven Island's town is at (5, 8), Six Island's at (17, 5).
//  How long to hold a direction on the Fly map to move the cursor n cells.
Milliseconds cursor_hold_ms(int cells){
    return frames_to_ms(5.0 * cells - 2.5);
}

//  Move the Fly map cursor one cell (a tap shorter than a cursor move).
void fly_cursor_tap(ProControllerContext& context, DpadPosition direction){
    pbf_press_dpad(context, direction, 50ms, 300ms);
}

//  With the Fly map open and the cursor near Seven Island, read the name of
//  the place under the cursor and step until it is Seven Island (5, 8).
//  Nearby on the Sevii 6-7 map:
//      (5, 6), (5, 7)              Trainer Tower
//      (5, 8)                      Seven Island
//      (5, 9)                      Canyon Entrance
//      (6, 9), (6, 10), (6, 11)    Sevault Canyon
//  Anywhere else (open sea, no name): go back to the corner and tap the way
//  there one cell at a time.
bool fly_cursor_to_seven_island(ConsoleHandle& console, ProControllerContext& context){
    bool tapped_from_corner = false;
    for (int moves = 0; moves < 10; moves++){
        pbf_wait(context, 300ms);
        context.wait_for_all_requests();
        std::string name = read_fly_map_name(console.logger(), console.video().snapshot());
        if (text_contains_any(name, {"seven", "even isl"})){
            return true;
        }
        if (name.empty() && tapped_from_corner){
            //  Tapping from the corner one cell at a time is reliable. If the
            //  name still can't be read, trust the taps.
            console.log("Can't read the place name. Trusting the cursor taps.", COLOR_ORANGE);
            return true;
        }
        if (text_contains_any(name, {"trainer", "tower"})){
            fly_cursor_tap(context, DPAD_DOWN);
        }else if (text_contains_any(name, {"entrance"})){
            fly_cursor_tap(context, DPAD_UP);
        }else if (text_contains_any(name, {"sevault", "canyon"})){
            fly_cursor_tap(context, DPAD_LEFT);
        }else{
            console.log("The cursor isn't on a known place near Seven Island. Starting over from the corner.", COLOR_ORANGE);
            pbf_move_left_joystick(context, {-1, +1}, 2500ms, 300ms);
            for (int i = 0; i < 5; i++){
                fly_cursor_tap(context, DPAD_RIGHT);
            }
            for (int i = 0; i < 8; i++){
                fly_cursor_tap(context, DPAD_DOWN);
            }
            tapped_from_corner = true;
        }
    }
    return false;
}

void fly_to_seven_island(ConsoleHandle& console, ProControllerContext& context, Walker& walker){
    console.log("Flying to Seven Island...");
    for (int attempt = 0; attempt < 3; attempt++){
        select_party_slot_from_last(console, context, 0);
        std::vector<std::string> actions = open_action_list(console, context);

        FlyMapWatcher fly_map(COLOR_RED);
        choose_action(console, context, actions, "fly");
        int ret = wait_until(console, context, 10000ms, { fly_map });
        if (ret < 0){
            console.log("The Fly map did not open. Backing out and trying again.", COLOR_RED);
            pbf_mash_button(context, BUTTON_B, 4000ms);
            continue;
        }
        pbf_wait(context, 1000ms);

        //  Hold up-left into the top-left corner (0, 0). From Six Island
        //  (17, 5), the farthest start, that is 17 cells.
        pbf_move_left_joystick(context, {-1, +1}, 2500ms, 300ms);
        //  Then to Seven Island at (5, 8): 5 cells down-right, 3 more down.
        //  Held, the cursor moves one cell every 5 frames (the timings of
        //  fly_from_kanto_map() fit this), so a hold of 5n - 2.5 frames
        //  moves exactly n cells with half a cell of margin either way.
        pbf_move_left_joystick(context, {+1, -1}, cursor_hold_ms(5), 300ms);
        pbf_move_left_joystick(context, {0, -1}, cursor_hold_ms(3), 300ms);

        //  The holds can be off by a cell. Read the name of the place under
        //  the cursor and step onto Seven Island from there.
        if (!fly_cursor_to_seven_island(console, context)){
            console.log("Could not get the cursor onto Seven Island. Backing out and trying again.", COLOR_RED);
            pbf_mash_button(context, BUTTON_B, 4000ms);
            continue;
        }

        BlackScreenWatcher flying(COLOR_RED);
        context.wait_for_all_requests();
        ret = run_until<ProControllerContext>(
            console, context,
            [](ProControllerContext& context){
                pbf_press_button(context, BUTTON_A, 200ms, 2800ms);
                pbf_press_button(context, BUTTON_A, 200ms, 2800ms);
            },
            { flying }
        );
        if (ret < 0){
            console.log("Could not start flying. Backing out and trying again.", COLOR_RED);
            pbf_mash_button(context, BUTTON_B, 4000ms);
            continue;
        }
        pbf_wait(context, 8000ms);
        context.wait_for_all_requests();
        walker.forget_facing();
        console.log("Landed on Seven Island.");
        return;
    }
    OperationFailedExceptionWithScreenshot::fire(
        ErrorReportMode::SEND_ERROR_REPORT,
        "fly_to_seven_island(): Failed to fly 3 times.",
        console
    );
}


//  Whether the bag's cursor is on a Max Repel: its description says 250
//  steps (Repel's 100, Super Repel's 200).
bool bag_cursor_on_max_repel(ConsoleHandle& console, ProControllerContext& context){
    pbf_wait(context, 300ms);
    context.wait_for_all_requests();
    std::string text = read_bag_description(console.logger(), console.video().snapshot());
    return text_contains_any(text, {"250", "25o", "2s0"});
}

//  From the overworld, use the Max Repel in the given Items pocket slot. If
//  the item there isn't a Max Repel (the bag changed), go to the top of the
//  Items pocket and look down the list for it. An item is only used once its
//  description says it is a Max Repel (using an Escape Rope by mistake would
//  leave the area).
void use_max_repel(ConsoleHandle& console, ProControllerContext& context, uint8_t slot){
    console.log("Using a Max Repel...");
    open_bag_from_overworld(console, context);
    pbf_wait(context, 500ms);

    auto to_top_of_items = [&]{
        //  The Items pocket is the leftmost; pockets don't wrap.
        for (int i = 0; i < 3; i++){
            pbf_press_dpad(context, DPAD_LEFT, 120ms, 400ms);
        }
        //  The item list doesn't wrap: holding up reaches the first item.
        pbf_press_dpad(context, DPAD_UP, 3000ms, 200ms);
    };
    to_top_of_items();
    for (uint8_t i = 1; i < slot; i++){
        pbf_press_dpad(context, DPAD_DOWN, 120ms, 250ms);
    }
    if (!bag_cursor_on_max_repel(console, context)){
        console.log("Item " + std::to_string(slot) + " isn't a Max Repel. Looking for it from the top of the Items pocket...", COLOR_ORANGE);
        to_top_of_items();
        bool found = false;
        for (int i = 0; i < 42 && !found; i++){
            if (bag_cursor_on_max_repel(console, context)){
                found = true;
                console.log("Max Repel is item " + std::to_string(i + 1) + " of the Items pocket. (Update the Max Repel Position option to match.)", COLOR_ORANGE);
                break;
            }
            pbf_press_dpad(context, DPAD_DOWN, 120ms, 250ms);
        }
        if (!found){
            console.log("No Max Repel found in the Items pocket. Going on without one.", COLOR_RED);
            pbf_mash_button(context, BUTTON_B, 3000ms);
            settle_in_overworld(console, context);
            return;
        }
    }

    //  A opens the item's menu with USE first; A again uses it.
    pbf_press_button(context, BUTTON_A, 200ms, 800ms);
    pbf_press_button(context, BUTTON_A, 200ms, 1500ms);
    context.wait_for_all_requests();

    VideoSnapshot screen = console.video().snapshot();
    std::string text = read_dialog_line(console.logger(), screen, DialogStyle::OVERWORLD, 0);
    if (!text_contains_any(text, {"repel", "used", "lingered"})){
        console.log("Did not see the Max Repel message. Check the Max Repel Position option.", COLOR_RED);
    }

    //  Close the message, the bag and the start menu.
    pbf_mash_button(context, BUTTON_B, 4000ms);
    settle_in_overworld(console, context);
}


//  Put party slot 2 (the switch-in Pokemon) first.
void swap_first_two_party_slots(ConsoleHandle& console, ProControllerContext& context){
    console.log("Moving the switch-in Pokemon to the front of the party...");
    open_party_menu_from_overworld(console, context);
    std::vector<std::string> actions = open_action_list(console, context);
    choose_action(console, context, actions, "switch");
    //  Pick slot 2 (to the right of slot 1).
    pbf_press_dpad(context, DPAD_RIGHT, 150ms, 400ms);
    pbf_press_button(context, BUTTON_A, 200ms, 2500ms);
    //  Close the party menu and the start menu.
    pbf_mash_button(context, BUTTON_B, 3000ms);
    settle_in_overworld(console, context);
}


//  Use Cut from the overworld. The Cut user is second to last in the party.
void use_cut(ConsoleHandle& console, ProControllerContext& context){
    select_party_slot_from_last(console, context, 1);
    std::vector<std::string> actions = open_action_list(console, context);
    choose_action(console, context, actions, "cut");
    //  The party menu closes and the Cut animation plays. The player keeps
    //  facing the same way.
    pbf_wait(context, 3500ms);
    context.wait_for_all_requests();
}



////////////////////////////////////////////////////////////////////////////////
//  Inside the Chansey house (map "House3": the door is at (4, 7), the dancer
//  at (2, 4). Row 7 is clear from x = 1 to x = 10).
////////////////////////////////////////////////////////////////////////////////

struct HouseState{
    int x = 4;
    int y = 7;
};

//  Walk along row 7 to x, from anywhere on row 7.
//  After walking into a wall the player keeps bumping for a moment, and the
//  game ignores the d-pad until that animation ends.
constexpr Milliseconds BUMP_SETTLE = 700ms;

//  For pacing: walk along row 7 to the left (x = 1) or right (x = 10) wall,
//  one extra step into the wall, so a lost step doesn't matter.
void house_walk_row(Walker& walker, HouseState& house, ProControllerContext& context, int x){
    if (x == house.x){
        return;
    }
    const Dir dir = x < house.x ? Dir::LEFT : Dir::RIGHT;
    const uint16_t steps = (uint16_t)std::abs(x - house.x);
    walker.walk(dir, steps + 1, false);
    walker.uncount(1);
    context.wait_for_all_requests();
    pbf_wait(context, BUMP_SETTLE);
    context.wait_for_all_requests();
    house.x = x;
}

//  Walk left into the wall to be sure the player is at x = 1 of row 7.
//  `known_x` is where the player should be (only used to count the steps).
void house_calibrate(Walker& walker, HouseState& house, ProControllerContext& context, int known_x){
    const uint16_t presses = 11;
    walker.walk(Dir::LEFT, presses, false);
    context.wait_for_all_requests();
    pbf_wait(context, BUMP_SETTLE);
    context.wait_for_all_requests();
    walker.uncount(presses - (uint32_t)std::max(0, known_x - 1));
    house.x = 1;
    house.y = 7;
}

//  The house is 11 x 9 tiles; its bottom row (8) is drawn black like the
//  outside, so 8 rows show.
constexpr int HOUSE_WIDTH = 11;
constexpr int HOUSE_FLOOR_ROWS = 8;

//  Floor the player can walk on: rows 3 to 7, x = 1 to 10, except the table
//  (x = 4 to 7, rows 4 and 5) and the dancer at (2, 4). The door mat (4, 7)
//  is only walked onto as a destination: pressing down on it leaves.
bool house_walkable(int x, int y, int goal_x, int goal_y){
    if (x == goal_x && y == goal_y){
        return true;
    }
    if (x < 1 || x > 10 || y < 3 || y > 7){
        return false;
    }
    if (x >= 4 && x <= 7 && (y == 4 || y == 5)){
        return false;
    }
    if ((x == 2 && y == 4) || (x == 4 && y == 7)){
        return false;
    }
    return true;
}

//  The first step of a shortest path from (x, y) to the goal. False if none.
bool house_next_step(int x, int y, int goal_x, int goal_y, Dir& first){
    const Dir DIRS[] = {Dir::UP, Dir::DOWN, Dir::LEFT, Dir::RIGHT};
    std::map<std::pair<int, int>, Dir> first_dir;
    std::deque<std::pair<int, int>> queue;
    first_dir[{x, y}] = Dir::UP;
    queue.emplace_back(x, y);
    while (!queue.empty()){
        std::pair<int, int> tile = queue.front();
        queue.pop_front();
        if (tile.first == goal_x && tile.second == goal_y){
            first = first_dir.at(tile);
            return true;
        }
        for (Dir dir : DIRS){
            std::pair<int, int> next{tile.first + dir_dx(dir), tile.second + dir_dy(dir)};
            if (first_dir.find(next) != first_dir.end()){
                continue;
            }
            if (!house_walkable(next.first, next.second, goal_x, goal_y)){
                continue;
            }
            //  Leaving the start, remember which way; later tiles inherit it.
            first_dir[next] = (tile.first == x && tile.second == y) ? dir : first_dir.at(tile);
            queue.emplace_back(next);
        }
    }
    return false;
}

//  Read the player's tile in the house from the screen. False if it can't
//  be read (e.g. a message box is open).
bool house_locate(ConsoleHandle& console, ProControllerContext& context, HouseState& house){
    for (int tries = 0; tries < 5; tries++){
        context.wait_for_all_requests();
        VideoSnapshot screen = console.video().snapshot();
        IndoorTile tile = read_indoor_position(screen, HOUSE_WIDTH, HOUSE_FLOOR_ROWS);
        if (tile.found){
            house.x = tile.x;
            house.y = tile.y;
            return true;
        }
        pbf_wait(context, 300ms);
    }
    return false;
}

//  Walk to (x, y) in the house, reading the position from the screen after
//  every step, so lost or extra steps are corrected on the way.
void house_go_to(
    ConsoleHandle& console, ProControllerContext& context,
    Walker& walker, HouseState& house, int x, int y
){
    int last_x = -1;
    int last_y = -1;
    for (int moves = 0; moves < 40; moves++){
        if (!house_locate(console, context, house)){
            OperationFailedExceptionWithScreenshot::fire(
                ErrorReportMode::SEND_ERROR_REPORT,
                "house_go_to(): Could not tell where the player is in the house.",
                console
            );
        }
        if (house.x == last_x && house.y == last_y){
            //  The step didn't happen (only a turn, or a bump).
            walker.uncount(1);
            pbf_wait(context, BUMP_SETTLE);
        }
        if (house.x == x && house.y == y){
            return;
        }
        Dir dir;
        if (!house_next_step(house.x, house.y, x, y, dir)){
            OperationFailedExceptionWithScreenshot::fire(
                ErrorReportMode::SEND_ERROR_REPORT,
                "house_go_to(): The player is at (" + std::to_string(house.x) + ", " +
                std::to_string(house.y) + ") in the house, with no way to (" +
                std::to_string(x) + ", " + std::to_string(y) + ").",
                console
            );
        }
        last_x = house.x;
        last_y = house.y;
        walker.walk(dir, 1, false);
    }
    OperationFailedExceptionWithScreenshot::fire(
        ErrorReportMode::SEND_ERROR_REPORT,
        "house_go_to(): Could not reach (" + std::to_string(x) + ", " + std::to_string(y) + ") in the house.",
        console
    );
}

//  Pace along row 7 until at least `steps` more steps are taken.
void house_pace(Walker& walker, HouseState& house, ProControllerContext& context, uint32_t steps){
    uint32_t start = walker.steps();
    while (walker.steps() - start < steps){
        house_walk_row(walker, house, context, house.x <= 5 ? 10 : 1);
    }
    context.wait_for_all_requests();
}

//  From the door, pace until the "Repel's effect wore off" message, then
//  close it. Ends on row 7 at x = 1.
void house_burn_repel(ConsoleHandle& console, ProControllerContext& context, Walker& walker, HouseState& house){
    console.log("Pacing until the Max Repel wears off...");
    WhiteDialogWatcher dialog(COLOR_RED);
    context.wait_for_all_requests();
    int ret = run_until<ProControllerContext>(
        console, context,
        [&](ProControllerContext& context){
            //  At most 300 steps: more than a whole Max Repel.
            for (int i = 0; i < 17; i++){
                pbf_press_dpad(context, DPAD_RIGHT, frames_to_ms(TURN_FRAMES + WALK_STEP_FRAMES * 9), 0ms);
                pbf_press_dpad(context, DPAD_LEFT, frames_to_ms(TURN_FRAMES + WALK_STEP_FRAMES * 9), 0ms);
            }
        },
        { dialog }
    );
    if (ret == 0){
        VideoSnapshot screen = console.video().snapshot();
        std::string text = read_dialog_line(console.logger(), screen, DialogStyle::OVERWORLD, 0);
        if (!text_contains_any(text, {"repel", "wore off", "effect"})){
            console.log("A message came up that doesn't look like the Repel one. Closing it anyway.", COLOR_ORANGE);
        }
        pbf_mash_button(context, BUTTON_B, 2000ms);
        console.log("The Max Repel wore off.");
    }else{
        console.log("No Repel message after 300 steps. Was the Max Repel used? Continuing.", COLOR_RED);
    }
    walker.set_facing(Dir::RIGHT);  //  Either way; the calibration turns left first.
    house_calibrate(walker, house, context, 10);
}

//  From row 7, talk to the dancer and dance (heals the party). Ends on row 7.
//  The heal only works once per visit (the game resets it when the house loads).
void house_heal(ConsoleHandle& console, ProControllerContext& context, Walker& walker, HouseState& house){
    console.log("Healing with the Chansey dance...");
    //  Below the dancer at (2, 4), facing him.
    house_go_to(console, context, walker, house, 2, 5);
    walker.face(Dir::UP);
    context.wait_for_all_requests();

    //  Talk, and press A through the dialogue up to "Would you like to dance?".
    //  A on that answers YES, which is what we want anyway.
    SelectionDialogWatcher yes_no(COLOR_RED);
    int ret = run_until<ProControllerContext>(
        console, context,
        [](ProControllerContext& context){
            for (int i = 0; i < 12; i++){
                pbf_press_button(context, BUTTON_A, 200ms, 1300ms);
            }
        },
        { yes_no }
    );
    if (ret < 0){
        OperationFailedExceptionWithScreenshot::fire(
            ErrorReportMode::SEND_ERROR_REPORT,
            "house_heal(): The dance prompt did not appear.",
            console
        );
    }
    pbf_press_button(context, BUTTON_A, 200ms, 1000ms);     //  YES
    //  The dance, the heal, and "You're all chipper now!". B only closes text now.
    pbf_mash_button(context, BUTTON_B, 14000ms);
    WhiteDialogOverWatcher closed(COLOR_RED);
    ret = wait_until(console, context, 5000ms, { closed });
    if (ret < 0){
        pbf_mash_button(context, BUTTON_B, 3000ms);
    }
    settle_in_overworld(console, context);

    //  Which way the dance leaves the player facing isn't known. Back to
    //  row 7, against the left wall (where pacing starts).
    walker.forget_facing();
    house_go_to(console, context, walker, house, 1, 7);
}

//  From row 7, leave through the door at (4, 7). Ends outside at (14, 62).
void house_leave(ConsoleHandle& console, ProControllerContext& context, Walker& walker, HouseState& house){
    //  Onto the door mat, reading the position on the way, then down.
    for (int attempt = 0;; attempt++){
        house_go_to(console, context, walker, house, 4, 7);
        if (try_go_through_door(console, context, walker, Dir::DOWN, "Sevault Canyon")){
            break;
        }
        if (attempt >= 2){
            OperationFailedExceptionWithScreenshot::fire(
                ErrorReportMode::SEND_ERROR_REPORT,
                "house_leave(): Could not find the house door after 3 tries.",
                console
            );
        }
        console.log("Missed the door. Reading the position and trying again.", COLOR_ORANGE);
    }
    //  The game walks the player out of the doorway, one step down.
    pbf_wait(context, 600ms);
    context.wait_for_all_requests();
    walker.add_steps(1);
    walker.set_facing(Dir::DOWN);
}



////////////////////////////////////////////////////////////////////////////////
//  VS Seeker
////////////////////////////////////////////////////////////////////////////////

enum class VsSeekerResult{
    FOUND,          //  No message: at least one trainer wants a rematch.
    NOT_READY,      //  "The other TRAINERS don't appear to be ready for battle."
    NOT_CHARGED,    //  "The battery isn't charged enough." `steps_needed` is set.
    NO_TRAINERS,    //  "There are no TRAINERS within range who can battle..."
};

//  Use the VS Seeker (registered to SELECT). Closes any message.
//  What each Cooltrainer showed right after the VS Seeker: "!!" or "X".
struct VsSeekerIcons{
    VsSeekerIcon north = VsSeekerIcon::NONE;
    VsSeekerIcon south = VsSeekerIcon::NONE;
};

//  Where the icons can be, seen from the VS Seeker tile, in GBA pixels: above
//  the north trainer's area (up-left of the player) and above the south
//  trainer's area (below the player, left of the house).
constexpr int NORTH_ICON_REGION[4] = {16, 0, 112, 64};
constexpr int SOUTH_ICON_REGION[4] = {64, 96, 152, 160};

const char* icon_name(VsSeekerIcon icon){
    switch (icon){
    case VsSeekerIcon::READY:
        return "\"!!\" (ready)";
    case VsSeekerIcon::NOT_READY:
        return "\"X\" (not ready)";
    default:
        return "no icon seen";
    }
}

//  `on_frame` is also given each frame watched for the icons.
VsSeekerResult use_vs_seeker(
    ConsoleHandle& console, ProControllerContext& context,
    int& steps_needed, VsSeekerIcons& icons,
    const std::function<void(const ImageViewRGB32&)>& on_frame = {}
){
    console.log("Using the VS Seeker...");
    context.wait_for_all_requests();
    pbf_press_button(context, BUTTON_MINUS, 200ms, 0ms);
    context.wait_for_all_requests();

    //  For 4 seconds, watch for a message (nobody ready, or not charged) and,
    //  meanwhile, for the icons above the trainers. Each icon shows for only
    //  about half a second.
    WhiteDialogDetector dialog(COLOR_RED);
    size_t north_ready = 0;
    size_t north_x = 0;
    size_t south_ready = 0;
    size_t south_x = 0;
    bool message = false;
    WallClock dialog_since = WallClock::max();
    WallClock end = current_time() + 4000ms;
    size_t frames = 0;
    while (current_time() < end){
        VideoSnapshot screen = console.video().snapshot();
        frames++;
        WallClock now = current_time();
        if (dialog.detect(screen)){
            if (dialog_since == WallClock::max()){
                dialog_since = now;
            }
            if (now - dialog_since >= 250ms){
                message = true;
                break;
            }
        }else{
            dialog_since = WallClock::max();
        }
        if (on_frame){
            on_frame(screen);
        }
        VsSeekerIcon north = find_vs_seeker_icon(screen,
            NORTH_ICON_REGION[0], NORTH_ICON_REGION[1], NORTH_ICON_REGION[2], NORTH_ICON_REGION[3]
        );
        VsSeekerIcon south = find_vs_seeker_icon(screen,
            SOUTH_ICON_REGION[0], SOUTH_ICON_REGION[1], SOUTH_ICON_REGION[2], SOUTH_ICON_REGION[3]
        );
        north_ready += north == VsSeekerIcon::READY ? 1 : 0;
        north_x += north == VsSeekerIcon::NOT_READY ? 1 : 0;
        south_ready += south == VsSeekerIcon::READY ? 1 : 0;
        south_x += south == VsSeekerIcon::NOT_READY ? 1 : 0;
        context.wait_for(30ms);
    }

    //  An icon counts if it was seen in at least 2 frames, and more often
    //  than the other one. An icon only shows for about half a second of the
    //  4: something seen in most frames is part of the scenery, not an icon.
    auto decide = [frames](size_t ready, size_t x){
        size_t too_many = std::max<size_t>(frames / 2, 4);
        if (ready >= 2 && ready > x && ready < too_many){
            return VsSeekerIcon::READY;
        }
        if (x >= 2 && x > ready && x < too_many){
            return VsSeekerIcon::NOT_READY;
        }
        return VsSeekerIcon::NONE;
    };
    icons.north = decide(north_ready, north_x);
    icons.south = decide(south_ready, south_x);
    console.log(
        std::string("VS Seeker icons: north ") + icon_name(icons.north) +
        " [" + std::to_string(north_ready) + " !! / " + std::to_string(north_x) + " X frames]" +
        ", south " + icon_name(icons.south) +
        " [" + std::to_string(south_ready) + " !! / " + std::to_string(south_x) + " X frames]" +
        ", of " + std::to_string(frames) + " frames"
    );

    if (!message){
        console.log("No VS Seeker message: someone wants a rematch.");
        return VsSeekerResult::FOUND;
    }

    pbf_wait(context, 500ms);
    context.wait_for_all_requests();
    VideoSnapshot screen = console.video().snapshot();
    std::string text = read_dialog_line(console.logger(), screen, DialogStyle::OVERWORLD, 0);

    VsSeekerResult result = VsSeekerResult::NOT_READY;
    if (text_contains_any(text, {"battery", "charged"})){
        result = VsSeekerResult::NOT_CHARGED;
        //  The next box: "No. of steps required to fully / charge the battery: N"
        pbf_press_button(context, BUTTON_A, 200ms, 1200ms);
        context.wait_for_all_requests();
        screen = console.video().snapshot();
        steps_needed = last_number_in(read_dialog_line(console.logger(), screen, DialogStyle::OVERWORLD, 1));
        if (steps_needed <= 0 || steps_needed > 100){
            console.log("Could not read the number of steps. Using 100.", COLOR_ORANGE);
            steps_needed = 100;
        }
        console.log("VS Seeker needs " + std::to_string(steps_needed) + " more steps.");
    }else if (text_contains_any(text, {"within range", "no trainers", "no trainer"})){
        result = VsSeekerResult::NO_TRAINERS;
    }else if (text_contains_any(text, {"ready", "appear", "other trainers"})){
        result = VsSeekerResult::NOT_READY;
    }else{
        OperationFailedExceptionWithScreenshot::fire(
            ErrorReportMode::SEND_ERROR_REPORT,
            "use_vs_seeker(): Unexpected message: \"" + text + "\". Is the VS Seeker registered to SELECT?",
            console
        );
    }

    //  Close the message.
    WhiteDialogOverWatcher closed(COLOR_RED);
    run_until<ProControllerContext>(
        console, context,
        [](ProControllerContext& context){
            for (int i = 0; i < 8; i++){
                pbf_press_button(context, BUTTON_B, 200ms, 800ms);
            }
        },
        { closed }
    );
    pbf_wait(context, 300ms);
    context.wait_for_all_requests();
    return result;
}


struct ReadyTrainers{
    bool north = false;
    MapTile north_tile{0, 0};
    bool south = false;
    MapTile south_tile{0, 0};
};

//  Right after a VS Seeker use that found rematches. Trainers that want a
//  rematch show "!!", then stand still and hop in place; the others show "X"
//  and keep wandering. The icon decides when it was seen. Both areas are
//  also watched for a few seconds to find each trainer's tile, and, where no
//  icon was seen, whether they hop (stay on one tile and jump).
ReadyTrainers find_ready_trainers(ConsoleHandle& console, ProControllerContext& context, const VsSeekerIcons& icons){
    struct Track{
        //  Tiles where the trainer's hair is on screen, and tiles so high up
        //  (Leroy's top row) that only the legs are: those are watched by
        //  the feet instead.
        std::vector<TileOffset> head_offsets;
        std::vector<TileOffset> feet_offsets;
        std::map<std::pair<int, int>, std::vector<double>> y_by_tile;
        size_t frames_found = 0;
    };
    auto make_track = [](const std::vector<MapTile>& tiles){
        Track track;
        for (const MapTile& t : tiles){
            TileOffset offset{t.x - VS_SEEKER_TILE.x, t.y - VS_SEEKER_TILE.y};
            if (head_box_on_screen(offset)){
                track.head_offsets.push_back(offset);
            }else{
                track.feet_offsets.push_back(offset);
            }
        }
        return track;
    };
    Track north = make_track(NORTH_TRAINER_TILES());
    Track south = make_track(SOUTH_TRAINER_TILES());

    //  Let the trainers start hopping.
    pbf_wait(context, 500ms);
    context.wait_for_all_requests();

    size_t frames = 0;
    WallClock end = current_time() + 4000ms;
    while (current_time() < end){
        VideoSnapshot screen = console.video().snapshot();
        frames++;
        for (Track* track : {&north, &south}){
            SpriteSighting sighting = find_purple_hair(screen, track->head_offsets);
            if (!sighting.found && !track->feet_offsets.empty()){
                sighting = find_sprite_feet(screen, track->feet_offsets);
            }
            if (sighting.found){
                track->frames_found++;
                track->y_by_tile[{sighting.tile.dx, sighting.tile.dy}].push_back(sighting.y);
            }
        }
        context.wait_for(40ms);
    }

    ReadyTrainers ret;
    auto judge = [&](const Track& track, const char* name, VsSeekerIcon icon, bool& ready, MapTile& tile){
        if (icon == VsSeekerIcon::NOT_READY){
            console.log(std::string(name) + " Cooltrainer showed \"X\": not ready.");
            return;
        }
        //  The tile the trainer was seen on most.
        const std::vector<double>* most = nullptr;
        std::pair<int, int> most_tile{0, 0};
        for (const auto& item : track.y_by_tile){
            if (most == nullptr || item.second.size() > most->size()){
                most = &item.second;
                most_tile = item.first;
            }
        }
        if (icon == VsSeekerIcon::READY){
            //  Ready trainers stand still: their tile is where they were seen
            //  most (their hops and raised hand can hide the hair at times).
            if (most == nullptr || most->size() < frames / 4){
                console.log(
                    std::string(name) + " Cooltrainer showed \"!!\", but couldn't be found on one tile. Skipping them.",
                    COLOR_ORANGE
                );
                return;
            }
            tile = {VS_SEEKER_TILE.x + most_tile.first, VS_SEEKER_TILE.y + most_tile.second};
            ready = true;
            console.log(std::string(name) + " Cooltrainer showed \"!!\": ready, at " + tile_str(tile) + ".");
            return;
        }
        if (most == nullptr || most->size() < frames * 7 / 10){
            console.log(std::string(name) + " Cooltrainer: not seen on one tile the whole time (walking or hidden): not ready.");
            return;
        }
        //  A hop lifts the sprite by about 6 pixels for a moment.
        std::vector<double> sorted = *most;
        std::sort(sorted.begin(), sorted.end());
        double median = sorted[sorted.size() / 2];
        double highest = sorted.front();
        double lift = median - highest;
        tile = {VS_SEEKER_TILE.x + most_tile.first, VS_SEEKER_TILE.y + most_tile.second};
        ready = lift >= 3.0;
        console.log(
            std::string(name) + " Cooltrainer at " + tile_str(tile) +
            ", seen in " + std::to_string(most->size()) + "/" + std::to_string(frames) +
            " frames, hop height " + std::to_string(lift) + " px: " + (ready ? "ready." : "not ready.")
        );
    };
    //  The VS Seeker found a rematch, so someone must have shown "!!". If no
    //  "!!" was seen, the icons were misread: judge by hopping alone.
    VsSeekerIcons used = icons;
    if (icons.north != VsSeekerIcon::READY && icons.south != VsSeekerIcon::READY){
        if (icons.north != VsSeekerIcon::NONE || icons.south != VsSeekerIcon::NONE){
            console.log("No \"!!\" was seen, but someone is ready. Ignoring the icons.", COLOR_ORANGE);
        }
        used = VsSeekerIcons();
    }
    judge(north, "North", used.north, ret.north, ret.north_tile);
    judge(south, "South", used.south, ret.south, ret.south_tile);
    return ret;
}



////////////////////////////////////////////////////////////////////////////////
//  Battles
////////////////////////////////////////////////////////////////////////////////

const std::set<std::string>& COOLTRAINER_SPECIES(){
    //  Leroy and Michelle, first battle and rematch teams.
    static const std::set<std::string> species{
        "rhydon", "slowbro", "kangaskhan", "machoke", "machamp", "ursaring",
        "persian", "dewgong", "ninetales", "rapidash", "girafarig",
    };
    return species;
}

struct BattleContext{
    ExpFarmer::EvolutionChoice evolution;
    ExpFarmer::MoveLearnChoice move_learn;
    bool switch_in;

    //  Who the opponents can be (their species names, to read them).
    const std::set<std::string>* opponent_species = &COOLTRAINER_SPECIES();

    //  Two Pokemon on each side (Mik & Kia on Kindle Road).
    bool double_battle = false;

    //  The battling Pokemon's moves, read once and kept until a move is
    //  learned. In a single battle there is one battling Pokemon (key ""); in
    //  a double battle, two, by the name the battle menu asks about.
    std::map<std::string, MoveSlots> moves_by_name;
    //  The strong Pokemon's species, in single battles.
    bool species_read = false;
    std::string own_species;

    //  A move that the opponent disabled (Disable lasts 2 to 5 turns). It is
    //  skipped for a few turns, then tried again.
    int disabled_slot = -1;
    int disabled_turns = 0;

    //  The opponent used Grudge ("X wants the opponent to bear a GRUDGE!"):
    //  if it faints from our next move, that move loses all its PP. So the
    //  next move is a different attacking move, or, with only one, a turn
    //  of something else (at most twice per battle).
    bool grudge_active = false;
    int grudge_stalls = 0;

    //  The Pokemon being leveled, by its level-up messages ("X grew to LV. N!").
    //    - BATTLING: the lead of each battle (the switch-in Pokemon, or slot 1
    //      without Switch-In). Its name and level are read from the HP box on
    //      the first turn.
    //    - EXP_SHARE: any Pokemon that didn't battle. The names in the HP box
    //      are read every turn to know which Pokemon battled.
    ExpFarmer::LevelingTarget leveling = ExpFarmer::LevelingTarget::BATTLING;
    uint8_t target_level = 100;
    std::string leveler_name;
    std::vector<std::string> battled_names;     //  This battle.
    int leveler_level = -1;

    //  "Player defeated X!" or the prize money was read: the battle is won.
    bool trainer_defeated = false;

    //  Set by the battle when the program should stop afterwards.
    bool stop_requested = false;
    std::string stop_reason;

    void set_leveler_level(Logger& logger, int level){
        if (level <= 0 || level == leveler_level){
            return;
        }
        leveler_level = level;
        logger.log("The Pokemon being leveled is now level " + std::to_string(level) + ".");
        if (level >= target_level && !stop_requested){
            stop_requested = true;
            stop_reason = "The Pokemon being leveled reached level " + std::to_string(level) + ".";
        }
    }
};

//  Letters only, lower case.
std::string letters_of(const std::string& text){
    std::string ret;
    for (char ch : text){
        if (std::isalpha((unsigned char)ch)){
            ret += (char)std::tolower((unsigned char)ch);
        }
    }
    return ret;
}
size_t edit_distance(const std::string& a, const std::string& b){
    std::vector<size_t> row(b.size() + 1);
    for (size_t j = 0; j <= b.size(); j++){
        row[j] = j;
    }
    for (size_t i = 1; i <= a.size(); i++){
        size_t diagonal = row[0];
        row[0] = i;
        for (size_t j = 1; j <= b.size(); j++){
            size_t up = row[j];
            row[j] = std::min({row[j] + 1, row[j - 1] + 1, diagonal + (a[i - 1] == b[j - 1] ? 0 : 1)});
            diagonal = up;
        }
    }
    return row[b.size()];
}

//  On a battle text box: if it says "<NAME> grew to LV. <N>!" and the name is
//  the leveled Pokemon's, update its level.
bool names_match(const std::string& a, const std::string& b){
    if (a.empty() || b.empty()){
        return false;
    }
    size_t tolerance = std::max<size_t>(1, b.size() / 4);
    return edit_distance(a, b) <= tolerance;
}
//  Battle text that waits for a button: notice Grudge, and level ups of the
//  Pokemon being leveled.
void check_level_up_text(ConsoleHandle& console, BattleContext& battle){
    VideoSnapshot screen = console.video().snapshot();
    std::string text =
        read_dialog_line(console.logger(), screen, DialogStyle::BATTLE, 0) + " " +
        read_dialog_line(console.logger(), screen, DialogStyle::BATTLE, 1);
    if (text_contains_any(text, {"defeated", "for winning"})){
        battle.trainer_defeated = true;
    }
    if (text_contains_any(text, {"grudge", "bear a"}) && !text_contains_any(text, {"lost"})){
        console.log("The opponent used Grudge. Not using the best move on the next turn.", COLOR_ORANGE);
        battle.grudge_active = true;
        return;
    }
    if (battle.leveling == ExpFarmer::LevelingTarget::BATTLING && battle.leveler_name.empty()){
        return;
    }
    size_t grew = text.find("grew");
    if (grew == std::string::npos){
        return;
    }
    std::string name = letters_of(text.substr(0, grew));
    bool tracked;
    if (battle.leveling == ExpFarmer::LevelingTarget::BATTLING){
        tracked = names_match(name, battle.leveler_name);
    }else{
        tracked = true;
        for (const std::string& battled : battle.battled_names){
            if (names_match(name, battled)){
                tracked = false;
            }
        }
    }
    if (!tracked){
        console.log("Level up of another Pokemon (\"" + name + "\"). Ignoring it.");
        return;
    }
    int level = last_number_in(text);
    //  The "!" after the level ("Lv. 92!") can be read as a "1": "921".
    //  Without that last digit it is the level.
    if (level > 100){
        level /= 10;
    }
    //  Levels never go down: a lower number is a misread.
    if (level < 2 || level > 100 || (battle.leveler_level > 0 && level < battle.leveler_level)){
        console.log("Could not read the new level from: \"" + text + "\"", COLOR_ORANGE);
        return;
    }
    battle.set_leveler_level(console.logger(), level);
}

enum class BattleOutcome{
    WON,
    STRONG_FAINTED,
    NO_BATTLE,      //  Talking to the trainer didn't start a battle.
};


//  At the battle menu: the name in "What will X do?", letters only.
std::string read_battling_name(ConsoleHandle& console){
    VideoSnapshot screen = console.video().snapshot();
    std::string line = read_dialog_line(console.logger(), screen, DialogStyle::BATTLE, 1);
    size_t end = line.rfind(" do");
    return letters_of(end == std::string::npos ? line : line.substr(0, end));
}


//  From the battle menu, open the party and shift to party slot 2. In battle
//  the party is listed in battle order, so slot 2 is always the other one of
//  the pair (strong <-> weak).
void battle_shift_to_slot_2(ConsoleHandle& console, ProControllerContext& context, bool from_battle_menu){
    if (from_battle_menu){
        if (!move_cursor_to_option(console, context, BattleMenuOption::POKEMON)){
            console.log("Could not confirm the cursor is on POKEMON. Pressing A anyway.", COLOR_RED);
        }
        pbf_press_button(context, BUTTON_A, 200ms, 300ms);
    }
    PartyMenuWatcher party_menu(COLOR_RED);
    int ret = wait_until(console, context, 5000ms, { party_menu });
    if (ret < 0){
        OperationFailedExceptionWithScreenshot::fire(
            ErrorReportMode::SEND_ERROR_REPORT,
            "battle_shift_to_slot_2(): The party menu did not open.",
            console
        );
    }
    pbf_wait(context, 300ms);
    pbf_press_dpad(context, DPAD_RIGHT, 150ms, 400ms);          //  slot 2
    pbf_press_button(context, BUTTON_A, 200ms, 800ms);          //  SHIFT / SUMMARY / CANCEL
    pbf_press_button(context, BUTTON_A, 200ms, 500ms);          //  SHIFT

    //  Wait until the battle screen is back ("Go! X!"), so the fade out of
    //  the party menu isn't mistaken for the end of the battle.
    BattleDialogWatcher battle_text(COLOR_RED);
    if (wait_until(console, context, 8000ms, { battle_text }) < 0){
        OperationFailedExceptionWithScreenshot::fire(
            ErrorReportMode::SEND_ERROR_REPORT,
            "battle_shift_to_slot_2(): Did not return to the battle after switching.",
            console
        );
    }
}


//  From the battle menu, pick and use the best move against the opponent.
//  Returns false if no damaging move has PP left.
bool battle_use_best_move(ConsoleHandle& console, ProControllerContext& context, BattleContext& battle){
    BattleMoveReader move_reader;
    VideoOverlaySet overlays(console.overlay());
    move_reader.make_overlays(overlays);

    //  In a double battle, the menu asks for each of our two Pokemon in turn
    //  ("What will X do?"). Their moves are remembered by that name, which is
    //  the species unless nicknamed. (The HP boxes are laid out differently
    //  there, so the singles readers don't apply.)
    std::string key;
    std::string own_species;
    if (battle.double_battle){
        key = read_battling_name(console);
        own_species = key;
    }else{
        if (!battle.species_read){
            VideoSnapshot screen = console.video().snapshot();
            battle.own_species = move_reader.read_own_species(console.logger(), Language::English, screen);
            battle.species_read = true;
            if (find_species(battle.own_species) == nullptr){
                console.log("Battling Pokemon not recognized (nickname?). Ignoring its types and stats when choosing moves.", COLOR_ORANGE);
            }else{
                console.log("Battling Pokemon: " + battle.own_species);
            }
        }
        own_species = battle.own_species;
    }

    //  Who are we fighting?
    std::string opponent;
    {
        WildEncounterReader reader(COLOR_RED);
        reader.make_overlays(overlays);
        VideoSnapshot screen = console.video().snapshot();
        opponent = reader.read_encounter(console.logger(), Language::English, screen, *battle.opponent_species).name;
    }

    //  Open the move list.
    if (!move_cursor_to_option(console, context, BattleMenuOption::FIGHT)){
        console.log("Could not confirm the cursor is on FIGHT. Pressing A anyway.", COLOR_RED);
    }
    pbf_press_button(context, BUTTON_A, 200ms, 500ms);
    context.wait_for_all_requests();

    auto known = battle.moves_by_name.find(key);
    if (known == battle.moves_by_name.end()){
        VideoSnapshot screen = console.video().snapshot();
        known = battle.moves_by_name.emplace(key, move_reader.read_moves(console.logger(), screen)).first;
        std::string log = key.empty() ? "Moves:" : "Moves of " + key + ":";
        for (const std::string& move : known->second){
            log += " [" + (move.empty() ? std::string("-") : move) + "]";
        }
        console.log(log);
    }
    const MoveSlots& moves = known->second;

    size_t usable = 0;
    std::array<int, 4> ranked = rank_moves(
        moves, find_species(own_species), find_species(opponent), usable
    );
    console.log("Opponent: " + (opponent.empty() ? std::string("(unread)") : opponent));

    //  A disabled move is skipped for a few turns, then tried again.
    if (battle.disabled_turns > 0 && --battle.disabled_turns == 0){
        battle.disabled_slot = -1;
    }

    //  The damaging moves, best first, without a disabled one. If a damaging
    //  move is disabled, any other move (even one that does nothing to this
    //  opponent) keeps the battle going until Disable wears off.
    std::vector<int> candidates;
    bool damaging_disabled = false;
    for (size_t i = 0; i < usable; i++){
        if (ranked[i] == battle.disabled_slot){
            damaging_disabled = true;
        }else{
            candidates.push_back(ranked[i]);
        }
    }
    if (damaging_disabled){
        for (int slot = 0; slot < 4; slot++){
            bool listed = slot == battle.disabled_slot ||
                std::find(candidates.begin(), candidates.end(), slot) != candidates.end();
            if (!listed && !moves[(size_t)slot].empty()){
                candidates.push_back(slot);
            }
        }
    }

    //  Grudge: if the opponent faints from the move used now, it loses all
    //  its PP. Use the next best attacking move instead. With only one, use
    //  something else for a turn (Grudge ends when the opponent moves again).
    //  Michelle's Ninetales knows Grudge, and its message may scroll by
    //  unseen: against it, always keep the best move out of it if there is
    //  another attacking move.
    bool grudge_seen = battle.grudge_active;
    battle.grudge_active = false;
    if (grudge_seen || opponent == "ninetales"){
        if (candidates.size() >= 2 && !damaging_disabled){
            console.log("Grudge: using " + moves[(size_t)candidates[1]] + " instead of " + moves[(size_t)candidates[0]] + ".");
            std::swap(candidates[0], candidates[1]);
        }else if (grudge_seen && candidates.size() == 1 && battle.grudge_stalls < 2){
            std::vector<int> others;
            for (int slot = 0; slot < 4; slot++){
                if (slot != candidates[0] && slot != battle.disabled_slot && !moves[(size_t)slot].empty()){
                    others.push_back(slot);
                }
            }
            if (!others.empty()){
                battle.grudge_stalls++;
                console.log("Grudge, and only one attacking move: using another move this turn.");
                others.push_back(candidates[0]);
                candidates = others;
            }
        }
    }

    //  (By index: a disabled move found below adds candidates.)
    for (size_t c = 0; c < candidates.size(); c++){
        int slot = candidates[c];
        if (!move_cursor_to_option(console, context, static_cast<BattleMoveOption>(slot))){
            console.log("Could not find the move cursor. Backing out to the battle menu.", COLOR_RED);
            pbf_press_button(context, BUTTON_B, 200ms, 800ms);
            return true;
        }
        BattleOutOfPpWatcher out_of_pp(COLOR_RED);
        if (wait_until(console, context, 500ms, { out_of_pp }) == 0){
            console.log(moves[slot] + " is out of PP.", COLOR_ORANGE);
            continue;
        }
        console.log("Using " + moves[slot] + " (slot " + std::to_string(slot + 1) + ").");
        pbf_press_button(context, BUTTON_A, 200ms, 1200ms);
        context.wait_for_all_requests();

        //  A disabled move gives a message ("X's MOVE is disabled!"), then
        //  goes back to the move list.
        {
            VideoSnapshot screen = console.video().snapshot();
            std::string text =
                read_dialog_line(console.logger(), screen, DialogStyle::BATTLE, 0) + " " +
                read_dialog_line(console.logger(), screen, DialogStyle::BATTLE, 1);
            if (text_contains_any(text, {"disabled", "disab"})){
                console.log(moves[slot] + " is disabled. Using another move for now.", COLOR_ORANGE);
                battle.disabled_slot = slot;
                battle.disabled_turns = 3;
                pbf_press_button(context, BUTTON_B, 200ms, 1000ms);
                context.wait_for_all_requests();
                if (!damaging_disabled){
                    //  Now there is a disabled damaging move: add the others.
                    damaging_disabled = true;
                    for (int other = 0; other < 4; other++){
                        bool listed = other == slot ||
                            std::find(candidates.begin(), candidates.end(), other) != candidates.end();
                        if (!listed && !moves[(size_t)other].empty()){
                            candidates.push_back(other);
                        }
                    }
                }
                continue;
            }
        }

        //  In a double battle, a move aimed at one Pokemon then asks for its
        //  target: the move list stays up and the target blinks. A confirms
        //  the one picked. (Moves that hit both foes go on right away.)
        if (battle.double_battle){
            BattleSelectionArrowWatcher choosing_target(COLOR_RED, &console.overlay(), BATTLE_MOVE_ARROW_BOX);
            if (wait_until(console, context, 500ms, { choosing_target }) == 0){
                console.log("Confirming the target.");
                pbf_press_button(context, BUTTON_A, 200ms, 1000ms);
                context.wait_for_all_requests();
            }
        }

        //  Other moves that can't be used stay on the move list.
        BattleSelectionArrowWatcher still_on_list(COLOR_RED, &console.overlay(), BATTLE_MOVE_ARROW_BOX);
        if (wait_until(console, context, 1500ms, { still_on_list }) == 0){
            console.log(moves[slot] + " can't be used right now. Trying the next move.", COLOR_ORANGE);
            pbf_press_button(context, BUTTON_B, 200ms, 600ms);
            context.wait_for_all_requests();
            continue;
        }
        return true;
    }
    pbf_press_button(context, BUTTON_B, 200ms, 600ms);
    return false;
}


//  Answer a battle YES/NO prompt by what it asks. Returns false if the text
//  isn't a known prompt (the YES/NO detector can fire on other battle text,
//  e.g. while a trainer sends out a Pokemon); then nothing is pressed.
bool answer_battle_prompt(
    ConsoleHandle& console, ProControllerContext& context,
    BattleContext& battle, bool& weak_out
){
    VideoSnapshot screen = console.video().snapshot();
    std::string text =
        read_dialog_line(console.logger(), screen, DialogStyle::BATTLE, 0) + " " +
        read_dialog_line(console.logger(), screen, DialogStyle::BATTLE, 1);

    if (text_contains_any(text, {"change", "about to use"})){
        if (battle.switch_in){
            console.log("Shift prompt: sending the switch-in Pokemon back in.");
            pbf_press_button(context, BUTTON_A, 200ms, 1000ms);     //  YES
            battle_shift_to_slot_2(console, context, false);
            weak_out = true;
        }else{
            pbf_press_button(context, BUTTON_B, 200ms, 1000ms);     //  NO
        }
        return true;
    }

    if (text_contains_any(text, {"stop learning", "stop"})){
        //  Only reached when declining: confirm.
        console.log("Not learning the move.");
        pbf_press_button(context, BUTTON_A, 200ms, 1000ms);
        return true;
    }

    if (text_contains_any(text, {"delete", "make room", "room for"})){
        switch (battle.move_learn){
        case ExpFarmer::MoveLearnChoice::DECLINE:
            console.log("Move-learn prompt: declining.");
            pbf_press_button(context, BUTTON_B, 200ms, 1000ms);     //  NO -> "Stop learning?"
            return true;
        case ExpFarmer::MoveLearnChoice::REPLACE_SLOT_4:{
            console.log("Move-learn prompt: forgetting the 4th move.");
            pbf_press_button(context, BUTTON_A, 200ms, 2500ms);     //  YES -> move list
            //  The list opens on the first move. The 4th is 3 down.
            for (int i = 0; i < 3; i++){
                pbf_press_dpad(context, DPAD_DOWN, 150ms, 350ms);
            }
            pbf_press_button(context, BUTTON_A, 200ms, 2500ms);
            context.wait_for_all_requests();
            //  The battling Pokemon may have changed its moves.
            battle.moves_by_name.clear();
            return true;
        }
        case ExpFarmer::MoveLearnChoice::STOP:
            battle.stop_requested = true;
            battle.stop_reason = "A Pokemon wants to learn a new move. Stopped at the prompt.";
            throw ProgramFinishedException(battle.stop_reason, console);
        }
    }

    console.log("Not a known YES/NO prompt: \"" + text + "\". Ignoring it.", COLOR_ORANGE);
    context.wait_for(500ms);
    return false;
}


//  After the battle fades out: handle evolutions (and their move prompts)
//  until back in the overworld.
void handle_after_battle(ConsoleHandle& console, ProControllerContext& context, BattleContext& battle){
    bool dummy_weak = false;
    for (int evolutions = 0; evolutions < 6; evolutions++){
        //  An evolution starts with a battle-style text box.
        BattleDialogWatcher evolving(COLOR_RED);
        if (wait_until(console, context, 3000ms, { evolving }) < 0){
            break;
        }
        console.log("A Pokemon is evolving.");

        if (battle.evolution == ExpFarmer::EvolutionChoice::CANCEL){
            //  B during the animation stops it.
            pbf_mash_button(context, BUTTON_B, 12000ms);
            context.wait_for_all_requests();
            console.log("Evolution cancelled.");
            continue;
        }

        //  Let it evolve. Press only A (B would cancel), and only when text is
        //  waiting, until the screen fades back to the overworld.
        WallClock deadline = current_time() + 60000ms;
        size_t unknown_prompts = 0;
        while (current_time() < deadline){
            AdvanceBattleDialogWatcher advance(COLOR_RED);
            BattleLearnDialogWatcher yes_no(COLOR_RED);
            BlackScreenWatcher done(COLOR_RED);
            int ret = wait_until(console, context, 25000ms, { advance, yes_no, done });
            if (ret == 0){
                pbf_press_button(context, BUTTON_A, 200ms, 600ms);
                continue;
            }
            if (ret == 1){
                if (!answer_battle_prompt(console, context, battle, dummy_weak)){
                    unknown_prompts++;
                    if (unknown_prompts >= 10){
                        OperationFailedExceptionWithScreenshot::fire(
                            ErrorReportMode::SEND_ERROR_REPORT,
                            "handle_after_battle(): Stuck on an unknown YES/NO prompt.",
                            console
                        );
                    }
                }
                continue;
            }
            break;
        }
        console.log("Evolution finished.");
        //  The battling Pokemon may be the one that evolved.
        battle.species_read = false;
        if (battle.evolution == ExpFarmer::EvolutionChoice::ALLOW_AND_STOP){
            battle.stop_requested = true;
            battle.stop_reason = "A Pokemon evolved.";
        }
    }

    //  Make sure we are in the overworld (closes any leftover text).
    if (!in_overworld(console, context)){
        pbf_mash_button(context, BUTTON_B, 3000ms);
        if (!in_overworld(console, context)){
            OperationFailedExceptionWithScreenshot::fire(
                ErrorReportMode::SEND_ERROR_REPORT,
                "handle_after_battle(): Could not get back to the overworld.",
                console
            );
        }
    }
}


BattleOutcome fight_trainer_battle(ConsoleHandle& console, ProControllerContext& context, BattleContext& battle);

//  Facing a trainer who wants a rematch: talk, and fight until the battle ends.
//  NO_BATTLE if talking twice doesn't start a battle (the press was lost, or
//  nobody is there).
BattleOutcome run_trainer_battle(
    ConsoleHandle& console, ProControllerContext& context,
    Walker& walker, Dir facing, BattleContext& battle
){
    //  Talk, and press B through their lines until the battle menu. If A
    //  brings up no text at all, nobody is there (or the press was lost):
    //  try once more, then give up quickly.
    BattleMenuWatcher battle_menu(COLOR_RED);
    int ret = -1;
    for (int attempt = 0; attempt < 2 && ret < 0; attempt++){
        if (attempt > 0){
            console.log("The battle didn't start. Facing the trainer and talking again.", COLOR_ORANGE);
            walker.face(facing);
        }
        WhiteDialogWatcher talking(COLOR_RED);
        BattleMenuWatcher started(COLOR_RED);
        context.wait_for_all_requests();
        pbf_press_button(context, BUTTON_A, 200ms, 0ms);
        if (wait_until(console, context, 4000ms, { talking, started }) < 0){
            console.log("Nobody answered.", COLOR_ORANGE);
            continue;
        }
        ret = run_until<ProControllerContext>(
            console, context,
            [](ProControllerContext& context){
                pbf_mash_button(context, BUTTON_B, 12000ms);
            },
            { battle_menu }
        );
    }
    if (ret < 0){
        console.log("No battle started. Skipping this trainer.", COLOR_RED);
        return BattleOutcome::NO_BATTLE;
    }
    return fight_trainer_battle(console, context, battle);
}

//  From the first battle menu of a trainer battle, fight until it ends.
BattleOutcome fight_trainer_battle(ConsoleHandle& console, ProControllerContext& context, BattleContext& battle){
    int ret = -1;

    //  Party slot 1 leads: the switch-in Pokemon when switching in. Either
    //  way, the lead is the Pokemon being leveled: read its name and level.
    //  (Not in a double battle, where both lead and the HP boxes differ.)
    battle.battled_names.clear();
    battle.trainer_defeated = false;
    battle.disabled_slot = -1;
    battle.disabled_turns = 0;
    battle.grudge_active = false;
    battle.grudge_stalls = 0;
    if (battle.double_battle){
        console.log("Double battle.");
    }else if (battle.leveling == ExpFarmer::LevelingTarget::BATTLING){
        BattleMoveReader reader;
        context.wait_for(300ms);
        VideoSnapshot screen = console.video().snapshot();
        std::string name = reader.read_own_name_text(console.logger(), screen);
        if (!name.empty()){
            battle.leveler_name = name;
        }
        battle.set_leveler_level(console.logger(), reader.read_own_level(console.logger(), screen));
    }

    //  In a double battle both lead Pokemon battle: there is no switching.
    bool weak_out = battle.switch_in && !battle.double_battle;
    size_t turns = 0;
    //  In a double battle, the menu comes up for each of our two Pokemon.
    const size_t max_turns = battle.double_battle ? 120 : 60;
    size_t unknown_prompts = 0;

    while (true){
        //  Our turn: we are at the battle menu.
        if (battle.leveling == ExpFarmer::LevelingTarget::EXP_SHARE){
            //  Remember who battled, so their level-ups aren't counted.
            context.wait_for(300ms);
            VideoSnapshot screen = console.video().snapshot();
            std::string name = battle.double_battle
                ? read_battling_name(console)
                : BattleMoveReader().read_own_name_text(console.logger(), screen);
            bool known = name.empty();
            for (const std::string& battled : battle.battled_names){
                if (names_match(name, battled)){
                    known = true;
                }
            }
            if (!known){
                battle.battled_names.emplace_back(std::move(name));
            }
        }
        if (turns++ > max_turns){
            OperationFailedExceptionWithScreenshot::fire(
                ErrorReportMode::SEND_ERROR_REPORT,
                "run_trainer_battle(): The battle is taking too long.",
                console
            );
        }
        if (weak_out){
            console.log("Switching to the strong Pokemon.");
            battle_shift_to_slot_2(console, context, true);
            weak_out = false;
        }else if (!battle_use_best_move(console, context, battle)){
            throw ProgramFinishedException(
                "The battling Pokemon has no damaging move with PP left. Stopping.",
                console
            );
        }

        //  Until the next turn or the end: press B only on text that waits
        //  for it, so a YES/NO prompt is never answered by accident.
        bool next_turn = false;
        while (!next_turn){
            BattleMenuWatcher menu(COLOR_RED);
            BattleLearnDialogWatcher yes_no(COLOR_RED);
            AdvanceBattleDialogWatcher advance(COLOR_RED);
            BattleLevelUpWatcher level_up(COLOR_RED, BattleLevelUpDialog::either, Language::English);
            BlackScreenWatcher faded(COLOR_RED);
            PartyMenuWatcher party_menu(COLOR_RED);
            //  Move animations take a few seconds. After "Player defeated X!",
            //  the trainer's last words aren't recognized: press on quickly.
            ret = wait_until(
                console, context, battle.trainer_defeated ? 1000ms : 3000ms,
                { menu, yes_no, advance, level_up, faded, party_menu }
            );
            switch (ret){
            case 0:
                next_turn = true;
                break;
            case 1:
                if (answer_battle_prompt(console, context, battle, weak_out)){
                    unknown_prompts = 0;
                }else if (++unknown_prompts >= 10){
                    OperationFailedExceptionWithScreenshot::fire(
                        ErrorReportMode::SEND_ERROR_REPORT,
                        "run_trainer_battle(): Stuck on an unknown YES/NO prompt.",
                        console
                    );
                }
                break;
            case 2:
                check_level_up_text(console, battle);
                pbf_press_button(context, BUTTON_B, 200ms, 300ms);
                break;
            case 3:
                pbf_press_button(context, BUTTON_B, 200ms, 300ms);
                break;
            case 4:{
                //  Either the battle ended, or the party screen is opening
                //  because our Pokemon fainted.
                PartyMenuWatcher forced_switch(COLOR_RED);
                if (!battle.trainer_defeated && wait_until(console, context, 3000ms, { forced_switch }) == 0){
                    return BattleOutcome::STRONG_FAINTED;
                }
                console.log("Battle won.");
                handle_after_battle(console, context, battle);
                return BattleOutcome::WON;
            }
            case 5:
                return BattleOutcome::STRONG_FAINTED;
            default:
                //  Something waiting that isn't recognized: nudge it.
                console.log("Nothing recognized. Pressing B.");
                pbf_press_button(context, BUTTON_B, 200ms, 800ms);
                break;
            }
        }
    }
}



////////////////////////////////////////////////////////////////////////////////
//  Seven Island run
////////////////////////////////////////////////////////////////////////////////

//  From the Pokemon Center door (outside, facing down) to the Chansey house,
//  using a Max Repel at the canyon entrance. Ends inside the house at the door.
//  Rock stair steps are from the decomp's map data.
void travel_pokecenter_to_house(ConsoleHandle& console, ProControllerContext& context, Walker& walker, uint8_t repel_slot){
    console.log("Running to the Sevault Canyon house...");
    walker.set_facing(Dir::DOWN);
    walker.walk_path({{Dir::LEFT, 4}, {Dir::DOWN, 16, 1}}, true);

    //  Menus don't change which way the player faces (still down).
    use_max_repel(console, context, repel_slot);

    walker.walk_path({
        {Dir::DOWN, 14}, {Dir::RIGHT, 4}, {Dir::UP, 2, 1, true}, {Dir::RIGHT, 3}, {Dir::DOWN, 12, 1},
        {Dir::LEFT, 3}, {Dir::DOWN, 6}, {Dir::RIGHT, 3}, {Dir::UP, 2, 1, true}, {Dir::RIGHT, 4},
        {Dir::DOWN, 3}, {Dir::RIGHT, 3}, {Dir::UP, 2, 1, true}, {Dir::RIGHT, 3}, {Dir::DOWN, 2},
        {Dir::RIGHT, 4}, {Dir::UP, 2, 1, true}, {Dir::RIGHT, 8}, {Dir::UP, 2, 1, true}, {Dir::LEFT, 3},
        {Dir::UP, 2, 1, true}, {Dir::RIGHT, 7}, {Dir::DOWN, 9}, {Dir::LEFT, 1}, {Dir::DOWN, 12, 3},
        {Dir::LEFT, 1}, {Dir::DOWN, 11}, {Dir::LEFT, 7}, {Dir::UP, 9, 2, true}, {Dir::LEFT, 4},
        {Dir::DOWN, 14}, {Dir::RIGHT, 6}, {Dir::DOWN, 19, 2},
    }, true);
    context.wait_for_all_requests();
}


struct CanyonPosition{
    MapTile tile = HOUSE_EXIT_TILE;
};

//  Where the purple part of the house roof is on screen when the player
//  stands on the VS Seeker tile (12, 59), in GBA pixels. Each tile the
//  player is right of (below) that tile moves the roof 16 pixels left (up).
constexpr int ROOF_LEFT_FROM_VS_SEEKER = 129;
constexpr int ROOF_TOP_FROM_VS_SEEKER = 72;
constexpr int ROOF_BOTTOM_FROM_VS_SEEKER = 101;

//  One reading of the player's tile from the house roof.
bool canyon_read_tile(const ImageViewRGB32& screen, MapTile& tile, std::string& details){
    HouseRoof roof = find_house_roof(screen);
    if (!roof.found){
        details = "no roof";
        return false;
    }
    double fx = VS_SEEKER_TILE.x + (ROOF_LEFT_FROM_VS_SEEKER - roof.left) / 16.0;
    double fy = roof.top >= 0
        ? VS_SEEKER_TILE.y + (ROOF_TOP_FROM_VS_SEEKER - roof.top) / 16.0
        : VS_SEEKER_TILE.y + (ROOF_BOTTOM_FROM_VS_SEEKER - roof.bottom) / 16.0;
    details = "roof left " + std::to_string(roof.left) +
        ", top " + std::to_string(roof.top) + ", bottom " + std::to_string(roof.bottom);
    double rx = std::round(fx);
    double ry = std::round(fy);
    tile = {(int)rx, (int)ry};
    //  Mid-step the roof isn't on a whole tile, and the player can't stand
    //  in a wall.
    return std::abs(fx - rx) <= 0.3 && std::abs(fy - ry) <= 0.3 && canyon_cell(tile) != '#';
}

//  Read the player's tile in the canyon from where the house roof is on
//  screen. Two readings a moment apart must agree, so a frame caught mid-
//  fade or mid-step isn't trusted. False if the roof can't be seen well
//  enough (e.g. far left, near the north trainer).
bool canyon_locate(ConsoleHandle& console, ProControllerContext& context, MapTile& tile){
    MapTile previous{-1, -1};
    std::string details;
    for (int tries = 0; tries < 6; tries++){
        context.wait_for_all_requests();
        MapTile seen{0, 0};
        if (canyon_read_tile(console.video().snapshot(), seen, details)){
            if (seen == previous){
                tile = seen;
                return true;
            }
            previous = seen;
            pbf_wait(context, 100ms);
            continue;
        }
        previous = {-1, -1};
        pbf_wait(context, 300ms);
    }
    console.log("Could not read the position from the house roof (" + details + ").", COLOR_ORANGE);
    return false;
}

//  If a wild battle started (a step into tall grass, e.g. going around a
//  wandering trainer: the Max Repel is used up by then), run from it.
//  Returns true if there was one.
bool escape_wild_battle(ConsoleHandle& console, ProControllerContext& context){
    BattleDialogWatcher battle_text(COLOR_RED);
    BattleMenuWatcher battle_menu(COLOR_RED);
    context.wait_for_all_requests();
    int ret = wait_until(console, context, 3000ms, {battle_text, battle_menu});
    if (ret < 0){
        return false;
    }
    console.log("A wild Pokemon appeared. Running from it.", COLOR_ORANGE);
    flee_battle(console, context);
    context.wait_for_all_requests();
    pbf_wait(context, 500ms);
    context.wait_for_all_requests();
    return true;
}

//  Tiles near the player where a Cooltrainer (purple hair) is standing.
std::set<MapTile> find_trainers_near(const ImageViewRGB32& screen, const MapTile& player){
    std::set<MapTile> ret;
    for (int dy = -3; dy <= 4; dy++){
        for (int dx = -6; dx <= 6; dx++){
            MapTile tile{player.x + dx, player.y + dy};
            if ((dx == 0 && dy == 0) || canyon_cell(tile) == '#'){
                continue;
            }
            TileOffset offset{dx, dy};
            if (!head_box_on_screen(offset)){
                continue;
            }
            if (find_purple_hair(screen, {offset}).found){
                ret.insert(tile);
            }
        }
    }
    return ret;
}

//  Walk to any of the goals, reading the position from the house roof before
//  every step. A lost or extra step is corrected, and trainers seen on screen
//  (or tiles a step was blocked on) are walked around. Where the roof can't be seen, the position is
//  counted from the steps instead.
void canyon_go_to(
    ConsoleHandle& console, ProControllerContext& context,
    Walker& walker, CanyonPosition& pos,
    const std::set<MapTile>& goals, const std::set<MapTile>& blocked, bool grass_cut,
    const std::string& where
){
    MapTile last{-1, -1};
    MapTile tried{-1, -1};          //  The tile the last step went toward.
    //  Tiles a step into failed, and the move when that last happened. They
    //  are avoided for a while (whoever stood there may move on).
    std::map<MapTile, int> in_the_way;
    const int REMEMBER_MOVES = 12;
    std::set<MapTile> trainers_seen;
    bool counting = false;
    int stuck = 0;
    for (int moves = 0; moves < 80; moves++){
        MapTile seen{0, 0};
        trainers_seen.clear();
        bool located = canyon_locate(console, context, seen);
        if (!located && escape_wild_battle(console, context)){
            //  Back in the overworld, on the tile the battle started on.
            walker.forget_facing();
            located = canyon_locate(console, context, seen);
        }
        if (located){
            if (counting || !(seen == pos.tile)){
                console.log("Position: " + tile_str(seen) + (seen == pos.tile ? "" : " (expected " + tile_str(pos.tile) + ")"));
            }
            counting = false;
            pos.tile = seen;
            //  Where the Cooltrainers are standing, from their purple hair.
            trainers_seen = find_trainers_near(console.video().snapshot(), pos.tile);
        }else if (!counting){
            console.log("Can't see the house roof. Counting steps from " + tile_str(pos.tile) + ".", COLOR_ORANGE);
            counting = true;
        }

        if (pos.tile == last){
            //  The step didn't happen. Someone may be standing there (a
            //  trainer wanders again after a battle), or the player was
            //  facing another way and only turned.
            walker.uncount(1);
            walker.forget_facing();     //  The next press includes a turn's time.
            stuck++;
            console.log(
                "The step from " + tile_str(pos.tile) + " to " + tile_str(tried) +
                " didn't happen (" + std::to_string(stuck) + " in a row).",
                COLOR_ORANGE
            );
            if (stuck >= 2){
                //  Go around it if there's another way.
                in_the_way[tried] = moves;
            }
            if (stuck == 8){
                //  In case a message box the program didn't expect is open.
                pbf_mash_button(context, BUTTON_B, 1500ms);
            }
            if (stuck >= 20){
                OperationFailedExceptionWithScreenshot::fire(
                    ErrorReportMode::SEND_ERROR_REPORT,
                    "canyon_go_to(): Stuck at " + tile_str(pos.tile) + " on the way to " + where + ".",
                    console
                );
            }
            pbf_wait(context, 800ms);
        }else{
            stuck = 0;
        }
        if (goals.find(pos.tile) != goals.end()){
            return;
        }

        //  Avoid trainers seen on screen and tiles that recently blocked a step.
        std::set<MapTile> avoid = blocked;
        for (auto it = in_the_way.begin(); it != in_the_way.end();){
            if (moves - it->second > REMEMBER_MOVES){
                it = in_the_way.erase(it);
            }else{
                avoid.insert(it->first);
                ++it;
            }
        }
        for (const MapTile& t : trainers_seen){
            avoid.insert(t);
        }
        std::vector<Dir> path;
        MapTile end{0, 0};
        if (!find_path(pos.tile, goals, avoid, grass_cut, path, end)){
            //  No way around: keep trying the direct way.
            if (!find_path(pos.tile, goals, blocked, grass_cut, path, end)){
                OperationFailedExceptionWithScreenshot::fire(
                    ErrorReportMode::SEND_ERROR_REPORT,
                    "canyon_go_to(): No path from " + tile_str(pos.tile) + " to " + where + ".",
                    console
                );
            }
        }
        //  A blocked step is only noticed when the roof is seen. Without it,
        //  the step is assumed to have happened.
        last = counting ? MapTile{-1, -1} : pos.tile;
        tried = {pos.tile.x + dir_dx(path[0]), pos.tile.y + dir_dy(path[0])};
        walker.walk(path[0], 1, true);
        //  Where the step should land. The next reading logs any difference.
        pos.tile = tried;
    }
    OperationFailedExceptionWithScreenshot::fire(
        ErrorReportMode::SEND_ERROR_REPORT,
        "canyon_go_to(): Could not reach " + where + ".",
        console
    );
}

//  From outside the house door to the VS Seeker tile. The trainers are only
//  watched from that exact tile.
void walk_to_vs_seeker_tile(ConsoleHandle& console, ProControllerContext& context, Walker& walker, CanyonPosition& pos){
    canyon_go_to(console, context, walker, pos, {VS_SEEKER_TILE}, {}, false, "the VS Seeker tile");
}

//  From anywhere near the house to the house door, and inside.
void walk_into_house(
    ConsoleHandle& console, ProControllerContext& context,
    Walker& walker, CanyonPosition& pos, const std::set<MapTile>& blocked, bool grass_cut
){
    canyon_go_to(console, context, walker, pos, {HOUSE_EXIT_TILE}, blocked, grass_cut, "the house door");
    context.wait_for_all_requests();
    go_through_door(console, context, walker, Dir::UP, "the Chansey house");
    walker.set_facing(Dir::UP);
}


//  Cut from the VS Seeker tile toward the north trainer: left, Cut, left, up,
//  Cut, ending at (10, 58). The grass stays cut until the map reloads. Each
//  move is checked against the house roof.
void cut_path_north(ConsoleHandle& console, ProControllerContext& context, Walker& walker, CanyonPosition& pos){
    console.log("Cutting through the grass...");
    const std::vector<MapTile>& centers = CUT_CENTERS();
    for (size_t i = 0; i < centers.size(); i++){
        //  Only grass already cut may be walked on: before the first Cut,
        //  none; before the second, the first one's square.
        canyon_go_to(console, context, walker, pos, {centers[i]}, {}, i > 0, "Cut spot " + tile_str(centers[i]));
        use_cut(console, context);
    }
    context.wait_for_all_requests();
}


//  Find a trainer from where the player stands: look for purple hair on the
//  tiles of the trainer's area over several frames and take the tile seen
//  most. False if they weren't seen (or the position can't be read).
bool locate_trainer(
    ConsoleHandle& console, ProControllerContext& context,
    CanyonPosition& pos, const std::vector<MapTile>& area, MapTile& trainer
){
    MapTile here{0, 0};
    if (!canyon_locate(console, context, here)){
        return false;
    }
    pos.tile = here;
    std::vector<TileOffset> offsets;
    for (const MapTile& t : area){
        offsets.push_back({t.x - here.x, t.y - here.y});
    }
    std::map<MapTile, int> votes;
    for (int frame = 0; frame < 8; frame++){
        SpriteSighting sighting = find_purple_hair(console.video().snapshot(), offsets);
        if (sighting.found){
            votes[{here.x + sighting.tile.dx, here.y + sighting.tile.dy}]++;
        }
        context.wait_for(80ms);
    }
    int best = 0;
    for (const auto& item : votes){
        if (item.second > best){
            best = item.second;
            trainer = item.first;
        }
    }
    return best >= 3;
}

//  Walk next to a ready (standing still) trainer, face them and battle. The
//  trainer is looked for again from closer up, and once more when next to
//  them: a tile read from farther away can be off by one.
BattleOutcome approach_and_battle(
    ConsoleHandle& console, ProControllerContext& context,
    Walker& walker, CanyonPosition& pos,
    const MapTile& seen_from_vs_seeker, const std::vector<MapTile>& area,
    const std::set<MapTile>& blocked,
    bool grass_cut, BattleContext& battle
){
    MapTile trainer = seen_from_vs_seeker;
    for (int attempt = 0; attempt < 3; attempt++){
        MapTile seen = trainer;
        if (locate_trainer(console, context, pos, area, seen) && !(seen == trainer)){
            console.log("The trainer is at " + tile_str(seen) + " (seen at " + tile_str(trainer) + " before).");
            trainer = seen;
        }

        std::set<MapTile> goals;
        for (Dir dir : {Dir::UP, Dir::DOWN, Dir::LEFT, Dir::RIGHT}){
            goals.insert({trainer.x + dir_dx(dir), trainer.y + dir_dy(dir)});
        }
        //  Only the trainer's current tile is avoided, not where they were
        //  seen before (that may be the only way in).
        std::set<MapTile> avoid = blocked;
        avoid.erase(seen_from_vs_seeker);
        avoid.insert(trainer);

        std::vector<Dir> check_path;
        MapTile check_end{0, 0};
        if (!find_path(pos.tile, goals, avoid, grass_cut, check_path, check_end)){
            console.log("No way to the trainer at " + tile_str(trainer) + " from " + tile_str(pos.tile) + ". Skipping them.", COLOR_RED);
            return BattleOutcome::NO_BATTLE;
        }
        console.log("Walking to the trainer at " + tile_str(trainer) + "...");
        canyon_go_to(console, context, walker, pos, goals, avoid, grass_cut, "the trainer at " + tile_str(trainer));

        //  Next to them now: check they are where we think.
        seen = trainer;
        if (!locate_trainer(console, context, pos, area, seen) || seen == trainer){
            break;
        }
        console.log("Next to " + tile_str(trainer) + ", but the trainer looks to be at " + tile_str(seen) + ". Adjusting.", COLOR_ORANGE);
        trainer = seen;
        if (std::abs(pos.tile.x - trainer.x) + std::abs(pos.tile.y - trainer.y) == 1){
            break;      //  Already next to them anyway.
        }
    }
    Dir facing = direction_to(pos.tile, trainer);
    walker.face(facing);
    context.wait_for_all_requests();
    return run_trainer_battle(console, context, walker, facing, battle);
}


////////////////////////////////////////////////////////////////////////////////
//  One Island: Kindle Road and the Ember Spa.
//    '#' blocked, '.' floor, 'g' tall grass, '~' water, with each tile's
//    elevation (hex digit). The player can't step between two different
//    elevations, except onto or off a tile of elevation 0 or 15, which keeps
//    the elevation the player had. Generated from the decomp's map blocks
//    and metatile behaviors. Kindle Road rows 50 to 134.
////////////////////////////////////////////////////////////////////////////////

const char* const KINDLE_CELLS[] = {
    "~##~~~~~~~~~~~~#########",     //  y = 50
    "~##~##~~~~~~~..#########",     //  y = 51
    "~~####~~~~....##########",     //  y = 52
    "~~##~~~~~....###########",     //  y = 53
    "~~~##~~~~....###########",     //  y = 54
    "~~~##~~~~....###########",     //  y = 55
    "##~~##~~~....###########",     //  y = 56
    "##~~##~~~...############",     //  y = 57
    "~~##~##~~....##.########",     //  y = 58
    "~~##~##~.....#..########",     //  y = 59
    "~~~~~##~........########",     //  y = 60
    "~~~~~##~.##......#######",     //  y = 61
    "#######~.##....#########",     //  y = 62
    "###########....#########",     //  y = 63
    "###########......#######",     //  y = 64
    "############.....#######",     //  y = 65
    "###########......#######",     //  y = 66
    "##########....##.#######",     //  y = 67
    "########.....###.#######",     //  y = 68
    "########....####.#######",     //  y = 69
    "########....##..#.######",     //  y = 70
    "########..####...####~~~",     //  y = 71
    "########..####...####~~~",     //  y = 72
    "########..#####.#####~~~",     //  y = 73
    "########..#####.#######~",     //  y = 74
    "########....###.#######~",     //  y = 75
    "########...........#.~##",     //  y = 76
    "########.............~##",     //  y = 77
    "#######.............~~##",     //  y = 78
    "#######......###....~~##",     //  y = 79
    "#######......##.....~~##",     //  y = 80
    "#######...#......#.~~~##",     //  y = 81
    "#######g..#......#.~~~##",     //  y = 82
    "#######gg.#......#.~~~##",     //  y = 83
    "#######gg.#......#.~~##~",     //  y = 84
    "#######ggg##..#..#.~~##~",     //  y = 85
    "#######ggg....#..#.~~##~",     //  y = 86
    "#######ggg....#..#.~~##~",     //  y = 87
    "########g.....#..#.~~##~",     //  y = 88
    "########......#..#.~~##~",     //  y = 89
    "########......#..#.~~##~",     //  y = 90
    "########.g....#..#.~~##~",     //  y = 91
    "########ggg...#..#.~~~##",     //  y = 92
    "########gggg.#...#.~~~##",     //  y = 93
    "#######gggggg#...#.~~~##",     //  y = 94
    "#######ggg..g#..##.~~~##",     //  y = 95
    "#######ggggg.#..#..~~##~",     //  y = 96
    "#######gggg..#..#~~~~##~",     //  y = 97
    "########gg...#..#~~~##~~",     //  y = 98
    "########.....#..#~~~##~~",     //  y = 99
    "########.....#..#~~~##~~",     //  y = 100
    "########..gg.#..#~~~##~~",     //  y = 101
    "########.ggg#...#..~~##~",     //  y = 102
    "########gggg#...#..~~##~",     //  y = 103
    "#######g.ggg#......#~~##",     //  y = 104
    "#######...gg#......#~~##",     //  y = 105
    "#######..ggg#......#~~##",     //  y = 106
    "#######...gg##..#..#~~##",     //  y = 107
    "########..ggg#..#..#~~##",     //  y = 108
    "########..ggg#..#..#~~##",     //  y = 109
    "########g.ggg####..#~~##",     //  y = 110
    "########ggggg...#..#~~##",     //  y = 111
    "########gggg....##.#~~##",     //  y = 112
    "########gggg.....#.#~~##",     //  y = 113
    "########g.g......#.#~~##",     //  y = 114
    "#########........###~~##",     //  y = 115
    "########..........~~~~##",     //  y = 116
    "########..........~~~~##",     //  y = 117
    "#######...........~~~~##",     //  y = 118
    "~######...........~~~~##",     //  y = 119
    "~######..........~~~~##~",     //  y = 120
    "~#######.........~~~~##~",     //  y = 121
    "~########.......~~~~~~##",     //  y = 122
    "~########..~~~~~~~~~~~##",     //  y = 123
    "~~~######~~~~~~~~~~~~~##",     //  y = 124
    "##~~##~~~~~~~~~~~~##~~##",     //  y = 125
    "######~~~~~~~~~~~~##~##~",     //  y = 126
    ".~##~~~~~~~~~~~~~~~~~##~",     //  y = 127
    ".~~~~~~~~~~~~~~~~~~~##~~",     //  y = 128
    ".~~~~~~~~~~~~~~~~~~~##~~",     //  y = 129
    "...~~~~~~~~~##~~~~~~~~##",     //  y = 130
    "....~~~~~~~~##~~....~~##",     //  y = 131
    "....~~~~~~~~~~~.....~~##",     //  y = 132
    "....~~~~~~~~~~~.....~~##",     //  y = 133
    "###.~~~##~~##~~~...~~##~",     //  y = 134
};
const char* const KINDLE_ELEVATION[] = {
    "000111111111111000000000",     //  y = 50
    "000100111111133000000000",     //  y = 51
    "010000111133330000000000",     //  y = 52
    "010011111333300000000000",     //  y = 53
    "001001111333300000000000",     //  y = 54
    "001001111333300000000000",     //  y = 55
    "001100111333300000000000",     //  y = 56
    "001100111333000000000000",     //  y = 57
    "000010011333300300000000",     //  y = 58
    "000010013333303300000000",     //  y = 59
    "001110013333333300000000",     //  y = 60
    "011110013003333330000000",     //  y = 61
    "000000013003333000000000",     //  y = 62
    "000000000003333000000000",     //  y = 63
    "000000000003333330000000",     //  y = 64
    "000000000000333330000000",     //  y = 65
    "000000000003333330000000",     //  y = 66
    "000000000033330030000000",     //  y = 67
    "000000003333300030000000",     //  y = 68
    "000000003333000030000000",     //  y = 69
    "000000003333003303000000",     //  y = 70
    "000000003300003330000111",     //  y = 71
    "000000003300003330000111",     //  y = 72
    "000000003300000300000111",     //  y = 73
    "000000003300000300000001",     //  y = 74
    "000000003333000300000001",     //  y = 75
    "000000003333333333303100",     //  y = 76
    "000000003333333333333100",     //  y = 77
    "000000033333333333331100",     //  y = 78
    "000000033333300033331100",     //  y = 79
    "000000033333300333331100",     //  y = 80
    "000000033304444440311100",     //  y = 81
    "000000033304444440311100",     //  y = 82
    "000000033304444440311100",     //  y = 83
    "000000033304444440311001",     //  y = 84
    "000000033300000440311000",     //  y = 85
    "000000033333330440311000",     //  y = 86
    "000000033333330440311000",     //  y = 87
    "000000003333330440311000",     //  y = 88
    "000000003333330440311000",     //  y = 89
    "000000003333330440311000",     //  y = 90
    "000000003333330440311001",     //  y = 91
    "000000003333330440311100",     //  y = 92
    "000000003333304440311100",     //  y = 93
    "000000033333304440311100",     //  y = 94
    "000000033333304400311100",     //  y = 95
    "000000033333304403311001",     //  y = 96
    "000000033333304401111001",     //  y = 97
    "000000003333304401110010",     //  y = 98
    "000000003333304401110010",     //  y = 99
    "000000003333304401110010",     //  y = 100
    "000000003333304401110010",     //  y = 101
    "000000003333044403311000",     //  y = 102
    "000000003333044403311001",     //  y = 103
    "000000033333044444401100",     //  y = 104
    "000000033333044444401100",     //  y = 105
    "000000033333044444401100",     //  y = 106
    "000000033333000004401100",     //  y = 107
    "000000003333303304401100",     //  y = 108
    "000000003333303304401100",     //  y = 109
    "000000003333300004401100",     //  y = 110
    "000000003333333304401100",     //  y = 111
    "000000003333333300401100",     //  y = 112
    "000000003333333330401100",     //  y = 113
    "000000003333333330401100",     //  y = 114
    "000000000333333330001100",     //  y = 115
    "000000003333333333111100",     //  y = 116
    "000000003333333333111100",     //  y = 117
    "000000033333333333111100",     //  y = 118
    "100000033333333333111100",     //  y = 119
    "100000033333333331111001",     //  y = 120
    "100000003333333331111000",     //  y = 121
    "100000000333333311111100",     //  y = 122
    "100000000331111111111100",     //  y = 123
    "111000000111111111111100",     //  y = 124
    "001100111111111111001100",     //  y = 125
    "000000111111111111001000",     //  y = 126
    "310011111111111111111000",     //  y = 127
    "311111111111111111110010",     //  y = 128
    "311111111111111111110010",     //  y = 129
    "333111111111001111111100",     //  y = 130
    "333311111111001133331100",     //  y = 131
    "333311111111111333331100",     //  y = 132
    "333311111111111333331100",     //  y = 133
    "000311100110011133311000",     //  y = 134
};
struct AreaMap{
    const char* name;
    int width;
    int first_row;
    int rows;
    const char* const* cells;
    const char* const* elevation;

    bool inside(const MapTile& t) const{
        return t.x >= 0 && t.x < width && t.y >= first_row && t.y < first_row + rows;
    }
    char cell(const MapTile& t) const{
        return inside(t) ? cells[t.y - first_row][t.x] : '#';
    }
    int elevation_of(const MapTile& t) const{
        if (!inside(t)){
            return 0;
        }
        char ch = elevation[t.y - first_row][t.x];
        return ch <= '9' ? ch - '0' : ch - 'a' + 10;
    }
};
static_assert(sizeof(KINDLE_CELLS) / sizeof(KINDLE_CELLS[0]) == 85, "Kindle Road rows");
static_assert(sizeof(KINDLE_ELEVATION) / sizeof(KINDLE_ELEVATION[0]) == 85, "Kindle Road rows");
const AreaMap KINDLE_ROAD{"Kindle Road", 24, 50, 85, KINDLE_CELLS, KINDLE_ELEVATION};


//  Surfing north from the beach, (4, 127) is the first tile a rock stops.
//  From there, east and north on the water, then west onto land: walking
//  west stops at the wall left of (9, 123), and south of it is water.
const MapTile KINDLE_LANDING{9, 123};
//  The spa's entrance (a cave door): leaving the spa puts the player on it.
const MapTile SPA_DOOR{15, 58};
const MapTile SPA_DOOR_FRONT{15, 59};
//  Where the VS Seeker is used: all four trainers are within its range (7
//  tiles across, 5 up or down) whenever Sharon is at the south end of her walk.
const MapTile KINDLE_VS_TILE{11, 63};

//  Inside the spa (map OneIsland_KindleRoad_EmberSpa): arriving puts the
//  player on the exit (13, 36). Stepping into the hot spring at (13, 13)
//  heals the party, once per visit. Row 19 is clear from wall to wall.
constexpr int SPA_PACE_ROW = 19;
constexpr int SPA_PACE_LEFT = 5;
constexpr int SPA_PACE_RIGHT = 21;

//  People who don't move, Rock Smash rocks and item balls (from the decomp's
//  object events).
const std::set<MapTile>& KINDLE_OBSTACLES(){
    static const std::set<MapTile> tiles{
        {9, 108}, {11, 89}, {8, 68}, {9, 68}, {19, 79}, {17, 132},
        {8, 104}, {11, 95}, {12, 96}, {10, 95}, {9, 105}, {15, 75}, {9, 84},
        {15, 74}, {15, 73}, {18, 111}, {10, 86}, {9, 110}, {18, 112},
        {15, 70}, {17, 103}, {18, 114},
    };
    return tiles;
}
//  Their rematch teams (and first teams).
const std::set<std::string>& KINDLE_SPECIES(){
    static const std::set<std::string> species{
        "mankey", "primeape", "machop", "machoke", "machamp",
    };
    return species;
}

enum KindleTrainer : size_t{
    HUGH = 0,
    SHEA = 1,
    MIK_AND_KIA = 2,
    SHARON = 3,
};
constexpr size_t KINDLE_TRAINERS = 4;
const char* kindle_trainer_name(size_t who){
    switch (who){
    case HUGH:          return "Black Belt Hugh";
    case SHEA:          return "Black Belt Shea";
    case MIK_AND_KIA:   return "Crush Kin Mik & Kia";
    case SHARON:        return "Crush Girl Sharon";
    }
    return "?";
}
//  The tiles each one can stand on.
const std::vector<MapTile>& kindle_trainer_tiles(size_t who){
    static const std::vector<MapTile> TILES[KINDLE_TRAINERS] = {
        {{11, 64}, {12, 64}, {13, 64}},     //  Hugh walks left and right from (11, 64).
        {{14, 64}, {15, 64}, {16, 64}},     //  Shea, from (16, 64).
        {{8, 68}, {9, 68}},                 //  Mik & Kia stand side by side.
        {{9, 53}, {9, 54}, {9, 55}, {9, 56}, {9, 57}, {9, 58}, {9, 59}, {9, 60}},   //  Sharon walks up and down.
    };
    return TILES[who < KINDLE_TRAINERS ? who : 0];
}
//  Where Hugh and Shea jog. Walked across only when there is no other way.
const std::set<MapTile>& JOGGER_ROW(){
    static const std::set<MapTile> tiles{{11, 64}, {12, 64}, {13, 64}, {14, 64}, {15, 64}, {16, 64}};
    return tiles;
}
int tile_distance(const MapTile& a, const MapTile& b){
    return std::abs(a.x - b.x) + std::abs(a.y - b.y);
}
//  Which trainer stands on (or next to) a tile, or -1.
int kindle_trainer_at(const MapTile& tile){
    int best = -1;
    int best_distance = 2;
    for (size_t who = 0; who < KINDLE_TRAINERS; who++){
        for (const MapTile& t : kindle_trainer_tiles(who)){
            int d = tile_distance(t, tile);
            if (d < best_distance){
                best_distance = d;
                best = (int)who;
            }
        }
    }
    return best;
}


std::vector<Segment> to_segments(const std::vector<Dir>& path){
    std::vector<Segment> segments;
    for (Dir dir : path){
        if (!segments.empty() && segments.back().dir == dir){
            segments.back().steps++;
        }else{
            segments.push_back({dir, 1});
        }
    }
    return segments;
}

//  Shortest path from `start` to any tile in `goals` over floor and grass,
//  following the elevation rule above, avoiding `blocked`. Among paths of the
//  same length, the one with the fewest turns.
bool area_find_path(
    const AreaMap& map, const MapTile& start,
    const std::set<MapTile>& goals, const std::set<MapTile>& blocked,
    std::vector<Dir>& path
){
    struct State{
        MapTile tile;
        int elevation;
        int dir;        //  The last step's direction, 4 at the start.
        bool operator<(const State& s) const{
            if (!(tile == s.tile)){
                return tile < s.tile;
            }
            if (elevation != s.elevation){
                return elevation < s.elevation;
            }
            return dir < s.dir;
        }
    };
    int start_elevation = map.elevation_of(start);
    if (start_elevation == 0 || start_elevation == 15){
        start_elevation = 3;
    }
    const Dir DIRS[] = {Dir::UP, Dir::DOWN, Dir::LEFT, Dir::RIGHT};

    //  Cost: 16 per step, 1 per turn.
    State first{start, start_elevation, 4};
    std::map<State, int> cost{{first, 0}};
    std::map<State, State> previous;
    std::set<std::pair<int, State>> queue{{0, first}};
    while (!queue.empty()){
        auto [c, state] = *queue.begin();
        queue.erase(queue.begin());
        if (c > cost[state]){
            continue;
        }
        if (goals.find(state.tile) != goals.end()){
            path.clear();
            while (state < first || first < state){
                path.emplace_back(DIRS[state.dir]);
                state = previous.at(state);
            }
            std::reverse(path.begin(), path.end());
            return true;
        }
        for (int d = 0; d < 4; d++){
            Dir dir = DIRS[d];
            MapTile next{state.tile.x + dir_dx(dir), state.tile.y + dir_dy(dir)};
            if (blocked.find(next) != blocked.end()){
                continue;
            }
            char cell = map.cell(next);
            if (cell == '#' || cell == '~'){
                continue;
            }
            int elevation = map.elevation_of(next);
            bool any = elevation == 0 || elevation == 15;
            if (!any && elevation != state.elevation){
                continue;
            }
            State to{next, any ? state.elevation : elevation, d};
            int next_cost = c + 16 + (state.dir != 4 && state.dir != d ? 1 : 0);
            auto it = cost.find(to);
            if (it != cost.end() && it->second <= next_cost){
                continue;
            }
            cost[to] = next_cost;
            previous.insert_or_assign(to, state);
            queue.insert({next_cost, to});
        }
    }
    return false;
}


//  The player's tile in an area, read by comparing the screen with screens
//  remembered there.
struct Area{
    explicit Area(const AreaMap& m)
        : map(m)
    {}
    const AreaMap& map;
    ScreenOdometry odometry;
    MapTile tile{0, 0};
};

//  Standing still on a tile known for certain (just through a door, or
//  stopped by a known wall): remember the screen there.
void area_anchor(ConsoleHandle& console, ProControllerContext& context, Area& area, const MapTile& tile){
    pbf_wait(context, 600ms);
    context.wait_for_all_requests();
    area.tile = tile;
    area.odometry.remember(console.video().snapshot(), tile.x, tile.y);
    console.log("Position: " + tile_str(tile) + " in " + area.map.name + ".");
}

//  Read the player's tile, within `radius` tiles of the last known one. Along
//  the way, screens are remembered every few tiles, so the next screens
//  always overlap a remembered one.
bool area_locate(ConsoleHandle& console, ProControllerContext& context, Area& area, int radius, MapTile& tile){
    std::string details;
    for (int tries = 0; tries < 4; tries++){
        context.wait_for_all_requests();
        VideoSnapshot screen = console.video().snapshot();
        int x = 0;
        int y = 0;
        if (area.odometry.locate(screen, area.tile.x, area.tile.y, radius, x, y, details)){
            MapTile seen{x, y};
            if (area.map.cell(seen) != '#'){
                tile = seen;
                if (!area.odometry.has_reference_near(x, y, 2)){
                    area.odometry.remember(screen, x, y);
                }
                return true;
            }
            details += ", a wall";
        }
        pbf_wait(context, 150ms);
    }
    console.log("Could not read the position in " + std::string(area.map.name) + " (" + details + ").", COLOR_ORANGE);
    return false;
}


//  Called when a trainer battle starts on the way somewhere (a trainer who
//  wants a rematch battles whoever walks into their view), at the battle
//  menu. Given the player's tile.
using TrainerBattleHandler = std::function<void(const MapTile& where)>;

//  If a battle started while walking, or someone is talking: run from a
//  wild Pokemon (tall grass once the Repel is used up), fight a trainer, or
//  close the Repel's message. False if none of these.
bool handle_battle_on_the_way(
    ConsoleHandle& console, ProControllerContext& context,
    const MapTile& where, const TrainerBattleHandler& on_trainer_battle
){
    BattleDialogWatcher battle_text(COLOR_RED);
    BattleMenuWatcher battle_menu(COLOR_RED);
    WhiteDialogWatcher talking(COLOR_RED);
    context.wait_for_all_requests();
    //  A short look: this runs whenever the position can't be read. While a
    //  battle is still fading in, the next look catches it.
    int ret = wait_until(console, context, 500ms, {battle_text, battle_menu, talking});
    if (ret < 0){
        return false;
    }

    bool trainer = false;
    bool wild = false;
    if (ret != 1){
        //  A text box: a wild Pokemon or a trainer's battle starting, a
        //  trainer who saw the player, the Repel's message, or a sign walked
        //  into (signs show their text when walked into). Read it, close it,
        //  and see whether a battle follows.
        VideoSnapshot screen = console.video().snapshot();
        std::string text =
            read_dialog_line(console.logger(), screen, DialogStyle::OVERWORLD, 0) + " " +
            read_dialog_line(console.logger(), screen, DialogStyle::OVERWORLD, 1) + " " +
            read_dialog_line(console.logger(), screen, DialogStyle::BATTLE, 0) + " " +
            read_dialog_line(console.logger(), screen, DialogStyle::BATTLE, 1);
        if (text_contains_any(text, {"wild"})){
            wild = true;
        }
        bool repel = text_contains_any(text, {"repel", "wore off"});
        pbf_mash_button(context, BUTTON_B, 1500ms);
        BattleMenuWatcher starting_menu(COLOR_RED);
        AdvanceBattleDialogWatcher starting_text(COLOR_RED);
        context.wait_for_all_requests();
        if (wait_until(console, context, 6000ms, {starting_menu, starting_text}) < 0){
            console.log(repel
                ? "The Repel wore off."
                : "A message (a sign?) came up and was closed. No battle."
            );
            pbf_wait(context, 300ms);
            context.wait_for_all_requests();
            return true;
        }
        if (!wild){
            console.log("A battle is starting.", COLOR_ORANGE);
        }
    }

    //  Through the battle's first lines to the battle menu: "Wild X
    //  appeared!", or "X would like to battle!" and "X sent out Y!".
    WallClock deadline = current_time() + 30000ms;
    while (true){
        if (current_time() > deadline){
            OperationFailedExceptionWithScreenshot::fire(
                ErrorReportMode::SEND_ERROR_REPORT,
                "handle_battle_on_the_way(): A battle seemed to start, but its menu never came up.",
                console
            );
        }
        BattleMenuWatcher menu(COLOR_RED);
        AdvanceBattleDialogWatcher advance(COLOR_RED);
        WhiteDialogWatcher overworld_text(COLOR_RED);
        ret = wait_until(console, context, 5000ms, {menu, advance, overworld_text});
        if (ret == 0){
            break;
        }
        if (ret == 1){
            VideoSnapshot screen = console.video().snapshot();
            std::string text =
                read_dialog_line(console.logger(), screen, DialogStyle::BATTLE, 0) + " " +
                read_dialog_line(console.logger(), screen, DialogStyle::BATTLE, 1);
            if (text_contains_any(text, {"wild"})){
                wild = true;
            }
            if (text_contains_any(text, {"would like", "to battle", "sent out"})){
                trainer = true;
            }
        }else if (ret == 2){
            trainer = true;
        }
        pbf_press_button(context, BUTTON_B, 200ms, 400ms);
    }

    if (wild && !trainer){
        console.log("A wild Pokemon appeared. Running from it.", COLOR_ORANGE);
        flee_battle(console, context);
        context.wait_for_all_requests();
        pbf_wait(context, 500ms);
        context.wait_for_all_requests();
        return true;
    }
    //  A trainer, or not sure (fighting a wild Pokemon works too).
    if (!on_trainer_battle){
        OperationFailedExceptionWithScreenshot::fire(
            ErrorReportMode::SEND_ERROR_REPORT,
            "handle_battle_on_the_way(): A trainer battle started where none was expected.",
            console
        );
    }
    on_trainer_battle(where);
    return true;
}


//  Walk to any of the goals in an area. Each straight stretch of the path
//  is walked in one go (as far as a remembered screen will still show where
//  it ends), then the position is read and the path planned again. Tiles a
//  step was blocked on are walked around for a while (someone walking
//  there). Battles on the way are handled.
void area_go_to(
    ConsoleHandle& console, ProControllerContext& context,
    Walker& walker, Area& area,
    const std::set<MapTile>& goals, const std::set<MapTile>& blocked,
    const std::string& where, const TrainerBattleHandler& on_trainer_battle,
    const std::set<MapTile>& keep_off = {}
){
    const MapTile NOWHERE{-1000, -1000};
    MapTile start = NOWHERE;        //  Where the last stretch started.
    MapTile tried = NOWHERE;        //  Its first step.
    uint16_t walked = 0;            //  Its length (most it could be).
    bool held = false;              //  It was a held press of unknown length.
    std::map<MapTile, int> in_the_way;
    const int REMEMBER_MOVES = 12;
    bool counting = false;
    int stuck = 0;
    int unread = 0;
    for (int moves = 0; moves < 250; moves++){
        MapTile seen{0, 0};
        bool located = area_locate(console, context, area, (int)walked + 2, seen);
        if (!located && handle_battle_on_the_way(console, context, area.tile, on_trainer_battle)){
            //  Back in the overworld, where the battle started.
            walker.forget_facing();
            start = NOWHERE;
            stuck = 0;
            located = area_locate(console, context, area, (int)walked + 2, seen);
        }
        if (located){
            if (counting || !(seen == area.tile)){
                console.log("Position: " + tile_str(seen) + (seen == area.tile ? "" : " (expected " + tile_str(area.tile) + ")"));
            }
            if (held && !(start == NOWHERE)){
                walker.add_steps((uint32_t)tile_distance(seen, start));
            }else if (!(start == NOWHERE) && !(seen == area.tile)){
                //  Steps that didn't happen don't charge the VS Seeker.
                int short_by = tile_distance(seen, area.tile);
                walker.uncount((uint32_t)std::min<int>(short_by, walked));
            }
            counting = false;
            unread = 0;
            area.tile = seen;
        }else{
            if (!counting){
                console.log("Counting steps from " + tile_str(area.tile) + ".", COLOR_ORANGE);
            }
            counting = true;
            if (++unread >= 12){
                OperationFailedExceptionWithScreenshot::fire(
                    ErrorReportMode::SEND_ERROR_REPORT,
                    "area_go_to(): Lost the position in " + std::string(area.map.name) + " near " + tile_str(area.tile) + ".",
                    console
                );
            }
        }

        if (located && area.tile == start){
            walker.forget_facing();
            stuck++;
            console.log(
                "The step from " + tile_str(area.tile) + " to " + tile_str(tried) +
                " didn't happen (" + std::to_string(stuck) + " in a row).",
                COLOR_ORANGE
            );
            //  A trainer who saw the player stops them and talks.
            if (WhiteDialogDetector(COLOR_RED).detect(console.video().snapshot()) &&
                handle_battle_on_the_way(console, context, area.tile, on_trainer_battle)
            ){
                walker.forget_facing();
                start = NOWHERE;
                stuck = 0;
                continue;
            }
            if (stuck >= 2){
                in_the_way[tried] = moves;
            }
            if (stuck == 8){
                pbf_mash_button(context, BUTTON_B, 1500ms);
            }
            if (stuck >= 20){
                OperationFailedExceptionWithScreenshot::fire(
                    ErrorReportMode::SEND_ERROR_REPORT,
                    "area_go_to(): Stuck at " + tile_str(area.tile) + " on the way to " + where + ".",
                    console
                );
            }
            pbf_wait(context, 800ms);
        }else if (located){
            stuck = 0;
        }
        if (goals.find(area.tile) != goals.end()){
            return;
        }

        std::set<MapTile> avoid = blocked;
        for (auto it = in_the_way.begin(); it != in_the_way.end();){
            if (moves - it->second > REMEMBER_MOVES){
                it = in_the_way.erase(it);
            }else{
                avoid.insert(it->first);
                ++it;
            }
        }
        //  Tiles to keep off if there is another way (the Black Belts' row).
        std::set<MapTile> avoid_more = avoid;
        for (const MapTile& t : keep_off){
            if (goals.find(t) == goals.end() && !(t == area.tile)){
                avoid_more.insert(t);
            }
        }
        std::vector<Dir> path;
        if (!area_find_path(area.map, area.tile, goals, avoid_more, path) &&
            !area_find_path(area.map, area.tile, goals, avoid, path) &&
            !area_find_path(area.map, area.tile, goals, blocked, path)
        ){
            OperationFailedExceptionWithScreenshot::fire(
                ErrorReportMode::SEND_ERROR_REPORT,
                "area_go_to(): No path from " + tile_str(area.tile) + " to " + where + ".",
                console
            );
        }

        //  Run as much of the path as can be read at its end (a remembered
        //  screen near where it ends), through its turns without stopping.
        //  When counting steps, one at a time.
        Dir dir = path[0];
        size_t steps = 1;
        if (!counting){
            MapTile end = area.tile;
            for (size_t n = 1; n <= path.size() && n <= 24; n++){
                end = {end.x + dir_dx(path[n - 1]), end.y + dir_dy(path[n - 1])};
                if (area.odometry.has_reference_within(end.x, end.y, 6, 5)){
                    steps = n;
                }
            }
        }
        start = counting ? NOWHERE : area.tile;
        tried = {area.tile.x + dir_dx(dir), area.tile.y + dir_dy(dir)};
        held = stuck > 0 && !counting;
        if (held){
            //  The last try was blocked (someone walking across, like the
            //  Black Belts jogging on row 64): hold the direction, so the
            //  player goes the moment the tile is free. Walking, and briefly,
            //  so it goes at most two tiles too far; the position is read
            //  after.
            walked = 4;
            walker.hold(dir, false, 700ms);
            pbf_wait(context, 200ms);
            walker.set_facing(dir);
            area.tile = tried;
            continue;
        }
        walked = (uint16_t)std::min<size_t>(steps, 8);
        std::vector<Dir> part(path.begin(), path.begin() + (ptrdiff_t)steps);
        walker.run_path(to_segments(part));
        for (Dir d : part){
            area.tile = {area.tile.x + dir_dx(d), area.tile.y + dir_dy(d)};
        }
    }
    OperationFailedExceptionWithScreenshot::fire(
        ErrorReportMode::SEND_ERROR_REPORT,
        "area_go_to(): Could not reach " + where + ".",
        console
    );
}


//  With the Fly map open on the Sevii Islands 1, 2, 3 page and the cursor
//  near One Island (1, 8), read the name under the cursor and step onto it.
//  Nearby (region_map_layout_sevii_123.h): Kindle Road in column 2,
//  Treasure Beach at (1, 9) and (1, 10), nothing named at (1, 7) or (0, 8).
bool fly_cursor_to_one_island(ConsoleHandle& console, ProControllerContext& context){
    bool tapped_from_corner = false;
    for (int moves = 0; moves < 10; moves++){
        pbf_wait(context, 300ms);
        context.wait_for_all_requests();
        std::string name = read_fly_map_name(console.logger(), console.video().snapshot());
        if (text_contains_any(name, {"one isl", "one is", "ne island"})){
            return true;
        }
        if (name.empty() && tapped_from_corner){
            console.log("Can't read the place name. Trusting the cursor taps.", COLOR_ORANGE);
            return true;
        }
        if (text_contains_any(name, {"kindle", "road"})){
            fly_cursor_tap(context, DPAD_LEFT);
        }else if (text_contains_any(name, {"treasure", "beach"})){
            fly_cursor_tap(context, DPAD_UP);
        }else{
            console.log("The cursor isn't on a known place near One Island. Starting over from the corner.", COLOR_ORANGE);
            pbf_move_left_joystick(context, {-1, +1}, 2500ms, 300ms);
            fly_cursor_tap(context, DPAD_RIGHT);
            for (int i = 0; i < 8; i++){
                fly_cursor_tap(context, DPAD_DOWN);
            }
            tapped_from_corner = true;
        }
    }
    return false;
}

void fly_to_one_island(ConsoleHandle& console, ProControllerContext& context, Walker& walker){
    console.log("Flying to One Island...");
    for (int attempt = 0; attempt < 3; attempt++){
        select_party_slot_from_last(console, context, 0);
        std::vector<std::string> actions = open_action_list(console, context);

        FlyMapWatcher fly_map(COLOR_RED);
        choose_action(console, context, actions, "fly");
        int ret = wait_until(console, context, 10000ms, { fly_map });
        if (ret < 0){
            console.log("The Fly map did not open. Backing out and trying again.", COLOR_RED);
            pbf_mash_button(context, BUTTON_B, 4000ms);
            continue;
        }
        pbf_wait(context, 1000ms);

        //  The Fly map only shows the region the player is in (it has no
        //  SWITCH button), so the player must be somewhere on the Sevii
        //  Islands 1, 2, 3 page. To the top-left corner (0, 0), then to One
        //  Island at (1, 8).
        pbf_move_left_joystick(context, {-1, +1}, 2500ms, 300ms);
        pbf_move_left_joystick(context, {+1, -1}, cursor_hold_ms(1), 300ms);
        pbf_move_left_joystick(context, {0, -1}, cursor_hold_ms(7), 300ms);

        if (!fly_cursor_to_one_island(console, context)){
            console.log(
                "Could not get the cursor onto One Island. Is the player on One, Two or Three Island? "
                "Backing out and trying again.",
                COLOR_RED
            );
            pbf_mash_button(context, BUTTON_B, 4000ms);
            continue;
        }

        BlackScreenWatcher flying(COLOR_RED);
        context.wait_for_all_requests();
        ret = run_until<ProControllerContext>(
            console, context,
            [](ProControllerContext& context){
                pbf_press_button(context, BUTTON_A, 200ms, 2800ms);
                pbf_press_button(context, BUTTON_A, 200ms, 2800ms);
            },
            { flying }
        );
        if (ret < 0){
            console.log("Could not start flying. Backing out and trying again.", COLOR_RED);
            pbf_mash_button(context, BUTTON_B, 4000ms);
            continue;
        }
        pbf_wait(context, 8000ms);
        context.wait_for_all_requests();
        walker.forget_facing();
        console.log("Landed on One Island.");
        return;
    }
    OperationFailedExceptionWithScreenshot::fire(
        ErrorReportMode::SEND_ERROR_REPORT,
        "fly_to_one_island(): Failed to fly 3 times.",
        console
    );
}


//  Landed in front of the Pokemon Center (14, 6): go in, heal, come out.
//  Inside, the nurse isn't in line with the door (9, 9): she is talked to
//  across the counter from (5, 4), going below the girl at (7, 4). Both
//  ways stay clear of the boy wandering at x = 5 to 7, y = 7 to 9. No running
//  indoors here.
void one_island_pokecenter_heal(ConsoleHandle& console, ProControllerContext& context, Walker& walker){
    enter_pokecenter(console, context);
    walker.set_facing(Dir::UP);
    //  Up 4 and left 4 to (5, 5); holding up to talk takes the last step.
    walker.walk_path({{Dir::UP, 4}, {Dir::LEFT, 4}}, false);
    heal_at_pokecenter(console, context);
    walker.set_facing(Dir::UP);
    walker.walk_path({{Dir::DOWN, 1}, {Dir::RIGHT, 4}, {Dir::DOWN, 4}}, false);
    leave_pokecenter(console, context);
    walker.set_facing(Dir::DOWN);
}


//  On the beach facing the water: Surf (any party Pokemon that knows it;
//  the game asks when the player faces the water and presses A).
void start_surfing(ConsoleHandle& console, ProControllerContext& context){
    console.log("Starting to Surf...");
    for (int attempt = 0; attempt < 2; attempt++){
        WhiteDialogWatcher question(COLOR_RED);
        context.wait_for_all_requests();
        pbf_press_button(context, BUTTON_A, 200ms, 0ms);
        if (wait_until(console, context, 3000ms, { question }) < 0){
            console.log("No Surf question. Trying again.", COLOR_ORANGE);
            continue;
        }
        //  "The water is dyed a deep blue... Would you like to SURF?", then
        //  YES / NO with YES selected.
        pbf_wait(context, 2000ms);
        context.wait_for_all_requests();
        VideoSnapshot screen = console.video().snapshot();
        std::string text =
            read_dialog_line(console.logger(), screen, DialogStyle::OVERWORLD, 0) + " " +
            read_dialog_line(console.logger(), screen, DialogStyle::OVERWORLD, 1);
        if (!text_contains_any(text, {"surf", "water", "blue"})){
            console.log("This doesn't look like the Surf question. Answering YES anyway.", COLOR_ORANGE);
        }
        pbf_press_button(context, BUTTON_A, 200ms, 1500ms);
        //  "X used SURF!", the Pokemon's picture, then the player hops on.
        pbf_press_button(context, BUTTON_B, 200ms, 3500ms);
        context.wait_for_all_requests();
        return;
    }
    OperationFailedExceptionWithScreenshot::fire(
        ErrorReportMode::SEND_ERROR_REPORT,
        "start_surfing(): Couldn't start Surfing. Does a party Pokemon know Surf?",
        console
    );
}


//  From the Pokemon Center door to in front of the Ember Spa: through town
//  to the beach east of it, Max Repel, Surf north, land and walk up Kindle
//  Road. Surfing goes by walls and timed steps, since open water has nothing
//  to read the position from.
void one_island_walk_to_spa(
    ConsoleHandle& console, ProControllerContext& context,
    Walker& walker, Area& kindle, uint8_t repel_slot,
    const TrainerBattleHandler& on_trainer_battle
){
    console.log("Running to the beach...");
    //  In Kindle Road coordinates, from (-10, 126) to (3, 132) on the beach.
    walker.set_facing(Dir::DOWN);
    walker.walk_path({{Dir::DOWN, 4}, {Dir::RIGHT, 4}, {Dir::DOWN, 2}, {Dir::RIGHT, 9}}, true);
    //  The next tile is water: holding on only faces it.
    walker.hold(Dir::RIGHT, false, 400ms);
    pbf_wait(context, 300ms);
    context.wait_for_all_requests();

    //  Menus don't change which way the player faces (the water).
    use_max_repel(console, context, repel_slot);
    walker.reset_steps();       //  To know when it wears off.
    start_surfing(console, context);

    //  Now on (4, 132) (or (4, 133)). Surfing goes at running speed. North
    //  until the rock stops the player at (4, 127).
    walker.set_facing(Dir::RIGHT);
    walker.walk(Dir::UP, 8, true);
    walker.uncount(3);
    //  East to (12, 127), north to (12, 123).
    walker.walk(Dir::RIGHT, 8, true);
    walker.walk(Dir::UP, 4, true);
    //  West: off the water at (10, 123), stopped by the wall at (9, 123).
    //  Then south into the water's edge, in case the last stretch north
    //  went one tile too far.
    walker.hold(Dir::LEFT, false, 2000ms);
    pbf_wait(context, 300ms);
    walker.hold(Dir::DOWN, false, 600ms);
    walker.add_steps(4);
    context.wait_for_all_requests();
    walker.set_facing(Dir::DOWN);
    area_anchor(console, context, kindle, KINDLE_LANDING);

    //  Nobody moves on the way up until the Black Belts, and there are no
    //  remembered screens ahead to read the position from on the first
    //  trip: run the whole way up to (11, 66) without stopping between
    //  turns, like the Seven Island run. That corner has walls to the left
    //  and above: run into both to be sure of the tile.
    console.log("Running up Kindle Road...");
    const MapTile CORNER{11, 66};
    std::vector<Dir> path;
    if (!area_find_path(KINDLE_ROAD, KINDLE_LANDING, {CORNER}, KINDLE_OBSTACLES(), path)){
        OperationFailedExceptionWithScreenshot::fire(
            ErrorReportMode::SEND_ERROR_REPORT,
            "one_island_walk_to_spa(): No path up Kindle Road.",
            console
        );
    }
    walker.run_path(to_segments(path));
    walker.hold(Dir::LEFT, false, 400ms);
    pbf_wait(context, 200ms);
    walker.hold(Dir::UP, false, 400ms);
    walker.set_facing(Dir::UP);
    area_anchor(console, context, kindle, CORNER);

    console.log("Walking up Kindle Road to the Ember Spa...");
    //  One right, then straight up column 12 past the Black Belts.
    area_go_to(console, context, walker, kindle, {{12, 61}}, KINDLE_OBSTACLES(), "past the Black Belts", on_trainer_battle);
    area_go_to(console, context, walker, kindle, {SPA_DOOR_FRONT}, KINDLE_OBSTACLES(), "the Ember Spa", on_trainer_battle);
}


//  Inside the spa, walls stop every move used here, so no position reading
//  is needed (its corridor rows look alike, which fools the screen
//  comparison):
//    - From the entrance (13, 36), running up stops at (13, 19): the wall
//      above is the pacing row's.
//    - On row 19, running left or right stops at x = 5 or x = 21.
//    - Column 21 runs from (21, 7) to (21, 20) between walls, column 13
//      from (13, 8) down to the exit (13, 36).
//  Moves up or down are held into a wall (or a message): (21, 10), (21, 11)
//  and (13, 10) are rock stairs, which slow the player, so they can't be
//  timed. Only moves along flat rows are timed.
//  Only the player's x on row 19 is tracked. Screens remembered at known x
//  on row 19 tell x when a message stopped the player somewhere on it.
//  (Along the row they differ; it's up and down the corridor that rows look
//  alike.)
struct SpaPosition{
    int x = 13;
    int y = SPA_PACE_ROW;       //  19, or 20 (the row below, also wall to wall).
    ScreenOdometry row;

    void remember(ConsoleHandle& console, ProControllerContext& context){
        pbf_wait(context, 200ms);
        context.wait_for_all_requests();
        row.remember(console.video().snapshot(), x, y);
    }
    //  Read x (and which of rows 19 and 20) from the screen. False if it
    //  can't be.
    bool read_x(ConsoleHandle& console, ProControllerContext& context){
        pbf_wait(context, 300ms);
        context.wait_for_all_requests();
        int seen_x = 0;
        int seen_y = 0;
        std::string details;
        if (!row.locate(console.video().snapshot(), 13, SPA_PACE_ROW, 8, seen_x, seen_y, details, 1) ||
            seen_x < SPA_PACE_LEFT || seen_x > SPA_PACE_RIGHT ||
            seen_y < SPA_PACE_ROW || seen_y > SPA_PACE_ROW + 1
        ){
            console.log("Could not read the position near the pacing row (" + details + ").", COLOR_ORANGE);
            return false;
        }
        console.log("In the spa at (" + std::to_string(seen_x) + ", " + std::to_string(seen_y) + ").");
        x = seen_x;
        y = seen_y;
        return true;
    }
};

//  Run along row 19 to x, holding on into the wall at either end.
void spa_row_to(Walker& walker, SpaPosition& spa, int x){
    if (x == spa.x){
        return;
    }
    Dir dir = x > spa.x ? Dir::RIGHT : Dir::LEFT;
    uint16_t distance = (uint16_t)std::abs(x - spa.x);
    bool wall = x == SPA_PACE_LEFT || x == SPA_PACE_RIGHT;
    walker.walk(dir, (uint16_t)(distance + (wall ? 2 : 0)), true);
    if (wall){
        walker.uncount(2);
    }
    spa.x = x;
}

//  From anywhere near the spa, go in and run up to the pacing row.
void spa_enter(
    ConsoleHandle& console, ProControllerContext& context,
    Walker& walker, Area& kindle, SpaPosition& spa,
    const TrainerBattleHandler& on_trainer_battle
){
    area_go_to(console, context, walker, kindle, {SPA_DOOR_FRONT}, KINDLE_OBSTACLES(), "the Ember Spa", on_trainer_battle, JOGGER_ROW());
    go_through_door(console, context, walker, Dir::UP, "the Ember Spa");
    pbf_wait(context, 300ms);
    walker.set_facing(Dir::UP);
    //  17 tiles up, then into the wall.
    walker.walk(Dir::UP, 19, true);
    walker.uncount(2);
    context.wait_for_all_requests();
    spa.x = 13;
    spa.y = SPA_PACE_ROW;
    spa.remember(console, context);
}

//  From the pacing row, out to Kindle Road: into the right wall (sure of x),
//  8 left to x = 13 (checked on the screen), then hold down: column 13 runs
//  straight from row 19 to the exit (13, 36), clear of the bald man who
//  wanders at x = 10 to 12. Ends on the spa's door, facing down.
void spa_leave(ConsoleHandle& console, ProControllerContext& context, Walker& walker, SpaPosition& spa, Area& kindle){
    console.log("Leaving the spa...");
    if (spa.x != SPA_PACE_RIGHT){
        spa_row_to(walker, spa, SPA_PACE_RIGHT);
    }
    spa_row_to(walker, spa, 13);
    context.wait_for_all_requests();
    for (int fix = 0; fix < 2; fix++){
        if (!spa.read_x(console, context) || spa.x == 13){
            break;
        }
        console.log("Not at x = 13. Correcting.", COLOR_ORANGE);
        int seen = spa.x;
        spa.x = seen;
        spa_row_to(walker, spa, 13);
        context.wait_for_all_requests();
    }

    //  If the run down doesn't leave, the player is a tile off to a side
    //  somewhere in the corridor: shift and run down again.
    //  (This order gets out from anywhere between x = 11 and x = 17.)
    const struct{ Dir dir; uint16_t steps; } SHIFTS[] = {
        {Dir::DOWN, 0}, {Dir::RIGHT, 2}, {Dir::LEFT, 1}, {Dir::LEFT, 3},
    };
    for (const auto& shift : SHIFTS){
        if (shift.steps > 0){
            console.log(std::string("Still in the spa. Shifting ") + dir_name(shift.dir) + " and running down again.", COLOR_ORANGE);
            walker.forget_facing();
            walker.walk(shift.dir, shift.steps, false);
            context.wait_for_all_requests();
        }
        BlackScreenOverWatcher warp_finished(COLOR_RED);
        int ret = run_until<ProControllerContext>(
            console, context,
            [](ProControllerContext& context){
                pbf_controller_state(context, BUTTON_B, DPAD_DOWN, {0, 0}, {0, 0}, 4000ms);
                pbf_wait(context, 2000ms);
            },
            { warp_finished }
        );
        if (ret == 0){
            walker.add_steps(17);
            walker.set_facing(Dir::DOWN);
            area_anchor(console, context, kindle, SPA_DOOR);
            return;
        }
    }
    OperationFailedExceptionWithScreenshot::fire(
        ErrorReportMode::SEND_ERROR_REPORT,
        "spa_leave(): Did not leave the spa.",
        console
    );
}

//  From the pacing row, step into the hot spring (13, 13) and come back:
//    into the right wall (21, 19); hold up to the wall at (21, 7); down 1 and
//    left 8 along row 8 to (13, 8); hold down until the hot spring's message
//    stops the player on (13, 13).
//  Back: hold up to the wall at (13, 8); hold right to the wall at (23, 8);
//  hold down to the wall at (23, 9); left 2 to (21, 9); hold down to the
//  wall at (21, 20); up 1. Only the 2 left are timed.
void spa_heal(ConsoleHandle& console, ProControllerContext& context, Walker& walker, SpaPosition& spa){
    console.log("Healing in the hot spring...");
    spa_row_to(walker, spa, SPA_PACE_RIGHT);
    walker.hold(Dir::UP, true, 2200ms);
    walker.set_facing(Dir::UP);
    walker.walk_path({{Dir::DOWN, 1}, {Dir::LEFT, 8}}, true);
    WhiteDialogWatcher message(COLOR_RED);
    context.wait_for_all_requests();
    int ret = run_until<ProControllerContext>(
        console, context,
        [](ProControllerContext& context){
            pbf_controller_state(context, BUTTON_B, DPAD_DOWN, {0, 0}, {0, 0}, 1500ms);
            pbf_wait(context, 6000ms);
        },
        { message }
    );
    walker.add_steps(5 + 12);
    if (ret < 0){
        console.log("No message after stepping into the hot spring. Was the party healed?", COLOR_RED);
    }else{
        //  "The water is exquisite... Your POKEMON are fully refreshed!"
        pbf_mash_button(context, BUTTON_B, 1500ms);
        console.log("Healed.");
    }
    pbf_wait(context, 500ms);
    context.wait_for_all_requests();
    walker.forget_facing();
    walker.hold(Dir::UP, true, 1200ms);
    walker.hold(Dir::RIGHT, true, 1900ms);
    walker.hold(Dir::DOWN, true, 600ms);
    pbf_wait(context, 300ms);
    walker.set_facing(Dir::DOWN);
    walker.walk(Dir::LEFT, 2, true);
    walker.hold(Dir::DOWN, true, 2200ms);
    walker.set_facing(Dir::DOWN);
    walker.add_steps(5 + 10 + 1 + 11);
    //  Against the wall at (21, 20). Up onto row 19, letting the bump into
    //  the wall finish first (or the press can be lost), and check it.
    spa.x = SPA_PACE_RIGHT;
    spa.y = SPA_PACE_ROW + 1;
    for (int tries = 0; tries < 3 && spa.y != SPA_PACE_ROW; tries++){
        pbf_wait(context, 300ms);
        walker.walk(Dir::UP, 1, true);
        context.wait_for_all_requests();
        spa.y = SPA_PACE_ROW;
        if (!spa.read_x(console, context)){
            console.log("Not sure where the walk back from the hot spring ended.", COLOR_ORANGE);
            break;
        }
    }
}

//  Run back and forth on row 19 without stopping: one long input that turns
//  around a tile before each wall (between x = 6 and x = 20), so the player
//  never bumps. A turn while running costs no time. Timing errors would add
//  up over a long run, so every 6 laps the player runs into the right wall
//  once (one short stop) to be sure of x again. Returns against the right
//  wall. If a message comes up (the Repel's), it stops the player: it is
//  closed and x found again the same way.
//  `sure_of_row_end` false: a message may have stopped the player without
//  showing anymore (closed by B), so always run the whole row to the wall.
void spa_pace_smoothly(
    ConsoleHandle& console, ProControllerContext& context,
    Walker& walker, SpaPosition& spa, uint32_t steps, bool sure_of_row_end
){
    const int LEFT_TURN = SPA_PACE_LEFT + 1;
    const int RIGHT_TURN = SPA_PACE_RIGHT - 1;
    const uint16_t LAP = (uint16_t)(RIGHT_TURN - LEFT_TURN);
    spa_row_to(walker, spa, SPA_PACE_RIGHT);
    uint32_t done = 0;
    while (done < steps){
        //  From the right wall: 15 left to x = 6, then 14 each way.
        std::vector<std::pair<Dir, uint16_t>> legs;
        legs.emplace_back(Dir::LEFT, (uint16_t)(SPA_PACE_RIGHT - LEFT_TURN));
        uint32_t planned = legs.back().second;
        while (legs.size() < 12 && done + planned < steps){
            legs.emplace_back(legs.back().first == Dir::LEFT ? Dir::RIGHT : Dir::LEFT, LAP);
            planned += LAP;
        }
        //  One unbroken hold, B (running) all the way. Released in the middle
        //  of the last step, so the player stops after it.
        context.wait_for_all_requests();
        for (size_t i = 0; i < legs.size(); i++){
            double frames = RUN_STEP_FRAMES * legs[i].second;
            if (i == 0){
                frames += TURN_FRAMES;      //  Facing the wall.
            }
            if (i + 1 == legs.size()){
                frames -= RUN_STEP_FRAMES / 2;
            }
            pbf_controller_state(context, BUTTON_B, to_dpad(legs[i].first), {0, 0}, {0, 0}, frames_to_ms(frames));
        }
        pbf_wait(context, frames_to_ms(RUN_STEP_FRAMES) + 80ms);
        context.wait_for_all_requests();
        walker.add_steps(planned);
        done += planned;
        walker.set_facing(legs.back().first);
        spa.x = legs.back().first == Dir::LEFT ? LEFT_TURN : RIGHT_TURN;

        //  Back against the right wall (sure of x again). If a message
        //  stopped the player somewhere on the row, run the whole row.
        if (WhiteDialogDetector(COLOR_RED).detect(console.video().snapshot())){
            pbf_mash_button(context, BUTTON_B, 1500ms);
            context.wait_for_all_requests();
            walker.forget_facing();
            if (!spa.read_x(console, context)){
                walker.walk(Dir::RIGHT, 18, true);
                spa.x = SPA_PACE_RIGHT;
            }
            if (!sure_of_row_end){
                //  The Repel's message: done.
                return;
            }
        }else if (sure_of_row_end){
            spa_row_to(walker, spa, SPA_PACE_RIGHT);
        }else if (!spa.read_x(console, context)){
            walker.walk(Dir::RIGHT, (uint16_t)(SPA_PACE_RIGHT - spa.x + 16), true);
            walker.uncount(16);
            spa.x = SPA_PACE_RIGHT;
        }
    }
    context.wait_for_all_requests();
}

//  Pace until `steps` are walked, to charge the VS Seeker.
void spa_pace(
    ConsoleHandle& console, ProControllerContext& context,
    Walker& walker, SpaPosition& spa, uint32_t steps
){
    console.log("Pacing " + std::to_string(steps) + " steps to charge the VS Seeker...");
    spa_pace_smoothly(console, context, walker, spa, steps, true);
}

//  Pace until the Max Repel has run out: it lasts 250 steps from when it was
//  used, and `steps_left` is about how many are left.
void spa_burn_repel(
    ConsoleHandle& console, ProControllerContext& context,
    Walker& walker, SpaPosition& spa, uint32_t steps_left
){
    //  A few extra, in case some steps weren't counted.
    uint32_t steps = steps_left + 10;
    console.log("Pacing " + std::to_string(steps) + " steps until the Max Repel wears off...");
    //  Remember the row's two ends (walls stop the player there), to read x
    //  wherever the message stops the player.
    uint32_t start = walker.steps();
    spa_row_to(walker, spa, SPA_PACE_LEFT);
    spa.remember(console, context);
    spa_row_to(walker, spa, SPA_PACE_RIGHT);
    spa.remember(console, context);
    uint32_t walked = walker.steps() - start;
    spa_pace_smoothly(console, context, walker, spa, steps > walked ? steps - walked : 0, false);
    //  If its message is still up, close it.
    context.wait_for_all_requests();
    if (WhiteDialogDetector(COLOR_RED).detect(console.video().snapshot())){
        pbf_mash_button(context, BUTTON_B, 1500ms);
        if (!spa.read_x(console, context)){
            walker.forget_facing();
            walker.walk(Dir::RIGHT, 18, true);
            spa.x = SPA_PACE_RIGHT;
        }
    }
    console.log("The Max Repel should have worn off.");
}


//  How much dark sprite outline is on a tile (its bottom and the 8 rows
//  above: a standing person's legs and body). Sand, rock and footprints have
//  none (0 on empty sand); a person shows 40 or more. Footprints fool a
//  comparison with remembered screens, but not this.
constexpr double PERSON_OUTLINE = 20;
double person_outline(const ImageViewRGB32& screen, const MapTile& player, const MapTile& tile){
    SpriteSighting s = find_sprite_feet(screen, {{tile.x - player.x, tile.y - player.y}});
    return s.score;
}


//  Sharon walks up and down column 9 (rows 53 to 60). From the VS Seeker
//  tile only rows 59 and 60 are on screen, and the VS Seeker only reaches
//  trainers up to 5 rows away (58). Wait until she comes into view on her
//  way down: (9, 59) shows someone while (9, 60) is still clear. She then
//  reaches (9, 60) as the VS Seeker is used, where her icon is on screen.
void wait_for_sharon(ConsoleHandle& console, ProControllerContext& context, Area& kindle){
    const MapTile AT_59{9, 59};
    const MapTile AT_60{9, 60};
    bool was_clear = false;
    WallClock end = current_time() + 20000ms;
    while (current_time() < end){
        VideoSnapshot screen = console.video().snapshot();
        double d59 = person_outline(screen, kindle.tile, AT_59);
        double d60 = person_outline(screen, kindle.tile, AT_60);
        if (d59 < 8 && d60 < 8){
            was_clear = true;
        }else if (was_clear && d59 >= PERSON_OUTLINE && d60 < PERSON_OUTLINE){
            console.log("Sharon is coming into view (" + std::to_string((int)d59) + ", " + std::to_string((int)d60) + ").");
            return;
        }
        context.wait_for(30ms);
    }
    console.log("Sharon wasn't seen coming into view in 20 seconds. Using the VS Seeker anyway.", COLOR_ORANGE);
}


//  The icons seen after the VS Seeker, by trainer.
struct KindleIcons{
    size_t frames = 0;
    std::array<size_t, KINDLE_TRAINERS> ready{};
    std::array<size_t, KINDLE_TRAINERS> not_ready{};
    std::array<std::map<MapTile, size_t>, KINDLE_TRAINERS> ready_tiles;

    //  In how many frames each (tile, icon) was seen.
    std::map<std::pair<MapTile, VsSeekerIcon>, size_t> seen;

    void add(const ImageViewRGB32& frame, const MapTile& player){
        frames++;
        std::set<std::pair<MapTile, VsSeekerIcon>> in_frame;
        for (const VsSeekerIconSighting& s : find_vs_seeker_icons(frame)){
            //  A bubble cut off by the top of the screen (Sharon one tile
            //  further up) can't be told apart: its lower part can look like
            //  the other kind.
            if (s.y < 6){
                continue;
            }
            //  The icon's center is 8 pixels right of the trainer's tile's
            //  left edge and 17 pixels above its top edge.
            int dx = (int)std::lround((s.x - 8 - 112) / 16.0);
            int dy = (int)std::lround((s.y + 17 - 72) / 16.0);
            in_frame.insert({{player.x + dx, player.y + dy}, s.icon});
        }
        for (const auto& item : in_frame){
            seen[item]++;
        }
    }

    //  Count by trainer. An icon shows for about half a second of the 4:
    //  something seen in most frames is part of the scenery (or the
    //  player's own sprite, which can look like "!!"), not an icon.
    void tally(Logger& logger){
        size_t too_many = std::max<size_t>(frames / 2, 4);
        for (const auto& item : seen){
            const MapTile& tile = item.first.first;
            if (item.second >= too_many){
                logger.log("Ignoring an icon-like sight at " + tile_str(tile) + " (in " + std::to_string(item.second) + " frames).");
                continue;
            }
            int who = kindle_trainer_at(tile);
            if (who < 0){
                continue;
            }
            if (item.first.second == VsSeekerIcon::READY){
                ready[(size_t)who] += item.second;
                ready_tiles[(size_t)who][tile] += item.second;
            }else if (item.first.second == VsSeekerIcon::NOT_READY){
                not_ready[(size_t)who] += item.second;
            }
        }
    }
};

struct KindleReady{
    std::array<bool, KINDLE_TRAINERS> ready{};
    std::array<MapTile, KINDLE_TRAINERS> tile{};
    //  No icon was seen for Hugh: he may be right below the VS Seeker tile,
    //  where his icon is drawn over the player's own sprite.
    bool hugh_maybe_below = false;
};

//  Right after a VS Seeker use that found rematches: who wants one, and
//  where they stand. A trainer who wants a rematch shows "!!" and then stands
//  still; the others show "X" and keep walking (vs_seeker.c). Sharon is also
//  watched for a few seconds: if she stands on the same tile the whole time
//  she is ready.
KindleReady find_kindle_ready(ConsoleHandle& console, ProControllerContext& context, Area& kindle, KindleIcons& icons){
    icons.tally(console.logger());
    std::array<VsSeekerIcon, KINDLE_TRAINERS> icon;
    bool any_ready = false;
    for (size_t who = 0; who < KINDLE_TRAINERS; who++){
        size_t r = icons.ready[who];
        size_t x = icons.not_ready[who];
        icon[who] = VsSeekerIcon::NONE;
        if (r >= 2 && r > x){
            icon[who] = VsSeekerIcon::READY;
            any_ready = true;
        }else if (x >= 2 && x > r){
            icon[who] = VsSeekerIcon::NOT_READY;
        }
        console.log(
            std::string(kindle_trainer_name(who)) + ": " + icon_name(icon[who]) +
            " [" + std::to_string(r) + " !! / " + std::to_string(x) + " X of " + std::to_string(icons.frames) + " frames]"
        );
    }
    if (!any_ready){
        console.log("No \"!!\" was seen, but someone is ready. Judging by who stands still.", COLOR_ORANGE);
        icon.fill(VsSeekerIcon::NONE);
    }

    //  Whether Sharon stands still (rows 59 and 60 are on screen). Ready
    //  trainers stop walking. Seen by her sprite's outline: someone on (9, 60)
    //  also shows a little on (9, 59) (the head), so (9, 60) is checked first.
    std::vector<size_t> occupied(2, 0);
    size_t frames = 0;
    pbf_wait(context, 300ms);
    context.wait_for_all_requests();
    WallClock end = current_time() + 3000ms;
    while (current_time() < end){
        VideoSnapshot screen = console.video().snapshot();
        double d59 = person_outline(screen, kindle.tile, {9, 59});
        double d60 = person_outline(screen, kindle.tile, {9, 60});
        frames++;
        if (d60 >= PERSON_OUTLINE){
            occupied[1]++;
        }else if (d59 >= PERSON_OUTLINE){
            occupied[0]++;
        }
        context.wait_for(50ms);
    }
    console.log(
        "Sharon's tiles occupied (of " + std::to_string(frames) + " frames): (9, 59) " +
        std::to_string(occupied[0]) + ", (9, 60) " + std::to_string(occupied[1])
    );

    KindleReady ret;

    //  The Black Belts: by their icons. Hugh's icon is hidden under the
    //  player's own sprite (ignored as scenery) when he stands right below
    //  the VS Seeker tile, at (11, 64): then only a few frames of it show.
    for (size_t who : {(size_t)HUGH, (size_t)SHEA}){
        if (icon[who] != VsSeekerIcon::READY){
            continue;
        }
        ret.ready[who] = true;
        const std::vector<MapTile>& path = kindle_trainer_tiles(who);
        size_t best = 0;
        MapTile tile = path[0];
        for (const auto& item : icons.ready_tiles[who]){
            if (item.second > best){
                best = item.second;
                tile = item.first;
            }
        }
        if (who == HUGH && icons.ready[who] < 5){
            tile = {KINDLE_VS_TILE.x, KINDLE_VS_TILE.y + 1};
        }
        if (std::find(path.begin(), path.end(), tile) == path.end()){
            MapTile nearest = path[0];
            for (const MapTile& t : path){
                if (tile_distance(t, tile) < tile_distance(nearest, tile)){
                    nearest = t;
                }
            }
            tile = nearest;
        }
        ret.tile[who] = tile;
        console.log(std::string(kindle_trainer_name(who)) + " is ready, at " + tile_str(tile) + ".");
    }

    ret.hugh_maybe_below = icon[HUGH] == VsSeekerIcon::NONE;

    //  Sharon: by standing still, or by her icon.
    {
        bool at_60 = occupied[1] * 10 >= frames * 7;
        bool at_59 = occupied[0] * 10 >= frames * 7;
        MapTile still = at_60 ? MapTile{9, 60} : MapTile{9, 59};
        //  One who doesn't want a rematch shows "X" and walks on (she walks
        //  without stopping), so standing still decides, whatever icon was
        //  read: hers is off screen when she stops one tile further up.
        if (frames > 0 && (at_59 || at_60)){
            ret.ready[SHARON] = true;
            ret.tile[SHARON] = still;
        }else if (icon[SHARON] == VsSeekerIcon::READY){
            ret.ready[SHARON] = true;
            ret.tile[SHARON] = MapTile{9, 60};
        }
        if (ret.ready[SHARON]){
            console.log(std::string("Crush Girl Sharon is ready, at ") + tile_str(ret.tile[SHARON]) + ".");
        }
    }
    if (icon[MIK_AND_KIA] == VsSeekerIcon::READY){
        ret.ready[MIK_AND_KIA] = true;
        ret.tile[MIK_AND_KIA] = {9, 68};
        console.log("Crush Kin Mik & Kia are ready.");
    }

    bool anyone = false;
    for (bool r : ret.ready){
        anyone = anyone || r;
    }
    if (!anyone && icon[MIK_AND_KIA] == VsSeekerIcon::NONE && !ret.hugh_maybe_below){
        //  They never move, so only their icon tells. Someone is ready.
        console.log("Nobody else looks ready: trying Mik & Kia.", COLOR_ORANGE);
        ret.ready[MIK_AND_KIA] = true;
        ret.tile[MIK_AND_KIA] = {9, 68};
    }
    return ret;
}


//  On the VS Seeker tile: talk to whoever is right below (Hugh, if his icon
//  was hidden under the player's sprite). WON or STRONG_FAINTED if he
//  battled; NO_BATTLE if nobody answered or he didn't want to battle.
BattleOutcome kindle_try_below(
    ConsoleHandle& console, ProControllerContext& context,
    Walker& walker, BattleContext& battle
){
    console.log("No icon seen for Black Belt Hugh. Checking right below the VS Seeker tile...");
    context.wait_for_all_requests();
    if (!walker.is_facing(Dir::DOWN)){
        //  A tap turns without stepping.
        pbf_press_dpad(context, DPAD_DOWN, 60ms, 300ms);
        walker.set_facing(Dir::DOWN);
    }
    WhiteDialogWatcher talking(COLOR_RED);
    BattleMenuWatcher started(COLOR_RED);
    context.wait_for_all_requests();
    pbf_press_button(context, BUTTON_A, 200ms, 0ms);
    if (wait_until(console, context, 2000ms, { talking, started }) < 0){
        console.log("Nobody there.");
        return BattleOutcome::NO_BATTLE;
    }
    BattleMenuWatcher battle_menu(COLOR_RED);
    int ret = run_until<ProControllerContext>(
        console, context,
        [](ProControllerContext& context){
            pbf_mash_button(context, BUTTON_B, 12000ms);
        },
        { battle_menu }
    );
    if (ret < 0){
        console.log("Hugh is there but doesn't want a rematch.");
        return BattleOutcome::NO_BATTLE;
    }
    return fight_trainer_battle(console, context, battle);
}


//  Sharon, ready, stands still somewhere in column 9 (rows 53 to 60). Go to
//  the bottom of the column, (9, 60), and run up until she blocks the way:
//  then the player is right in front of her, facing her. If she is on
//  (9, 60) herself, talk to her from the right.
BattleOutcome kindle_battle_sharon(
    ConsoleHandle& console, ProControllerContext& context,
    Walker& walker, Area& kindle, const MapTile& seen_at,
    const std::set<MapTile>& blocked, BattleContext& battle,
    const TrainerBattleHandler& on_trainer_battle,
    const bool& fought_on_the_way
){
    const MapTile BOTTOM{9, 60};
    std::set<MapTile> avoid = blocked;
    for (const MapTile& t : kindle_trainer_tiles(SHARON)){
        avoid.erase(t);
    }
    if (seen_at == BOTTOM){
        console.log("Walking to Crush Girl Sharon at the bottom of her walk...");
        std::set<MapTile> avoid_her = avoid;
        avoid_her.insert(BOTTOM);
        area_go_to(console, context, walker, kindle, {{10, 60}}, avoid_her, "Crush Girl Sharon", on_trainer_battle, JOGGER_ROW());
        if (fought_on_the_way){
            return BattleOutcome::NO_BATTLE;
        }
        walker.face(Dir::LEFT);
        BattleOutcome outcome = run_trainer_battle(console, context, walker, Dir::LEFT, battle);
        if (outcome != BattleOutcome::NO_BATTLE){
            return outcome;
        }
        console.log("Sharon isn't on (9, 60): she is further up.", COLOR_ORANGE);
    }
    console.log("Walking up Crush Girl Sharon's column until she's in the way...");
    area_go_to(console, context, walker, kindle, {BOTTOM}, avoid, "the bottom of Sharon's walk", on_trainer_battle, JOGGER_ROW());
    if (fought_on_the_way){
        return BattleOutcome::NO_BATTLE;
    }
    //  At most 6 tiles up to her; holding on just bumps into her.
    walker.hold(Dir::UP, true, 1500ms);
    pbf_wait(context, 300ms);
    walker.set_facing(Dir::UP);
    context.wait_for_all_requests();
    BattleOutcome outcome = run_trainer_battle(console, context, walker, Dir::UP, battle);

    //  Somewhere up the column now: back down it. The wall below (9, 60)
    //  stops the player there, so the position is known again. (She is
    //  above, so nobody is in the way.)
    pbf_wait(context, 300ms);
    walker.hold(Dir::DOWN, true, 1500ms);
    pbf_wait(context, 300ms);
    walker.set_facing(Dir::DOWN);
    context.wait_for_all_requests();
    kindle.tile = BOTTOM;
    return outcome;
}


//  Walk next to a ready trainer, face them and battle. Mik & Kia are one
//  double battle: talking to either starts it. If talking doesn't start a
//  battle, the trainer may be a tile off: look again and try once more.
BattleOutcome kindle_battle(
    ConsoleHandle& console, ProControllerContext& context,
    Walker& walker, Area& kindle, size_t who, MapTile tile,
    const std::set<MapTile>& blocked, BattleContext& battle,
    const TrainerBattleHandler& on_trainer_battle,
    const bool& fought_on_the_way
){
    //  Set when the trainer saw the player on the way and battled: done.
    auto fought = [&]{
        if (fought_on_the_way){
            console.log(std::string(kindle_trainer_name(who)) + " was battled on the way.");
        }
        return fought_on_the_way;
    };
    if (who == SHARON){
        return kindle_battle_sharon(console, context, walker, kindle, tile, blocked, battle, on_trainer_battle, fought_on_the_way);
    }
    BattleOutcome outcome = BattleOutcome::NO_BATTLE;
    std::set<MapTile> tried;
    for (int attempt = 0; attempt < 3; attempt++){
        tried.insert(tile);
        std::vector<MapTile> trainer = who == MIK_AND_KIA
            ? kindle_trainer_tiles(MIK_AND_KIA)
            : std::vector<MapTile>{tile};
        std::set<MapTile> avoid = blocked;
        for (const MapTile& t : trainer){
            avoid.insert(t);
        }
        std::set<MapTile> goals;
        for (const MapTile& t : trainer){
            for (Dir dir : {Dir::UP, Dir::DOWN, Dir::LEFT, Dir::RIGHT}){
                MapTile next{t.x + dir_dx(dir), t.y + dir_dy(dir)};
                char cell = KINDLE_ROAD.cell(next);
                if (cell != '#' && cell != '~' && avoid.find(next) == avoid.end()){
                    goals.insert(next);
                }
            }
        }
        std::vector<Dir> check;
        if (!area_find_path(KINDLE_ROAD, kindle.tile, goals, avoid, check)){
            console.log(std::string("No way to ") + kindle_trainer_name(who) + " at " + tile_str(tile) + ". Skipping them.", COLOR_RED);
            return BattleOutcome::NO_BATTLE;
        }
        console.log(std::string("Walking to ") + kindle_trainer_name(who) + " at " + tile_str(tile) + "...");
        area_go_to(console, context, walker, kindle, goals, avoid, std::string(kindle_trainer_name(who)), on_trainer_battle, JOGGER_ROW());
        if (fought()){
            return BattleOutcome::NO_BATTLE;
        }

        MapTile next_to = trainer[0];
        for (const MapTile& t : trainer){
            if (tile_distance(t, kindle.tile) == 1){
                next_to = t;
            }
        }
        Dir facing = direction_to(kindle.tile, next_to);
        walker.face(facing);
        context.wait_for_all_requests();
        battle.double_battle = who == MIK_AND_KIA;
        outcome = run_trainer_battle(console, context, walker, facing, battle);
        battle.double_battle = false;
        if (outcome != BattleOutcome::NO_BATTLE || who == MIK_AND_KIA){
            return outcome;
        }
        MapTile next = tile;
        {
            //  A Black Belt is on another tile of his three: the nearest one
            //  to where he was looked for, not tried yet.
            bool found = false;
            for (const MapTile& t : kindle_trainer_tiles(who)){
                if (!tried.count(t) && (!found || tile_distance(t, tile) < tile_distance(next, tile))){
                    next = t;
                    found = true;
                }
            }
            if (!found){
                return outcome;
            }
        }
        console.log(std::string("Looking for ") + kindle_trainer_name(who) + " at " + tile_str(next) + " instead.", COLOR_ORANGE);
        tile = next;
    }
    return outcome;
}



}   //  namespace



void ExpFarmer::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    switch (LOCATION){
    case Location::SEVEN_ISLAND:
        run_seven_island(env, context);
        return;
    case Location::ONE_ISLAND:
        run_one_island(env, context);
        return;
    }
}

void ExpFarmer::run_seven_island(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    ExpFarmer_Descriptor::Stats& stats = env.current_stats<ExpFarmer_Descriptor::Stats>();
    DeferredStopButtonOption::ResetOnExit reset_on_exit(STOP_AFTER_CURRENT);
    ConsoleHandle& console = env.console;

    BattleContext battle;
    battle.evolution = EVOLUTION;
    battle.move_learn = MOVE_LEARN;
    battle.switch_in = SWITCH_IN;
    battle.target_level = TARGET_LEVEL;
    battle.leveling = LEVELING;

    Walker walker(console, context);
    HouseState house;
    CanyonPosition pos;

    auto check_done = [&]{
        return STOP_AFTER_CURRENT.should_stop();
    };

    //  Strong Pokemon fainted: stop and tell the user.
    auto strong_fainted = [&]{
        stats.errors++;
        env.update_stats();
        OperationFailedExceptionWithScreenshot::fire(
            ErrorReportMode::NO_ERROR_REPORT,
            "The strong Pokemon fainted. Train it further (it should out-level the Cooltrainers' "
            "level 52-56 Pokemon comfortably) and restart the program.",
            console
        );
    };

    //  Start: fly to Seven Island, heal, run to the house.
    fly_to_seven_island(console, context, walker);
    enter_pokecenter(console, context);
    heal_at_pokecenter(console, context);
    leave_pokecenter(console, context);
    travel_pokecenter_to_house(console, context, walker, MAX_REPEL_SLOT);
    //  Now at about (10, 61), just above the southern Cooltrainer's area. The
    //  house roof is in view from here on.
    pos.tile = {10, 61};
    walk_into_house(console, context, walker, pos, {}, false);
    house = HouseState();

    house_burn_repel(console, context, walker, house);
    if (SWITCH_IN){
        swap_first_two_party_slots(console, context);
    }else{
        console.log("Switch-In is off: party slot 1 battles from the start.");
    }
    walker.reset_steps();

    bool need_heal = false;
    //  Steps still needed before the VS Seeker can be used. Unknown at the
    //  start: try it, and its message says how many are missing.
    uint32_t charge_needed = 0;

    while (true){
        send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE);

        //  In the house, on row 7: heal if a battle happened, charge, leave.
        if (need_heal){
            house_heal(console, context, walker, house);
            need_heal = false;
        }
        if (walker.steps() < charge_needed){
            //  Leaving takes a few steps too; a small margin covers miscounts.
            uint32_t remaining = charge_needed - walker.steps() + 3;
            console.log("Pacing " + std::to_string(remaining) + " steps to charge the VS Seeker...");
            house_pace(walker, house, context, remaining);
        }
        house_leave(console, context, walker, house);
        pos.tile = HOUSE_EXIT_TILE;

        //  Use the VS Seeker, pacing next to it if it isn't charged yet.
        VsSeekerResult result = VsSeekerResult::NOT_CHARGED;
        VsSeekerIcons icons;
        for (int tries = 0; tries < 3; tries++){
            int steps_needed = 0;
            walk_to_vs_seeker_tile(console, context, walker, pos);
            result = use_vs_seeker(console, context, steps_needed, icons);
            if (result != VsSeekerResult::NOT_CHARGED){
                break;
            }
            //  Pace between the VS Seeker tile and the tile below it. Each step
            //  is checked: one lost step would leave the player off the tile.
            uint32_t pace = (uint32_t)steps_needed + 2;
            pace += pace % 2;
            for (uint32_t i = 0; i < pace; i += 2){
                walker.checked_step(Dir::DOWN, true);
                walker.checked_step(Dir::UP, true);
            }
            context.wait_for_all_requests();
        }

        stats.cycles++;
        env.update_stats();

        switch (result){
        case VsSeekerResult::NOT_CHARGED:
            OperationFailedExceptionWithScreenshot::fire(
                ErrorReportMode::SEND_ERROR_REPORT,
                "The VS Seeker still isn't charged after pacing the steps it asked for.",
                console
            );
        case VsSeekerResult::NO_TRAINERS:
            OperationFailedExceptionWithScreenshot::fire(
                ErrorReportMode::SEND_ERROR_REPORT,
                "The VS Seeker found no trainers in range. The player is probably not on the VS Seeker tile.",
                console
            );
        case VsSeekerResult::NOT_READY:{
            //  Nobody wants a rematch. Charge again in the house (no heal).
            console.log("Nobody is ready. Charging again in the house.");
            stats.not_ready++;
            env.update_stats();
            walker.reset_steps();
            charge_needed = 100;
            walk_into_house(console, context, walker, pos, {}, false);
            house = HouseState();
            house_calibrate(walker, house, context, house.x);
            if (STOP_AFTER_CURRENT.should_stop()){
                break;
            }
            continue;
        }
        case VsSeekerResult::FOUND:
            break;
        }

        walker.reset_steps();
        charge_needed = 100;
        ReadyTrainers ready = find_ready_trainers(console, context, icons);
        if (!ready.north && !ready.south){
            console.log("Nobody seen hopping. Watching again.", COLOR_ORANGE);
            ready = find_ready_trainers(console, context, icons);
        }
        if (!ready.north && !ready.south){
            //  The VS Seeker found someone, but neither could be found. Don't
            //  stop the program for it: charge again in the house.
            console.log("The VS Seeker found a rematch, but the ready trainer couldn't be found. Charging again.", COLOR_RED);
            stats.missed++;
            env.update_stats();
            walk_into_house(console, context, walker, pos, {}, false);
            house = HouseState();
            house_calibrate(walker, house, context, house.x);
            if (STOP_AFTER_CURRENT.should_stop()){
                break;
            }
            continue;
        }

        //  Ready (and fought) trainers stand still: walk around them. Trainers
        //  that aren't ready keep wandering; if one steps in the way, the
        //  walk waits and goes on (the position is read before every step).
        std::set<MapTile> blocked;
        if (ready.north){
            blocked.insert(ready.north_tile);
        }
        if (ready.south){
            blocked.insert(ready.south_tile);
        }

        bool grass_cut = false;
        auto count_outcome = [&](BattleOutcome outcome){
            switch (outcome){
            case BattleOutcome::STRONG_FAINTED:
                strong_fainted();
                break;
            case BattleOutcome::NO_BATTLE:
                stats.missed++;
                break;
            case BattleOutcome::WON:
                stats.battles++;
                need_heal = true;
                break;
            }
            env.update_stats();
        };
        if (ready.north){
            cut_path_north(console, context, walker, pos);
            grass_cut = true;
            count_outcome(approach_and_battle(console, context, walker, pos, ready.north_tile, NORTH_TRAINER_TILES(), blocked, true, battle));
        }
        if (ready.south && !battle.stop_requested){
            count_outcome(approach_and_battle(console, context, walker, pos, ready.south_tile, SOUTH_TRAINER_TILES(), blocked, grass_cut, battle));
        }

        if (battle.stop_requested){
            console.log(battle.stop_reason + " Stopping.", COLOR_BLUE);
            send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH, battle.stop_reason);
            GO_HOME_WHEN_DONE.run_end_of_program(context);
            return;
        }

        //  Back into the house to heal and charge. After their battles the
        //  trainers wander again, so their tiles aren't avoided: if one is
        //  in the way, the walk goes around.
        walk_into_house(console, context, walker, pos, {}, grass_cut);
        house = HouseState();
        house_calibrate(walker, house, context, house.x);

        if (check_done()){
            break;
        }
    }

    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
    GO_HOME_WHEN_DONE.run_end_of_program(context);
}


void ExpFarmer::run_one_island(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    ExpFarmer_Descriptor::Stats& stats = env.current_stats<ExpFarmer_Descriptor::Stats>();
    DeferredStopButtonOption::ResetOnExit reset_on_exit(STOP_AFTER_CURRENT);
    ConsoleHandle& console = env.console;

    BattleContext battle;
    battle.evolution = EVOLUTION;
    battle.move_learn = MOVE_LEARN;
    battle.switch_in = SWITCH_IN;
    battle.target_level = TARGET_LEVEL;
    battle.leveling = LEVELING;
    battle.opponent_species = &KINDLE_SPECIES();

    Walker walker(console, context);
    Area kindle(KINDLE_ROAD);
    SpaPosition spa;

    bool need_heal = false;
    //  This cycle's trainers who want a rematch, and who has been fought.
    KindleReady ready;
    std::array<bool, KINDLE_TRAINERS> done{};

    auto strong_fainted = [&]{
        stats.errors++;
        env.update_stats();
        OperationFailedExceptionWithScreenshot::fire(
            ErrorReportMode::NO_ERROR_REPORT,
            "A battling Pokemon fainted. Train it further (it should out-level the Kindle Road "
            "trainers' level 50-56 Pokemon comfortably; in the Mik & Kia double battle both of the "
            "first two party Pokemon battle) and restart the program.",
            console
        );
    };
    auto count_outcome = [&](BattleOutcome outcome){
        switch (outcome){
        case BattleOutcome::STRONG_FAINTED:
            strong_fainted();
            break;
        case BattleOutcome::NO_BATTLE:
            stats.missed++;
            break;
        case BattleOutcome::WON:
            stats.battles++;
            need_heal = true;
            break;
        }
        env.update_stats();
    };
    //  A ready trainer saw the player on the way (they see 1 tile ahead):
    //  the one next to the player.
    TrainerBattleHandler on_trainer_battle = [&](const MapTile& where){
        int who = -1;
        for (size_t i = 0; i < KINDLE_TRAINERS; i++){
            if (!ready.ready[i] || done[i]){
                continue;
            }
            std::vector<MapTile> tiles = i == MIK_AND_KIA
                ? kindle_trainer_tiles(MIK_AND_KIA)
                : std::vector<MapTile>{ready.tile[i]};
            for (const MapTile& t : tiles){
                if (tile_distance(t, where) <= 1){
                    who = (int)i;
                }
            }
        }
        console.log(
            std::string("A trainer battle started near ") + tile_str(where) +
            (who >= 0 ? std::string(" (") + kindle_trainer_name((size_t)who) + ")." : std::string(".")),
            COLOR_ORANGE
        );
        bool mik_and_kia = false;
        for (const MapTile& t : kindle_trainer_tiles(MIK_AND_KIA)){
            mik_and_kia = mik_and_kia || tile_distance(t, where) <= 1;
        }
        battle.double_battle = mik_and_kia;
        BattleOutcome outcome = fight_trainer_battle(console, context, battle);
        battle.double_battle = false;
        if (who >= 0){
            done[(size_t)who] = true;
        }
        count_outcome(outcome);
    };

    auto finish = [&]{
        console.log(battle.stop_reason + " Stopping.", COLOR_BLUE);
        send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH, battle.stop_reason);
        GO_HOME_WHEN_DONE.run_end_of_program(context);
    };

    //  Start: fly to One Island, heal, go to the spa. In the spa, wait out
    //  the Max Repel (it lasts past the walk there).
    fly_to_one_island(console, context, walker);
    one_island_pokecenter_heal(console, context, walker);
    one_island_walk_to_spa(console, context, walker, kindle, MAX_REPEL_SLOT, on_trainer_battle);
    if (battle.stop_requested){
        finish();
        return;
    }
    spa_enter(console, context, walker, kindle, spa, on_trainer_battle);
    //  Steps since the Max Repel was used (just before Surfing).
    uint32_t repel_steps_used = walker.steps();
    spa_burn_repel(console, context, walker, spa, 250 > repel_steps_used ? 250 - repel_steps_used : 0);
    if (SWITCH_IN){
        swap_first_two_party_slots(console, context);
    }else{
        console.log("Switch-In is off: party slot 1 battles from the start.");
    }
    walker.reset_steps();
    need_heal = false;

    //  Steps still needed before the VS Seeker can be used. Unknown at the
    //  start: try it, and its message says how many are missing.
    uint32_t charge_needed = 0;
    //  From the pacing row to the VS Seeker tile.
    const uint32_t STEPS_TO_VS_SEEKER = 25;

    while (true){
        send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE);

        //  In the spa: heal if a battle happened, charge, leave.
        if (need_heal){
            spa_heal(console, context, walker, spa);
            need_heal = false;
        }
        if (walker.steps() + STEPS_TO_VS_SEEKER < charge_needed){
            spa_pace(console, context, walker, spa, charge_needed - walker.steps() - STEPS_TO_VS_SEEKER + 3);
        }
        spa_leave(console, context, walker, spa, kindle);

        //  Use the VS Seeker, pacing next to it if it isn't charged yet.
        VsSeekerResult result = VsSeekerResult::NOT_CHARGED;
        KindleIcons icons;
        for (int tries = 0; tries < 3; tries++){
            area_go_to(console, context, walker, kindle, {KINDLE_VS_TILE}, KINDLE_OBSTACLES(), "the VS Seeker tile", on_trainer_battle, JOGGER_ROW());
            wait_for_sharon(console, context, kindle);
            icons = KindleIcons();
            int steps_needed = 0;
            VsSeekerIcons unused;
            result = use_vs_seeker(
                console, context, steps_needed, unused,
                [&](const ImageViewRGB32& frame){ icons.add(frame, KINDLE_VS_TILE); }
            );
            if (result != VsSeekerResult::NOT_CHARGED){
                break;
            }
            //  Pace between the VS Seeker tile and the one right of it (the
            //  Black Belts walk on the row below).
            uint32_t pace = (uint32_t)steps_needed + 2;
            pace += pace % 2;
            for (uint32_t i = 0; i < pace; i += 2){
                walker.checked_step(Dir::RIGHT, true);
                walker.checked_step(Dir::LEFT, true);
            }
            context.wait_for_all_requests();
        }

        stats.cycles++;
        env.update_stats();

        switch (result){
        case VsSeekerResult::NOT_CHARGED:
            OperationFailedExceptionWithScreenshot::fire(
                ErrorReportMode::SEND_ERROR_REPORT,
                "The VS Seeker still isn't charged after pacing the steps it asked for.",
                console
            );
        case VsSeekerResult::NO_TRAINERS:
            OperationFailedExceptionWithScreenshot::fire(
                ErrorReportMode::SEND_ERROR_REPORT,
                "The VS Seeker found no trainers in range. The player is probably not on the VS Seeker tile.",
                console
            );
        case VsSeekerResult::NOT_READY:
            console.log("Nobody is ready. Charging again in the spa.");
            stats.not_ready++;
            env.update_stats();
            walker.reset_steps();
            charge_needed = 100;
            spa_enter(console, context, walker, kindle, spa, on_trainer_battle);
            if (STOP_AFTER_CURRENT.should_stop()){
                break;
            }
            continue;
        case VsSeekerResult::FOUND:
            break;
        }
        if (result != VsSeekerResult::FOUND){
            break;
        }

        walker.reset_steps();
        charge_needed = 100;
        ready = find_kindle_ready(console, context, kindle, icons);
        done.fill(false);

        //  Ready trainers stand still: walk around them. The others walk
        //  about; one in the way is waited for or walked around.
        std::set<MapTile> blocked = KINDLE_OBSTACLES();
        for (size_t who = 0; who < KINDLE_TRAINERS; who++){
            if (ready.ready[who]){
                blocked.insert(ready.tile[who]);
            }
        }
        bool anyone = false;
        if (ready.hugh_maybe_below && !ready.ready[HUGH]){
            BattleOutcome outcome = kindle_try_below(console, context, walker, battle);
            if (outcome != BattleOutcome::NO_BATTLE){
                anyone = true;
                done[HUGH] = true;
                count_outcome(outcome);
            }
        }
        //  The nearest ready trainer each time, by walking distance.
        while (!battle.stop_requested){
            size_t who = KINDLE_TRAINERS;
            size_t shortest = 0;
            for (size_t i = 0; i < KINDLE_TRAINERS; i++){
                if (!ready.ready[i] || done[i]){
                    continue;
                }
                std::set<MapTile> next_to;
                for (const MapTile& t : i == MIK_AND_KIA ? kindle_trainer_tiles(MIK_AND_KIA) : std::vector<MapTile>{ready.tile[i]}){
                    for (Dir dir : {Dir::UP, Dir::DOWN, Dir::LEFT, Dir::RIGHT}){
                        next_to.insert({t.x + dir_dx(dir), t.y + dir_dy(dir)});
                    }
                }
                std::vector<Dir> path;
                size_t length = area_find_path(KINDLE_ROAD, kindle.tile, next_to, blocked, path) ? path.size() : 1000;
                if (who == KINDLE_TRAINERS || length < shortest){
                    who = i;
                    shortest = length;
                }
            }
            if (who == KINDLE_TRAINERS){
                break;
            }
            anyone = true;
            BattleOutcome outcome = kindle_battle(
                console, context, walker, kindle, who, ready.tile[who],
                blocked, battle, on_trainer_battle, done[who]
            );
            //  Battled on the way (already counted), or here.
            if (!done[who]){
                count_outcome(outcome);
            }
            done[who] = true;
            //  After their battle they walk about again.
            blocked.erase(ready.tile[who]);
            if (who == MIK_AND_KIA){
                blocked.insert(kindle_trainer_tiles(MIK_AND_KIA).begin(), kindle_trainer_tiles(MIK_AND_KIA).end());
            }
        }
        if (!anyone){
            console.log("The VS Seeker found a rematch, but the ready trainer couldn't be found. Charging again.", COLOR_RED);
            stats.missed++;
            env.update_stats();
        }

        if (battle.stop_requested){
            finish();
            return;
        }

        spa_enter(console, context, walker, kindle, spa, on_trainer_battle);
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
