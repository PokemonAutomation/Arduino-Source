/*  Blueberry Quests
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_BlueberryQuests_H
#define PokemonAutomation_PokemonSV_BlueberryQuests_H

#include <vector>
#include "NintendoSwitch/Controllers/NintendoSwitch_Controller.h"
#include "PokemonSV/Programs/ShinyHunting/PokemonSV_LetsGoTools.h"
#include "PokemonSV/Options/PokemonSV_BBQOption.h"

namespace PokemonAutomation{
    struct ProgramInfo;
namespace NintendoSwitch{
namespace PokemonSV{

enum class BBQuests{
    auto_10, make_tm, pickup_10, sneak_up, photo_fly, photo_swim, photo_canyon, photo_coastal, photo_polar, photo_savanna, tera_self_defeat,
    travel_500, catch_any, catch_normal, catch_fighting, catch_flying, catch_poison, catch_ground, catch_rock, catch_bug, catch_ghost, catch_steel,
    catch_fire, catch_water, catch_grass, catch_electric, catch_psychic, catch_ice, catch_dragon, catch_dark, catch_fairy,
    wash_pokemon, wild_tera, auto_30, tera_raid, sandwich_three, bitter_sandwich, sweet_sandwich, salty_sandwich, sour_sandwich, spicy_sandwich, hatch_egg,
    photo_normal, photo_fighting, photo_flying, photo_poison, photo_ground, photo_rock, photo_bug, photo_ghost, photo_steel, photo_fire, photo_water,
    photo_grass, photo_electric, photo_psychic, photo_ice, photo_dragon, photo_dark, photo_fairy,
    ditto_central, ditto_canyon, ditto_coastal, ditto_polar, ditto_savanna, group_canyon, group_coastal, group_polar, group_savanna, group_eyewear, group_nonuniform,
    group_masks, sandwich_four, catch_hint, catch_hint2,
    UnableToDetect
};

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
    BBQuests::UnableToDetect, BBQuests::pickup_10
};


// Open the BBQ panel and read the current amount of BP
int read_BP(const ProgramInfo& info, VideoStream& stream, SwitchControllerContext& context);

//Open the quest panel and read in the current quests.
//Currently only supports singleplayer.
//For multiplayer, we will want keep track of which quest are gold/red and scroll cursor down until all of the current player's blue quests are in.
std::vector<BBQuests> read_quests(
    const ProgramInfo& info,
    VideoStream& stream, SwitchControllerContext& context,
    const BBQOption& BBQ_OPTIONS
);

//Determine which quests are possible and which quests are not. (ex. no eggs, reroll egg hatcher.)
//Quests not possible are removed from the list. If the list is empty, then reroll all items.
std::vector<BBQuests> process_quest_list(
    const ProgramInfo& info,
    VideoStream& stream, SwitchControllerContext& context,
    const BBQOption& BBQ_OPTIONS,
    std::vector<BBQuests>& quest_list,
    uint8_t& eggs_hatched
);

//Take the current quest and calls the function to do it, then checks the quest was successful. Returns true if so.
bool process_and_do_quest(
    ProgramEnvironment& env,
    VideoStream& stream, SwitchControllerContext& context,
    BBQOption& BBQ_OPTIONS,
    BBQuests current_quest,
    uint8_t& eggs_hatched
);

//Iterate through TMs until a craftable one is found. Make the TM and return to position.
void quest_make_tm(
    const ProgramInfo& info,
    VideoStream& stream, SwitchControllerContext& context
);

//Do laps in central plaza.
void quest_travel_500(
    const ProgramInfo& info,
    VideoStream& stream, SwitchControllerContext& context
);

//Run around until you encounter a pokemon. Tera, then defeat it by spamming your first move.
void quest_tera_self_defeat(
    const ProgramInfo& info,
    VideoStream& stream, SwitchControllerContext& context,
    const BBQOption& BBQ_OPTIONS
);

//Sneak up on a pokemon
void quest_sneak_up(
    const ProgramInfo& info,
    VideoStream& stream, SwitchControllerContext& context,
    const BBQOption& BBQ_OPTIONS
);

//Kill a tera pokemon
void quest_wild_tera(
    const ProgramInfo& info,
    VideoStream& stream, SwitchControllerContext& context,
    const BBQOption& BBQ_OPTIONS
);

//Give bath
void quest_wash_pokemon(
    const ProgramInfo& info,
    VideoStream& stream, SwitchControllerContext& context
);

//Withdraw and hatch an egg
void quest_hatch_egg(
    const ProgramInfo& info,
    VideoStream& stream, SwitchControllerContext& context,
    const BBQOption& BBQ_OPTIONS
);

//Make a sandwich of type flavor
void quest_sandwich(
    ProgramEnvironment& env,
    VideoStream& stream, SwitchControllerContext& context,
    const BBQOption& BBQ_OPTIONS,
    BBQuests current_quest
);

//Complete a tera raid battle
void quest_tera_raid(
    ProgramEnvironment& env,
    VideoStream& stream, SwitchControllerContext& context,
    BBQOption& BBQ_OPTIONS
);

//Defeat 10/30 pokemon in autobattle
void quest_auto_battle(
    ProgramEnvironment& env,
    VideoStream& stream, SwitchControllerContext& context,
    const BBQOption& BBQ_OPTIONS,
    BBQuests current_quest
);

}
}
}
#endif
