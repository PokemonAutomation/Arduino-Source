/*  Item Printer RNG
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/PrettyPrint.h"
#include "Common/Qt/TimeQt.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/Tools/StatsTracking.h"
#include "CommonFramework/Tools/VideoResolutionCheck.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
//#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "NintendoSwitch/Inference/NintendoSwitch_DateReader.h"
#include "NintendoSwitch/Programs/NintendoSwitch_GameEntry.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSwSh/Commands/PokemonSwSh_Commands_DateSpam.h"
#include "PokemonSwSh/Inference/PokemonSwSh_IvJudgeReader.h"
#include "PokemonSV/PokemonSV_Settings.h"
#include "PokemonSV/Inference/PokemonSV_WhiteButtonDetector.h"
#include "PokemonSV/Inference/Dialogs/PokemonSV_DialogDetector.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_OverworldDetector.h"
//#include "PokemonSV/Programs/PokemonSV_GameEntry.h"
#include "PokemonSV_ItemPrinterRNG.h"
#include <iostream>

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{



static const EnumDatabase<ItemPrinterJobs>& ItemPrinterJobs_Database(){
    static const EnumDatabase<ItemPrinterJobs> database({
        {ItemPrinterJobs::Jobs_1, "1", "1 Job"},
        {ItemPrinterJobs::Jobs_5, "5", "5 Jobs"},
        {ItemPrinterJobs::Jobs_10, "10", "10 Jobs"},
    });
    return database;
}

static const EnumDatabase<ItemPrinterItems>& ItemPrinterItems_Database(){
    static const EnumDatabase<ItemPrinterItems> database({
        {ItemPrinterItems::NONE, "none", "---"},
        {ItemPrinterItems::ITEM_BONUS, "item-bonus", "Item Bonus"},
        {ItemPrinterItems::BALL_BONUS, "ball-bonus", "Ball Bonus"},

        // ordered by category
        {ItemPrinterItems::ABILITY_PATCH, "ability-patch", "Ability patch"},
        {ItemPrinterItems::EXP_CANDY, "exp-candy", "Exp Candy"},

        {ItemPrinterItems::BUG_TERA, "bug-tera", "Bug Tera Shard"},
        {ItemPrinterItems::DARK_TERA, "dark-tera", "Dark Tera Shard"},
        {ItemPrinterItems::DRAGON_TERA, "dragon-tera", "Dragon Tera Shard"},
        {ItemPrinterItems::ELECTRIC_TERA, "electric-tera", "Electric Tera Shard"},
        {ItemPrinterItems::FAIRY_TERA, "fairy-tera", "Fairy Tera Shard"},
        {ItemPrinterItems::FIGHTING_TERA, "fighting-tera", "Fighting Tera Shard"},
        {ItemPrinterItems::FIRE_TERA, "fire-tera", "Fire Tera Shard"},
        {ItemPrinterItems::FLYING_TERA, "flying-tera", "Flying Tera Shard"},
        {ItemPrinterItems::GHOST_TERA, "ghost-tera", "Ghost Tera Shard"},
        {ItemPrinterItems::GRASS_TERA, "grass-tera", "Grass Tera Shard"},
        {ItemPrinterItems::GROUND_TERA, "ground-tera", "Ground Tera Shard"},
        {ItemPrinterItems::ICE_TERA, "ice-tera", "Ice Tera Shard"},
        {ItemPrinterItems::NORMAL_TERA, "normal-tera", "Normal Tera Shard"},
        {ItemPrinterItems::POISON_TERA, "poison-tera", "Poison Tera Shard"},
        {ItemPrinterItems::PSYCHIC_TERA, "psychic-tera", "Psychic Tera Shard"},
        {ItemPrinterItems::ROCK_TERA, "rock-tera", "Rock Tera Shard"},
        {ItemPrinterItems::STEEL_TERA, "steel-tera", "Steel Tera Shard"},
        {ItemPrinterItems::STELLAR_TERA, "stellar-tera", "Stellar Tera Shard"},
        {ItemPrinterItems::WATER_TERA, "water-tera", "Water Tera Shard"},
        
        {ItemPrinterItems::BEAST_BALL, "beast-ball", "Beast Ball"},
        {ItemPrinterItems::DREAM_BALL, "dream-ball", "Dream Ball"},
        {ItemPrinterItems::FAST_BALL, "fast-ball", "Fast Ball"},
        {ItemPrinterItems::FRIEND_BALL, "friend-ball", "Friend Ball"},
        {ItemPrinterItems::HEAVY_BALL, "heavy-ball", "Heavy Ball"},
        {ItemPrinterItems::LEVEL_BALL, "level-ball", "Level Ball"},
        {ItemPrinterItems::LOVE_BALL, "love-ball", "Love Ball"},
        {ItemPrinterItems::LURE_BALL, "lure-ball", "Lure Ball"},
        {ItemPrinterItems::MASTER_BALL, "master-ball", "Master Ball"},
        {ItemPrinterItems::MOON_BALL, "moon-ball", "Moon Ball"},
        {ItemPrinterItems::SAFARI_BALL, "safari-ball", "Safari Ball"},
        {ItemPrinterItems::SPORT_BALL, "sport-ball", "Sport Ball"},

    });
    return database;
}

ItemPrinterRngRow::~ItemPrinterRngRow(){
    chain.remove_listener(*this);
}
ItemPrinterRngRow::ItemPrinterRngRow()
    : chain(LockMode::UNLOCK_WHILE_RUNNING, false)
    , date(
        LockMode::UNLOCK_WHILE_RUNNING,
        DateTimeOption::DATE_HOUR_MIN_SEC,
        DateTime{2000, 1, 1, 0, 1, 0},
        DateTime{2060, 12, 31, 23, 59, 59},
        DateTime{2024, 4, 22, 13, 27, 9}
    )
    , jobs(
        ItemPrinterJobs_Database(),
        LockMode::UNLOCK_WHILE_RUNNING,
        ItemPrinterJobs::Jobs_1
    )
    , desired_item(
        ItemPrinterItems_Database(),
        LockMode::UNLOCK_WHILE_RUNNING,
        ItemPrinterItems::NONE
    )
    , prev_desired_item(ItemPrinterItems::NONE)
    // , prev_desired_item_is_none(true)
{
    PA_ADD_OPTION(chain);
    PA_ADD_OPTION(date);
    PA_ADD_OPTION(jobs);
    PA_ADD_OPTION(desired_item);
    
    date.add_listener(*this);
    chain.add_listener(*this);
    
    jobs.add_listener(*this);
    desired_item.add_listener(*this);
}
ItemPrinterRngRow::ItemPrinterRngRow(bool p_chain, const DateTime& p_date, ItemPrinterJobs p_jobs)
    : ItemPrinterRngRow()
{
    chain = p_chain;
    date.set(p_date);
    jobs.set(p_jobs);
}
ItemPrinterRngRowSnapshot ItemPrinterRngRow::snapshot() const{
    return ItemPrinterRngRowSnapshot{chain, date, jobs};
}
std::unique_ptr<EditableTableRow> ItemPrinterRngRow::clone() const{
    std::unique_ptr<ItemPrinterRngRow> ret(new ItemPrinterRngRow());
    ret->chain = (bool)chain;
    ret->date.set(date);
    ret->jobs.set(jobs);
    return ret;
}


// - Specific item seeds taken from Anubis's Item printer seeds
//   - https://gist.github.com/Lusamine/112d4230919fadd254f0e6dfca850471
// - How I chose the specific item seeds
//   - seeds (and adjacent seeds +/- 2) that didn't also trigger item/ball bonuses. This is because
// accidentally triggering a ball bonus, may prevent you from intentionally triggering an item bonus
// on the next job, and vice versa for the item bonus.
//   - seconds value: minimum of 8. ideally less than 20.
// - Confirmation of seeds done with Kurt's ItemPrinterDeGacha tool
void ItemPrinterRngRow::set_seed_based_on_desired_item(){
    int64_t seed;
    switch(desired_item) {
    case ItemPrinterItems::ITEM_BONUS:
        // 2044-05-06 15:33:08
        // Calcium
        seed = 2346161588;
        jobs.set(ItemPrinterJobs::Jobs_1);
        break;
    case ItemPrinterItems::BALL_BONUS:
        // 2024-06-04 00:37:08 
        // Full Restore
        seed = 1717461428;
        jobs.set(ItemPrinterJobs::Jobs_1);
        break;        
    case ItemPrinterItems::ABILITY_PATCH:
        // 2000-05-12 22:59:28
        // total 8 Ability Patch
        // x18 Dark Tera Shard; x2 Ability Patch; x2 Ability Patch; x2 Ability Patch; x2 Ability Patch
        seed = 958172368;
        jobs.set(ItemPrinterJobs::Jobs_5);
        break;
    case ItemPrinterItems::EXP_CANDY:
        // 2033-02-08 10:48:09
        // x5 Exp. Candy XL; x2 Protein: x10 Exp. Candy L; x4 Exp. Candy XL; x5 Exp. Candy XL
        // 520000 exp
        seed = 1991472489;
        jobs.set(ItemPrinterJobs::Jobs_5);
        break;
    case ItemPrinterItems::NORMAL_TERA:
        // 2041-03-11 01:10:10
        // x20 Normal Tera Shard; x14 Normal Tera Shard; x8 Normal Tera Shard; x19 Normal Tera Shard; x6 Normal Tera Shard
        // 67 Normal Tera Shards
        seed = 2246577010;
        jobs.set(ItemPrinterJobs::Jobs_5);
        break;
    case ItemPrinterItems::FIRE_TERA:
        // 2054-08-05 03:35:10
        // 74 Fire Tera Shards, 74 total Tera Shards
        // x20 Fire Tera Shard; x18 Fire Tera Shard; x20 Fire Tera Shard; x16 Fire Tera Shard; x10 Honey
        seed = 2669513710;
        jobs.set(ItemPrinterJobs::Jobs_5);
        break;
    case ItemPrinterItems::WATER_TERA:
        // 2046-09-02 18:42:14
        // 60 Water Tera Shards, 77 total Tera Shards
        // x20 Water Tera Shard; x4 Full Restore; x20 Water Tera Shard; x17 Flying Tera Shard; x20 Water Tera Shard
        seed = 2419526534;
        jobs.set(ItemPrinterJobs::Jobs_5);
        break;
    case ItemPrinterItems::ELECTRIC_TERA:
        // 2011-03-14 16:54:13
        // 69 Electric Tera Shards, 69 total Tera Shards
        // x13 Electric Tera Shard; x2 Light Clay; x20 Electric Tera Shard; x20 Electric Tera Shard; x16 Electric Tera Shard
        seed = 1300121653;
        jobs.set(ItemPrinterJobs::Jobs_5);
        break;
    case ItemPrinterItems::GRASS_TERA:
        // 2009-11-15 23:04:10
        // 61 Grass Tera Shards, 61 total Tera Shards
        // x15 Grass Tera Shard; x6 Tiny Mushroom; x19 Grass Tera Shard; x17 Grass Tera Shard; x10 Grass Tera Shard
        seed = 1258326250;
        jobs.set(ItemPrinterJobs::Jobs_5);
        break;
    case ItemPrinterItems::ICE_TERA:
        // 2051-05-10 10:07:10
        // 67 Ice Tera Shards, 67 total Tera Shards
        // x19 Ice Tera Shard; x8 Ice Tera Shard; x20 Ice Tera Shard; x20 Ice Tera Shard; x1 Metal Alloy
        seed = 2567326030;
        jobs.set(ItemPrinterJobs::Jobs_5);
        break;
    case ItemPrinterItems::FIGHTING_TERA:
        // 2017-11-14 18:02:15
        // 66 Fighting Tera Shards, 66 total Tera Shards
        // x20 Fighting Tera Shard; x16 Fighting Tera Shard; x10 Fighting Tera Shard; x20 Fighting Tera Shard; x3 Shiny Stone
        seed = 1510682535;
        jobs.set(ItemPrinterJobs::Jobs_5);
        break;
    case ItemPrinterItems::POISON_TERA:
        // 2034-09-09 06:36:12
        // 70 Poison Tera Shards, 70 total Tera Shards
        // x2 Electirizer; x16 Poison Tera Shard; x18 Poison Tera Shard; x19 Poison Tera Shard; x17 Poison Tera Shard
        seed = 2041396572;
        jobs.set(ItemPrinterJobs::Jobs_5);
        break;
    case ItemPrinterItems::GROUND_TERA:
        // 2016-05-15 14:18:09
        // 65 Ground Tera Shards, 65 total Tera Shards
        // x5 PP Up; x14 Ground Tera Shard; x17 Ground Tera Shard; x15 Ground Tera Shard; x19 Ground Tera Shard
        seed = 1463321889;
        jobs.set(ItemPrinterJobs::Jobs_5);
        break;
    case ItemPrinterItems::FLYING_TERA:
        // 2020-02-02 08:51:40
        // 64 Flying Tera Shards, 64 total Tera Shards
        // x11 Flying Tera Shard; x15 Flying Tera Shard; x14 Flying Tera Shard; x19 Flying Tera Shard; x5 Flying Tera Shard
        seed = 1580633500;
        jobs.set(ItemPrinterJobs::Jobs_5);
        break;
    case ItemPrinterItems::PSYCHIC_TERA:
        // 2039-06-02 02:15:13
        // 67 Psychic Tera Shards, 67 total Tera Shards
        // x15 Psychic Tera Shard
        // x2 Never-Melt Ice
        // x14 Psychic Tera Shard
        // x18 Psychic Tera Shard
        // x20 Psychic Tera Shard
        seed = 2190593713;
        jobs.set(ItemPrinterJobs::Jobs_5);
        break;
    case ItemPrinterItems::BUG_TERA:
        // 2047-11-23 08:17:28
        // 67 Bug Tera Shards, 67 total Tera Shards
        // x2 Ether
        // x19 Bug Tera Shard
        // x17 Bug Tera Shard
        // x16 Bug Tera Shard
        // x15 Bug Tera Shard
        seed = 2458109848;
        jobs.set(ItemPrinterJobs::Jobs_5);
        break;
    case ItemPrinterItems::ROCK_TERA:
        // 2058-10-16 00:38:09
        // 60 Rock Tera Shards, 60 total Tera Shards
        // x20 Rock Tera Shard
        // x20 Rock Tera Shard
        // x8 Pretty Feather
        // x20 Rock Tera Shard
        // x9 Stardust
        seed = 2801954289;
        jobs.set(ItemPrinterJobs::Jobs_5);
        break;
    case ItemPrinterItems::GHOST_TERA:
        // 2017-11-05 17:51:11
        // 61 Ghost Tera Shards, 61 total Tera Shards
        // x19 Ghost Tera Shard
        // x18 Ghost Tera Shard
        // x6 Ghost Tera Shard
        // x2 Magnet
        // x18 Ghost Tera Shard
        seed = 1509904271;
        jobs.set(ItemPrinterJobs::Jobs_5);
        break;
    case ItemPrinterItems::DRAGON_TERA:
        // 2038-04-23 22:25:09
        // 68 Dragon Tera Shards, 68 total Tera Shards
        // x1 Ability Patch
        // x12 Dragon Tera Shard
        // x19 Dragon Tera Shard
        // x20 Dragon Tera Shard
        // x17 Dragon Tera Shard
        seed = 2155674309;
        jobs.set(ItemPrinterJobs::Jobs_5);
        break;
    case ItemPrinterItems::DARK_TERA:
        // 2050-05-26 12:20:13
        // 60 Dark Tera Shards, 60 total Tera Shards
        // x10 Dark Tera Shard
        // x14 Dark Tera Shard
        // x16 Dark Tera Shard
        // x20 Dark Tera Shard
        // x2 Prism Scale
        seed = 2537180413;
        jobs.set(ItemPrinterJobs::Jobs_5);
        break;             
    case ItemPrinterItems::STEEL_TERA:
        // 2025-12-18 14:16:09
        // 60 Steel Tera Shards, 60 total Tera Shards
        // x20 Steel Tera Shard
        // x12 Steel Tera Shard
        // x19 Steel Tera Shard
        // x9 Steel Tera Shard
        // x2 Berry Sweet
        seed = 1766067369;
        jobs.set(ItemPrinterJobs::Jobs_5);
        break;
    case ItemPrinterItems::FAIRY_TERA:
        // 2039-03-06 12:56:14
        // 60 Fairy Tera Shards, 67 total Tera Shards
        // x20 Fairy Tera Shard
        // x20 Fairy Tera Shard
        // x7 Water Tera Shard
        // x6 Tiny Bamboo Shoot
        // x20 Fairy Tera Shard
        seed = 2183028974;
        jobs.set(ItemPrinterJobs::Jobs_5);
        break;
    case ItemPrinterItems::STELLAR_TERA:
        // 2006-12-09 08:47:14
        // 44 Stellar Tera Shards, 44 total Tera Shards
        // x15 Stellar Tera Shard
        // x3 Max Revive
        // x14 Stellar Tera Shard
        // x15 Stellar Tera Shard
        // x1 Clover Sweet
        seed = 1165654034;
        jobs.set(ItemPrinterJobs::Jobs_5);
        break;
    case ItemPrinterItems::MASTER_BALL:
        // 2005-11-07 11:32:08
        // x1 Master Ball
        // x1 Master Ball
        // x5 Great Ball
        // x1 Master Ball
        // x1 Master Ball
        seed = 1131363128;
        jobs.set(ItemPrinterJobs::Jobs_5);
        break;
    case ItemPrinterItems::SAFARI_BALL:
        // 2049-01-02 01:20:22
        seed = 2493163222;
        jobs.set(ItemPrinterJobs::Jobs_5);
        break;
    case ItemPrinterItems::FAST_BALL:
        // 2034-09-01 02:33:39
        seed = 2040690819;
        jobs.set(ItemPrinterJobs::Jobs_5);
        break;
    case ItemPrinterItems::LEVEL_BALL:
        // 2045-03-17 10:05:38
        seed = 2373357938;
        jobs.set(ItemPrinterJobs::Jobs_5);
        break;   
    case ItemPrinterItems::LURE_BALL:
        // 2057-11-21 04:28:37
        seed = 2773542517;
        jobs.set(ItemPrinterJobs::Jobs_5);
        break;
    case ItemPrinterItems::HEAVY_BALL:
        // 2029-07-24 11:53:11
        seed = 1879588391;
        jobs.set(ItemPrinterJobs::Jobs_5);
        break;
    case ItemPrinterItems::LOVE_BALL:
        // 2023-05-22 05:36:11
        seed = 1684733771;
        jobs.set(ItemPrinterJobs::Jobs_5);
        break;
    case ItemPrinterItems::FRIEND_BALL:
        // 2046-12-11 02:23:17
        seed = 2428107797;
        jobs.set(ItemPrinterJobs::Jobs_5);
        break;          
    case ItemPrinterItems::MOON_BALL:
        // 2042-10-14 05:56:33
        seed = 2296878993;
        jobs.set(ItemPrinterJobs::Jobs_5);
        break;   
    case ItemPrinterItems::SPORT_BALL:
        // 2009-06-05 20:17:11
        seed = 1244233031;
        jobs.set(ItemPrinterJobs::Jobs_5);
        break;
    case ItemPrinterItems::DREAM_BALL:
        // 2054-03-22 09:31:21
        seed = 2657784681;
        jobs.set(ItemPrinterJobs::Jobs_5);
        break;
    case ItemPrinterItems::BEAST_BALL:
        // 2037-02-02 15:37:15
        seed = 2117201835;
        jobs.set(ItemPrinterJobs::Jobs_5);
        break;
    default:
        seed = -1;
    }

    if (seed == -1){
        return;
    }

    chain = false;
    DateTime set_date = from_seconds_since_epoch(seed);
    date.set(set_date);
}

void ItemPrinterRngRow::value_changed(){
    date.set_visibility(chain ? ConfigOptionState::DISABLED : ConfigOptionState::ENABLED);
    if (prev_desired_item != desired_item){ // check if desired_item has changed.
        set_seed_based_on_desired_item();
        prev_desired_item = desired_item;
        return;
    }
    // std::cout << chain << std::endl;
    // std::cout << "changed" << std::endl;
    desired_item.set(ItemPrinterItems::NONE);
}

ItemPrinterRngTable::ItemPrinterRngTable(std::string label)
    : EditableTableOption_t<ItemPrinterRngRow>(
        std::move(label),
        LockMode::UNLOCK_WHILE_RUNNING,
        make_defaults()
    )
{}
std::vector<std::string> ItemPrinterRngTable::make_header() const{
    return std::vector<std::string>{
        "Continue Previous?",
        "Date Seed",
        "Jobs to Print",
        "Desired Item",
    };
}
std::vector<std::unique_ptr<EditableTableRow>> ItemPrinterRngTable::make_defaults(){
    std::vector<std::unique_ptr<EditableTableRow>> ret;
    ret.emplace_back(std::make_unique<ItemPrinterRngRow>(false, DateTime{2039, 3, 21, 12, 14, 42}, ItemPrinterJobs::Jobs_1));
    ret.emplace_back(std::make_unique<ItemPrinterRngRow>(true, DateTime{2016, 5, 20, 2, 11, 13}, ItemPrinterJobs::Jobs_10));
    return ret;
}




ItemPrinterRNG_Descriptor::ItemPrinterRNG_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSV:ItemPrinterRNG",
        Pokemon::STRING_POKEMON + " SV", "Item Printer RNG",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSV/ItemPrinterRNG.md",
        "Farm the Item Printer using RNG Manipulation.",
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}

struct ItemPrinterRNG_Descriptor::Stats : public StatsTracker{
    Stats()
        : iterations(m_stats["Rounds"])
        , prints(m_stats["Prints"])
//        , total_jobs(m_stats["Total Jobs"])
        , seed_hits(m_stats["Seed Hits"])
        , seed_misses(m_stats["Seed Misses"])
        , errors(m_stats["Errors"])
    {
        m_display_order.emplace_back("Rounds");
        m_display_order.emplace_back("Prints");
//        m_display_order.emplace_back("Total Jobs", HIDDEN_IF_ZERO);
        m_display_order.emplace_back("Seed Hits", HIDDEN_IF_ZERO);
        m_display_order.emplace_back("Seed Misses", HIDDEN_IF_ZERO);
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);
    }
    std::atomic<uint64_t>& iterations;
    std::atomic<uint64_t>& prints;
//    std::atomic<uint64_t>& total_jobs;
    std::atomic<uint64_t>& seed_hits;
    std::atomic<uint64_t>& seed_misses;
    std::atomic<uint64_t>& errors;
};
std::unique_ptr<StatsTracker> ItemPrinterRNG_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}



ItemPrinterRNG::ItemPrinterRNG()
    : LANGUAGE(
        "<b>Game Language:</b>",
        PokemonSwSh::IV_READER().languages(),
        LockMode::UNLOCK_WHILE_RUNNING,
        true
    )
    , TOTAL_ROUNDS(
        "<b>Total Rounds:</b><br>Iterate the rounds table this many times before stopping the program.",
        LockMode::UNLOCK_WHILE_RUNNING, 10
    )
#if 0
    , DATE0(
        "<b>Initial Date:</b><br>Date/Time for the first print. This one should setup the bonus print.",
        LockMode::UNLOCK_WHILE_RUNNING,
        DateTimeOption::DATE_HOUR_MIN_SEC,
        DateTime{2000, 1, 1, 0, 1, 0},
        DateTime{2060, 12, 31, 23, 59, 59},
        DateTime{2024, 4, 22, 13, 27, 9}
    )
    , DATE1(
        "<b>Second Date:</b><br>Date/Time for the second print. This one gets you the main items.",
        LockMode::UNLOCK_WHILE_RUNNING,
        DateTimeOption::DATE_HOUR_MIN_SEC,
        DateTime{2000, 1, 1, 0, 1, 0},
        DateTime{2060, 12, 31, 23, 59, 59},
        DateTime{2016, 5, 20, 2, 11, 13}
    )
#endif
    , TABLE(
        "<b>Rounds Table:</b><br>Run the following prints in order and repeat. "
        "Changes to this table take effect the next time the table starts from the beginning."
    )
    , DELAY_MILLIS(
        "<b>Delay (Milliseconds):</b><br>"
        "The delay from when you press A to when the game reads the date for the seed. "
        "For OLED Switches, this delay is about 750 milliseconds. For older Switches, this delay is closer to 1750 milliseconds. "
        "You will need to experiment to find the correct delay.",
        LockMode::UNLOCK_WHILE_RUNNING, 750,
        0, 10000
    )
    , GO_HOME_WHEN_DONE(false)
    , FIX_TIME_WHEN_DONE(
        "<b>Fix Time When Done:</b><br>Fix the time after the program finishes.",
        LockMode::UNLOCK_WHILE_RUNNING, true
    )
    , NOTIFICATION_STATUS_UPDATE("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS_UPDATE,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_OPTION(LANGUAGE);
    PA_ADD_OPTION(TOTAL_ROUNDS);
//    PA_ADD_OPTION(DATE0);
//    PA_ADD_OPTION(DATE1);
    PA_ADD_OPTION(TABLE);
    PA_ADD_OPTION(DELAY_MILLIS);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(FIX_TIME_WHEN_DONE);
    PA_ADD_OPTION(NOTIFICATIONS);
}


void ItemPrinterRNG::run_print_at_date(
    SingleSwitchProgramEnvironment& env, BotBaseContext& context,
    const DateTime& date, ItemPrinterJobs jobs
) const{
    ItemPrinterRNG_Descriptor::Stats& stats = env.current_stats<ItemPrinterRNG_Descriptor::Stats>();

    bool printed = false;
    bool overworld_seen = false;
    while (true){
        context.wait_for_all_requests();

        OverworldWatcher overworld(COLOR_BLUE);
        AdvanceDialogWatcher dialog(COLOR_RED);
        PromptDialogWatcher prompt(COLOR_GREEN);
        DateChangeWatcher date_reader;
        WhiteButtonWatcher material(COLOR_GREEN, WhiteButton::ButtonX, {0.63, 0.93, 0.17, 0.06});
        int ret = wait_until(
            env.console, context, std::chrono::seconds(120),
            {
                overworld,
                dialog,
                prompt,
                date_reader,
                material,
            }
        );
        switch (ret){
        case 0:
            overworld_seen = true;
            if (printed){
                env.log("Detected overworld... (unexpected)", COLOR_RED);
                return;
            }
            env.log("Detected overworld... Starting print.");
            pbf_press_button(context, BUTTON_A, 20, 30);
            continue;

        case 1:
            env.log("Detected advance dialog.");
            pbf_press_button(context, BUTTON_B, 20, 30);
            continue;

        case 2:{
            env.log("Detected prompt dialog.");
            pbf_press_button(context, BUTTON_HOME, 10, GameSettings::instance().GAME_TO_HOME_DELAY);
            home_to_date_time(context, true, false);
            pbf_press_button(context, BUTTON_A, 10, 30);
            context.wait_for_all_requests();
            continue;
        }
        case 3:{
            env.log("Detected date change.");
            uint16_t delay_mills = DELAY_MILLIS;

            //  This is the seed we intend to hit.
            uint64_t seed_epoch_millis = to_seconds_since_epoch(date) * 1000;
            seed_epoch_millis -= delay_mills;

            //  This is the time we intend to set the clock to.
            uint64_t clock_epoch = (seed_epoch_millis - 5000) / 1000;
            clock_epoch /= 60;  //  Round down to minute.
            clock_epoch *= 60;
            DateTime set_date = from_seconds_since_epoch(clock_epoch);

            std::chrono::milliseconds trigger_delay(seed_epoch_millis - clock_epoch * 1000);

            VideoOverlaySet overlays(env.console.overlay());
            date_reader.make_overlays(overlays);
            date_reader.set_date(env.program_info(), env.console, context, set_date);

            //  Commit the date and start the timer.
            pbf_press_button(context, BUTTON_A, 20, 30);
            WallClock trigger_time = std::chrono::system_clock::now() + trigger_delay;
            env.log("Will commit in " + tostr_u_commas(trigger_delay.count()) + " milliseconds.");

            //  Re-enter the game.
            pbf_press_button(context, BUTTON_HOME, 20, ConsoleSettings::instance().SETTINGS_TO_HOME_DELAY);
            resume_game_from_home(env.console, context, false);

            if (!prompt.detect(env.console.video().snapshot())){
                env.log("Expected to be on prompt menu. Backing out.", COLOR_RED);
                stats.errors++;
                env.update_stats();
                pbf_mash_button(context, BUTTON_B, 500);
                continue;
            }

            //  Wait for trigger time.
            context.wait_until(trigger_time);
            pbf_press_button(context, BUTTON_A, 10, 10);
            continue;
        }
        case 4:{
            env.log("Detected material selection.");
            if (printed){
                return;
            }
            if (!overworld_seen){
                pbf_press_button(context, BUTTON_B, 20, 30);
                continue;
            }
            item_printer_start_print(env.console, context, LANGUAGE, jobs);
            stats.prints++;
            env.update_stats();
            printed = true;
            item_printer_finish_print(env.inference_dispatcher(), env.console, context, LANGUAGE);
//            pbf_press_button(context, BUTTON_B, 20, 30);
            continue;
        }
        default:
            stats.errors++;
            env.update_stats();
            throw OperationFailedException(
                ErrorReport::SEND_ERROR_REPORT,
                env.logger(),
                ""
            );
        }
    }
}
void ItemPrinterRNG::print_again(
    SingleSwitchProgramEnvironment& env, BotBaseContext& context,
    ItemPrinterJobs jobs
) const{
    ItemPrinterRNG_Descriptor::Stats& stats = env.current_stats<ItemPrinterRNG_Descriptor::Stats>();

    bool printed = false;
    while (true){
        context.wait_for_all_requests();

        OverworldWatcher overworld(COLOR_BLUE);
        AdvanceDialogWatcher dialog(COLOR_RED);
//        PromptDialogWatcher prompt(COLOR_GREEN);
        WhiteButtonWatcher material(COLOR_GREEN, WhiteButton::ButtonX, {0.63, 0.93, 0.17, 0.06});
        int ret = wait_until(
            env.console, context, std::chrono::seconds(120),
            {
                overworld,
                dialog,
//                prompt,
                material,
            }
        );
        switch (ret){
        case 0:
            env.log("Detected overworld... (unexpected)", COLOR_RED);
            return;

        case 1:
            env.log("Detected advance dialog.");
            pbf_press_button(context, BUTTON_B, 20, 30);
            continue;

        case 2:{
            env.log("Detected material selection.");
            if (printed){
                return;
            }
            item_printer_start_print(env.console, context, LANGUAGE, jobs);
            stats.prints++;
            env.update_stats();
            printed = true;
            item_printer_finish_print(env.inference_dispatcher(), env.console, context, LANGUAGE);
            continue;
        }
        default:
            stats.errors++;
            env.update_stats();
            throw OperationFailedException(
                ErrorReport::SEND_ERROR_REPORT,
                env.logger(),
                ""
            );
        }
    }
}


void ItemPrinterRNG::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    assert_16_9_720p_min(env.logger(), env.console);
    ItemPrinterRNG_Descriptor::Stats& stats = env.current_stats<ItemPrinterRNG_Descriptor::Stats>();

    env.update_stats();
    for (uint32_t c = 0; c < TOTAL_ROUNDS; c++){
        std::vector<ItemPrinterRngRowSnapshot> table = TABLE.snapshot<ItemPrinterRngRowSnapshot>();
        for (const ItemPrinterRngRowSnapshot& row : table){
            if (row.chain){
                print_again(env, context, row.jobs);
            }else{
                run_print_at_date(env, context, row.date, row.jobs);
            }
        }
//        run_print_at_date(env, context, DATE0, 1);
//        run_print_at_date(env, context, DATE1, 10);
        stats.iterations++;
        env.update_stats();
    }

    if (FIX_TIME_WHEN_DONE){
        pbf_press_button(context, BUTTON_HOME, 10, GameSettings::instance().GAME_TO_HOME_DELAY);
        home_to_date_time(context, false, false);
        pbf_press_button(context, BUTTON_A, 20, 105);
        pbf_press_button(context, BUTTON_A, 20, 105);
        pbf_press_button(context, BUTTON_HOME, 20, ConsoleSettings::instance().SETTINGS_TO_HOME_DELAY);
        resume_game_from_home(env.console, context);
    }
    GO_HOME_WHEN_DONE.run_end_of_program(context);
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}




























}
}
}
