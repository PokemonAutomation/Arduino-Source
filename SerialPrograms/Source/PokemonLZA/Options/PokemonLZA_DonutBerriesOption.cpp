/*  Donut Berries Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Logging/Logger.h"
#include "PokemonLZA/Resources/PokemonLZA_DonutBerries.h"
#include "PokemonLZA_DonutBerriesOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{

const EnumDropdownDatabase<Flavor_Powers>& flavor_power_enum_database(){
    static EnumDropdownDatabase<Flavor_Powers> database{
        {Flavor_Powers::any,                "any",                  "Any Power"},

        {Flavor_Powers::sparkling_any,      "sparkling-any",        "Sparkling: *"},
        {Flavor_Powers::sparkling_all,      "sparkling-all",        "Sparkling: All Types"},
        {Flavor_Powers::sparkling_normal,   "sparkling-normal",     "Sparkling: Normal"},
        {Flavor_Powers::sparkling_fire,     "sparkling-fire",       "Sparkling: Fire"},
        {Flavor_Powers::sparkling_water,    "sparkling-water",      "Sparkling: Water"},
        {Flavor_Powers::sparkling_electric, "sparkling-electric",   "Sparkling: Electric"},
        {Flavor_Powers::sparkling_grass,    "sparkling-grass",      "Sparkling: Grass"},
        {Flavor_Powers::sparkling_ice,      "sparkling-ice",        "Sparkling: Ice"},
        {Flavor_Powers::sparkling_fighting, "sparkling-fighting",   "Sparkling: Fighting"},
        {Flavor_Powers::sparkling_poison,   "sparkling-poison",     "Sparkling: Poison"},
        {Flavor_Powers::sparkling_ground,   "sparkling-ground",     "Sparkling: Ground"},
        {Flavor_Powers::sparkling_flying,   "sparkling-flying",     "Sparkling: Flying"},
        {Flavor_Powers::sparkling_psychic,  "sparkling-psychic",    "Sparkling: Psychic"},
        {Flavor_Powers::sparkling_bug,      "sparkling-bug",        "Sparkling: Bug"},
        {Flavor_Powers::sparkling_rock,     "sparkling-rock",       "Sparkling: Rock"},
        {Flavor_Powers::sparkling_ghost,    "sparkling-ghost",      "Sparkling: Ghost"},
        {Flavor_Powers::sparkling_dragon,   "sparkling-dragon",     "Sparkling: Dragon"},
        {Flavor_Powers::sparkling_dark,     "sparkling-dark",       "Sparkling: Dark"},
        {Flavor_Powers::sparkling_steel,    "sparkling-steel",      "Sparkling: Steel"},
        {Flavor_Powers::sparkling_fairy,    "sparkling-fairy",      "Sparkling: Fairy"},

        {Flavor_Powers::alpha,      "alpha",        "Alpha"},
        {Flavor_Powers::humungo,    "humungo",      "Humungo"},
        {Flavor_Powers::teensy,     "teensy",       "Teensy"},

        {Flavor_Powers::item_any,       "item-any",         "Item: *"},
        {Flavor_Powers::item_berries,   "item-berries",     "Item: Berries"},
        {Flavor_Powers::item_candies,   "item-candies",     "Item: Candies"},
        {Flavor_Powers::item_treasure,  "item-treasure",    "Item: Treasure"},
        {Flavor_Powers::item_pokeballs, "item-pokeballs",   "Item: Poké Balls"},
        {Flavor_Powers::item_special,   "item-special",     "Item: Special"},
        {Flavor_Powers::item_coins,     "item-coins",       "Item: Coins"},
        {Flavor_Powers::bighaul,        "big-haul",          "Big Haul"},
        {Flavor_Powers::megacharge,     "mega-charge",      "Mega Charge"},
        {Flavor_Powers::megaconserve,   "mega-conserve",    "Mega Conserve"},

        {Flavor_Powers::catching_any,      "catching-any",        "Catching: *"},
        {Flavor_Powers::catching_all,      "catching-all",        "Catching: All Types"},
        {Flavor_Powers::catching_normal,   "catching-normal",     "Catching: Normal"},
        {Flavor_Powers::catching_fire,     "catching-fire",       "Catching: Fire"},
        {Flavor_Powers::catching_water,    "catching-water",      "Catching: Water"},
        {Flavor_Powers::catching_electric, "catching-electric",   "Catching: Electric"},
        {Flavor_Powers::catching_grass,    "catching-grass",      "Catching: Grass"},
        {Flavor_Powers::catching_ice,      "catching-ice",        "Catching: Ice"},
        {Flavor_Powers::catching_fighting, "catching-fighting",   "Catching: Fighting"},
        {Flavor_Powers::catching_poison,   "catching-poison",     "Catching: Poison"},
        {Flavor_Powers::catching_ground,   "catching-ground",     "Catching: Ground"},
        {Flavor_Powers::catching_flying,   "catching-flying",     "Catching: Flying"},
        {Flavor_Powers::catching_psychic,  "catching-psychic",    "Catching: Psychic"},
        {Flavor_Powers::catching_bug,      "catching-bug",        "Catching: Bug"},
        {Flavor_Powers::catching_rock,     "catching-rock",       "Catching: Rock"},
        {Flavor_Powers::catching_ghost,    "catching-ghost",      "Catching: Ghost"},
        {Flavor_Powers::catching_dragon,   "catching-dragon",     "Catching: Dragon"},
        {Flavor_Powers::catching_dark,     "catching-dark",       "Catching: Dark"},
        {Flavor_Powers::catching_steel,    "catching-steel",      "Catching: Steel"},
        {Flavor_Powers::catching_fairy,    "catching-fairy",      "Catching: Fairy"},
        {Flavor_Powers::encounter,        "encounter",          "Encounter"},

        {Flavor_Powers::move_any,      "move-any",        "Move: *"},
        {Flavor_Powers::move_normal,   "move-normal",     "Move: Normal"},
        {Flavor_Powers::move_fire,     "move-fire",       "Move: Fire"},
        {Flavor_Powers::move_water,    "move-water",      "Move: Water"},
        {Flavor_Powers::move_electric, "move-electric",   "Move: Electric"},
        {Flavor_Powers::move_grass,    "move-grass",      "Move: Grass"},
        {Flavor_Powers::move_ice,      "move-ice",        "Move: Ice"},
        {Flavor_Powers::move_fighting, "move-fighting",   "Move: Fighting"},
        {Flavor_Powers::move_poison,   "move-poison",     "Move: Poison"},
        {Flavor_Powers::move_ground,   "move-ground",     "Move: Ground"},
        {Flavor_Powers::move_flying,   "move-flying",     "Move: Flying"},
        {Flavor_Powers::move_psychic,  "move-psychic",    "Move: Psychic"},
        {Flavor_Powers::move_bug,      "move-bug",        "Move: Bug"},
        {Flavor_Powers::move_rock,     "move-rock",       "Move: Rock"},
        {Flavor_Powers::move_ghost,    "move-ghost",      "Move: Ghost"},
        {Flavor_Powers::move_dragon,   "move-dragon",     "Move: Dragon"},
        {Flavor_Powers::move_dark,     "move-dark",       "Move: Dark"},
        {Flavor_Powers::move_steel,    "move-steel",      "Move: Steel"},
        {Flavor_Powers::move_fairy,    "move-fairy",      "Move: Fairy"},
        {Flavor_Powers::atk,           "atk",             "Attack"},
        {Flavor_Powers::spatk,         "spatk",           "Sp. Atk"},
        {Flavor_Powers::speed,         "speed",           "Speed"},

        {Flavor_Powers::resistance_any,      "resistance-any",        "Resistance: *"},
        {Flavor_Powers::resistance_normal,   "resistance-normal",     "Resistance: Normal"},
        {Flavor_Powers::resistance_fire,     "resistance-fire",       "Resistance: Fire"},
        {Flavor_Powers::resistance_water,    "resistance-water",      "Resistance: Water"},
        {Flavor_Powers::resistance_electric, "resistance-electric",   "Resistance: Electric"},
        {Flavor_Powers::resistance_grass,    "resistance-grass",      "Resistance: Grass"},
        {Flavor_Powers::resistance_ice,      "resistance-ice",        "Resistance: Ice"},
        {Flavor_Powers::resistance_fighting, "resistance-fighting",   "Resistance: Fighting"},
        {Flavor_Powers::resistance_poison,   "resistance-poison",     "Resistance: Poison"},
        {Flavor_Powers::resistance_ground,   "resistance-ground",     "Resistance: Ground"},
        {Flavor_Powers::resistance_flying,   "resistance-flying",     "Resistance: Flying"},
        {Flavor_Powers::resistance_psychic,  "resistance-psychic",    "Resistance: Psychic"},
        {Flavor_Powers::resistance_bug,      "resistance-bug",        "Resistance: Bug"},
        {Flavor_Powers::resistance_rock,     "resistance-rock",       "Resistance: Rock"},
        {Flavor_Powers::resistance_ghost,    "resistance-ghost",      "Resistance: Ghost"},
        {Flavor_Powers::resistance_dragon,   "resistance-dragon",     "Resistance: Dragon"},
        {Flavor_Powers::resistance_dark,     "resistance-dark",       "Resistance: Dark"},
        {Flavor_Powers::resistance_steel,    "resistance-steel",      "Resistance: Steel"},
        {Flavor_Powers::resistance_fairy,    "resistance-fairy",      "Resistance: Fairy"},
        {Flavor_Powers::def,           "def",             "Defense"},
        {Flavor_Powers::spdef,         "spdef",           "Sp. Def"},
    };
    return database;
};

const EnumDropdownDatabase<Power_Level>& power_level_enum_database(){
    static EnumDropdownDatabase<Power_Level> database{
        {Power_Level::any,          "any",          "Any Level"},
        {Power_Level::one,          "one",          "Lv. 1"},
        {Power_Level::two,          "two",          "Lv. 2"},
        {Power_Level::three,        "three",        "Lv. 3"},
        {Power_Level::one_two,      "one_two",      "Lv. 1 or 2"},
        {Power_Level::two_three,    "two_three",    "Lv. 2 or 3"},
    };
    return database;
}

std::string FlavorPowerTableEntry::to_str() const{
    std::string result;
    result += "Limit: " + std::to_string(limit) + ", ";
    result += get_single_flavor_power_string(1) + ", ";
    result += get_single_flavor_power_string(2) + ", ";
    result += get_single_flavor_power_string(3);
    return result;
}

std::vector<std::string> FlavorPowerTableEntry::get_entry_flavor_power_strings() const{
    std::vector<std::string> powers;
    for (int c = 1; c <= 3; c++){
        std::string power_string = get_single_flavor_power_string(c);
        powers.push_back(power_string);
    }
    return powers;
}

std::string FlavorPowerTableEntry::get_single_flavor_power_string(int power_index) const{
    std::string selected_power;
    Flavor_Powers power;
    Power_Level level;

    switch (power_index){
    case 1:
        power = power_1;
        level = level_1;
        break;
    case 2:
        power = power_2;
        level = level_2;
        break;
    case 3:
        power = power_3;
        level = level_3;
        break;
    default:
        throw InternalProgramError(
            nullptr, PA_CURRENT_FUNCTION, 
            "Invalid power index: " + std::to_string(power_index)
        );
    }

    switch (power){
    case Flavor_Powers::any:
        selected_power += "any-power-";
        break;

    case Flavor_Powers::sparkling_any:
        selected_power += "sparkling-power-any-";
        break;
    case Flavor_Powers::sparkling_all:
        selected_power += "sparkling-power-all-types-";
        break;
    case Flavor_Powers::sparkling_normal:
        selected_power += "sparkling-power-normal-";
        break;
    case Flavor_Powers::sparkling_fire:
        selected_power += "sparkling-power-fire-";
        break;
    case Flavor_Powers::sparkling_water:
        selected_power += "sparkling-power-water-";
        break;
    case Flavor_Powers::sparkling_electric:
        selected_power += "sparkling-power-electric-";
        break;
    case Flavor_Powers::sparkling_grass:
        selected_power += "sparkling-power-grass-";
        break;
    case Flavor_Powers::sparkling_ice:
        selected_power += "sparkling-power-ice-";
        break;
    case Flavor_Powers::sparkling_fighting:
        selected_power += "sparkling-power-fighting-";
        break;
    case Flavor_Powers::sparkling_poison:
        selected_power += "sparkling-power-poison-";
        break;
    case Flavor_Powers::sparkling_ground:
        selected_power += "sparkling-power-ground-";
        break;
    case Flavor_Powers::sparkling_flying:
        selected_power += "sparkling-power-flying-";
        break;
    case Flavor_Powers::sparkling_psychic:
        selected_power += "sparkling-power-psychic-";
        break;
    case Flavor_Powers::sparkling_bug:
        selected_power += "sparkling-power-bug-";
        break;
    case Flavor_Powers::sparkling_rock:
        selected_power += "sparkling-power-rock-";
        break;
    case Flavor_Powers::sparkling_ghost:
        selected_power += "sparkling-power-ghost-";
        break;
    case Flavor_Powers::sparkling_dragon:
        selected_power += "sparkling-power-dragon-";
        break;
    case Flavor_Powers::sparkling_dark:
        selected_power += "sparkling-power-dark-";
        break;
    case Flavor_Powers::sparkling_steel:
        selected_power += "sparkling-power-steel-";
        break;
    case Flavor_Powers::sparkling_fairy:
        selected_power += "sparkling-power-fairy-";
        break;

    case Flavor_Powers::alpha:
        selected_power += "alpha-power-";
        break;
    case Flavor_Powers::humungo:
        selected_power += "humungo-power-";
        break;
    case Flavor_Powers::teensy:
        selected_power += "teensy-power-";
        break;

    case Flavor_Powers::item_any:
        selected_power += "item-power-any-";
        break;
    case Flavor_Powers::item_berries:
        selected_power += "item-power-berries-";
        break;
    case Flavor_Powers::item_candies:
        selected_power += "item-power-candies-";
        break;
    case Flavor_Powers::item_treasure:
        selected_power += "item-power-treasure-";
        break;
    case Flavor_Powers::item_pokeballs:
        selected_power += "item-power-pokeballs-";
        break;
    case Flavor_Powers::item_special:
        selected_power += "item-power-special-";
        break;
    case Flavor_Powers::item_coins:
        selected_power += "item-power-coins-";
        break;
    case Flavor_Powers::bighaul:
        selected_power += "big-haul-power-";
        break;
    case Flavor_Powers::megacharge:
        selected_power += "mega-charge-power-";
        break;
    case Flavor_Powers::megaconserve:
        selected_power += "mega-conserve-power-";
        break;

    case Flavor_Powers::catching_any:
        selected_power += "catching-power-any-";
        break;
    case Flavor_Powers::catching_all:
        selected_power += "catching-power-all-";
        break;
    case Flavor_Powers::catching_normal:
        selected_power += "catching-power-normal-";
        break;
    case Flavor_Powers::catching_fire:
        selected_power += "catching-power-fire-";
        break;
    case Flavor_Powers::catching_water:
        selected_power += "catching-power-water-";
        break;
    case Flavor_Powers::catching_electric:
        selected_power += "catching-power-electric-";
        break;
    case Flavor_Powers::catching_grass:
        selected_power += "catching-power-grass-";
        break;
    case Flavor_Powers::catching_ice:
        selected_power += "catching-power-ice-";
        break;
    case Flavor_Powers::catching_fighting:
        selected_power += "catching-power-fighting-";
        break;
    case Flavor_Powers::catching_poison:
        selected_power += "catching-power-poison-";
        break;
    case Flavor_Powers::catching_ground:
        selected_power += "catching-power-ground-";
        break;
    case Flavor_Powers::catching_flying:
        selected_power += "catching-power-flying-";
        break;
    case Flavor_Powers::catching_psychic:
        selected_power += "catching-power-psychic-";
        break;
    case Flavor_Powers::catching_bug:
        selected_power += "catching-power-bug-";
        break;
    case Flavor_Powers::catching_rock:
        selected_power += "catching-power-rock-";
        break;
    case Flavor_Powers::catching_ghost:
        selected_power += "catching-power-ghost-";
        break;
    case Flavor_Powers::catching_dragon:
        selected_power += "catching-power-dragon-";
        break;
    case Flavor_Powers::catching_dark:
        selected_power += "catching-power-dark-";
        break;
    case Flavor_Powers::catching_steel:
        selected_power += "catching-power-steel-";
        break;
    case Flavor_Powers::catching_fairy:
        selected_power += "catching-power-fairy-";
        break;
    case Flavor_Powers::encounter:
        selected_power += "encounter-power-";
        break;

    case Flavor_Powers::move_any:
        selected_power += "move-power-any-";
        break;
    case Flavor_Powers::move_normal:
        selected_power += "move-power-normal-";
        break;
    case Flavor_Powers::move_fire:
        selected_power += "move-power-fire-";
        break;
    case Flavor_Powers::move_water:
        selected_power += "move-power-water-";
        break;
    case Flavor_Powers::move_electric:
        selected_power += "move-power-electric-";
        break;
    case Flavor_Powers::move_grass:
        selected_power += "move-power-grass-";
        break;
    case Flavor_Powers::move_ice:
        selected_power += "move-power-ice-";
        break;
    case Flavor_Powers::move_fighting:
        selected_power += "move-power-fighting-";
        break;
    case Flavor_Powers::move_poison:
        selected_power += "move-power-poison-";
        break;
    case Flavor_Powers::move_ground:
        selected_power += "move-power-ground-";
        break;
    case Flavor_Powers::move_flying:
        selected_power += "move-power-flying-";
        break;
    case Flavor_Powers::move_psychic:
        selected_power += "move-power-psychic-";
        break;
    case Flavor_Powers::move_bug:
        selected_power += "move-power-bug-";
        break;
    case Flavor_Powers::move_rock:
        selected_power += "move-power-rock-";
        break;
    case Flavor_Powers::move_ghost:
        selected_power += "move-power-ghost-";
        break;
    case Flavor_Powers::move_dragon:
        selected_power += "move-power-dragon-";
        break;
    case Flavor_Powers::move_dark:
        selected_power += "move-power-dark-";
        break;
    case Flavor_Powers::move_steel:
        selected_power += "move-power-steel-";
        break;
    case Flavor_Powers::move_fairy:
        selected_power += "move-power-fairy-";
        break;
    case Flavor_Powers::atk:
        selected_power += "attack-power-";
        break;
    case Flavor_Powers::spatk:
        selected_power += "sp-atk-power-";
        break;
    case Flavor_Powers::speed:
        selected_power += "speed-power-";
        break;

    case Flavor_Powers::resistance_any:
        selected_power += "resistance-power-any-";
        break;
    case Flavor_Powers::resistance_normal:
        selected_power += "resistance-power-normal-";
        break;
    case Flavor_Powers::resistance_fire:
        selected_power += "resistance-power-fire-";
        break;
    case Flavor_Powers::resistance_water:
        selected_power += "resistance-power-water-";
        break;
    case Flavor_Powers::resistance_electric:
        selected_power += "resistance-power-electric-";
        break;
    case Flavor_Powers::resistance_grass:
        selected_power += "resistance-power-grass-";
        break;
    case Flavor_Powers::resistance_ice:
        selected_power += "resistance-power-ice-";
        break;
    case Flavor_Powers::resistance_fighting:
        selected_power += "resistance-power-fighting-";
        break;
    case Flavor_Powers::resistance_poison:
        selected_power += "resistance-power-poison-";
        break;
    case Flavor_Powers::resistance_ground:
        selected_power += "resistance-power-ground-";
        break;
    case Flavor_Powers::resistance_flying:
        selected_power += "resistance-power-flying-";
        break;
    case Flavor_Powers::resistance_psychic:
        selected_power += "resistance-power-psychic-";
        break;
    case Flavor_Powers::resistance_bug:
        selected_power += "resistance-power-bug-";
        break;
    case Flavor_Powers::resistance_rock:
        selected_power += "resistance-power-rock-";
        break;
    case Flavor_Powers::resistance_ghost:
        selected_power += "resistance-power-ghost-";
        break;
    case Flavor_Powers::resistance_dragon:
        selected_power += "resistance-power-dragon-";
        break;
    case Flavor_Powers::resistance_dark:
        selected_power += "resistance-power-dark-";
        break;
    case Flavor_Powers::resistance_steel:
        selected_power += "resistance-power-steel-";
        break;
    case Flavor_Powers::resistance_fairy:
        selected_power += "resistance-power-fairy-";
        break;
    case Flavor_Powers::def:
        selected_power += "defense-power-";
        break;
    case Flavor_Powers::spdef:
        selected_power += "sp-def-power-";
        break;
    default:
        throw InternalProgramError(
            nullptr, PA_CURRENT_FUNCTION, 
            "Invalid flavor power: " + std::to_string(static_cast<int>(power))
        );
    }

    switch (level){
    case Power_Level::any:
        selected_power += "any";
        break;
    case Power_Level::one:
        selected_power += "1";
        break;
    case Power_Level::two:
        selected_power += "2";
        break;
    case Power_Level::three:
        selected_power += "3";
        break;
    case Power_Level::one_two:
        selected_power += "12";
        break;
    case Power_Level::two_three:
        selected_power += "23";
        break;
    default:
        throw InternalProgramError(
            nullptr, PA_CURRENT_FUNCTION, 
            "Invalid power level: " + std::to_string(static_cast<int>(level))
        );
    }
    return selected_power;
}

StringSelectDatabase make_donut_berries_database(){
    StringSelectDatabase ret;
    for (const auto& slug : DONUT_BERRIES_SLUGS()){
        const DonutBerries& data = get_berry_name(slug);
        const SpriteDatabase::Sprite* sprite = DONUT_BERRIES_DATABASE().get_nothrow(slug);
        if (sprite == nullptr){
            ret.add_entry(StringSelectEntry(slug, data.display_name()));
            global_logger_tagged().log("Missing sprite for: " + slug, COLOR_RED);
        }else{
            ret.add_entry(StringSelectEntry(slug, data.display_name(), sprite->icon));
        }
    }
    return ret;
}
const StringSelectDatabase& DONUT_BERRY_DATABASE(){
    static StringSelectDatabase database = make_donut_berries_database();
    return database;
}


DonutBerriesTableCell::DonutBerriesTableCell(
    const std::string& default_slug
)
    : StringSelectCell(
        DONUT_BERRY_DATABASE(),
        LockMode::LOCK_WHILE_RUNNING,
        default_slug
    )
{}

DonutBerriesTableRow::DonutBerriesTableRow(EditableTableOption& parent_table)
    : EditableTableRow(parent_table)
    , berry("hyper-cheri-berry")
{
    PA_ADD_OPTION(berry);
}
std::unique_ptr<EditableTableRow> DonutBerriesTableRow::clone() const{
    std::unique_ptr<DonutBerriesTableRow> ret(new DonutBerriesTableRow(parent()));
    ret->berry.set_by_index(berry.index());
    return ret;
}

DonutBerriesTable::DonutBerriesTable(std::string label)
    : EditableTableOption_t<DonutBerriesTableRow>(
        std::move(label),
        LockMode::LOCK_WHILE_RUNNING,
        make_defaults()
        )
{}


std::vector<std::string> DonutBerriesTable::make_header() const{
    return std::vector<std::string>{
        "Berry",
    };
}

std::vector<std::unique_ptr<EditableTableRow>> DonutBerriesTable::make_defaults(){
    std::vector<std::unique_ptr<EditableTableRow>> ret;
    ret.emplace_back(std::make_unique<DonutBerriesTableRow>(*this));
    return ret;
}




FlavorPowerTableRow::FlavorPowerTableRow(EditableTableOption& parent_table)
    : EditableTableRow(parent_table)
    , limit(LockMode::LOCK_WHILE_RUNNING, 1, 0, 999)
    , power_1(flavor_power_enum_database(), LockMode::LOCK_WHILE_RUNNING, Flavor_Powers::alpha)
    , level_1(power_level_enum_database(), LockMode::LOCK_WHILE_RUNNING, Power_Level::three)
    , power_2(flavor_power_enum_database(), LockMode::LOCK_WHILE_RUNNING, Flavor_Powers::any)
    , level_2(power_level_enum_database(), LockMode::LOCK_WHILE_RUNNING, Power_Level::any)
    , power_3(flavor_power_enum_database(), LockMode::LOCK_WHILE_RUNNING, Flavor_Powers::any)
    , level_3(power_level_enum_database(), LockMode::LOCK_WHILE_RUNNING, Power_Level::any)
{
    PA_ADD_OPTION(limit);
    PA_ADD_OPTION(power_1);
    PA_ADD_OPTION(level_1);
    PA_ADD_OPTION(power_2);
    PA_ADD_OPTION(level_2);
    PA_ADD_OPTION(power_3);
    PA_ADD_OPTION(level_3);
}
std::unique_ptr<EditableTableRow> FlavorPowerTableRow::clone() const{
    std::unique_ptr<FlavorPowerTableRow> ret(new FlavorPowerTableRow(parent()));
    ret->limit.set(limit);
    ret->power_1.set(power_1);
    ret->level_1.set(level_1);
    ret->power_2.set(power_2);
    ret->level_2.set(level_2);
    ret->power_3.set(power_3);
    ret->level_3.set(level_3);
    return ret;
}
FlavorPowerTableEntry FlavorPowerTableRow::snapshot() const{
    return FlavorPowerTableEntry{limit, power_1, level_1, power_2, level_2, power_3, level_3};
}

FlavorPowerTable::FlavorPowerTable()
    : EditableTableOption_t<FlavorPowerTableRow>(
        "<b>Donuts Table:</b><br>"
        "Add all desired donuts to this table. "
        "The program will check the powers of a made donut and compare the powers to each row in the table."
        "<br>A keep limit can be set for each donut. Make sure you have enough berries to make this many donuts!"
        "<br>A single donut can match and count towards the limit of multiple entries in the table."
        "<br>The order in which you list Flavor Powers 1, 2, and 3 does not matter."
        "<br>Note: \"All Types\" means the All Types Power in-game. \"*\" means match any type for the specified power",
        LockMode::LOCK_WHILE_RUNNING,
        make_defaults()
    )
{}
std::vector<FlavorPowerTableEntry> FlavorPowerTable::snapshot(){
    return EditableTableOption_t<FlavorPowerTableRow>::snapshot<FlavorPowerTableEntry>();
}
std::vector<std::string> FlavorPowerTable::make_header() const{
    return {
        "Keep Limit",
        "Flavor Power 1   ",
        "Level 1  ",
        "Flavor Power 2   ",
        "Level 2  ",
        "Flavor Power 3   ",
        "Level 3  ",
    };
}
std::vector<std::unique_ptr<EditableTableRow>> FlavorPowerTable::make_defaults(){
    std::vector<std::unique_ptr<EditableTableRow>> ret;
    ret.emplace_back(new FlavorPowerTableRow(*this));
    return ret;
}



}
}
}
