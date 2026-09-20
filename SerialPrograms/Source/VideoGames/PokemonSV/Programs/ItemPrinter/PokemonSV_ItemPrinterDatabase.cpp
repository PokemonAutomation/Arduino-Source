/*  Item Printer Database
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <vector>
#include <map>
#include "Common/Cpp/Exceptions.h"
#include "PokemonSV_ItemPrinterDatabase.h"
#include <iostream>

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{
namespace ItemPrinter{


const EnumDropdownDatabase<PrebuiltOptions>& PrebuiltOptions_Database(){
    static const EnumDropdownDatabase<PrebuiltOptions> database({
        {PrebuiltOptions::NONE, "none", "---"},
        {PrebuiltOptions::ITEM_BONUS, "item-bonus", "Item Bonus"},
        {PrebuiltOptions::BALL_BONUS, "ball-bonus", "Ball Bonus"},

        // ordered by category
        {PrebuiltOptions::ABILITY_PATCH, "ability-patch", "Ability Patch"},
        {PrebuiltOptions::EXP_CANDY, "exp-candy-xl", "Exp Candy"},
        {PrebuiltOptions::PP_MAX, "pp-max", "PP Max"},

        {PrebuiltOptions::ASSORTED_BALLS_1, "assorted-balls-1", "Assorted Balls 1"},
        {PrebuiltOptions::ASSORTED_BALLS_2, "assorted-balls-2", "Assorted Balls 2"},
        {PrebuiltOptions::ASSORTED_BALLS_3, "assorted-balls-3", "Assorted Balls 3"},

        {PrebuiltOptions::MASTER_BALL, "master-ball", "Master Ball"},
        {PrebuiltOptions::FAST_BALL, "fast-ball", "Fast Ball"},
        {PrebuiltOptions::FRIEND_BALL, "friend-ball", "Friend Ball"},
        {PrebuiltOptions::LURE_BALL, "lure-ball", "Lure Ball"},
        {PrebuiltOptions::LEVEL_BALL, "level-ball", "Level Ball"},
        {PrebuiltOptions::HEAVY_BALL, "heavy-ball", "Heavy Ball"},
        {PrebuiltOptions::LOVE_BALL, "love-ball", "Love Ball"},
        {PrebuiltOptions::MOON_BALL, "moon-ball", "Moon Ball"},
        {PrebuiltOptions::DREAM_BALL, "dream-ball", "Dream Ball"},
        {PrebuiltOptions::SPORT_BALL, "sport-ball", "Sport Ball"},
        {PrebuiltOptions::SAFARI_BALL, "safari-ball", "Safari Ball"},
        {PrebuiltOptions::BEAST_BALL, "beast-ball", "Beast Ball"},

        {PrebuiltOptions::BUG_TERA, "bug-tera-shard", "Bug Tera Shard"},
        {PrebuiltOptions::DARK_TERA, "dark-tera-shard", "Dark Tera Shard"},
        {PrebuiltOptions::DRAGON_TERA, "dragon-tera-shard", "Dragon Tera Shard"},
        {PrebuiltOptions::ELECTRIC_TERA, "electric-tera-shard", "Electric Tera Shard"},
        {PrebuiltOptions::FAIRY_TERA, "fairy-tera-shard", "Fairy Tera Shard"},
        {PrebuiltOptions::FIGHTING_TERA, "fighting-tera-shard", "Fighting Tera Shard"},
        {PrebuiltOptions::FIRE_TERA, "fire-tera-shard", "Fire Tera Shard"},
        {PrebuiltOptions::FLYING_TERA, "flying-tera-shard", "Flying Tera Shard"},
        {PrebuiltOptions::GHOST_TERA, "ghost-tera-shard", "Ghost Tera Shard"},
        {PrebuiltOptions::GRASS_TERA, "grass-tera-shard", "Grass Tera Shard"},
        {PrebuiltOptions::GROUND_TERA, "ground-tera-shard", "Ground Tera Shard"},
        {PrebuiltOptions::ICE_TERA, "ice-tera-shard", "Ice Tera Shard"},
        {PrebuiltOptions::NORMAL_TERA, "normal-tera-shard", "Normal Tera Shard"},
        {PrebuiltOptions::POISON_TERA, "poison-tera-shard", "Poison Tera Shard"},
        {PrebuiltOptions::PSYCHIC_TERA, "psychic-tera-shard", "Psychic Tera Shard"},
        {PrebuiltOptions::ROCK_TERA, "rock-tera-shard", "Rock Tera Shard"},
        {PrebuiltOptions::STEEL_TERA, "steel-tera-shard", "Steel Tera Shard"},
        {PrebuiltOptions::WATER_TERA, "water-tera-shard", "Water Tera Shard"},
        {PrebuiltOptions::STELLAR_TERA, "stellar-tera-shard", "Stellar Tera Shard"},

    });
    return database;
}

const EnumDropdownDatabase<PrebuiltOptions>& PrebuiltOptions_AutoMode_Database(){
    static const EnumDropdownDatabase<PrebuiltOptions> database({

        // ordered by category
        {PrebuiltOptions::ABILITY_PATCH, "ability-patch", "Ability Patch"},
        {PrebuiltOptions::EXP_CANDY, "exp-candy-xl", "Exp Candy"},
        {PrebuiltOptions::PP_MAX, "pp-max", "PP Max"},

        {PrebuiltOptions::MASTER_BALL, "master-ball", "Master Ball"},
        {PrebuiltOptions::FAST_BALL, "fast-ball", "Fast Ball"},
        {PrebuiltOptions::FRIEND_BALL, "friend-ball", "Friend Ball"},
        {PrebuiltOptions::LURE_BALL, "lure-ball", "Lure Ball"},
        {PrebuiltOptions::LEVEL_BALL, "level-ball", "Level Ball"},
        {PrebuiltOptions::HEAVY_BALL, "heavy-ball", "Heavy Ball"},
        {PrebuiltOptions::LOVE_BALL, "love-ball", "Love Ball"},
        {PrebuiltOptions::MOON_BALL, "moon-ball", "Moon Ball"},
        {PrebuiltOptions::DREAM_BALL, "dream-ball", "Dream Ball"},
        {PrebuiltOptions::SPORT_BALL, "sport-ball", "Sport Ball"},
        {PrebuiltOptions::SAFARI_BALL, "safari-ball", "Safari Ball"},
        {PrebuiltOptions::BEAST_BALL, "beast-ball", "Beast Ball"},

        {PrebuiltOptions::BUG_TERA, "bug-tera-shard", "Bug Tera Shard"},
        {PrebuiltOptions::DARK_TERA, "dark-tera-shard", "Dark Tera Shard"},
        {PrebuiltOptions::DRAGON_TERA, "dragon-tera-shard", "Dragon Tera Shard"},
        {PrebuiltOptions::ELECTRIC_TERA, "electric-tera-shard", "Electric Tera Shard"},
        {PrebuiltOptions::FAIRY_TERA, "fairy-tera-shard", "Fairy Tera Shard"},
        {PrebuiltOptions::FIGHTING_TERA, "fighting-tera-shard", "Fighting Tera Shard"},
        {PrebuiltOptions::FIRE_TERA, "fire-tera-shard", "Fire Tera Shard"},
        {PrebuiltOptions::FLYING_TERA, "flying-tera-shard", "Flying Tera Shard"},
        {PrebuiltOptions::GHOST_TERA, "ghost-tera-shard", "Ghost Tera Shard"},
        {PrebuiltOptions::GRASS_TERA, "grass-tera-shard", "Grass Tera Shard"},
        {PrebuiltOptions::GROUND_TERA, "ground-tera-shard", "Ground Tera Shard"},
        {PrebuiltOptions::ICE_TERA, "ice-tera-shard", "Ice Tera Shard"},
        {PrebuiltOptions::NORMAL_TERA, "normal-tera-shard", "Normal Tera Shard"},
        {PrebuiltOptions::POISON_TERA, "poison-tera-shard", "Poison Tera Shard"},
        {PrebuiltOptions::PSYCHIC_TERA, "psychic-tera-shard", "Psychic Tera Shard"},
        {PrebuiltOptions::ROCK_TERA, "rock-tera-shard", "Rock Tera Shard"},
        {PrebuiltOptions::STEEL_TERA, "steel-tera-shard", "Steel Tera Shard"},
        {PrebuiltOptions::WATER_TERA, "water-tera-shard", "Water Tera Shard"},
        {PrebuiltOptions::STELLAR_TERA, "stellar-tera-shard", "Stellar Tera Shard"},

    });
    return database;
}


static const std::vector<ItemPrinterEnumOption>& ItemPrinter_AllOptions(){
    // - Specific item seeds taken from Anubis's Item printer seeds
    //   - https://gist.github.com/Lusamine/112d4230919fadd254f0e6dfca850471
    // - How I chose the specific item seeds
    //   - seeds (and adjacent seeds +/- 2) that didn't also trigger item/ball bonuses. This is because
    // accidentally triggering a ball bonus, may prevent you from intentionally triggering an item bonus
    // on the next job, and vice versa for the item bonus.
    //   - seconds value: minimum of 8. ideally less than 20.
    // - Confirmation of seeds done with Kurt's ItemPrinterDeGacha tool
    static const std::vector<ItemPrinterEnumOption> database{
        // 2044-05-06 15:33:08
        // Calcium
        {2346161588, PrebuiltOptions::ITEM_BONUS,       ItemPrinterJobs::Jobs_1, 0},

        // 2024-06-04 00:37:08
        // Full Restore
        {1717461428, PrebuiltOptions::BALL_BONUS,       ItemPrinterJobs::Jobs_1, 0},

        // 958172368, 2000-05-12 22:59:28
        // 16 total Ability Patches
        // x36 Dark Tera Shard
        // x4 Ability Patch
        // x4 Ability Patch
        // x4 Ability Patch
        // x4 Ability Patch
        {958172368,  PrebuiltOptions::ABILITY_PATCH,    ItemPrinterJobs::Jobs_5, 16},

        // 1991472489, 2033-02-08 10:48:09
        // 1040000 exp, 28 total Exp. Candy XL
        // x10 Exp. Candy XL
        // x4 Protein
        // x20 Exp. Candy L
        // x8 Exp. Candy XL
        // x10 Exp. Candy XL
        {1991472489, PrebuiltOptions::EXP_CANDY,        ItemPrinterJobs::Jobs_5, 28},

        // 2030-03-11 16:44:09
        // 18 total PP Maxes
        // 6x PP Max, 6x PP Max, 14x Honey, 10x Revive, 6x PP Max
        {1899477849, PrebuiltOptions::PP_MAX,           ItemPrinterJobs::Jobs_5, 18},

        // 2049-08-18 23:51:08
        {2512943468, PrebuiltOptions::ASSORTED_BALLS_1, ItemPrinterJobs::Jobs_10, 1},

        // 2031-10-08 07:09:09
        {1949209749, PrebuiltOptions::ASSORTED_BALLS_2, ItemPrinterJobs::Jobs_10, 1},

        // 2020-03-03 06:38:18
        {1583217498, PrebuiltOptions::ASSORTED_BALLS_3, ItemPrinterJobs::Jobs_10, 1},

        // 2005-11-07 11:32:08
        // x1 Master Ball
        // x1 Master Ball
        // x5 Great Ball
        // x1 Master Ball
        // x1 Master Ball
        {1131363128, PrebuiltOptions::MASTER_BALL,      ItemPrinterJobs::Jobs_5, 4},

        // 2034-09-01 02:33:39
        {2040690819, PrebuiltOptions::FAST_BALL,        ItemPrinterJobs::Jobs_5, 5},

        // 2046-12-11 02:23:17
        {2428107797, PrebuiltOptions::FRIEND_BALL,      ItemPrinterJobs::Jobs_5, 5},

        // 2057-11-21 04:28:37
        {2773542517, PrebuiltOptions::LURE_BALL,        ItemPrinterJobs::Jobs_5, 5},

        // 2045-03-17 10:05:38
        {2373357938, PrebuiltOptions::LEVEL_BALL,       ItemPrinterJobs::Jobs_5, 5},

        // 2029-07-24 11:53:11
        {1879588391, PrebuiltOptions::HEAVY_BALL,       ItemPrinterJobs::Jobs_5, 5},

        // 2023-05-22 05:36:11
        {1684733771, PrebuiltOptions::LOVE_BALL,        ItemPrinterJobs::Jobs_5, 5},

        // 2042-10-14 05:56:33
        {2296878993, PrebuiltOptions::MOON_BALL,        ItemPrinterJobs::Jobs_5, 5},

        // 2054-03-22 09:31:21
        {2657784681, PrebuiltOptions::DREAM_BALL,       ItemPrinterJobs::Jobs_5, 5},

        // 2009-06-05 20:17:11
        {1244233031, PrebuiltOptions::SPORT_BALL,       ItemPrinterJobs::Jobs_5, 5},

        // 2049-01-02 01:20:22
        {2493163222, PrebuiltOptions::SAFARI_BALL,      ItemPrinterJobs::Jobs_5, 5},

        // 2037-02-02 15:37:15
        {2117201835, PrebuiltOptions::BEAST_BALL,       ItemPrinterJobs::Jobs_5, 5},

        // 2246577010, 2041-03-11 01:10:10
        // 134 Normal Tera Shards, 134 total Tera Shards
        // x40 Normal Tera Shard
        // x28 Normal Tera Shard
        // x16 Normal Tera Shard
        // x38 Normal Tera Shard
        // x12 Normal Tera Shard
        {2246577010, PrebuiltOptions::NORMAL_TERA,      ItemPrinterJobs::Jobs_5, 134},

        // 2669513710, 2054-08-05 03:35:10
        // 148 Fire Tera Shards, 148 total Tera Shards
        // x40 Fire Tera Shard
        // x36 Fire Tera Shard
        // x40 Fire Tera Shard
        // x32 Fire Tera Shard
        // x20 Honey
        {2669513710, PrebuiltOptions::FIRE_TERA,        ItemPrinterJobs::Jobs_5, 148},

        // 2419526534, 2046-09-02 18:42:14
        // 120 Water Tera Shards, 154 total Tera Shards
        // x40 Water Tera Shard
        // x8 Full Restore
        // x40 Water Tera Shard
        // x34 Flying Tera Shard
        // x40 Water Tera Shard        
        {2419526534, PrebuiltOptions::WATER_TERA,       ItemPrinterJobs::Jobs_5, 120},

        // 1300121653, 2011-03-14 16:54:13
        // 138 Electric Tera Shards, 138 total Tera Shards
        // x26 Electric Tera Shard
        // x4 Light Clay
        // x40 Electric Tera Shard
        // x40 Electric Tera Shard
        // x32 Electric Tera Shard
        {1300121653, PrebuiltOptions::ELECTRIC_TERA,    ItemPrinterJobs::Jobs_5, 138},

        // 1258326250, 2009-11-15 23:04:10
        // 122 Grass Tera Shards, 122 total Tera Shards
        // x30 Grass Tera Shard
        // x12 Tiny Mushroom
        // x38 Grass Tera Shard
        // x34 Grass Tera Shard
        // x20 Grass Tera Shard
        {1258326250, PrebuiltOptions::GRASS_TERA,       ItemPrinterJobs::Jobs_5, 122},

        // 2567326030, 2051-05-10 10:07:10
        // 134 Ice Tera Shards, 134 total Tera Shards
        // x38 Ice Tera Shard
        // x16 Ice Tera Shard
        // x40 Ice Tera Shard
        // x40 Ice Tera Shard
        // x2 Metal Alloy
        {2567326030, PrebuiltOptions::ICE_TERA,         ItemPrinterJobs::Jobs_5, 134},

        // 1510682535, 2017-11-14 18:02:15
        // 132 Fighting Tera Shards, 132 total Tera Shards
        // x40 Fighting Tera Shard
        // x32 Fighting Tera Shard
        // x20 Fighting Tera Shard
        // x40 Fighting Tera Shard
        // x6 Shiny Stone
        {1510682535, PrebuiltOptions::FIGHTING_TERA,    ItemPrinterJobs::Jobs_5, 132},

        // 2041396572, 2034-09-09 06:36:12
        // 140 Poison Tera Shards, 140 total Tera Shards
        // x4 Electirizer
        // x32 Poison Tera Shard
        // x36 Poison Tera Shard
        // x38 Poison Tera Shard
        // x34 Poison Tera Shard
        {2041396572, PrebuiltOptions::POISON_TERA,      ItemPrinterJobs::Jobs_5, 140},

        // 1463321889, 2016-05-15 14:18:09
        // 130 Ground Tera Shards, 130 total Tera Shards
        // x10 PP Up
        // x28 Ground Tera Shard
        // x34 Ground Tera Shard
        // x30 Ground Tera Shard
        // x38 Ground Tera Shard
        {1463321889, PrebuiltOptions::GROUND_TERA,      ItemPrinterJobs::Jobs_5, 130},

        // 1580633500, 2020-02-02 08:51:40
        // 128 Flying Tera Shards, 128 total Tera Shards
        // x22 Flying Tera Shard
        // x30 Flying Tera Shard
        // x28 Flying Tera Shard
        // x38 Flying Tera Shard
        // x10 Flying Tera Shard
        {1580633500, PrebuiltOptions::FLYING_TERA,      ItemPrinterJobs::Jobs_5, 128},

        // 2190593713, 2039-06-02 02:15:13
        // 134 Psychic Tera Shards, 134 total Tera Shards
        // x30 Psychic Tera Shard
        // x4 Never-Melt Ice
        // x28 Psychic Tera Shard
        // x36 Psychic Tera Shard
        // x40 Psychic Tera Shard
        {2190593713, PrebuiltOptions::PSYCHIC_TERA,     ItemPrinterJobs::Jobs_5, 134},

        // 2458109848, 2047-11-23 08:17:28
        // 134 Bug Tera Shards, 134 total Tera Shards
        // x4 Ether
        // x38 Bug Tera Shard
        // x34 Bug Tera Shard
        // x32 Bug Tera Shard
        // x30 Bug Tera Shard
        {2458109848, PrebuiltOptions::BUG_TERA,         ItemPrinterJobs::Jobs_5, 134},

        // 2801954289, 2058-10-16 00:38:09
        // 120 Rock Tera Shards, 120 total Tera Shards
        // x40 Rock Tera Shard
        // x40 Rock Tera Shard
        // x16 Pretty Feather
        // x40 Rock Tera Shard
        // x18 Stardust
        {2801954289, PrebuiltOptions::ROCK_TERA,        ItemPrinterJobs::Jobs_5, 120},

        // 1509904271, 2017-11-05 17:51:11
        // 122 Ghost Tera Shards, 122 total Tera Shards
        // x38 Ghost Tera Shard
        // x36 Ghost Tera Shard
        // x12 Ghost Tera Shard
        // x4 Magnet
        // x36 Ghost Tera Shard
        {1509904271, PrebuiltOptions::GHOST_TERA,       ItemPrinterJobs::Jobs_5, 122},

        // 2155674309, 2038-04-23 22:25:09
        // 136 Dragon Tera Shards, 136 total Tera Shards
        // x2 Ability Patch
        // x24 Dragon Tera Shard
        // x38 Dragon Tera Shard
        // x40 Dragon Tera Shard
        // x34 Dragon Tera Shard
        {2155674309, PrebuiltOptions::DRAGON_TERA,      ItemPrinterJobs::Jobs_5, 136},

        // 2537180413, 2050-05-26 12:20:13
        // 120 Dark Tera Shards, 120 total Tera Shards
        // x20 Dark Tera Shard
        // x28 Dark Tera Shard
        // x32 Dark Tera Shard
        // x40 Dark Tera Shard
        // x4 Prism Scale
        {2537180413, PrebuiltOptions::DARK_TERA,        ItemPrinterJobs::Jobs_5, 120},

        // 1766067369, 2025-12-18 14:16:09
        // 120 Steel Tera Shards, 120 total Tera Shards
        // x40 Steel Tera Shard
        // x24 Steel Tera Shard
        // x38 Steel Tera Shard
        // x18 Steel Tera Shard
        // x4 Berry Sweet
        {1766067369, PrebuiltOptions::STEEL_TERA,       ItemPrinterJobs::Jobs_5, 120},

        // 2183028974, 2039-03-06 12:56:14
        // 120 Fairy Tera Shards, 134 total Tera Shards
        // x40 Fairy Tera Shard
        // x40 Fairy Tera Shard
        // x14 Water Tera Shard
        // x12 Tiny Bamboo Shoot
        // x40 Fairy Tera Shard
        {2183028974, PrebuiltOptions::FAIRY_TERA,       ItemPrinterJobs::Jobs_5, 120},

        // 1165654034, 2006-12-09 08:47:14
        // 88 Stellar Tera Shards, 88 total Tera Shards
        // x30 Stellar Tera Shard
        // x6 Max Revive
        // x28 Stellar Tera Shard
        // x30 Stellar Tera Shard
        // x2 Clover Sweet
        {1165654034, PrebuiltOptions::STELLAR_TERA,     ItemPrinterJobs::Jobs_5, 88},
    };
    return database;
};

static std::map<PrebuiltOptions, const ItemPrinterEnumOption*> make_ItemPrinter_option_lookup_by_enum(){
    std::map<PrebuiltOptions, const ItemPrinterEnumOption*> ret;
    for (const ItemPrinterEnumOption& item : ItemPrinter_AllOptions()){
        if (!ret.emplace(item.enum_value, &item).second){
            throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Duplicate Enum: " + std::to_string((int)item.enum_value));
        }
    }
    return ret;
}
const ItemPrinterEnumOption& option_lookup_by_enum(PrebuiltOptions enum_value){
    static const std::map<PrebuiltOptions, const ItemPrinterEnumOption*> database = make_ItemPrinter_option_lookup_by_enum();
    auto iter = database.find(enum_value);
    if (iter != database.end()){
        return *iter->second;
    }else{
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Unknown Enum: " + std::to_string((int)enum_value));
    }
}
static std::map<int64_t, const ItemPrinterEnumOption*> make_ItemPrinter_option_lookup_by_seed(){
    std::map<int64_t, const ItemPrinterEnumOption*> ret;
    for (const ItemPrinterEnumOption& item : ItemPrinter_AllOptions()){
        if (!ret.emplace(item.seed, &item).second){
            throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Duplicate Seed: " + std::to_string(item.seed));
        }
    }
    return ret;
}
const ItemPrinterEnumOption* option_lookup_by_seed(int64_t seed){
    static const std::map<int64_t, const ItemPrinterEnumOption*> database = make_ItemPrinter_option_lookup_by_seed();
    auto iter = database.find(seed);
    if (iter != database.end()){
        return iter->second;
    }else{
        return nullptr;
    }
}



DateSeed get_date_seed(int64_t seed){
    //  This is the function that we would replace with Kurt's seed calculator.

    static const std::map<int64_t, DateSeed> DATABASE{
        {2346161588, {2346161588, {"calcium"}}},
        {2346161588 - 2, {2346161588 - 2, {"stardust"}}},
        {2346161588 - 1, {2346161588 - 1, {"tiny-mushroom"}}},
        {2346161588 + 1, {2346161588 + 1, {"max-ether"}}},
        {2346161588 + 2, {2346161588 + 2, {"electric-tera-shard"}}},

        {1717461428, {1717461428, {"full-restore"}}},
        {1717461428 - 2, {1717461428 - 2, {"balm-mushroom"}}},
        {1717461428 - 1, {1717461428 - 1, {"revive"}}},
        {1717461428 + 1, {1717461428 + 1, {"fairy-tera-shard"}}},
        {1717461428 + 2, {1717461428 + 2, {"big-bamboo-shoot"}}},

        {958172368, {958172368, {"dark-tera-shard", "ability-patch", "ability-patch", "ability-patch", "ability-patch"}}},
        {958172368 - 2, {958172368 - 2, {"dark-tera-shard", "revive", "reaper-cloth", "ability-patch", "poison-tera-shard"}}},
        {958172368 - 1, {958172368 - 1, {"dark-tera-shard", "fire-tera-shard", "big-bamboo-shoot", "balm-mushroom", "super-potion"}}},
        {958172368 + 1, {958172368 + 1, {"dark-tera-shard", "pearl", "spell-tag", "flower-sweet", "ribbon-sweet"}}},
        {958172368 + 2, {958172368 + 2, {"dark-tera-shard", "water-stone", "max-potion", "tiny-mushroom", "electric-tera-shard"}}},

        {1991472489, {1991472489, {"exp-candy-xl", "protein", "exp-candy-l", "exp-candy-xl", "exp-candy-xl"}}},
        {1991472489 - 2, {1991472489 - 2, {"exp-candy-xl", "tiny-bamboo-shoot", "exp-candy-s", "iron", "tiny-bamboo-shoot"}}},
        {1991472489 - 1, {1991472489 - 1, {"exp-candy-xl", "toxic-orb", "rare-bone", "silver-powder", "lucky-egg"}}},
        {1991472489 + 1, {1991472489 + 1, {"exp-candy-xl", "silver-powder", "dubious-disc", "mystic-water", "toxic-orb"}}},
        {1991472489 + 2, {1991472489 + 2, {"exp-candy-xl", "tiny-bamboo-shoot", "toxic-orb", "ice-stone", "charcoal"}}},

        {2246577010, {2246577010, {"normal-tera-shard", "normal-tera-shard", "normal-tera-shard", "normal-tera-shard", "normal-tera-shard"}}},
        {2246577010 - 2, {2246577010 - 2, {"normal-tera-shard", "exp-candy-l", "star-piece", "tiny-mushroom", "rock-tera-shard"}}},
        {2246577010 - 1, {2246577010 - 1, {"normal-tera-shard", "oval-stone", "ether", "stardust", "steel-tera-shard"}}},
        {2246577010 + 1, {2246577010 + 1, {"normal-tera-shard", "exp-candy-s", "ether", "elixir", "big-pearl"}}},
        {2246577010 + 2, {2246577010 + 2, {"normal-tera-shard", "dragon-tera-shard", "ice-tera-shard", "exp-candy-xl", "black-sludge"}}},

        {2669513710, {2669513710, {"fire-tera-shard", "fire-tera-shard", "fire-tera-shard", "fire-tera-shard", "honey"}}},
        {2669513710 - 2, {2669513710 - 2, {"fire-tera-shard", "flame-orb", "steel-tera-shard", "fire-stone", "max-revive"}}},
        {2669513710 - 1, {2669513710 - 1, {"fire-tera-shard", "exp-candy-xs", "miracle-seed", "sun-stone", "iron"}}},
        {2669513710 + 1, {2669513710 + 1, {"fire-tera-shard", "poison-tera-shard", "love-sweet", "safety-goggles", "exp-candy-m"}}},
        {2669513710 + 2, {2669513710 + 2, {"fire-tera-shard", "rare-bone", "dubious-disc", "berry-sweet", "hp-up"}}},

        {2419526534, {2419526534, {"water-tera-shard", "full-restore", "water-tera-shard", "flying-tera-shard", "water-tera-shard"}}},
        {2419526534 - 2, {2419526534 - 2, {"water-tera-shard", "exp-candy-xs", "flame-orb", "exp-candy-s", "honey"}}},
        {2419526534 - 1, {2419526534 - 1, {"water-tera-shard", "never-melt-ice", "rock-tera-shard", "hp-up", "sun-stone"}}},
        {2419526534 + 1, {2419526534 + 1, {"water-tera-shard", "heavy-duty-boots", "moon-stone", "strawberry-sweet", "exp-candy-xs"}}},
        {2419526534 + 2, {2419526534 + 2, {"water-tera-shard", "ghost-tera-shard", "max-revive", "ground-tera-shard", "exp-candy-xl"}}},

        {1300121653, {1300121653, {"electric-tera-shard", "light-clay", "electric-tera-shard", "electric-tera-shard", "electric-tera-shard"}}},
        {1300121653 - 2, {1300121653 - 2, {"electric-tera-shard", "elixir", "full-restore", "upgrade", "toxic-orb"}}},
        {1300121653 - 1, {1300121653 - 1, {"electric-tera-shard", "exp-candy-xs", "steel-tera-shard", "big-mushroom", "exp-candy-s"}}},
        {1300121653 + 1, {1300121653 + 1, {"electric-tera-shard", "calcium", "ground-tera-shard", "water-tera-shard", "upgrade"}}},
        {1300121653 + 2, {1300121653 + 2, {"electric-tera-shard", "max-potion", "charcoal", "exp-candy-xs", "grass-tera-shard"}}},

        {1258326250, {1258326250, {"grass-tera-shard", "tiny-mushroom", "grass-tera-shard", "grass-tera-shard", "grass-tera-shard"}}},
        {1258326250 - 2, {1258326250 - 2, {"grass-tera-shard", "pretty-feather", "exp-candy-m", "bug-tera-shard", "tiny-mushroom"}}},
        {1258326250 - 1, {1258326250 - 1, {"grass-tera-shard", "ghost-tera-shard", "bug-tera-shard", "silver-powder", "berry-sweet"}}},
        {1258326250 + 1, {1258326250 + 1, {"grass-tera-shard", "steel-tera-shard", "honey", "strawberry-sweet", "normal-tera-shard"}}},
        {1258326250 + 2, {1258326250 + 2, {"grass-tera-shard", "exp-candy-xl", "exp-candy-xs", "zinc", "dragon-fang"}}},

        {2567326030, {2567326030, {"ice-tera-shard", "ice-tera-shard", "ice-tera-shard", "ice-tera-shard", "metal-alloy"}}},
        {2567326030 - 2, {2567326030 - 2, {"ice-tera-shard", "pearl-string", "dragon-tera-shard", "sun-stone", "water-tera-shard"}}},
        {2567326030 - 1, {2567326030 - 1, {"ice-tera-shard", "scope-lens", "full-restore", "dubious-disc", "lucky-egg"}}},
        {2567326030 + 1, {2567326030 + 1, {"ice-tera-shard", "poison-barb", "leaf-stone", "cracked-pot", "ghost-tera-shard"}}},
        {2567326030 + 2, {2567326030 + 2, {"ice-tera-shard", "exp-candy-xs", "rock-tera-shard", "loaded-dice", "Pretty Feather"}}},

        {1510682535, {1510682535, {"fighting-tera-shard", "fighting-tera-shard", "fighting-tera-shard", "fighting-tera-shard", "shiny-stone"}}},
        {1510682535 - 2, {1510682535 - 2, {"fairy-feather", "normal-tera-shard", "air-balloon", "poison-tera-shard", "pearl"}}},
        {1510682535 - 1, {1510682535 - 1, {"strawberry-sweet", "electric-tera-shard", "calcium", "revive", "ghost-tera-shard"}}},
        {1510682535 + 1, {1510682535 + 1, {"miracle-seed", "fighting-tera-shard", "ice-tera-shard", "light-clay", "dark-tera-shard"}}},
        {1510682535 + 2, {1510682535 + 2, {"rare-bone", "max-elixir", "leaf-stone", "metal-alloy", "magmarizer"}}},

        {2041396572, {2041396572, {"electirizer", "poison-tera-shard", "poison-tera-shard", "poison-tera-shard", "poison-tera-shard"}}},
        {2041396572 - 2, {2041396572 - 2, {"electirizer", "upgrade", "zinc", "elixir", "revive"}}},
        {2041396572 - 1, {2041396572 - 1, {"electirizer", "flame-orb", "magmarizer", "hp-up", "pretty-feather"}}},
        {2041396572 + 1, {2041396572 + 1, {"electirizer", "poison-tera-shard", "poison-tera-shard", "poison-tera-shard", "poison-tera-shard"}}},
        {2041396572 + 2, {2041396572 + 2, {"electirizer", "pp-up", "rock-tera-shard", "big-mushroom", "light-clay"}}},

        {1463321889, {1463321889, {"pp-up", "ground-tera-shard", "ground-tera-shard", "ground-tera-shard", "ground-tera-shard"}}},
        {1463321889 - 2, {1463321889 - 2, {"pp-up", "pretty-feather", "max-ether", "big-pearl", "carbos"}}},
        {1463321889 - 1, {1463321889 - 1, {"pp-up", "spell-tag", "bug-tera-shard", "pearl", "fighting-tera-shard"}}},
        {1463321889 + 1, {1463321889 + 1, {"pp-up", "metal-alloy", "full-restore", "pearl", "silver-powder"}}},
        {1463321889 + 2, {1463321889 + 2, {"pp-up", "ghost-tera-shard", "nugget", "safety-goggles", "leftovers"}}},

        {1580633500, {1580633500, {"flying-tera-shard", "flying-tera-shard", "flying-tera-shard", "flying-tera-shard", "flying-tera-shard"}}},
        {1580633500 - 2, {1580633500 - 2, {"black-glasses", "big-nugget", "max-revive", "pearl-string", "balm-mushroom"}}},
        {1580633500 - 1, {1580633500 - 1, {"loaded-dice", "exp-candy-s", "strawberry-sweet", "never-melt-ice", "moon-stone"}}},
        {1580633500 + 1, {1580633500 + 1, {"water-stone", "rare-bone", "max-ether", "moon-stone", "ability-patch"}}},
        {1580633500 + 2, {1580633500 + 2, {"max-ether", "big-bamboo-shoot", "iron", "big-mushroom", "pearl"}}},

        {2190593713, {2190593713, {"psychic-tera-shard", "never-melt-ice", "psychic-tera-shard", "psychic-tera-shard", "psychic-tera-shard"}}},
        {2190593713 - 2, {2190593713 - 2, {"psychic-tera-shard", "big-mushroom", "fairy-feather", "fairy-tera-shard", "reaper-cloth"}}},
        {2190593713 - 1, {2190593713 - 1, {"psychic-tera-shard", "electric-tera-shard", "pearl", "stellar-tera-shard", "dawn-stone"}}},
        {2190593713 + 1, {2190593713 + 1, {"psychic-tera-shard", "pearl-string", "twisted-spoon", "big-pearl", "iron"}}},
        {2190593713 + 2, {2190593713 + 2, {"psychic-tera-shard", "heavy-duty-boots", "water-tera-shard", "rock-tera-shard", "twisted-spoon"}}},

        {2458109848, {2458109848, {"ether", "bug-tera-shard", "bug-tera-shard", "bug-tera-shard", "bug-tera-shard"}}},
        {2458109848 - 2, {2458109848 - 2, {"ether", "big-nugget", "mystic-water", "pretty-feather", "pretty-feather"}}},
        {2458109848 - 1, {2458109848 - 1, {"ether", "psychic-tera-shard", "exp-candy-s", "honey", "sharp-beak"}}},
        {2458109848 + 1, {2458109848 + 1, {"ether", "black-glasses", "fighting-tera-shard", "tiny-bamboo-shoot", "poison-barb"}}},
        {2458109848 + 2, {2458109848 + 2, {"ether", "exp-candy-xl", "normal-tera-shard", "fairy-tera-shard", "balm-mushroom"}}},

        {2801954289, {2801954289, {"rock-tera-shard", "rock-tera-shard", "pretty-feather", "rock-tera-shard", "stardust"}}},
        {2801954289 - 2, {2801954289 - 2, {"max-revive", "scope-lens", "razor-claw", "big-nugget", "tiny-bamboo-shoot"}}},
        {2801954289 - 1, {2801954289 - 1, {"iron", "dark-tera-shard", "air-balloon", "rare-bone", "ability-patch"}}},
        {2801954289 + 1, {2801954289 + 1, {"normal-tera-shard", "ground-tera-shard", "honey", "upgrade", "honey"}}},
        {2801954289 + 2, {2801954289 + 2, {"heavy-duty-boots", "ice-tera-shard", "sun-stone", "dubious-disc", "pearl"}}},

        {1509904271, {1509904271, {"ghost-tera-shard", "ghost-tera-shard", "ghost-tera-shard", "magnet", "ghost-tera-shard"}}},
        {1509904271 - 2, {1509904271 - 2, {"ghost-tera-shard", "hyper-potion", "toxic-orb", "magmarizer", "black-glasses"}}},
        {1509904271 - 1, {1509904271 - 1, {"ghost-tera-shard", "black-belt", "pp-up", "ice-tera-shard", "magmarizer"}}},
        {1509904271 + 1, {1509904271 + 1, {"ghost-tera-shard", "big-nugget", "hp-up", "gold-bottle-cap", "big-bamboo-shoot"}}},
        {1509904271 + 2, {1509904271 + 2, {"ghost-tera-shard", "hp-up", "elixir", "water-tera-shard", "pp-up"}}},

        {2155674309, {2155674309, {"ability-patch", "dragon-tera-shard", "dragon-tera-shard", "dragon-tera-shard", "dragon-tera-shard"}}},
        {2155674309 - 2, {2155674309 - 2, {"comet-shard", "soft-sand", "big-mushroom", "steel-tera-shard", "metronome"}}},
        {2155674309 - 1, {2155674309 - 1, {"pearl", "balm-mushroom", "moon-stone", "protein", "ability-patch"}}},
        {2155674309 + 1, {2155674309 + 1, {"reaper-cloth", "hp-up", "revive", "light-clay", "focus-band"}}},
        {2155674309 + 2, {2155674309 + 2, {"spell-tag", "love-sweet", "fire-tera-shard", "ether", "black-belt"}}},

        {2537180413, {2537180413, {"dark-tera-shard", "dark-tera-shard", "dark-tera-shard", "dark-tera-shard", "prism-scale"}}},
        {2537180413 - 2, {2537180413 - 2, {"dark-tera-shard", "upgrade", "big-mushroom", "normal-tera-shard", "exp-candy-m"}}},
        {2537180413 - 1, {2537180413 - 1, {"dark-tera-shard", "pp-max", "calcium", "rock-tera-shard", "hard-stone"}}},
        {2537180413 + 1, {2537180413 + 1, {"dark-tera-shard", "leaf-stone", "big-pearl", "comet-shard", "exp-candy-m"}}},
        {2537180413 + 2, {2537180413 + 2, {"dark-tera-shard", "ground-tera-shard", "fairy-tera-shard", "exp-candy-xs", "fighting-tera-shard"}}},

        {1766067369, {1766067369, {"steel-tera-shard", "steel-tera-shard", "steel-tera-shard", "steel-tera-shard", "berry-sweet"}}},
        {1766067369 - 2, {1766067369 - 2, {"steel-tera-shard", "bug-tera-shard", "exp-candy-xl", "pretty-feather", "silk-scarf"}}},
        {1766067369 - 1, {1766067369 - 1, {"steel-tera-shard", "tiny-mushroom", "super-potion", "sharp-beak", "ice-tera-shard"}}},
        {1766067369 + 1, {1766067369 + 1, {"steel-tera-shard", "moon-stone", "exp-candy-xl", "iron", "exp-candy-xs"}}},
        {1766067369 + 2, {1766067369 + 2, {"steel-tera-shard", "metal-alloy", "ability-patch", "pp-up", "rare-bone"}}},

        {2183028974, {2183028974, {"fairy-tera-shard", "fairy-tera-shard", "water-tera-shard", "tiny-bamboo-shoot", "fairy-tera-shard"}}},
        {2183028974 - 2, {2183028974 - 2, {"max-elixir", "never-melt-ice", "pearl", "max-potion", "pp-up"}}},
        {2183028974 - 1, {2183028974 - 1, {"full-restore", "psychic-tera-shard", "gold-bottle-cap", "dawn-stone", "never-melt-ice"}}},
        {2183028974 + 1, {2183028974 + 1, {"ice-tera-shard", "twisted-spoon", "exp-candy-s", "max-elixir", "razor-fang"}}},
        {2183028974 + 2, {2183028974 + 2, {"exp-candy-xl", "bug-tera-shard", "star-piece", "hp-up", "bug-tera-shard"}}},

        {1165654034, {1165654034, {"stellar-tera-shard", "max-revive", "stellar-tera-shard", "stellar-tera-shard", "clover-sweet"}}},
        {1165654034 - 2, {1165654034 - 2, {"stellar-tera-shard", "pp-up", "scope-lens", "electric-tera-shard", "metronome"}}},
        {1165654034 - 1, {1165654034 - 1, {"stellar-tera-shard", "dragon-tera-shard", "honey", "steel-tera-shard", "max-potion"}}},
        {1165654034 + 1, {1165654034 + 1, {"stellar-tera-shard", "full-heal", "toxic-orb", "max-potion", "miracle-seed"}}},
        {1165654034 + 2, {1165654034 + 2, {"stellar-tera-shard", "big-bamboo-shoot", "poison-tera-shard", "honey", "oval-stone"}}},

        {1131363128, {1131363128, {}, {"master-ball", "master-ball", "great-ball", "master-ball", "master-ball"}}},
        {1131363128 - 2, {1131363128 - 2, {}, {"master-ball", "great-ball", "quick-ball", "luxury-ball", "heal-ball"}}},
        {1131363128 - 1, {1131363128 - 1, {}, {"master-ball", "great-ball", "timer-ball", "fast-ball", "love-ball"}}},
        {1131363128 + 1, {1131363128 + 1, {}, {"master-ball", "premier-ball", "great-ball", "luxury-ball", "heal-ball"}}},
        {1131363128 + 2, {1131363128 + 2, {}, {"master-ball", "quick-ball", "poke-ball", "premier-ball", "poke-ball"}}},

        {2040690819, {2040690819, {}, {"fast-ball", "fast-ball", "fast-ball", "fast-ball", "fast-ball"}}},
        {2040690819 - 2, {2040690819 - 2, {}, {"fast-ball", "lure-ball", "dream-ball", "friend-ball", "level-ball"}}},
        {2040690819 - 1, {2040690819 - 1, {}, {"fast-ball", "great-ball", "quick-ball", "great-ball", "heal-ball"}}},
        {2040690819 + 1, {2040690819 + 1, {}, {"fast-ball", "dream-ball", "luxury-ball", "luxury-ball", "premier-ball"}}},
        {2040690819 + 2, {2040690819 + 2, {}, {"fast-ball", "luxury-ball", "great-ball", "poke-ball", "premier-ball"}}},

        {2428107797, {2428107797, {}, {"friend-ball", "friend-ball", "friend-ball", "friend-ball", "friend-ball"}}},
        {2428107797 - 2, {2428107797 - 2, {}, {"friend-ball", "great-ball", "sport-ball", "great-ball", "great-ball"}}},
        {2428107797 - 1, {2428107797 - 1, {}, {"friend-ball", "premier-ball", "heal-ball", "repeat-ball", "nest-ball"}}},
        {2428107797 + 1, {2428107797 + 1, {}, {"friend-ball", "premier-ball", "heavy-ball", "timer-ball", "friend-ball"}}},
        {2428107797 + 2, {2428107797 + 2, {}, {"friend-ball", "heal-ball", "great-ball", "beast-ball", "moon-ball"}}},

        {2773542517, {2773542517, {}, {"lure-ball", "lure-ball", "lure-ball", "lure-ball", "lure-ball"}}},
        {2773542517 - 2, {2773542517 - 2, {}, {"lure-ball", "premier-ball", "premier-ball", "great-ball", "poke-ball"}}},
        {2773542517 - 1, {2773542517 - 1, {}, {"dream-ball", "poke-ball", "poke-ball", "luxury-ball", "poke-ball"}}},
        {2773542517 + 1, {2773542517 + 1, {}, {"luxury-ball", "premier-ball", "great-ball", "luxury-ball", "fast-ball"}}},
        {2773542517 + 2, {2773542517 + 2, {}, {"timer-ball", "safari-ball", "dive-ball", "poke-ball", "premier-ball"}}},

        {2373357938, {2373357938, {}, {"level-ball", "level-ball", "level-ball", "level-ball", "level-ball"}}},
        {2373357938 - 2, {2373357938 - 2, {}, {"dusk-ball", "heal-ball", "luxury-ball", "premier-ball", "poke-ball"}}},
        {2373357938 - 1, {2373357938 - 1, {}, {"great-ball", "dive-ball", "dream-ball", "luxury-ball", "luxury-ball"}}},
        {2373357938 + 1, {2373357938 + 1, {}, {"repeat-ball", "heal-ball", "heal-ball", "luxury-ball", "dive-ball"}}},
        {2373357938 + 2, {2373357938 + 2, {}, {"luxury-ball", "master-ball", "premier-ball", "ultra-ball", "heal-ball"}}},

        {1879588391, {1879588391, {}, {"heavy-ball", "heavy-ball", "heavy-ball", "heavy-ball", "heavy-ball"}}},
        {1879588391 - 2, {1879588391 - 2, {}, {"heavy-ball", "nest-ball", "nest-ball", "heal-ball", "luxury-ball"}}},
        {1879588391 - 1, {1879588391 - 1, {}, {"heavy-ball", "poke-ball", "premier-ball", "premier-ball", "heal-ball"}}},
        {1879588391 + 1, {1879588391 + 1, {}, {"heavy-ball", "heal-ball", "premier-ball", "timer-ball", "premier-ball"}}},
        {1879588391 + 2, {1879588391 + 2, {}, {"heavy-ball", "ultra-ball", "heal-ball", "quick-ball", "beast-ball"}}},

        {1684733771, {1684733771, {}, {"love-ball", "love-ball", "love-ball", "love-ball", "love-ball"}}},
        {1684733771 - 2, {1684733771 - 2, {}, {"love-ball", "sport-ball", "great-ball", "poke-ball", "poke-ball"}}},
        {1684733771 - 1, {1684733771 - 1, {}, {"love-ball", "poke-ball", "great-ball", "luxury-ball", "master-ball"}}},
        {1684733771 + 1, {1684733771 + 1, {}, {"love-ball", "great-ball", "luxury-ball", "luxury-ball", "poke-ball"}}},
        {1684733771 + 2, {1684733771 + 2, {}, {"love-ball", "luxury-ball", "premier-ball", "poke-ball", "quick-ball"}}},

        {2296878993, {2296878993, {}, {"moon-ball", "moon-ball", "moon-ball", "moon-ball", "moon-ball"}}},
        {2296878993 - 2, {2296878993 - 2, {}, {"luxury-ball", "timer-ball", "premier-ball", "luxury-ball", "poke-ball"}}},
        {2296878993 - 1, {2296878993 - 1, {}, {"nest-ball", "poke-ball", "heal-ball", "premier-ball", "premier-ball"}}},
        {2296878993 + 1, {2296878993 + 1, {}, {"fast-ball", "heal-ball", "poke-ball", "dusk-ball", "luxury-ball"}}},
        {2296878993 + 2, {2296878993 + 2, {}, {"lure-ball", "heavy-ball", "luxury-ball", "heal-ball", "poke-ball"}}},

        {2657784681, {2657784681, {}, {"dream-ball", "dream-ball", "dream-ball", "dream-ball", "dream-ball"}}},
        {2657784681 - 2, {2657784681 - 2, {}, {"master-ball", "poke-ball", "luxury-ball", "level-ball", "dream-ball"}}},
        {2657784681 - 1, {2657784681 - 1, {}, {"timer-ball", "beast-ball", "poke-ball", "premier-ball", "heal-ball"}}},
        {2657784681 + 1, {2657784681 + 1, {}, {"heavy-ball", "poke-ball", "friend-ball", "fast-ball", "heavy-ball"}}},
        {2657784681 + 2, {2657784681 + 2, {}, {"luxury-ball", "poke-ball", "luxury-ball", "quick-ball", "friend-ball"}}},

        {1244233031, {1244233031, {}, {"sport-ball", "sport-ball", "sport-ball", "sport-ball", "sport-ball"}}},
        {1244233031 - 2, {1244233031 - 2, {}, {"sport-ball", "heal-ball", "luxury-ball", "great-ball", "heal-ball"}}},
        {1244233031 - 1, {1244233031 - 1, {}, {"sport-ball", "dream-ball", "level-ball", "poke-ball", "luxury-ball"}}},
        {1244233031 + 1, {1244233031 + 1, {}, {"moon-ball", "heal-ball", "poke-ball", "poke-ball", "heal-ball"}}},
        {1244233031 + 2, {1244233031 + 2, {}, {"moon-ball", "poke-ball", "ultra-ball", "poke-ball", "great-ball"}}},

        {2493163222, {2493163222, {}, {"safari-ball", "safari-ball", "safari-ball", "safari-ball", "safari-ball"}}},
        {2493163222 - 2, {2493163222 - 2, {}, {"premier-ball", "great-ball", "lure-ball", "ultra-ball", "safari-ball"}}},
        {2493163222 - 1, {2493163222 - 1, {}, {"premier-ball", "great-ball", "poke-ball", "poke-ball", "poke-ball"}}},
        {2493163222 + 1, {2493163222 + 1, {}, {"poke-ball", "poke-ball", "poke-ball", "premier-ball", "great-ball"}}},
        {2493163222 + 2, {2493163222 + 2, {}, {"great-ball", "great-ball", "level-ball", "poke-ball", "safari-ball"}}},

        {2117201835, {2117201835, {}, {"beast-ball", "beast-ball", "beast-ball", "beast-ball", "beast-ball"}}},
        {2117201835 - 2, {2117201835 - 2, {}, {"luxury-ball", "poke-ball", "lure-ball", "master-ball", "dive-ball"}}},
        {2117201835 - 1, {2117201835 - 1, {}, {"heavy-ball", "premier-ball", "premier-ball", "poke-ball", "level-ball"}}},
        {2117201835 + 1, {2117201835 + 1, {}, {"heal-ball", "great-ball", "luxury-ball", "heavy-ball", "great-ball"}}},
        {2117201835 + 2, {2117201835 + 2, {}, {"poke-ball", "premier-ball", "great-ball", "poke-ball", "dive-ball"}}},

        {2512943468, {2512943468, {}, {"level-ball", "level-ball", "friend-ball", "lure-ball", "beast-ball", "moon-ball", "fast-ball", "dream-ball", "love-ball", "beast-ball"}}},
        {2512943468 - 2, {2512943468 - 2, {}, {"level-ball", "dusk-ball", "sport-ball", "great-ball", "dusk-ball", "great-ball", "timer-ball", "premier-ball", "premier-ball", "safari-ball"}}},
        {2512943468 - 1, {2512943468 - 1, {}, {"level-ball", "premier-ball", "dream-ball", "beast-ball", "dream-ball", "premier-ball", "luxury-ball", "lure-ball", "luxury-ball", "poke-ball"}}},
        {2512943468 + 1, {2512943468 + 1, {}, {"level-ball", "premier-ball", "sport-ball", "premier-ball", "poke-ball", "great-ball", "great-ball", "poke-ball", "luxury-ball", "dive-ball"}}},
        {2512943468 + 2, {2512943468 + 2, {}, {"level-ball", "lure-ball", "moon-ball", "heal-ball", "heal-ball", "dream-ball", "great-ball", "safari-ball", "premier-ball", "premier-ball"}}},

        {1949209749, {1949209749, {}, {"love-ball", "dream-ball", "heavy-ball", "lure-ball", "fast-ball", "fast-ball", "safari-ball", "safari-ball", "friend-ball", "heavy-ball"}}},
        {1949209749 - 2, {1949209749 - 2, {}, {"ultra-ball", "luxury-ball", "luxury-ball", "poke-ball", "poke-ball", "moon-ball", "heal-ball", "timer-ball", "dusk-ball", "great-ball"}}},
        {1949209749 - 1, {1949209749 - 1, {}, {"net-ball", "heal-ball", "heavy-ball", "heal-ball", "dive-ball", "great-ball", "great-ball", "premier-ball", "luxury-ball", "love-ball"}}},
        {1949209749 + 1, {1949209749 + 1, {}, {"luxury-ball", "heal-ball", "poke-ball", "poke-ball", "premier-ball", "sport-ball", "luxury-ball", "premier-ball", "poke-ball", "poke-ball"}}},
        {1949209749 + 2, {1949209749 + 2, {}, {"dusk-ball", "lure-ball", "fast-ball", "premier-ball", "great-ball", "luxury-ball", "poke-ball", "safari-ball", "luxury-ball", "premier-ball"}}},

        {1583217498, {1583217498, {}, {"sport-ball", "master-ball", "fast-ball", "master-ball", "dream-ball", "friend-ball", "lure-ball", "sport-ball", "moon-ball", "dream-ball"}}},
        {1583217498 - 2, {1583217498 - 2, {}, {"heal-ball", "net-ball", "poke-ball", "premier-ball", "poke-ball", "repeat-ball", "luxury-ball", "poke-ball", "great-ball", "heal-ball"}}},
        {1583217498 - 1, {1583217498 - 1, {}, {"poke-ball", "heal-ball", "heavy-ball", "luxury-ball", "moon-ball", "timer-ball", "premier-ball", "great-ball", "great-ball", "friend-ball"}}},
        {1583217498 + 1, {1583217498 + 1, {}, {"level-ball", "dive-ball", "great-ball", "luxury-ball", "moon-ball", "friend-ball", "timer-ball", "heal-ball", "poke-ball", "quick-ball"}}},
        {1583217498 + 2, {1583217498 + 2, {}, {"luxury-ball", "friend-ball", "dream-ball", "heal-ball", "luxury-ball", "great-ball", "premier-ball", "dusk-ball", "level-ball", "beast-ball"}}},

        //  Template
//        {1717461428 - 2, {1717461428 - 2, {}, }},
//        {1717461428 - 1, {1717461428 - 1, {}, }},
//        {1717461428 + 1, {1717461428 + 1, {}, }},
//        {1717461428 + 2, {1717461428 + 2, {}, }},

    };

    auto iter = DATABASE.find(seed);
    if (iter != DATABASE.end()){
        return iter->second;
    }

    return DateSeed();
}





}
}
}
}
