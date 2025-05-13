/*  BBQ Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <map>
#include "PokemonSV_BBQOption.h"
#include "PokemonSV/Inference/Boxes/PokemonSV_IvJudgeReader.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

//Does not include multiplayer quests
const EnumDropdownDatabase<BBQuests>& BBQuests_database(){
    static EnumDropdownDatabase<BBQuests> database{
        {BBQuests::auto_10,         "auto-10",          "Defeat 10 wild Pokemon using Auto Battle!"},
        {BBQuests::make_tm,         "make-tm",          "Make yourself a TM!"},
        //{BBQuests::pickup_10,       "pickup-10",        "Pick up items on the ground 10 times!"},
        {BBQuests::sneak_up,        "sneak-up",         "Successfully sneak up on 1 Pokemon and surprise them with a battle!"},
        {BBQuests::photo_fly,       "photo-fly",        "Take a photo of a wild Pokemon in flight!"},
        {BBQuests::photo_swim,      "photo-swim",       "Take a photo of a wild Pokemon that is swimming!"},
        {BBQuests::photo_canyon,    "photo-canyon",     "Take a photo of a wild Pokemon in the Canyon Biome!"},
        {BBQuests::photo_coastal,   "photo-coastal",    "Take a photo of a wild Pokemon in the Coastal Biome!"},
        {BBQuests::photo_polar,     "photo-polar",      "Take a photo of a wild Pokemon in the Polar Biome!"},
        {BBQuests::photo_savanna,   "photo-savanna",    "Take a photo of a wild Pokemon in the Savanna Biome!"},
        {BBQuests::tera_self_defeat,"tera-self-defeat", "Terastallize your Pokemon to defeat a wild Pokemon!"},
        {BBQuests::travel_500,      "travel-500",       "Travel over 500 yards!"},
        {BBQuests::catch_any,       "catch-any",        "Catch 1 Pokemon!"},
        {BBQuests::catch_normal,    "catch-normal",     "Catch 1 Normal-type Pokemon!"},
        {BBQuests::catch_fighting,   "catch-fighting",   "Catch 1 Fighting-type Pokemon!"},
        {BBQuests::catch_flying,    "catch-flying",     "Catch 1 Flying-type Pokemon!"},
        {BBQuests::catch_poison,    "catch-poison",     "Catch 1 Poison-type Pokemon!"},
        {BBQuests::catch_ground,    "catch-ground",     "Catch 1 Ground-type Pokemon!"},
        {BBQuests::catch_rock,      "catch-rock",       "Catch 1 Rock-type Pokemon!"},
        {BBQuests::catch_bug,       "catch-bug",        "Catch 1 Bug-type Pokemon!"},
        {BBQuests::catch_ghost,     "catch-ghost",      "Catch 1 Ghost-type Pokemon!"},
        {BBQuests::catch_steel,     "catch-steel",      "Catch 1 Steel-type Pokemon!"},
        {BBQuests::catch_fire,      "catch-fire",       "Catch 1 Fire-type Pokemon!"},
        {BBQuests::catch_water,     "catch-water",      "Catch 1 Water-type Pokemon!"},
        {BBQuests::catch_grass,     "catch-grass",      "Catch 1 Grass-type Pokemon!"},
        {BBQuests::catch_electric,  "catch-electric",   "Catch 1 Electric-type Pokemon!"},
        {BBQuests::catch_psychic,   "catch-psychic",    "Catch 1 Psychic-type Pokemon!"},
        {BBQuests::catch_ice,       "catch-ice",        "Catch 1 Ice-type Pokemon!"},
        {BBQuests::catch_dragon,    "catch-dragon",     "Catch 1 Dragon-type Pokemon!"},
        {BBQuests::catch_dark,      "catch-dark",       "Catch 1 Dark-type Pokemon!"},
        {BBQuests::catch_fairy,     "catch-fairy",      "Catch 1 Fairy-type Pokemon!"},
        {BBQuests::wash_pokemon,    "wash-pokemon",     "Give your Pokemon a nice washing!"},
        {BBQuests::wild_tera,       "wild-tera",        "Battle a wild Tera Pokemon!"},
        {BBQuests::auto_30,         "auto-30",          "Defeat 30 wild Pokemon using Auto Battle!"},
        {BBQuests::tera_raid,       "tera-raid",        "Claim victory in a Tera Raid Battle!"},
        {BBQuests::sandwich_three,  "sandwich-three",   "Make a sandwich that uses at least 3 ingredients!"},
        {BBQuests::bitter_sandwich, "bitter-sandwich",  "Make a bitter sandwich!"},
        {BBQuests::sweet_sandwich,  "sweet-sandwich",   "Make a sweet sandwich!"},
        {BBQuests::salty_sandwich,  "salty-sandwich",   "Make a salty sandwich!"},
        {BBQuests::sour_sandwich,   "sour-sandwich",    "Make a sour sandwich!"},
        {BBQuests::spicy_sandwich,  "spicy-sandwich",   "Make a spicy sandwich!"},
        //{BBQuests::hatch_egg,       "hatch-egg",        "Hatch a Pokemon Egg!"},
        {BBQuests::photo_normal,    "photo-normal",     "Take a photo of a wild Normal-type Pokemon!"},
        {BBQuests::photo_fighting,  "photo-fighting",   "Take a photo of a wild Fighting-type Pokemon!"},
        {BBQuests::photo_flying,    "photo-flying",     "Take a photo of a wild Flying-type Pokemon!"},
        {BBQuests::photo_poison,    "photo-poison",     "Take a photo of a wild Poison-type Pokemon!"},
        {BBQuests::photo_ground,    "photo-ground",     "Take a photo of a wild Ground-type Pokemon!"},
        {BBQuests::photo_rock,      "photo-rock",       "Take a photo of a wild Rock-type Pokemon!"},
        {BBQuests::photo_bug,       "photo-bug",        "Take a photo of a wild Bug-type Pokemon!"},
        {BBQuests::photo_ghost,     "photo-ghost",      "Take a photo of a wild Ghost-type Pokemon!"},
        {BBQuests::photo_steel,     "photo-steel",      "Take a photo of a wild Steel-type Pokemon!"},
        {BBQuests::photo_fire,      "photo-fire",       "Take a photo of a wild Fire-type Pokemon!"},
        {BBQuests::photo_water,     "photo-water",      "Take a photo of a wild Water-type Pokemon!"},
        {BBQuests::photo_grass,     "photo-grass",      "Take a photo of a wild Grass-type Pokemon!"},
        {BBQuests::photo_electric,  "photo-electric",   "Take a photo of a wild Electric-type Pokemon!"},
        {BBQuests::photo_psychic,   "photo-psychic",    "Take a photo of a wild Psychic-type Pokemon!"},
        {BBQuests::photo_ice,       "photo-ice",        "Take a photo of a wild Ice-type Pokemon!"},
        {BBQuests::photo_dragon,    "photo-dragon",     "Take a photo of a wild Dragon-type Pokemon!"},
        {BBQuests::photo_dark,      "photo-dark",       "Take a photo of a wild Dark-type Pokemon!"},
        {BBQuests::photo_fairy,     "photo-fairy",      "Take a photo of a wild Fairy-type Pokemon!"},
    };
    return database;
}

const EnumDropdownDatabase<BBQAction>& BBQAction_database(){
    static EnumDropdownDatabase<BBQAction> database{
        {BBQAction::run,    "run",      "Run Quest"},
        {BBQAction::skip,   "skip",     "Skip"},
        {BBQAction::reroll, "reroll",   "Reroll"},
    };
    return database;
}

BBQuestTableRow::BBQuestTableRow(EditableTableOption& parent_table)
    : EditableTableRow(parent_table)
    , quest(BBQuests_database(), LockMode::UNLOCK_WHILE_RUNNING, BBQuests::auto_10)
    , action(BBQAction_database(), LockMode::UNLOCK_WHILE_RUNNING, BBQAction::skip)
{
    PA_ADD_OPTION(quest);
    PA_ADD_OPTION(action);
}
std::unique_ptr<EditableTableRow> BBQuestTableRow::clone() const{
    std::unique_ptr<BBQuestTableRow> ret(new BBQuestTableRow(parent()));
    ret->quest.set(quest);
    ret->action.set(action);
    return ret;
}

BBQuestTable::BBQuestTable()
    : EditableTableOption_t<BBQuestTableRow>(
        "<b>Quest Exclusions:</b><br>"
        "<b>Warning: Skipping Bonus quests will block the bonus slot.</b><br>"
        "Exclude the quests in the table. If you want to skip a quest, select it below. "
        "(Quests can be explicitly included, but this is not necessary.) "
        "Do not exclude too many quests, as rerolling costs BP. "
        "The program will automatically reroll all quests if none are possible, but will not handle being out of BP. "
        "Does not include egg hatching or pickup quests, as eggs are handled in other options and pickup is not possible. ",
        //"Duplicates of the same quest",
        //duplicates will work in table order. might lead to weird behavior like rerolling and then marking a quest as complete.
        //won't break the program though.
        LockMode::LOCK_WHILE_RUNNING,
        make_defaults()
    )
{}

std::vector<std::string> BBQuestTable::make_header() const{
    return {
        "Quest",
        "Action",
    };
}
std::vector<std::unique_ptr<EditableTableRow>> BBQuestTable::make_defaults(){
    std::vector<std::unique_ptr<EditableTableRow>> ret;
    ret.emplace_back(new BBQuestTableRow(*this));
    return ret;
}


BBQOption::BBQOption(OCR::LanguageOCROption* language_option)
    : GroupOption("Blueberry Quests", LockMode::UNLOCK_WHILE_RUNNING)
    , m_language_owner(language_option == nullptr
        ? new OCR::LanguageOCROption(
            "<b>Game Language:</b><br>This is required to read quests.",
            IV_READER().languages(),
            LockMode::LOCK_WHILE_RUNNING,
            true
        )
        : nullptr
    )
    , LANGUAGE(language_option == nullptr ? *m_language_owner : *language_option)
    , NUM_QUESTS(
        "<b>Number of Quests to run:</b>",
        LockMode::UNLOCK_WHILE_RUNNING,
        1000
    )
    , SAVE_NUM_QUESTS(
        "<b>Save and reset the game after attempting this many quests:</b><br>This preserves progress and prevents potential game lags from long runs.<br>0 disables this option.",
        LockMode::UNLOCK_WHILE_RUNNING, 50
    )
    , INVERTED_FLIGHT(
        "<b>Inverted Flight:</b><br>Check this box if inverted flight controls are set.",
        LockMode::LOCK_WHILE_RUNNING,
        false
    )
    , QUICKBALL(
        "<b>Catch Quest - Throw Quick Ball:</b><br>When attempting to catch a non-tera Pokemon, use a Quick Ball on the first turn.",
        LockMode::UNLOCK_WHILE_RUNNING,
        true
    )
    , BALL_SELECT(
        "<b>Catch Quest - Ball Select:</b><br>What ball to use when performing a catch quest.",
        LockMode::UNLOCK_WHILE_RUNNING,
        "ultra-ball"
    )
    , NUM_EGGS(
        "<b>Hatch Quest - Number of Eggs:</b><br>Amount of eggs located in your current box.",
        LockMode::UNLOCK_WHILE_RUNNING, 30, 0, 30
    )
    , OUT_OF_EGGS(
        "<b>Hatch Quest - Zero Eggs:</b><br>When out of eggs to hatch, do the selected option.",
        {
            {OOEggs::Stop,      "stop",         "Stop Program"},
            {OOEggs::Reroll,    "reroll",       "Reroll Quest - Costs 50BP"},
            {OOEggs::KeepGoing, "keep-going",   "Keep Going - Warning: Bonus(Red) quests will be blocked!"},
        },
        LockMode::UNLOCK_WHILE_RUNNING,
        OOEggs::Stop
    )
    , FIX_TIME_FOR_HATCH(
        "<b>Hatch Quest - Fix time for eggs:</b><br>Fix the time before doing a hatch egg quest.",
        LockMode::UNLOCK_WHILE_RUNNING, false
    )
    , CATCH_ON_WIN(true)
    , NUM_RETRIES(
        "<b>Number of Retries:</b><br>Reattempt a quest this many times if it fails.",
        LockMode::UNLOCK_WHILE_RUNNING, 1, 0, 5
    )
    , FIX_TIME_WHEN_DONE(
        "<b>Fix Time when Done:</b><br>Fix the time after the program finishes.",
        LockMode::UNLOCK_WHILE_RUNNING, false
    )
{
    if (m_language_owner){
        PA_ADD_OPTION(LANGUAGE);
    }
    PA_ADD_OPTION(NUM_QUESTS);
    PA_ADD_OPTION(SAVE_NUM_QUESTS);
    PA_ADD_OPTION(QUEST_EXCLUSIONS);
    PA_ADD_OPTION(INVERTED_FLIGHT);
    PA_ADD_OPTION(QUICKBALL);
    PA_ADD_OPTION(BALL_SELECT);
    PA_ADD_OPTION(NUM_EGGS);
    PA_ADD_OPTION(OUT_OF_EGGS);
    PA_ADD_OPTION(FIX_TIME_FOR_HATCH);
    PA_ADD_OPTION(BATTLE_AI);
    PA_ADD_OPTION(CATCH_ON_WIN);
    PA_ADD_OPTION(NUM_RETRIES);
    PA_ADD_OPTION(FIX_TIME_WHEN_DONE);
}


}
}
}
