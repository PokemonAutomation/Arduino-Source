/*  Blueberry Quests
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_BlueberryQuests_H
#define PokemonAutomation_PokemonSV_BlueberryQuests_H

#include<vector>
#include "PokemonSV/Options/PokemonSV_BBQOption.h"

using namespace std;

namespace PokemonAutomation{
    class BotBaseContext;
    class ConsoleHandle;
    struct ProgramInfo;
namespace NintendoSwitch{
namespace PokemonSV{

//Basic blue quests
const std::set<std::string> blue_quests = {
    "auto-10", "make-tm", "pickup-10", "sneak-up", "photo-fly", "photo-swim", "photo-canyon", "photo-coastal", "photo-polar", "photo-savanna", "tera-self-defeat",
    "travel-500", "catch-any", "catch-normal", "catch-fighting", "catch-flying", "catch-poison", "catch-ground", "catch-rock", "catch-bug", "catch-ghost", "catch-steel",
    "catch-fire", "catch-water", "catch-grass", "catch-electric", "catch-psychic", "catch-ice", "catch-dragon", "catch-dark", "catch-fairy"
};

//Red quests that appear at the top when a player completes 10 blues.
//In multiplayer, check if picnic is active before attempting some as only one player can picnic at a time.
const std::set<std::string> red_quests = {
    "wash-pokemon", "wild-tera", "auto-30", "tera-raid", "sandwich-three", "bitter-sandwich", "sweet-sandwich", "salty-sandwich", "sour-sandwich", "spicy-sandwich", "hatch-egg",
    "photo-normal", "photo-fighting", "photo-flying", "photo-poison", "photo-ground", "photo-rock", "photo-bug", "photo-ghost", "photo-steel", "photo-fire", "photo-water",
    "photo-grass", "photo-electric", "photo-psychic", "photo-ice", "photo-dragon", "photo-dark", "photo-fairy"
};

//Multiplayer only. Appears at the top when 3 red quests are completed.
const std::set<std::string> gold_quests = {
    "ditto-central", "ditto-canyon", "ditto-coastal", "ditto-polar", "ditto-savanna", "group-canyon", "group-coastal", "group-polar", "group-savanna", "group-eyewear", "group-nonuniform",
    "group-masks", "sandwich-four", "catch-hint", "catch-hint2"
};

//Quests that are not currently supported. Gold quests currently excluded as this is singleplayer only right now.
const std::set<std::string> not_possible_quests = {
    "auto-10", "pickup-10", "sneak-up", "auto-30", "tera-raid"//, "photo-swim", "catch-any", "tera-self-defeat"
};

// Return to Central Plaza from anywhere in the map.
void return_to_plaza(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context);

//Open the quest panel and read in the current quests.
//Currently only supports singleplayer.
//For multiplayer, we will want keep track of which quest are gold/red and scroll cursor down until all of the current player's blue quests are in.
void read_quests(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context, BBQOption& BBQ_OPTIONS, std::vector<std::string>& quest_list);

//Determine which quests are possible and which quests are not. (ex. no eggs, reroll egg hatcher.)
//Quests not possible are removed from the list. If the list is empty, then reroll all items.
void process_quest_list(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context, BBQOption& BBQ_OPTIONS, std::vector<std::string>& quest_list, std::vector<std::string>& quests_to_do, int& eggs_hatched);

}
}
}
#endif
