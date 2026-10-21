/*  Blueberry Quests
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_BlueberryQuests_H
#define PokemonAutomation_PokemonSV_BlueberryQuests_H

#include <optional>
#include <vector>
#include "NintendoSwitch/Controllers/Procon/NintendoSwitch_ProController.h"
#include "NintendoSwitch/NintendoSwitch_ConsoleHandle.h"
#include "VideoGames/PokemonSV/Programs/ShinyHunting/PokemonSV_LetsGoTools.h"
#include "VideoGames/PokemonSV/Options/PokemonSV_BBQOption.h"

namespace PokemonAutomation{
    struct ProgramInfo;
namespace NintendoSwitch{
namespace PokemonSV{

BBQuests BBQuests_string_to_enum(const std::string& token);

enum class CameraAngle{
    none,
    up,
    down
};

//Basic blue quests
const std::set<BBQuests> blue_quests = {
    BBQuests::auto_10, BBQuests::make_tm, BBQuests::pickup_10, BBQuests::sneak_up, BBQuests::photo_fly, BBQuests::photo_swim, BBQuests::photo_canyon, BBQuests::photo_coastal, BBQuests::photo_polar, BBQuests::photo_savanna, BBQuests::tera_self_defeat, 
    BBQuests::travel_500, BBQuests::catch_any, BBQuests::catch_normal, BBQuests::catch_fighting, BBQuests::catch_flying, BBQuests::catch_poison, BBQuests::catch_ground, BBQuests::catch_rock, BBQuests::catch_bug, BBQuests::catch_ghost, BBQuests::catch_steel, 
    BBQuests::catch_fire, BBQuests::catch_water, BBQuests::catch_grass, BBQuests::catch_electric, BBQuests::catch_psychic, BBQuests::catch_ice, BBQuests::catch_dragon, BBQuests::catch_dark, BBQuests::catch_fairy
};

//Red quests that appear at the top when a player completes 10 blues.
//In multiplayer, check if picnic is active before attempting some as only one player can picnic at a time.
const std::set<BBQuests> red_quests = {
    BBQuests::wash_pokemon, BBQuests::wild_tera, BBQuests::auto_30, BBQuests::tera_raid, BBQuests::sandwich_three, BBQuests::bitter_sandwich, BBQuests::sweet_sandwich, BBQuests::salty_sandwich, BBQuests::sour_sandwich, BBQuests::spicy_sandwich, BBQuests::hatch_egg, 
    BBQuests::photo_normal, BBQuests::photo_fighting, BBQuests::photo_flying, BBQuests::photo_poison, BBQuests::photo_ground, BBQuests::photo_rock, BBQuests::photo_bug, BBQuests::photo_ghost, BBQuests::photo_steel, BBQuests::photo_fire, BBQuests::photo_water, 
    BBQuests::photo_grass, BBQuests::photo_electric, BBQuests::photo_psychic, BBQuests::photo_ice, BBQuests::photo_dragon, BBQuests::photo_dark, BBQuests::photo_fairy
};

//Multiplayer only. Appears at the top when 3 red quests are completed.
const std::set<BBQuests> gold_quests = {
    BBQuests::ditto_central, BBQuests::ditto_canyon, BBQuests::ditto_coastal, BBQuests::ditto_polar, BBQuests::ditto_savanna, BBQuests::group_canyon, BBQuests::group_coastal, BBQuests::group_polar, BBQuests::group_savanna, BBQuests::group_eyewear, BBQuests::group_nonuniform, 
    BBQuests::group_masks, BBQuests::sandwich_four, BBQuests::catch_hint, BBQuests::catch_hint2
};

//Quests that are not currently supported. Gold quests currently excluded as this is singleplayer only right now.
const std::set<BBQuests> not_possible_quests = {
    BBQuests::UnableToDetect, BBQuests::pickup_10, BBQuests::photo_flying, BBQuests::catch_flying
};


// Open the BBQ panel and read the current amount of BP
int read_BP(const ProgramInfo& info, VideoStream& stream, ProControllerContext& context);

//Open the quest panel and read in the current quests.
//Currently only supports singleplayer.
//For multiplayer, we will want keep track of which quest are gold/red and scroll cursor down until all of the current player's blue quests are in.
std::vector<BBQuests> read_quests(
    const ProgramInfo& info,
    VideoStream& stream, ProControllerContext& context,
    const BBQOption& BBQ_OPTIONS
);

//What to do with the quests read from the panel. Slot 0 is the top quest on the panel.
struct BBQuestPlan{
    std::vector<BBQuests> to_do;        //Quests to attempt, in panel order.
    std::vector<size_t> reroll_slots;   //Slots to reroll, in panel order.
    bool reroll_all = false;            //Nothing to do and nothing rerolled: reroll every slot.
};
//Decide what to do with each quest from the options alone, without touching the game, so
//the decision can be tested. Quests not possible are left out. Out-of-egg hatch quests and
//quests set to reroll in the exclusions table are rerolled. A quest with several table rows
//gets the action of each row, in table order.
BBQuestPlan plan_quests(
    Logger& logger,
    const BBQOption& BBQ_OPTIONS,
    const std::vector<BBQuests>& quest_list,
    uint8_t eggs_hatched
);

//The steps of rerolling one quest on the quest panel. They are injected so that the reroll
//(which quest it pays for, when it gives up) runs in tests without a Switch.
class BBQRerollActions{
public:
    virtual ~BBQRerollActions() = default;

    //Open the quest panel from the overworld. Returns false if it did not open.
    virtual bool open_panel() = 0;

    //Press A on the quest under the cursor, which opens its reroll menu. Returns the slot of
    //the quest the menu opened next to, or nothing if no menu was seen.
    virtual std::optional<size_t> open_reroll_menu() = 0;

    //Close the reroll menu without rerolling.
    virtual void close_reroll_menu() = 0;

    //Move the cursor down this many slots, or up if negative.
    virtual void move_cursor(int slots) = 0;

    //Choose to reroll in the open menu. This spends BP.
    virtual void confirm_reroll() = 0;

    //Close the quest panel and return to the overworld.
    virtual void close_panel() = 0;

    //Open the quest panel and read the quests on it, as read_quests() does.
    virtual std::vector<BBQuests> read_quests() = 0;
};

enum class BBQRerollResult{
    Rerolled,       //The quest in the slot was replaced.
    PanelNotOpened, //Nothing was spent.
    MenuNotFound,   //Nothing was spent.
    NotReplaced,    //The reroll was confirmed, but the slot still shows the same quest.
};

//Reroll the quest in `slot` of `quests`, the quests last read from the panel. Only pays once
//the reroll menu is open next to that slot, then reads the panel again to check the quest was
//replaced. On Rerolled, `quests[slot]` is updated to the new quest.
BBQRerollResult reroll_quest(
    Logger& logger,
    BBQRerollActions& actions,
    std::vector<BBQuests>& quests,
    size_t slot
);

//Determine which quests are possible and which quests are not. (ex. no eggs, reroll egg hatcher.)
//Quests not possible are removed from the list. If the list is empty, then reroll all items.
std::vector<BBQuests> process_quest_list(
    const ProgramInfo& info,
    VideoStream& stream, ProControllerContext& context,
    const BBQOption& BBQ_OPTIONS,
    std::vector<BBQuests>& quest_list,
    uint8_t& eggs_hatched
);

//Take the current quest and calls the function to do it, then checks the quest was successful. Returns true if so.
bool process_and_do_quest(
    ProgramEnvironment& env,
    ConsoleHandle& console, ProControllerContext& context,
    BBQOption& BBQ_OPTIONS,
    BBQuests current_quest,
    uint8_t& eggs_hatched
);

//Iterate through TMs until a craftable one is found. Make the TM and return to position.
void quest_make_tm(
    const ProgramInfo& info,
    VideoStream& stream, ProControllerContext& context
);

//Do laps in central plaza.
void quest_travel_500(
    const ProgramInfo& info,
    VideoStream& stream, ProControllerContext& context
);

//Run around until you encounter a pokemon. Tera, then defeat it by spamming your first move.
void quest_tera_self_defeat(
    const ProgramInfo& info,
    ConsoleHandle& console, ProControllerContext& context,
    const BBQOption& BBQ_OPTIONS
);

//Sneak up on a pokemon
void quest_sneak_up(
    const ProgramInfo& info,
    ConsoleHandle& console, ProControllerContext& context,
    const BBQOption& BBQ_OPTIONS
);

//Kill a tera pokemon
void quest_wild_tera(
    const ProgramInfo& info,
    ConsoleHandle& console, ProControllerContext& context,
    const BBQOption& BBQ_OPTIONS
);

//Give bath
void quest_wash_pokemon(
    const ProgramInfo& info,
    VideoStream& stream, ProControllerContext& context
);

//Withdraw and hatch an egg
void quest_hatch_egg(
    ProgramEnvironment& env,
    ConsoleHandle& console, ProControllerContext& context,
    const BBQOption& BBQ_OPTIONS
);

//Make a sandwich of type flavor
void quest_sandwich(
    ProgramEnvironment& env,
    VideoStream& stream, ProControllerContext& context,
    const BBQOption& BBQ_OPTIONS,
    BBQuests current_quest
);

//Complete a tera raid battle
void quest_tera_raid(
    ProgramEnvironment& env,
    ConsoleHandle& console, ProControllerContext& context,
    BBQOption& BBQ_OPTIONS
);

//Defeat 10/30 pokemon in autobattle
void quest_auto_battle(
    ProgramEnvironment& env,
    VideoStream& stream, ProControllerContext& context,
    const BBQOption& BBQ_OPTIONS,
    BBQuests current_quest
);

}
}
}
#endif
