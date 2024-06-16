/*  Item Printer RNG
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/PrettyPrint.h"
#include "Common/Qt/TimeQt.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/Exceptions/ProgramFinishedException.h"
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

        {ItemPrinterItems::MASTER_BALL, "master-ball", "Master Ball"},
        {ItemPrinterItems::FAST_BALL, "fast-ball", "Fast Ball"},
        {ItemPrinterItems::FRIEND_BALL, "friend-ball", "Friend Ball"},
        {ItemPrinterItems::LURE_BALL, "lure-ball", "Lure Ball"},
        {ItemPrinterItems::LEVEL_BALL, "level-ball", "Level Ball"},
        {ItemPrinterItems::HEAVY_BALL, "heavy-ball", "Heavy Ball"},
        {ItemPrinterItems::LOVE_BALL, "love-ball", "Love Ball"},
        {ItemPrinterItems::MOON_BALL, "moon-ball", "Moon Ball"},
        {ItemPrinterItems::DREAM_BALL, "dream-ball", "Dream Ball"},
        {ItemPrinterItems::SPORT_BALL, "sport-ball", "Sport Ball"},
        {ItemPrinterItems::SAFARI_BALL, "safari-ball", "Safari Ball"},
        {ItemPrinterItems::BEAST_BALL, "beast-ball", "Beast Ball"},

        {ItemPrinterItems::ASSORTED_BALLS_1, "assorted-balls-1", "Assorted Balls 1"},
        {ItemPrinterItems::ASSORTED_BALLS_2, "assorted-balls-2", "Assorted Balls 2"},
        {ItemPrinterItems::ASSORTED_BALLS_3, "assorted-balls-3", "Assorted Balls 3"},

    });
    return database;
}


struct ItemPrinterEnumOption{
    int64_t seed;
    ItemPrinterItems enum_value;
    ItemPrinterJobs jobs;
};
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
        {2346161588, ItemPrinterItems::ITEM_BONUS,      ItemPrinterJobs::Jobs_1},

        // 2024-06-04 00:37:08
        // Full Restore
        {1717461428, ItemPrinterItems::BALL_BONUS,      ItemPrinterJobs::Jobs_1},

        // 2000-05-12 22:59:28
        // total 8 Ability Patch
        // x18 Dark Tera Shard; x2 Ability Patch; x2 Ability Patch; x2 Ability Patch; x2 Ability Patch
        {958172368,  ItemPrinterItems::ABILITY_PATCH,   ItemPrinterJobs::Jobs_5},

        // 2033-02-08 10:48:09
        // x5 Exp. Candy XL; x2 Protein: x10 Exp. Candy L; x4 Exp. Candy XL; x5 Exp. Candy XL
        // 520000 exp
        {1991472489, ItemPrinterItems::EXP_CANDY,       ItemPrinterJobs::Jobs_5},

        // 2041-03-11 01:10:10
        // x20 Normal Tera Shard; x14 Normal Tera Shard; x8 Normal Tera Shard; x19 Normal Tera Shard; x6 Normal Tera Shard
        // 67 Normal Tera Shards
        {2246577010, ItemPrinterItems::NORMAL_TERA,     ItemPrinterJobs::Jobs_5},

        // 2054-08-05 03:35:10
        // 74 Fire Tera Shards, 74 total Tera Shards
        // x20 Fire Tera Shard; x18 Fire Tera Shard; x20 Fire Tera Shard; x16 Fire Tera Shard; x10 Honey
        {2669513710, ItemPrinterItems::FIRE_TERA,       ItemPrinterJobs::Jobs_5},

        // 2046-09-02 18:42:14
        // 60 Water Tera Shards, 77 total Tera Shards
        // x20 Water Tera Shard; x4 Full Restore; x20 Water Tera Shard; x17 Flying Tera Shard; x20 Water Tera Shard
        {2419526534, ItemPrinterItems::WATER_TERA,      ItemPrinterJobs::Jobs_5},

        // 2011-03-14 16:54:13
        // 69 Electric Tera Shards, 69 total Tera Shards
        // x13 Electric Tera Shard; x2 Light Clay; x20 Electric Tera Shard; x20 Electric Tera Shard; x16 Electric Tera Shard
        {1300121653, ItemPrinterItems::ELECTRIC_TERA,   ItemPrinterJobs::Jobs_5},

        // 2009-11-15 23:04:10
        // 61 Grass Tera Shards, 61 total Tera Shards
        // x15 Grass Tera Shard; x6 Tiny Mushroom; x19 Grass Tera Shard; x17 Grass Tera Shard; x10 Grass Tera Shard
        {1258326250, ItemPrinterItems::GRASS_TERA,      ItemPrinterJobs::Jobs_5},

        // 2051-05-10 10:07:10
        // 67 Ice Tera Shards, 67 total Tera Shards
        // x19 Ice Tera Shard; x8 Ice Tera Shard; x20 Ice Tera Shard; x20 Ice Tera Shard; x1 Metal Alloy
        {2567326030, ItemPrinterItems::ICE_TERA,        ItemPrinterJobs::Jobs_5},

        // 2017-11-14 18:02:15
        // 66 Fighting Tera Shards, 66 total Tera Shards
        // x20 Fighting Tera Shard; x16 Fighting Tera Shard; x10 Fighting Tera Shard; x20 Fighting Tera Shard; x3 Shiny Stone
        {1510682535, ItemPrinterItems::FIGHTING_TERA,   ItemPrinterJobs::Jobs_5},

        // 2034-09-09 06:36:12
        // 70 Poison Tera Shards, 70 total Tera Shards
        // x2 Electirizer; x16 Poison Tera Shard; x18 Poison Tera Shard; x19 Poison Tera Shard; x17 Poison Tera Shard
        {2041396572, ItemPrinterItems::POISON_TERA,     ItemPrinterJobs::Jobs_5},

        // 2016-05-15 14:18:09
        // 65 Ground Tera Shards, 65 total Tera Shards
        // x5 PP Up; x14 Ground Tera Shard; x17 Ground Tera Shard; x15 Ground Tera Shard; x19 Ground Tera Shard
        {1463321889, ItemPrinterItems::GROUND_TERA,     ItemPrinterJobs::Jobs_5},

        // 2020-02-02 08:51:40
        // 64 Flying Tera Shards, 64 total Tera Shards
        // x11 Flying Tera Shard; x15 Flying Tera Shard; x14 Flying Tera Shard; x19 Flying Tera Shard; x5 Flying Tera Shard
        {1580633500, ItemPrinterItems::FLYING_TERA,     ItemPrinterJobs::Jobs_5},

        // 2039-06-02 02:15:13
        // 67 Psychic Tera Shards, 67 total Tera Shards
        // x15 Psychic Tera Shard
        // x2 Never-Melt Ice
        // x14 Psychic Tera Shard
        // x18 Psychic Tera Shard
        // x20 Psychic Tera Shard
        {2190593713, ItemPrinterItems::PSYCHIC_TERA,    ItemPrinterJobs::Jobs_5},

        // 2047-11-23 08:17:28
        // 67 Bug Tera Shards, 67 total Tera Shards
        // x2 Ether
        // x19 Bug Tera Shard
        // x17 Bug Tera Shard
        // x16 Bug Tera Shard
        // x15 Bug Tera Shard
        {2458109848, ItemPrinterItems::BUG_TERA,        ItemPrinterJobs::Jobs_5},

        // 2058-10-16 00:38:09
        // 60 Rock Tera Shards, 60 total Tera Shards
        // x20 Rock Tera Shard
        // x20 Rock Tera Shard
        // x8 Pretty Feather
        // x20 Rock Tera Shard
        // x9 Stardust
        {2801954289, ItemPrinterItems::ROCK_TERA,       ItemPrinterJobs::Jobs_5},

        // 2017-11-05 17:51:11
        // 61 Ghost Tera Shards, 61 total Tera Shards
        // x19 Ghost Tera Shard
        // x18 Ghost Tera Shard
        // x6 Ghost Tera Shard
        // x2 Magnet
        // x18 Ghost Tera Shard
        {1509904271, ItemPrinterItems::GHOST_TERA,      ItemPrinterJobs::Jobs_5},

        // 2038-04-23 22:25:09
        // 68 Dragon Tera Shards, 68 total Tera Shards
        // x1 Ability Patch
        // x12 Dragon Tera Shard
        // x19 Dragon Tera Shard
        // x20 Dragon Tera Shard
        // x17 Dragon Tera Shard
        {2155674309, ItemPrinterItems::DRAGON_TERA,     ItemPrinterJobs::Jobs_5},

        // 2050-05-26 12:20:13
        // 60 Dark Tera Shards, 60 total Tera Shards
        // x10 Dark Tera Shard
        // x14 Dark Tera Shard
        // x16 Dark Tera Shard
        // x20 Dark Tera Shard
        // x2 Prism Scale
        {2537180413, ItemPrinterItems::DARK_TERA,       ItemPrinterJobs::Jobs_5},

        // 2025-12-18 14:16:09
        // 60 Steel Tera Shards, 60 total Tera Shards
        // x20 Steel Tera Shard
        // x12 Steel Tera Shard
        // x19 Steel Tera Shard
        // x9 Steel Tera Shard
        // x2 Berry Sweet
        {1766067369, ItemPrinterItems::STEEL_TERA,      ItemPrinterJobs::Jobs_5},

        // 2039-03-06 12:56:14
        // 60 Fairy Tera Shards, 67 total Tera Shards
        // x20 Fairy Tera Shard
        // x20 Fairy Tera Shard
        // x7 Water Tera Shard
        // x6 Tiny Bamboo Shoot
        // x20 Fairy Tera Shard
        {2183028974, ItemPrinterItems::FAIRY_TERA,      ItemPrinterJobs::Jobs_5},

        // 2006-12-09 08:47:14
        // 44 Stellar Tera Shards, 44 total Tera Shards
        // x15 Stellar Tera Shard
        // x3 Max Revive
        // x14 Stellar Tera Shard
        // x15 Stellar Tera Shard
        // x1 Clover Sweet
        {1165654034, ItemPrinterItems::STELLAR_TERA,    ItemPrinterJobs::Jobs_5},

        // 2005-11-07 11:32:08
        // x1 Master Ball
        // x1 Master Ball
        // x5 Great Ball
        // x1 Master Ball
        // x1 Master Ball
        {1131363128, ItemPrinterItems::MASTER_BALL,     ItemPrinterJobs::Jobs_5},

        // 2034-09-01 02:33:39
        {2040690819, ItemPrinterItems::FAST_BALL,       ItemPrinterJobs::Jobs_5},

        // 2046-12-11 02:23:17
        {2428107797, ItemPrinterItems::FRIEND_BALL,     ItemPrinterJobs::Jobs_5},

        // 2057-11-21 04:28:37
        {2773542517, ItemPrinterItems::LURE_BALL,       ItemPrinterJobs::Jobs_5},

        // 2045-03-17 10:05:38
        {2373357938, ItemPrinterItems::LEVEL_BALL,      ItemPrinterJobs::Jobs_5},

        // 2029-07-24 11:53:11
        {1879588391, ItemPrinterItems::HEAVY_BALL,      ItemPrinterJobs::Jobs_5},

        // 2023-05-22 05:36:11
        {1684733771, ItemPrinterItems::LOVE_BALL,       ItemPrinterJobs::Jobs_5},

        // 2042-10-14 05:56:33
        {2296878993, ItemPrinterItems::MOON_BALL,       ItemPrinterJobs::Jobs_5},

        // 2054-03-22 09:31:21
        {2657784681, ItemPrinterItems::DREAM_BALL,      ItemPrinterJobs::Jobs_5},

        // 2009-06-05 20:17:11
        {1244233031, ItemPrinterItems::SPORT_BALL,      ItemPrinterJobs::Jobs_5},

        // 2049-01-02 01:20:22
        {2493163222, ItemPrinterItems::SAFARI_BALL,     ItemPrinterJobs::Jobs_5},

        // 2037-02-02 15:37:15
        {2117201835, ItemPrinterItems::BEAST_BALL,      ItemPrinterJobs::Jobs_5},

        // 2049-08-18 23:51:08
        {2512943468, ItemPrinterItems::ASSORTED_BALLS_1,ItemPrinterJobs::Jobs_10},

        // 2031-10-08 07:09:09
        {1949209749, ItemPrinterItems::ASSORTED_BALLS_2,ItemPrinterJobs::Jobs_10},

        // 2020-03-03 06:38:18
        {1583217498, ItemPrinterItems::ASSORTED_BALLS_3,ItemPrinterJobs::Jobs_10},
    };
    return database;
};

static std::map<ItemPrinterItems, const ItemPrinterEnumOption*> make_ItemPrinter_option_lookup_by_enum(){
    std::map<ItemPrinterItems, const ItemPrinterEnumOption*> ret;
    for (const ItemPrinterEnumOption& item : ItemPrinter_AllOptions()){
        if (!ret.emplace(item.enum_value, &item).second){
            throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Duplicate Enum: " + std::to_string((int)item.enum_value));
        }
    }
    return ret;
}
static const ItemPrinterEnumOption& option_lookup_by_enum(ItemPrinterItems enum_value){
    static const std::map<ItemPrinterItems, const ItemPrinterEnumOption*> database = make_ItemPrinter_option_lookup_by_enum();
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
static const ItemPrinterEnumOption* option_lookup_by_seed(int64_t seed){
    static const std::map<int64_t, const ItemPrinterEnumOption*> database = make_ItemPrinter_option_lookup_by_seed();
    auto iter = database.find(seed);
    if (iter != database.end()){
        return iter->second;
    }else{
        return nullptr;
    }
}



struct DateSeed{
    //  Seed details.
    int64_t seed;
    std::array<std::string, 10> items;
    std::array<std::string, 10> balls;

    DateSeed(
        int64_t p_seed,
        std::array<std::string, 10> p_items,
        std::array<std::string, 10> p_balls = {}
    )
        : seed(p_seed)
        , items(std::move(p_items))
        , balls(std::move(p_balls))
    {}

    operator bool() const{
        return seed >= 0;
    }
};
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
        {1949209749, {1949209749, {}, {"love-ball", "dream-ball", "heavy-ball", "lure-ball", "fast-ball", "fast-ball", "safari-ball", "safari-ball", "friend-ball", "heavy-ball"}}},
        {1583217498, {1583217498, {}, {"sport-ball", "master-ball", "fast-ball", "master-ball", "dream-ball", "friend-ball", "lure-ball", "sport-ball", "moon-ball", "dream-ball"}}},

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

    return DateSeed{0, {}};
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
        DateTime{2024,  6,  4,  0, 37,  8}
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



// - always update the date's visibility when chain is changed.
// - if desired_item has changed, set the seed (and number of jobs) accordingly
// - if any other value changes, set desired_item to NONE
void ItemPrinterRngRow::value_changed(){
    date.set_visibility(chain ? ConfigOptionState::HIDDEN : ConfigOptionState::ENABLED);

    ItemPrinterItems option = desired_item;
    if (option != ItemPrinterItems::NONE && prev_desired_item != desired_item){ // check if desired_item has changed.
        const ItemPrinterEnumOption& option_data = option_lookup_by_enum(option);

        chain = false;
        DateTime set_date = from_seconds_since_epoch(option_data.seed);
        date.set(set_date);
        jobs.set(option_data.jobs);

        prev_desired_item = option;
        return;
    }

    const ItemPrinterEnumOption* option_data = option_lookup_by_seed(to_seconds_since_epoch(date));
    if (option_data == nullptr){
        desired_item.set(ItemPrinterItems::NONE);
    }else{
        //  Infinite recursion + deadlock.
//        desired_item.set(option_data->enum_value);
    }

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
    ret.emplace_back(std::make_unique<ItemPrinterRngRow>(false, DateTime{2024,  6,  4,  0, 37,  8}, ItemPrinterJobs::Jobs_1));
    ret.emplace_back(std::make_unique<ItemPrinterRngRow>(false, DateTime{2049,  8, 18, 23, 51,  8}, ItemPrinterJobs::Jobs_10));
    ret.emplace_back(std::make_unique<ItemPrinterRngRow>(false, DateTime{2024,  6,  4,  0, 37,  8}, ItemPrinterJobs::Jobs_1));
    ret.emplace_back(std::make_unique<ItemPrinterRngRow>(false, DateTime{2031, 10,  8,  7,  9,  9}, ItemPrinterJobs::Jobs_10));
    ret.emplace_back(std::make_unique<ItemPrinterRngRow>(false, DateTime{2024,  6,  4,  0, 37,  8}, ItemPrinterJobs::Jobs_1));
    ret.emplace_back(std::make_unique<ItemPrinterRngRow>(false, DateTime{2020,  3,  3,  6, 38, 18}, ItemPrinterJobs::Jobs_10));
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
        , frame_hits(m_stats["Frame Hits"])
        , frame_misses(m_stats["Frame Misses"])
        , errors(m_stats["Errors"])
    {
        m_display_order.emplace_back("Rounds");
        m_display_order.emplace_back("Prints");
//        m_display_order.emplace_back("Total Jobs", HIDDEN_IF_ZERO);
        m_display_order.emplace_back("Frame Hits", HIDDEN_IF_ZERO);
        m_display_order.emplace_back("Frame Misses", HIDDEN_IF_ZERO);
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);
    }
    std::atomic<uint64_t>& iterations;
    std::atomic<uint64_t>& prints;
//    std::atomic<uint64_t>& total_jobs;
    std::atomic<uint64_t>& frame_hits;
    std::atomic<uint64_t>& frame_misses;
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
        &LANGUAGE,
        NOTIFICATION_STATUS_UPDATE,
        NOTIFICATION_PROGRAM_FINISH,
        NOTIFICATION_ERROR_RECOVERABLE,
        NOTIFICATION_ERROR_FATAL
    )
{
    PA_ADD_OPTION(LANGUAGE);

    if (PreloadSettings::instance().DEVELOPER_MODE){
        PA_ADD_OPTION(AUTO_MATERIAL_FARMING);
        PA_ADD_OPTION(NUM_ROUNDS_OF_ITEM_PRINTER_TO_MATERIAL_FARM);
    }
    PA_ADD_OPTION(NUM_ITEM_PRINTER_ROUNDS);
    PA_ADD_OPTION(AFTER_ITEM_PRINTER_DONE_EXPLANATION);
//    PA_ADD_OPTION(DATE0);
//    PA_ADD_OPTION(DATE1);
    PA_ADD_OPTION(TABLE);
    PA_ADD_OPTION(DELAY_MILLIS);
    PA_ADD_OPTION(ADJUST_DELAY);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(FIX_TIME_WHEN_DONE);

    if (PreloadSettings::instance().DEVELOPER_MODE){
        PA_ADD_OPTION(MATERIAL_FARMER_DISABLED_EXPLANATION);
        PA_ADD_OPTION(MATERIAL_FARMER_OPTIONS);
    }

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
){
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
                adjust_delay(env, print_results, seed, env.console);
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
    SingleSwitchProgramEnvironment& env,
    const std::array<std::string, 10>& print_results, 
    uint64_t seed, ConsoleHandle& console
){
    ItemPrinterRNG_Descriptor::Stats& stats = env.current_stats<ItemPrinterRNG_Descriptor::Stats>();

    DistanceFromTarget distance_from_target = get_distance_from_target(print_results, seed, console);
    int16_t delay_adjustment;
    int16_t current_delay_mills = DELAY_MILLIS;
    switch (distance_from_target){
    case DistanceFromTarget::UNKNOWN:
        delay_adjustment = 0;
        break;
    case DistanceFromTarget::ON_TARGET:
        delay_adjustment = 0;
        stats.frame_hits++;
        break;
    case DistanceFromTarget::MINUS_1:
        delay_adjustment = -50;
        stats.frame_misses++;
        break;
    case DistanceFromTarget::MINUS_2:
        delay_adjustment = -1000;
        stats.frame_misses++;
        break;
    case DistanceFromTarget::PLUS_1:
        delay_adjustment = 50;
        stats.frame_misses++;
        break;
    case DistanceFromTarget::PLUS_2:
        delay_adjustment = 1000;
        stats.frame_misses++;
        break;        
    default:
        delay_adjustment = 0;
        break;
    }
    env.update_stats();

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
    DistanceFromTarget distance_from_target = DistanceFromTarget::UNKNOWN;
    do{
        DateSeed seed_data = get_date_seed(seed);
        if (results_approximately_match(print_results, seed_data.items)){
            distance_from_target = DistanceFromTarget::ON_TARGET;
            break;
        }
        if (results_approximately_match(print_results, seed_data.balls)){
            distance_from_target = DistanceFromTarget::ON_TARGET;
            break;
        }

        seed_data = get_date_seed(seed - 1);
        if (results_approximately_match(print_results, seed_data.items)){
            distance_from_target = DistanceFromTarget::MINUS_1;
            break;
        }
        if (results_approximately_match(print_results, seed_data.balls)){
            distance_from_target = DistanceFromTarget::MINUS_1;
            break;
        }

        seed_data = get_date_seed(seed + 1);
        if (results_approximately_match(print_results, seed_data.items)){
            distance_from_target = DistanceFromTarget::PLUS_1;
            break;
        }
        if (results_approximately_match(print_results, seed_data.balls)){
            distance_from_target = DistanceFromTarget::PLUS_1;
            break;
        }

        seed_data = get_date_seed(seed - 2);
        if (results_approximately_match(print_results, seed_data.items)){
            distance_from_target = DistanceFromTarget::MINUS_2;
            break;
        }
        if (results_approximately_match(print_results, seed_data.balls)){
            distance_from_target = DistanceFromTarget::MINUS_2;
            break;
        }

        seed_data = get_date_seed(seed + 2);
        if (results_approximately_match(print_results, seed_data.items)){
            distance_from_target = DistanceFromTarget::PLUS_2;
            break;
        }
        if (results_approximately_match(print_results, seed_data.balls)){
            distance_from_target = DistanceFromTarget::PLUS_2;
            break;
        }

    }while (false);

    switch (distance_from_target){
    case DistanceFromTarget::ON_TARGET:
        console.log("Frame Result: Target hit", COLOR_BLUE);
        break;
    case DistanceFromTarget::MINUS_1:
        console.log("Frame Result: Missed. (Target - 1)", COLOR_ORANGE);
        break;
    case DistanceFromTarget::PLUS_1:
        console.log("Frame Result: Missed. (Target + 1)", COLOR_ORANGE);
        break;
    case DistanceFromTarget::MINUS_2:
        console.log("Frame Result: Missed. (Target - 2)", COLOR_RED);
        break;
    case DistanceFromTarget::PLUS_2:
        console.log("Frame Result: Missed. (Target + 2)", COLOR_RED);
        break;
    default:
        console.log("Frame Result: Unknown. No seed not found in database.", COLOR_ORANGE);
    }

    return distance_from_target;
}



bool ItemPrinterRNG::results_approximately_match(
    const std::array<std::string, 10>& print_results, 
    const std::array<std::string, 10>& expected_result
){
    size_t total_items = 0;
    size_t mismatches = 0;
    for (uint8_t i = 0; i < 10; i++){
        const std::string& x = print_results[i];
        const std::string& y = expected_result[i];
        if (x.empty() || y.empty()){
            continue;
        }

        total_items++;

        if (x == y){
            continue;
        }

        mismatches++;
    }

    if (total_items == 0){
        return false;
    }

    return mismatches * 5 <= total_items;
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
        send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE);

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

    try{
        if (!AUTO_MATERIAL_FARMING){
            run_item_printer_rng(env, context, stats);
        }else{
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
                {audio_detector}
            );
            audio_detector.throw_if_no_sound(std::chrono::milliseconds(1000));

            // Don't allow the material farmer stats to affect the Item Printer's stats.
            MaterialFarmerStats mat_farm_stats;// = env.current_stats<MaterialFarmer_Descriptor::Stats>();
            for (int i = 0; i < NUM_ROUNDS_OF_ITEM_PRINTER_TO_MATERIAL_FARM; i++){
                run_item_printer_rng(env, context, stats);
                press_Bs_to_back_to_overworld(env.program_info(), env.console, context);
                move_from_item_printer_to_material_farming(env, context);
                run_material_farmer(env, context, MATERIAL_FARMER_OPTIONS, mat_farm_stats);
                move_from_material_farming_to_item_printer(env, context);
            }
        }
    }catch (ProgramFinishedException&){}

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
