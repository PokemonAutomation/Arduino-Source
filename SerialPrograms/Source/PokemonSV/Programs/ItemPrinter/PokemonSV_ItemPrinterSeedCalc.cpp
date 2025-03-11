/*  Item Printer Seed Calculation
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  This file is ported from:
 *  https://github.com/kwsch/ItemPrinterDeGacha/blob/main/ItemPrinterDeGacha.Core/ItemPrinter.cs
 *
 *  The actual RNG logic is from Anubis.
 *
 */

#include <vector>
#include <map>
#include "Common/Compiler.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonArray.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "CommonFramework/Globals.h"
#include "Pokemon/Pokemon_Xoroshiro128Plus.h"
#include "PokemonSV_ItemPrinterSeedCalc.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{
namespace ItemPrinter{


const char* item_id_to_slug(int item_id){
    //  Taken from: https://github.com/kwsch/ItemPrinterDeGacha/blob/main/ItemPrinterDeGacha.Core/Resources/text/items_en.txt
    static const std::map<int, const char*> database{
        {1, "master-ball"},
        {2, "ultra-ball"},
        {3, "great-ball"},
        {4, "poke-ball"},
        {5, "safari-ball"},
        {6, "net-ball"},
        {7, "dive-ball"},
        {8, "nest-ball"},
        {9, "repeat-ball"},
        {10, "timer-ball"},
        {11, "luxury-ball"},
        {12, "premier-ball"},
        {13, "dusk-ball"},
        {14, "heal-ball"},
        {15, "quick-ball"},

        {23, "full-restore"},
        {24, "max-potion"},
        {25, "hyper-potion"},
        {26, "super-potion"},
        {27, "full-heal"},
        {28, "revive"},
        {29, "max-revive"},

        {38, "ether"},
        {39, "max-ether"},
        {40, "elixir"},
        {41, "max-elixir"},

        {45, "hp-up"},
        {46, "protein"},
        {47, "iron"},
        {48, "carbos"},
        {49, "calcium"},

        {51, "pp-up"},
        {52, "zinc"},
        {53, "pp-max"},

        {80, "sun-stone"},
        {81, "moon-stone"},
        {82, "fire-stone"},
        {83, "thunder-stone"},
        {84, "water-stone"},
        {85, "leaf-stone"},
        {86, "tiny-mushroom"},
        {87, "big-mushroom"},
        {88, "pearl"},
        {89, "big-pearl"},
        {90, "stardust"},
        {91, "star-piece"},
        {92, "nugget"},

        {94, "honey"},

        {106, "rare-bone"},
        {107, "shiny-stone"},
        {108, "dusk-stone"},
        {109, "dawn-stone"},
        {110, "oval-stone"},

        {221, "kings-rock"},
        {222, "silver-powder"},
        {223, "amulet-coin"},

        {229, "everstone"},
        {230, "focus-band"},
        {231, "lucky-egg"},
        {232, "scope-lens"},
        {233, "metal-coat"},
        {234, "leftovers"},
        {235, "dragon-scale"},

        {237, "soft-sand"},
        {238, "hard-stone"},
        {239, "miracle-seed"},
        {240, "black-glasses"},
        {241, "black-belt"},
        {242, "magnet"},
        {243, "mystic-water"},
        {244, "sharp-beak"},
        {245, "poison-barb"},
        {246, "never-melt-ice"},
        {247, "spell-tag"},
        {248, "twisted-spoon"},
        {249, "charcoal"},
        {250, "dragon-fang"},
        {251, "silk-scarf"},
        {252, "upgrade"},

        {269, "light-clay"},

        {272, "toxic-orb"},
        {273, "flame-orb"},

        {277, "metronome"},

        {281, "black-sludge"},

        {321, "protector"},
        {322, "electirizer"},
        {323, "magmarizer"},
        {324, "dubious-disc"},
        {325, "reaper-cloth"},
        {326, "razor-claw"},
        {327, "razor-fang"},

        {492, "fast-ball"},
        {493, "level-ball"},
        {494, "lure-ball"},
        {495, "heavy-ball"},
        {496, "love-ball"},
        {497, "friend-ball"},
        {498, "moon-ball"},
        {499, "sport-ball"},

        {537, "prism-scale"},

        {541, "air-balloon"},

        {571, "pretty-feather"},

        {576, "dream-ball"},

        {580, "balm-mushroom"},
        {581, "big-nugget"},
        {582, "pearl-string"},
        {583, "comet-shard"},

        {645, "ability-capsule"},

        {650, "safety-goggles"},

        {795, "bottle-cap"},
        {796, "gold-bottle-cap"},

        {849, "ice-stone"},

        {851, "beast-ball"},

        {1109, "strawberry-sweet"},
        {1110, "love-sweet"},
        {1111, "berry-sweet"},
        {1112, "clover-sweet"},
        {1113, "flower-sweet"},
        {1114, "star-sweet"},
        {1115, "ribbon-sweet"},

        {1120, "heavy-duty-boots"},

        {1124, "exp-candy-xs"},
        {1125, "exp-candy-s"},
        {1126, "exp-candy-m"},
        {1127, "exp-candy-l"},
        {1128, "exp-candy-xl"},

        {1253, "cracked-pot"},
        {1254, "chipped-pot"},

        {1606, "ability-patch"},

        {1842, "tiny-bamboo-shoot"},
        {1843, "big-bamboo-shoot"},

        {1862, "normal-tera-shard"},
        {1863, "fire-tera-shard"},
        {1864, "water-tera-shard"},
        {1865, "electric-tera-shard"},
        {1866, "grass-tera-shard"},
        {1867, "ice-tera-shard"},
        {1868, "fighting-tera-shard"},
        {1869, "poison-tera-shard"},
        {1870, "ground-tera-shard"},
        {1871, "flying-tera-shard"},
        {1872, "psychic-tera-shard"},
        {1873, "bug-tera-shard"},
        {1874, "rock-tera-shard"},
        {1875, "ghost-tera-shard"},
        {1876, "dragon-tera-shard"},
        {1877, "dark-tera-shard"},
        {1878, "steel-tera-shard"},
        {1879, "fairy-tera-shard"},

        {1885, "covert-cloak"},
        {1886, "loaded-dice"},

        {2401, "fairy-feather"},

        {2403, "unremarkable-teacup"},
        {2404, "masterpiece-teacup"},

        {2482, "metal-alloy"},

        {2549, "stellar-tera-shard"},
    };
    auto iter = database.find(item_id);
    if (iter != database.end()){
        return iter->second;
    }else{
        return nullptr;
    }
}

struct ItemPrinterItemData{
    const char* slug;
    uint16_t weight;
    uint8_t min_quantity;
    uint8_t max_quantity;
};

std::vector<ItemPrinterItemData> make_item_prize_list(){
    //  This is taken from:
    //      https://github.com/kwsch/ItemPrinterDeGacha/blob/main/ItemPrinterDeGacha.Core/Resources/item_table_array.json
    //  The file itself is originally from a pkNX dump of the game.
    const std::string path = "PokemonSV/ItemPrinterItems.json";
    JsonValue json = load_json_file(RESOURCE_PATH() + path);
    const JsonArray& array = json
        .to_object_throw(path)
        .get_array_throw("Table", path);

    std::vector<ItemPrinterItemData> ret;
    ret.reserve(array.size());

    for (const JsonValue& item : array){
        const JsonObject& entry = item.to_object_throw(path)
            .get_object_throw("Param", path)
            .get_object_throw("Value", path);
        int item_id = (int)entry.get_integer_throw("ItemId", path);
        const char* slug = item_id_to_slug(item_id);
        if (slug == nullptr){
            throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Unknown Item ID: " + std::to_string(item_id));
        }
        ret.emplace_back(ItemPrinterItemData{
            slug,
            (uint16_t)entry.get_integer_throw("EmergePercent", path),
            (uint8_t)entry.get_integer_throw("LotteryItemNumMin", path),
            (uint8_t)entry.get_integer_throw("LotteryItemNumMax", path)
        });
    }

    return ret;
}
std::vector<ItemPrinterItemData> make_ball_prize_list(){
    //  This is taken from:
    //      https://github.com/kwsch/ItemPrinterDeGacha/blob/main/ItemPrinterDeGacha.Core/Resources/special_item_table_array.json
    //  The file itself is originally from a pkNX dump of the game.
    const std::string path = "PokemonSV/ItemPrinterBalls.json";
    JsonValue json = load_json_file(RESOURCE_PATH() + path);
    const JsonArray& array = json
        .to_object_throw(path)
        .get_array_throw("Table", path)[0]
        .to_object_throw(path)
        .get_object_throw("Param", path)
        .get_array_throw("Table", path);

    std::vector<ItemPrinterItemData> ret;
    ret.reserve(array.size());

    for (const JsonValue& item : array){
        const JsonObject& entry = item.to_object_throw(path);
        int item_id = (int)entry.get_integer_throw("ItemId", path);
        const char* slug = item_id_to_slug(item_id);
        if (slug == nullptr){
            throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Unknown Item ID: " + std::to_string(item_id));
        }
        ret.emplace_back(ItemPrinterItemData{
            slug,
            (uint16_t)entry.get_integer_throw("EmergePercent", path),
            (uint8_t)entry.get_integer_throw("LotteryItemNumMin", path),
            (uint8_t)entry.get_integer_throw("LotteryItemNumMax", path)
        });
    }

    return ret;
}
std::vector<const ItemPrinterItemData*> make_item_prize_table(const std::vector<ItemPrinterItemData>& prize_list){
    const size_t RAND_SLOTS = 10001;
    std::vector<const ItemPrinterItemData*> ret;
    ret.reserve(RAND_SLOTS);

    //  Sort by weight.
    std::multimap<uint16_t, const ItemPrinterItemData*> sorted;
    for (const ItemPrinterItemData& item : prize_list){
        sorted.emplace(item.weight, &item);
    }

    //  Handle quirk where the first item has n+1 weight.
    ret.emplace_back();

    for (const auto& item : sorted){
        size_t weight = item.second->weight;
        while (weight-- > 0){
            ret.emplace_back(item.second);
        }
    }

    if (ret.size() != RAND_SLOTS){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Sum of weights doesn't match expected value: " + std::to_string(ret.size()));
    }

    //  Handle quirk where the first item has n+1 weight.
    ret[0] = ret[1];

    return ret;
}


std::vector<const ItemPrinterItemData*> make_item_prize_table(){
    static const std::vector<ItemPrinterItemData>& PRIZE_LIST = make_item_prize_list();
    return make_item_prize_table(PRIZE_LIST);
}
std::vector<const ItemPrinterItemData*> make_ball_prize_table(){
    static const std::vector<ItemPrinterItemData>& PRIZE_LIST = make_ball_prize_list();
    return make_item_prize_table(PRIZE_LIST);
}


enum class PrintMode{
    Regular = 0,
    ItemBonus = 1,
    BallBonus = 2,
};


std::array<std::string, 10> calculate_prizes(int64_t seed, PrintMode mode){
    static const std::vector<const ItemPrinterItemData*> ITEM_TABLE = make_item_prize_table();
    static const std::vector<const ItemPrinterItemData*> BALL_TABLE = make_ball_prize_table();

    const std::vector<const ItemPrinterItemData*>& table = mode == PrintMode::BallBonus
        ? BALL_TABLE
        : ITEM_TABLE;

    Pokemon::Xoroshiro128Plus rand(seed, 0x82A2B175229D6A5B);

    PrintMode return_mode = PrintMode::Regular;
    std::array<std::string, 10> ret;
    for (size_t c = 0; c < 10; c++){
        //  Always check for next bonus mode, even if not possible.
        uint64_t roll = rand.nextInt(1000);
        bool bonus = roll < 20;

        //  Determine the item to print.
        uint64_t item_roll = rand.nextInt(table.size());
        const ItemPrinterItemData& item = *table[item_roll];
        ret[c] = item.slug;

        //  Determine quantity.
        if (item.min_quantity != item.max_quantity){
            rand.nextInt(item.max_quantity - item.min_quantity + 1);
        }

        //  If we're lucky enough to get a bonus mode, pick one.
        //  Assume the player has both modes unlocked.
        //  If a bonus mode was previously set, don't recalculate.
        if (mode == PrintMode::Regular && bonus && return_mode == PrintMode::Regular){
            return_mode = (PrintMode)(1 + rand.nextInt(2));
        }

    }

    return ret;
}




DateSeed calculate_seed_prizes(int64_t seed){
    return DateSeed{
        seed,
        calculate_prizes(seed, PrintMode::Regular),
        calculate_prizes(seed, PrintMode::ItemBonus),
        calculate_prizes(seed, PrintMode::BallBonus)
    };
}




}
}
}
}
