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
#include "PokemonSV/Programs/Farming/PokemonSV_MaterialFarmerTools.h"
#include "PokemonSV/Programs/PokemonSV_Navigation.h"
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
    date.remove_listener(*this);
    jobs.remove_listener(*this);
    desired_item.remove_listener(*this);
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
{
    PA_ADD_OPTION(chain);
    PA_ADD_OPTION(date);
    PA_ADD_OPTION(jobs);
    PA_ADD_OPTION(desired_item);
    
    chain.add_listener(*this);
    date.add_listener(*this);
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

// - always update the date's visibility when chain is changed.
// - if desired_item has changed, set the seed (and number of jobs) accordingly
// - if any other value changes, set desired_item to NONE
void ItemPrinterRngRow::value_changed(){
    date.set_visibility(chain ? ConfigOptionState::DISABLED : ConfigOptionState::ENABLED);
    if (prev_desired_item != desired_item){ // check if desired_item has changed.
        set_seed_based_on_desired_item();
        prev_desired_item = desired_item;
        return;
    }
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
        FeedbackType::VIDEO_AUDIO,
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

ItemPrinterRNG::~ItemPrinterRNG(){
    AUTO_MATERIAL_FARMING.remove_listener(*this);
}

ItemPrinterRNG::ItemPrinterRNG()
    : LANGUAGE(
        "<b>Game Language:</b>",
        PokemonSwSh::IV_READER().languages(),
        LockMode::UNLOCK_WHILE_RUNNING,
        true
    )
    , NUM_ITEM_PRINTER_ROUNDS(
        "<b>Number of rounds per Item Printer session:</b><br>Iterate the rounds table this many times. ",
        LockMode::UNLOCK_WHILE_RUNNING, 10
    )
    , AFTER_ITEM_PRINTER_DONE_EXPLANATION(
        "Then proceed to material farming."
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
    , ADJUST_DELAY(
        "<b>Automatically adjust delay:</b><br>Adjust the delay, depending on the desired item and the actual print result. "
        "This only works if the \"Desired Item\" column is used for selecting Date Seeds. ",
        LockMode::UNLOCK_WHILE_RUNNING, true
    )
    , GO_HOME_WHEN_DONE(false)
    , FIX_TIME_WHEN_DONE(
        "<b>Fix Time When Done:</b><br>Fix the time after the program finishes.",
        LockMode::UNLOCK_WHILE_RUNNING, true
    )
    , AUTO_MATERIAL_FARMING(
        "<b>Automatic Material Farming:</b><br>"
        "After using the item printer, automatically fly to North Province (Area 3) to farm materials, "
        "then fly back to keep using the item printer.",
        // "This will do the item printer loops, then the material farmer loops, then repeat.<br>",
        // "The number of item printer loops is set by \"Number of rounds per Item Printer session\".<br>"
        // "The number of material farmer loops is set by \"Number of sandwich rounds to run\".<br>"
        // "The number of Item Printer to Material Farmer loops is set by \"Number of rounds of Item Printer → Material farm\".",
        LockMode::LOCK_WHILE_RUNNING,
        false
    )
    , NUM_ROUNDS_OF_ITEM_PRINTER_TO_MATERIAL_FARM(
        "<b>Number of rounds of Item Printer → Material farm:</b><br>"
        "One round is: Using the item printer (looping through the table for as many rounds as you specify below), "
        "then farming materials at North Provice area 3, "
        "then flying back to the item printer.<br>"
        "Automatic Material Farming (see above) must be enabled",
        // (as per \"Number of rounds per Item Printer session\" below)
        //(as per \"Number of sandwich rounds\" below, under \"Material Farmer\")
        LockMode::UNLOCK_WHILE_RUNNING,
        3
    )
    , NOTIFICATION_STATUS_UPDATE("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS_UPDATE,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_FATAL,
    })
    , MATERIAL_FARMER_DISABLED_EXPLANATION("")
    , MATERIAL_FARMER_OPTIONS(
        LANGUAGE, GO_HOME_WHEN_DONE, 
        NOTIFICATION_STATUS_UPDATE, NOTIFICATION_PROGRAM_FINISH,
        NOTIFICATION_ERROR_RECOVERABLE, NOTIFICATION_ERROR_FATAL
    )
{
    PA_ADD_OPTION(LANGUAGE);

    PA_ADD_OPTION(AUTO_MATERIAL_FARMING);
    PA_ADD_OPTION(NUM_ROUNDS_OF_ITEM_PRINTER_TO_MATERIAL_FARM);
    PA_ADD_OPTION(NUM_ITEM_PRINTER_ROUNDS);
    PA_ADD_OPTION(AFTER_ITEM_PRINTER_DONE_EXPLANATION);
//    PA_ADD_OPTION(DATE0);
//    PA_ADD_OPTION(DATE1);
    PA_ADD_OPTION(TABLE);
    PA_ADD_OPTION(DELAY_MILLIS);
    PA_ADD_OPTION(ADJUST_DELAY);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(FIX_TIME_WHEN_DONE);

    PA_ADD_OPTION(MATERIAL_FARMER_DISABLED_EXPLANATION);
    PA_ADD_OPTION(MATERIAL_FARMER_OPTIONS);

    PA_ADD_OPTION(NOTIFICATIONS);

    ItemPrinterRNG::value_changed();
    AUTO_MATERIAL_FARMING.add_listener(*this);
}

void ItemPrinterRNG::value_changed(){
    ConfigOptionState state = AUTO_MATERIAL_FARMING ? ConfigOptionState::ENABLED : ConfigOptionState::DISABLED;
    MATERIAL_FARMER_OPTIONS.set_visibility(state);
    NUM_ROUNDS_OF_ITEM_PRINTER_TO_MATERIAL_FARM.set_visibility(state);
    if (AUTO_MATERIAL_FARMING){
        AFTER_ITEM_PRINTER_DONE_EXPLANATION.set_text("Then proceed to material farming.");
        MATERIAL_FARMER_DISABLED_EXPLANATION.set_text("<br>");
    }else{
        AFTER_ITEM_PRINTER_DONE_EXPLANATION.set_text("Then stop the program.");
        MATERIAL_FARMER_DISABLED_EXPLANATION.set_text("<br>To enable the Material Farmer, enable \"Automatic Material Farming\" above");
    }
}


void ItemPrinterRNG::run_print_at_date(
    SingleSwitchProgramEnvironment& env, BotBaseContext& context,
    const DateTime& date, ItemPrinterJobs jobs
) {
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
            std::array<std::string, 10> print_results = item_printer_finish_print(env.inference_dispatcher(), env.console, context, LANGUAGE);
            if (ADJUST_DELAY){
                uint64_t seed = to_seconds_since_epoch(date);
                adjust_delay(print_results, seed, env.console);
            }

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

void ItemPrinterRNG::adjust_delay(
    const std::array<std::string, 10>& print_results, 
    uint64_t seed, ConsoleHandle& console
) {
    DistanceFromTarget distance_from_target = get_distance_from_target(print_results, seed, console);
    int16_t delay_adjustment;
    int16_t current_delay_mills = DELAY_MILLIS;
    switch (distance_from_target){
    case DistanceFromTarget::UNKNOWN:
    case DistanceFromTarget::ON_TARGET:
        delay_adjustment = 0;
        break;
    case DistanceFromTarget::MINUS_1:
        delay_adjustment = -50;
        break;
    case DistanceFromTarget::MINUS_2:
        delay_adjustment =  -1000;
        break;
    case DistanceFromTarget::PLUS_1:
        delay_adjustment =  50;
        break;
    case DistanceFromTarget::PLUS_2:
        delay_adjustment = 1000;
        break;        
    default:
        delay_adjustment = 0;
        break;
    }

    int16_t new_delay = current_delay_mills + delay_adjustment;
    if (new_delay < 0){
        new_delay = 0;
    }
    DELAY_MILLIS.set((uint16_t) new_delay);
    console.log("Current delay: " + std::to_string(new_delay));
    // std::cout << "Current delay:" << new_delay << std::endl;
    
}

DistanceFromTarget ItemPrinterRNG::get_distance_from_target(
    const std::array<std::string, 10>& print_results, 
    uint64_t seed, ConsoleHandle& console
){
    DistanceFromTarget distance_from_target;
    std::array<std::string, 10> target_result;
    std::array<std::string, 10> target_result_plus_1;
    std::array<std::string, 10> target_result_minus_1;
    std::array<std::string, 10> target_result_plus_2;
    std::array<std::string, 10> target_result_minus_2;

    // allow 1 misread/mismatch for all prints, except for the 
    // item and ball bonuses, since they only print 1.
    uint8_t max_number_of_mismatches = 1; 
    bool unknown_seed = false;

    std::string current_print;
    switch (seed){
    case 2346161588: // item bonus
        current_print = "Item bonus: ";
        target_result_minus_2 = {"stardust", "", "", "", "", "", "", "", "", ""};
        target_result_minus_1 = {"tiny-mushroom", "", "", "", "", "", "", "", "", ""};
        target_result = {"calcium", "", "", "", "", "", "", "", "", ""};
        target_result_plus_1 = {"max-ether", "", "", "", "", "", "", "", "", ""};
        target_result_plus_2 = {"electric-tera-shard", "", "", "", "", "", "", "", "", ""};
        max_number_of_mismatches = 0;
        break;
    case 1717461428: // ball bonus
        current_print = "Ball bonus: ";
        target_result_minus_2 = {"balm-mushroom", "", "", "", "", "", "", "", "", ""};
        target_result_minus_1 = {"revive", "", "", "", "", "", "", "", "", ""};
        target_result = {"full-restore", "", "", "", "", "", "", "", "", ""};
        target_result_plus_1 = {"fairy-tera-shard", "", "", "", "", "", "", "", "", ""};
        target_result_plus_2 = {"big-bamboo-shoot", "", "", "", "", "", "", "", "", ""};
        max_number_of_mismatches = 0;
        break;        
    case 958172368:
        current_print = "Ability patch: ";
        target_result_minus_2 = {"dark-tera-shard", "revive", "reaper-cloth", "ability-patch", "poison-tera-shard", "", "", "", "", ""};
        target_result_minus_1 = {"dark-tera-shard", "fire-tera-shard", "big-bamboo-shoot", "balm-mushroom", "super-potion", "", "", "", "", ""};
        target_result = {"dark-tera-shard", "ability-patch", "ability-patch", "ability-patch", "ability-patch", "", "", "", "", ""};
        target_result_plus_1 = {"dark-tera-shard", "pearl", "spell-tag", "flower-sweet", "ribbon-sweet", "", "", "", "", ""};
        target_result_plus_2 = {"dark-tera-shard", "water-stone", "max-potion", "tiny-mushroom", "electric-tera-shard", "", "", "", "", ""};
        break;
    case 1991472489:
        current_print = "EXP_CANDY: ";
        target_result_minus_2 = {"exp-candy-xl", "tiny-bamboo-shoot", "exp-candy-s", "iron", "tiny-bamboo-shoot", "", "", "", "", ""};
        target_result_minus_1 = {"exp-candy-xl", "toxic-orb", "rare-bone", "silver-powder", "lucky-egg", "", "", "", "", ""};
        target_result         = {"exp-candy-xl", "protein", "exp-candy-l", "exp-candy-xl", "exp-candy-xl", "", "", "", "", ""};
        target_result_plus_1  = {"exp-candy-xl", "silver-powder", "dubious-disc", "mystic-water", "toxic-orb", "", "", "", "", ""};
        target_result_plus_2  = {"exp-candy-xl", "tiny-bamboo-shoot", "toxic-orb", "ice-stone", "charcoal", "", "", "", "", ""};
        break;
    case 2246577010:
        current_print = "NORMAL_TERA: ";
        target_result_minus_2 = {"normal-tera-shard", "exp-candy-l", "star-piece", "tiny-mushroom", "rock-tera-shard", "", "", "", "", ""};
        target_result_minus_1 = {"normal-tera-shard", "oval-stone", "ether", "stardust", "steel-tera-shard", "", "", "", "", ""};
        target_result         = {"normal-tera-shard", "normal-tera-shard", "normal-tera-shard", "normal-tera-shard", "normal-tera-shard", "", "", "", "", ""};
        target_result_plus_1  = {"normal-tera-shard", "exp-candy-s", "ether", "elixir", "big-pearl", "", "", "", "", ""};
        target_result_plus_2  = {"normal-tera-shard", "dragon-tera-shard", "ice-tera-shard", "exp-candy-xl", "black-sludge", "", "", "", "", ""};
        break;
    case 2669513710:
        current_print = "FIRE_TERA: ";
        target_result_minus_2 = {"fire-tera-shard", "flame-orb", "steel-tera-shard", "fire-stone", "max-revive", "", "", "", "", ""};
        target_result_minus_1 = {"fire-tera-shard", "exp-candy-xs", "miracle-seed", "sun-stone", "iron", "", "", "", "", ""};
        target_result         = {"fire-tera-shard", "fire-tera-shard", "fire-tera-shard", "fire-tera-shard", "honey", "", "", "", "", ""};
        target_result_plus_1  = {"fire-tera-shard", "poison-tera-shard", "love-sweet", "safety-goggles", "exp-candy-m", "", "", "", "", ""};
        target_result_plus_2  = {"fire-tera-shard", "rare-bone", "dubious-disc", "berry-sweet", "hp-up", "", "", "", "", ""};
        break;
    case 2419526534:
        current_print = "WATER_TERA: ";
        target_result_minus_2 = {"water-tera-shard", "exp-candy-xs", "flame-orb", "exp-candy-s", "honey", "", "", "", "", ""};
        target_result_minus_1 = {"water-tera-shard", "never-melt-ice", "rock-tera-shard", "hp-up", "sun-stone", "", "", "", "", ""};
        target_result         = {"water-tera-shard", "full-restore", "water-tera-shard", "flying-tera-shard", "water-tera-shard", "", "", "", "", ""};
        target_result_plus_1  = {"water-tera-shard", "heavy-duty-boots", "moon-stone", "strawberry-sweet", "exp-candy-xs", "", "", "", "", ""};
        target_result_plus_2  = {"water-tera-shard", "ghost-tera-shard", "max-revive", "ground-tera-shard", "exp-candy-xl", "", "", "", "", ""};
        break;
    case 1300121653:
        current_print = "ELECTRIC_TERA: ";
        target_result_minus_2 = {"electric-tera-shard", "elixir", "full-restore", "upgrade", "toxic-orb", "", "", "", "", ""};
        target_result_minus_1 = {"electric-tera-shard", "exp-candy-xs", "steel-tera-shard", "big-mushroom", "exp-candy-s", "", "", "", "", ""};
        target_result         = {"electric-tera-shard", "light-clay", "electric-tera-shard", "electric-tera-shard", "electric-tera-shard", "", "", "", "", ""};
        target_result_plus_1  = {"electric-tera-shard", "calcium", "ground-tera-shard", "water-tera-shard", "upgrade", "", "", "", "", ""};
        target_result_plus_2  = {"electric-tera-shard", "max-potion", "charcoal", "exp-candy-xs", "grass-tera-shard", "", "", "", "", ""};
        break;                        
    case 1258326250:
        current_print = "GRASS_TERA: ";
        target_result_minus_2 = {"grass-tera-shard", "pretty-feather", "exp-candy-m", "bug-tera-shard", "tiny-mushroom", "", "", "", "", ""};
        target_result_minus_1 = {"grass-tera-shard", "ghost-tera-shard", "bug-tera-shard", "silver-powder", "berry-sweet", "", "", "", "", ""};
        target_result         = {"grass-tera-shard", "tiny-mushroom", "grass-tera-shard", "grass-tera-shard", "grass-tera-shard", "", "", "", "", ""};
        target_result_plus_1  = {"grass-tera-shard", "steel-tera-shard", "honey", "strawberry-sweet", "normal-tera-shard", "", "", "", "", ""};
        target_result_plus_2  = {"grass-tera-shard", "exp-candy-xl", "exp-candy-xs", "zinc", "dragon-fang", "", "", "", "", ""};
        break; 
    case 2567326030:
        current_print = "ICE_TERA: ";
        target_result_minus_2 = {"ice-tera-shard", "pearl-string", "dragon-tera-shard", "sun-stone", "water-tera-shard", "", "", "", "", ""};
        target_result_minus_1 = {"ice-tera-shard", "scope-lens", "full-restore", "dubious-disc", "lucky-egg", "", "", "", "", ""};
        target_result         = {"ice-tera-shard", "ice-tera-shard", "ice-tera-shard", "ice-tera-shard", "metal-alloy", "", "", "", "", ""};
        target_result_plus_1  = {"ice-tera-shard", "poison-barb", "leaf-stone", "cracked-pot", "ghost-tera-shard", "", "", "", "", ""};
        target_result_plus_2  = {"ice-tera-shard", "exp-candy-xs", "rock-tera-shard", "loaded-dice", "Pretty Feather", "", "", "", "", ""};
        break;
    case 1510682535:
        current_print = "FIGHTING_TERA: ";
        target_result_minus_2 = {"fairy-feather", "normal-tera-shard", "air-balloon", "poison-tera-shard", "pearl", "", "", "", "", ""};
        target_result_minus_1 = {"strawberry-sweet", "electric-tera-shard", "calcium", "revive", "ghost-tera-shard", "", "", "", "", ""};
        target_result         = {"fighting-tera-shard", "fighting-tera-shard", "fighting-tera-shard", "fighting-tera-shard", "shiny-stone", "", "", "", "", ""};
        target_result_plus_1  = {"miracle-seed", "fighting-tera-shard", "ice-tera-shard", "light-clay", "dark-tera-shard", "", "", "", "", ""};
        target_result_plus_2  = {"rare-bone", "max-elixir", "leaf-stone", "metal-alloy", "magmarizer", "", "", "", "", ""};
        break;
    case 2041396572:
        current_print = "POISON_TERA: ";
        target_result_minus_2 = {"electirizer", "upgrade", "zinc", "elixir", "revive", "", "", "", "", ""};
        target_result_minus_1 = {"electirizer", "flame-orb", "magmarizer", "hp-up", "pretty-feather", "", "", "", "", ""};
        target_result         = {"electirizer", "poison-tera-shard", "poison-tera-shard", "poison-tera-shard", "poison-tera-shard", "", "", "", "", ""};
        target_result_plus_1  = {"electirizer", "pp-up", "rock-tera-shard", "big-mushroom", "light-clay", "", "", "", "", ""};
        target_result_plus_2  = {"electirizer", "fire-tera-shard", "revive", "big-bamboo-shoot", "dark-tera-shard", "", "", "", "", ""};
        break;
    case 1463321889:
        current_print = "GROUND_TERA: ";
        target_result_minus_2 = {"pp-up", "pretty-feather", "max-ether", "big-pearl", "carbos", "", "", "", "", ""};
        target_result_minus_1 = {"pp-up", "spell-tag", "bug-tera-shard", "pearl", "fighting-tera-shard", "", "", "", "", ""};
        target_result         = {"pp-up", "ground-tera-shard", "ground-tera-shard", "ground-tera-shard", "ground-tera-shard", "", "", "", "", ""};
        target_result_plus_1  = {"pp-up", "metal-alloy", "full-restore", "pearl", "silver-powder", "", "", "", "", ""};
        target_result_plus_2  = {"pp-up", "ghost-tera-shard", "nugget", "safety-goggles", "leftovers", "", "", "", "", ""};
        break; 
    case 1580633500:
        current_print = "FLYING_TERA: ";
        target_result_minus_2 = {"black-glasses", "big-nugget", "max-revive", "pearl-string", "balm-mushroom", "", "", "", "", ""};
        target_result_minus_1 = {"loaded-dice", "exp-candy-s", "strawberry-sweet", "never-melt-ice", "moon-stone", "", "", "", "", ""};
        target_result         = {"flying-tera-shard", "flying-tera-shard", "flying-tera-shard", "flying-tera-shard", "flying-tera-shard", "", "", "", "", ""};
        target_result_plus_1  = {"water-stone", "rare-bone", "max-ether", "moon-stone", "ability-patch", "", "", "", "", ""};
        target_result_plus_2  = {"max-ether", "big-bamboo-shoot", "iron", "big-mushroom", "pearl", "", "", "", "", ""};
        break; 
    case 2190593713:
        current_print = "PSYCHIC_TERA: ";
        target_result_minus_2 = {"psychic-tera-shard", "big-mushroom", "fairy-feather", "fairy-tera-shard", "reaper-cloth", "", "", "", "", ""};
        target_result_minus_1 = {"psychic-tera-shard", "electric-tera-shard", "pearl", "stellar-tera-shard", "dawn-stone", "", "", "", "", ""};
        target_result         = {"psychic-tera-shard", "never-melt-ice", "psychic-tera-shard", "psychic-tera-shard", "psychic-tera-shard", "", "", "", "", ""};
        target_result_plus_1  = {"psychic-tera-shard", "pearl-string", "twisted-spoon", "big-pearl", "iron", "", "", "", "", ""};
        target_result_plus_2  = {"psychic-tera-shard", "heavy-duty-boots", "water-tera-shard", "rock-tera-shard", "twisted-spoon", "", "", "", "", ""};
        break;
    case 2458109848:
        current_print = "BUG_TERA: ";
        target_result_minus_2 = {"ether", "big-nugget", "mystic-water", "pretty-feather", "pretty-feather", "", "", "", "", ""};
        target_result_minus_1 = {"ether", "psychic-tera-shard", "exp-candy-s", "honey", "sharp-beak", "", "", "", "", ""};
        target_result         = {"ether", "bug-tera-shard", "bug-tera-shard", "bug-tera-shard", "bug-tera-shard", "", "", "", "", ""};
        target_result_plus_1  = {"ether", "black-glasses", "fighting-tera-shard", "tiny-bamboo-shoot", "poison-barb", "", "", "", "", ""};
        target_result_plus_2  = {"ether", "exp-candy-xl", "normal-tera-shard", "fairy-tera-shard", "balm-mushroom", "", "", "", "", ""};
        break;
    case 2801954289:
        current_print = "ROCK_TERA: ";
        target_result_minus_2 = {"max-revive", "scope-lens", "razor-claw", "big-nugget", "tiny-bamboo-shoot", "", "", "", "", ""};
        target_result_minus_1 = {"iron", "dark-tera-shard", "air-balloon", "rare-bone", "ability-patch", "", "", "", "", ""};
        target_result         = {"rock-tera-shard", "rock-tera-shard", "pretty-feather", "rock-tera-shard", "stardust", "", "", "", "", ""};
        target_result_plus_1  = {"normal-tera-shard", "ground-tera-shard", "honey", "upgrade", "honey", "", "", "", "", ""};
        target_result_plus_2  = {"heavy-duty-boots", "ice-tera-shard", "sun-stone", "dubious-disc", "pearl", "", "", "", "", ""};
        break;
    case 1509904271:
        current_print = "GHOST_TERA: ";
        target_result_minus_2 = {"ghost-tera-shard", "hyper-potion", "toxic-orb", "magmarizer", "black-glasses", "", "", "", "", ""};
        target_result_minus_1 = {"ghost-tera-shard", "black-belt", "pp-up", "ice-tera-shard", "magmarizer", "", "", "", "", ""};
        target_result         = {"ghost-tera-shard", "ghost-tera-shard", "ghost-tera-shard", "magnet", "ghost-tera-shard", "", "", "", "", ""};
        target_result_plus_1  = {"ghost-tera-shard", "big-nugget", "hp-up", "gold-bottle-cap", "big-bamboo-shoot", "", "", "", "", ""};
        target_result_plus_2  = {"ghost-tera-shard", "hp-up", "elixir", "water-tera-shard", "pp-up", "", "", "", "", ""};
        break;                        
    case 2155674309:
        current_print = "DRAGON_TERA: ";
        target_result_minus_2 = {"comet-shard", "soft-sand", "big-mushroom", "steel-tera-shard", "metronome", "", "", "", "", ""};
        target_result_minus_1 = {"pearl", "balm-mushroom", "moon-stone", "protein", "ability-patch", "", "", "", "", ""};
        target_result         = {"ability-patch", "dragon-tera-shard", "dragon-tera-shard", "dragon-tera-shard", "dragon-tera-shard", "", "", "", "", ""};
        target_result_plus_1  = {"reaper-cloth", "hp-up", "revive", "light-clay", "focus-band", "", "", "", "", ""};
        target_result_plus_2  = {"spell-tag", "love-sweet", "fire-tera-shard", "ether", "black-belt", "", "", "", "", ""};
        break; 
    case 2537180413:
        current_print = "DARK_TERA: ";
        target_result_minus_2 = {"dark-tera-shard", "upgrade", "big-mushroom", "normal-tera-shard", "exp-candy-m", "", "", "", "", ""};
        target_result_minus_1 = {"dark-tera-shard", "pp-max", "calcium", "rock-tera-shard", "hard-stone", "", "", "", "", ""};
        target_result         = {"dark-tera-shard", "dark-tera-shard", "dark-tera-shard", "dark-tera-shard", "prism-scale", "", "", "", "", ""};
        target_result_plus_1  = {"dark-tera-shard", "leaf-stone", "big-pearl", "comet-shard", "exp-candy-m", "", "", "", "", ""};
        target_result_plus_2  = {"dark-tera-shard", "ground-tera-shard", "fairy-tera-shard", "exp-candy-xs", "fighting-tera-shard", "", "", "", "", ""};
        break; 
    case 1766067369:
        current_print = "STEEL_TERA: ";
        target_result_minus_2 = {"steel-tera-shard", "bug-tera-shard", "exp-candy-xl", "pretty-feather", "silk-scarf", "", "", "", "", ""};
        target_result_minus_1 = {"steel-tera-shard", "tiny-mushroom", "super-potion", "sharp-beak", "ice-tera-shard", "", "", "", "", ""};
        target_result         = {"steel-tera-shard", "steel-tera-shard", "steel-tera-shard", "steel-tera-shard", "berry-sweet", "", "", "", "", ""};
        target_result_plus_1  = {"steel-tera-shard", "moon-stone", "exp-candy-xl", "iron", "exp-candy-xs", "", "", "", "", ""};
        target_result_plus_2  = {"steel-tera-shard", "metal-alloy", "ability-patch", "pp-up", "rare-bone", "", "", "", "", ""};
        break;
    case 2183028974:
        current_print = "FAIRY_TERA: ";
        target_result_minus_2 = {"max-elixir", "never-melt-ice", "pearl", "max-potion", "pp-up", "", "", "", "", ""};
        target_result_minus_1 = {"full-restore", "psychic-tera-shard", "gold-bottle-cap", "dawn-stone", "never-melt-ice", "", "", "", "", ""};
        target_result         = {"fairy-tera-shard", "fairy-tera-shard", "water-tera-shard", "tiny-bamboo-shoot", "fairy-tera-shard", "", "", "", "", ""};
        target_result_plus_1  = {"ice-tera-shard", "twisted-spoon", "exp-candy-s", "max-elixir", "razor-fang", "", "", "", "", ""};
        target_result_plus_2  = {"exp-candy-xl", "bug-tera-shard", "star-piece", "hp-up", "bug-tera-shard", "", "", "", "", ""};
        break;
    case 1165654034:
        current_print = "STELLAR_TERA: ";
        target_result_minus_2 = {"stellar-tera-shard", "pp-up", "scope-lens", "electric-tera-shard", "metronome", "", "", "", "", ""};
        target_result_minus_1 = {"stellar-tera-shard", "dragon-tera-shard", "honey", "steel-tera-shard", "max-potion", "", "", "", "", ""};
        target_result         = {"stellar-tera-shard", "max-revive", "stellar-tera-shard", "stellar-tera-shard", "clover-sweet", "", "", "", "", ""};
        target_result_plus_1  = {"stellar-tera-shard", "full-heal", "toxic-orb", "max-potion", "miracle-seed", "", "", "", "", ""};
        target_result_plus_2  = {"stellar-tera-shard", "big-bamboo-shoot", "poison-tera-shard", "honey", "oval-stone", "", "", "", "", ""};
        break;
    case 1131363128:
        current_print = "MASTER_BALL: ";
        target_result_minus_2 = {"master-ball", "great-ball", "quick-ball", "luxury-ball", "heal-ball", "", "", "", "", ""};
        target_result_minus_1 = {"master-ball", "great-ball", "timer-ball", "fast-ball", "love-ball", "", "", "", "", ""};
        target_result         = {"master-ball", "master-ball", "great-ball", "master-ball", "master-ball", "", "", "", "", ""};
        target_result_plus_1  = {"master-ball", "premier-ball", "great-ball", "luxury-ball", "heal-ball", "", "", "", "", ""};
        target_result_plus_2  = {"master-ball", "quick-ball", "poke-ball", "premier-ball", "poke-ball", "", "", "", "", ""};
        break;                        
    case 2493163222:
        current_print = "SAFARI_BALL: ";
        target_result_minus_2 = {"premier-ball", "great-ball", "lure-ball", "ultra-ball", "safari-ball", "", "", "", "", ""};
        target_result_minus_1 = {"premier-ball", "great-ball", "poke-ball", "poke-ball", "poke-ball", "", "", "", "", ""};
        target_result         = {"safari-ball", "safari-ball", "safari-ball", "safari-ball", "safari-ball", "", "", "", "", ""};
        target_result_plus_1  = {"poke-ball", "poke-ball", "poke-ball", "premier-ball", "great-ball", "", "", "", "", ""};
        target_result_plus_2  = {"great-ball", "great-ball", "level-ball", "poke-ball", "safari-ball", "", "", "", "", ""};
        break;       
    case 2040690819:
        current_print = "FAST_BALL: ";
        target_result_minus_2 = {"fast-ball", "lure-ball", "dream-ball", "friend-ball", "level-ball", "", "", "", "", ""};
        target_result_minus_1 = {"fast-ball", "great-ball", "quick-ball", "great-ball", "heal-ball", "", "", "", "", ""};
        target_result         = {"fast-ball", "fast-ball", "fast-ball", "fast-ball", "fast-ball", "", "", "", "", ""};
        target_result_plus_1  = {"fast-ball", "dream-ball", "luxury-ball", "luxury-ball", "premier-ball", "", "", "", "", ""};
        target_result_plus_2  = {"fast-ball", "luxury-ball", "great-ball", "poke-ball", "premier-ball", "", "", "", "", ""};
        break;  
    case 2373357938:
        current_print = "LEVEL_BALL: ";
        target_result_minus_2 = {"dusk-ball", "heal-ball", "luxury-ball", "premier-ball", "poke-ball", "", "", "", "", ""};
        target_result_minus_1 = {"great-ball", "dive-ball", "dream-ball", "luxury-ball", "luxury-ball", "", "", "", "", ""};
        target_result         = {"level-ball", "level-ball", "level-ball", "level-ball", "level-ball", "", "", "", "", ""};
        target_result_plus_1  = {"repeat-ball", "heal-ball", "heal-ball", "luxury-ball", "dive-ball", "", "", "", "", ""};
        target_result_plus_2  = {"luxury-ball", "master-ball", "premier-ball", "ultra-ball", "heal-ball", "", "", "", "", ""};
        break;  
    case 2773542517:
        current_print = "LURE_BALL: ";
        target_result_minus_2 = {"lure-ball", "premier-ball", "premier-ball", "great-ball", "poke-ball", "", "", "", "", ""};
        target_result_minus_1 = {"dream-ball", "poke-ball", "poke-ball", "luxury-ball", "poke-ball", "", "", "", "", ""};
        target_result         = {"lure-ball", "lure-ball", "lure-ball", "lure-ball", "lure-ball", "", "", "", "", ""};
        target_result_plus_1  = {"luxury-ball", "premier-ball", "great-ball", "luxury-ball", "fast-ball", "", "", "", "", ""};
        target_result_plus_2  = {"timer-ball", "safari-ball", "dive-ball", "poke-ball", "premier-ball", "", "", "", "", ""};
        break;       
    case 1879588391:
        current_print = "HEAVY_BALL: ";
        target_result_minus_2 = {"heavy-ball", "nest-ball", "nest-ball", "heal-ball", "luxury-ball", "", "", "", "", ""};
        target_result_minus_1 = {"heavy-ball", "poke-ball", "premier-ball", "premier-ball", "heal-ball", "", "", "", "", ""};
        target_result         = {"heavy-ball", "heavy-ball", "heavy-ball", "heavy-ball", "heavy-ball", "", "", "", "", ""};
        target_result_plus_1  = {"heavy-ball", "heal-ball", "premier-ball", "timer-ball", "premier-ball", "", "", "", "", ""};
        target_result_plus_2  = {"heavy-ball", "ultra-ball", "heal-ball", "quick-ball", "beast-ball", "", "", "", "", ""};
        break;  
    case 1684733771:
        current_print = "LOVE_BALL: ";
        target_result_minus_2 = {"love-ball", "sport-ball", "great-ball", "poke-ball", "poke-ball", "", "", "", "", ""};
        target_result_minus_1 = {"love-ball", "poke-ball", "great-ball", "luxury-ball", "master-ball", "", "", "", "", ""};
        target_result         = {"love-ball", "love-ball", "love-ball", "love-ball", "love-ball", "", "", "", "", ""};
        target_result_plus_1  = {"love-ball", "great-ball", "luxury-ball", "luxury-ball", "poke-ball", "", "", "", "", ""};
        target_result_plus_2  = {"love-ball", "luxury-ball", "premier-ball", "poke-ball", "quick-ball", "", "", "", "", ""};
        break; 
    case 2428107797:
        current_print = "FRIEND_BALL: ";
        target_result_minus_2 = {"friend-ball", "great-ball", "sport-ball", "great-ball", "great-ball", "", "", "", "", ""};
        target_result_minus_1 = {"friend-ball", "premier-ball", "heal-ball", "repeat-ball", "nest-ball", "", "", "", "", ""};
        target_result         = {"friend-ball", "friend-ball", "friend-ball", "friend-ball", "friend-ball", "", "", "", "", ""};
        target_result_plus_1  = {"friend-ball", "premier-ball", "heavy-ball", "timer-ball", "friend-ball", "", "", "", "", ""};
        target_result_plus_2  = {"friend-ball", "heal-ball", "great-ball", "beast-ball", "moon-ball", "", "", "", "", ""};
        break;       
    case 2296878993:
        current_print = "MOON_BALL: ";
        target_result_minus_2 = {"luxury-ball", "timer-ball", "premier-ball", "luxury-ball", "poke-ball", "", "", "", "", ""};
        target_result_minus_1 = {"nest-ball", "poke-ball", "heal-ball", "premier-ball", "premier-ball", "", "", "", "", ""};
        target_result         = {"moon-ball", "moon-ball", "moon-ball", "moon-ball", "moon-ball", "", "", "", "", ""};
        target_result_plus_1  = {"fast-ball", "heal-ball", "poke-ball", "dusk-ball", "luxury-ball", "", "", "", "", ""};
        target_result_plus_2  = {"lure-ball", "heavy-ball", "luxury-ball", "heal-ball", "poke-ball", "", "", "", "", ""};
        break;  
    case 1244233031:
        current_print = "SPORT_BALL: ";
        target_result_minus_2 = {"sport-ball", "heal-ball", "luxury-ball", "great-ball", "heal-ball", "", "", "", "", ""};
        target_result_minus_1 = {"sport-ball", "dream-ball", "level-ball", "poke-ball", "luxury-ball", "", "", "", "", ""};
        target_result         = {"sport-ball", "sport-ball", "sport-ball", "sport-ball", "sport-ball", "", "", "", "", ""};
        target_result_plus_1  = {"moon-ball", "heal-ball", "poke-ball", "poke-ball", "heal-ball", "", "", "", "", ""};
        target_result_plus_2  = {"moon-ball", "poke-ball", "ultra-ball", "poke-ball", "great-ball", "", "", "", "", ""};
        break;    
    case 2657784681:
        current_print = "DREAM_BALL: ";
        target_result_minus_2 = {"master-ball", "poke-ball", "luxury-ball", "level-ball", "dream-ball", "", "", "", "", ""};
        target_result_minus_1 = {"timer-ball", "beast-ball", "poke-ball", "premier-ball", "heal-ball", "", "", "", "", ""};
        target_result         = {"dream-ball", "dream-ball", "dream-ball", "dream-ball", "dream-ball", "", "", "", "", ""};
        target_result_plus_1  = {"heavy-ball", "poke-ball", "friend-ball", "fast-ball", "heavy-ball", "", "", "", "", ""};
        target_result_plus_2  = {"luxury-ball", "poke-ball", "luxury-ball", "quick-ball", "friend-ball", "", "", "", "", ""};
        break;  
    case 2117201835:
        current_print = "BEAST_BALL: ";
        target_result_minus_2 = {"luxury-ball", "poke-ball", "lure-ball", "master-ball", "dive-ball", "", "", "", "", ""};
        target_result_minus_1 = {"heavy-ball", "premier-ball", "premier-ball", "poke-ball", "level-ball", "", "", "", "", ""};
        target_result         = {"beast-ball", "beast-ball", "beast-ball", "beast-ball", "beast-ball", "", "", "", "", ""};
        target_result_plus_1  = {"heal-ball", "great-ball", "luxury-ball", "heavy-ball", "great-ball", "", "", "", "", ""};
        target_result_plus_2  = {"poke-ball", "premier-ball", "great-ball", "poke-ball", "dive-ball", "", "", "", "", ""};
        break;       
    default:
        unknown_seed = true;
        console.log("Unknown seed. Can't calculate distance from target.");
        break;
    }

    if (unknown_seed){
        return DistanceFromTarget::UNKNOWN;
    }

    if(check_print_results_match(print_results, target_result, max_number_of_mismatches)){
        distance_from_target = DistanceFromTarget::ON_TARGET;
        console.log("Attempted to get " + current_print + "Success. On target.");
    }else if (check_print_results_match(print_results, target_result_plus_1, max_number_of_mismatches)){
        distance_from_target = DistanceFromTarget::PLUS_1;
        console.log("Attempted to get " + current_print + "Missed. Target +1.");
    }else if (check_print_results_match(print_results, target_result_minus_1, max_number_of_mismatches)){
        distance_from_target = DistanceFromTarget::MINUS_1;
        console.log("Attempted to get " + current_print + "Missed. Target -1.");
    }else if (check_print_results_match(print_results, target_result_plus_2, max_number_of_mismatches)){
        distance_from_target = DistanceFromTarget::PLUS_2;
        console.log("Attempted to get " + current_print + "Missed. Target +2.");
    }else if (check_print_results_match(print_results, target_result_minus_2, max_number_of_mismatches)){
        distance_from_target = DistanceFromTarget::MINUS_2;
        console.log("Attempted to get " + current_print + "Missed. Target -2.");
    }else{
        distance_from_target = DistanceFromTarget::UNKNOWN;
        console.log("Attempted to get " + current_print + "Missed. Unknown distance from target.");
    }

    return distance_from_target;
}


// - return true if print_results matches with expected_result
// - the lists don't have to match exactly, as long as current_number_of_mismatches <= max_number_of_mismatches
bool ItemPrinterRNG::check_print_results_match(
    const std::array<std::string, 10>& print_results, 
    const std::array<std::string, 10>& expected_result,
    uint8_t max_number_of_mismatches
) {
    uint8_t current_number_of_mismatches = 0;
    for (uint8_t i = 0; i < 10; i++){
        if(print_results.at(i) != expected_result.at(i)){
            current_number_of_mismatches++;
            if(current_number_of_mismatches > max_number_of_mismatches){
                return false;
            }
        }
    }

    return true;
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

void ItemPrinterRNG::run_item_printer_rng(
    SingleSwitchProgramEnvironment& env, BotBaseContext& context, 
    ItemPrinterRNG_Descriptor::Stats& stats
){
    for (uint32_t c = 0; c < NUM_ITEM_PRINTER_ROUNDS; c++){
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
}

void ItemPrinterRNG::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    assert_16_9_720p_min(env.logger(), env.console);
    ItemPrinterRNG_Descriptor::Stats& stats = env.current_stats<ItemPrinterRNG_Descriptor::Stats>();
    env.update_stats();



    if (!AUTO_MATERIAL_FARMING){
        run_item_printer_rng(env, context, stats);
    }
    else {
        // Throw user setup errors early in program
        // - Ensure language is set
        const Language language = LANGUAGE;
        if (language == Language::None) {
            throw UserSetupError(env.console.logger(), "Must set game language option to read item printer rewards.");
        }        

        // - Ensure audio input is enabled
        LetsGoKillSoundDetector audio_detector(env.console, [](float){ return true; });
        wait_until(
            env.console, context,
            std::chrono::milliseconds(1100),
            {
                static_cast<AudioInferenceCallback&>(audio_detector)
            }
        );
        audio_detector.throw_if_no_sound(std::chrono::milliseconds(1000));

        // Don't allow the material farmer stats to affect the Item Printer's stats.
        MaterialFarmerTools::Stats mat_farm_stats;// = env.current_stats<MaterialFarmer_Descriptor::Stats>();
        for (int i = 0; i < NUM_ROUNDS_OF_ITEM_PRINTER_TO_MATERIAL_FARM; i++){
            run_item_printer_rng(env, context, stats);
            press_Bs_to_back_to_overworld(env.program_info(), env.console, context);
            move_from_item_printer_to_material_farming(env, context);
            run_material_farmer(env, context, MATERIAL_FARMER_OPTIONS, mat_farm_stats);
            move_from_material_farming_to_item_printer(env, context);
        }

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
