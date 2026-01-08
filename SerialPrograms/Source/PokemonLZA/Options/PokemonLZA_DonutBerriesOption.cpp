/*  Donut Berries Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Logging/Logger.h"
#include "PokemonLZA/Resources/PokemonLZA_DonutBerries.h"
#include "PokemonLZA_DonutBerriesOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{

const EnumDropdownDatabase<Flavor_Powers>& flavor_power_enum_database(){
    static EnumDropdownDatabase<Flavor_Powers> database{
        {Flavor_Powers::alpha,      "alpha",        "Alpha"},
        {Flavor_Powers::humungo,    "humungo",      "Humungo"},
        {Flavor_Powers::teensy,     "teensy",       "Teensy"},
        {Flavor_Powers::sparkling,  "sparkling",    "Sparkling"},
        {Flavor_Powers::atk,        "atk",          "Attack"},
        {Flavor_Powers::spatk,      "spatk",        "Sp. Atk"},
        {Flavor_Powers::move,       "move",         "Move"},
        {Flavor_Powers::speed,      "speed",        "Speed"},
        {Flavor_Powers::bighaul,    "bighaul",      "Big Haul"},
        {Flavor_Powers::item,       "item",         "Item"},
        {Flavor_Powers::megacharge, "megacharge",   "Mega Charge"},
        {Flavor_Powers::megaconserve,"megaconserve","Mega Conserve"},
        {Flavor_Powers::def,        "def",          "Defense"},
        {Flavor_Powers::spdef,      "spdef",        "Sp. Def"},
        {Flavor_Powers::resistance, "resistance",   "Resistance"},
        {Flavor_Powers::encounter,  "encounter",    "Encounter"},
        {Flavor_Powers::catching,   "catching",     "Catching"},
    };
    return database;
}
const EnumDropdownDatabase<Power_Pokemon_Types>& pokemon_power_enum_database(){
    static EnumDropdownDatabase<Power_Pokemon_Types> database{
        {Power_Pokemon_Types::all,      "all",      "All Types"},
        {Power_Pokemon_Types::normal,   "normal",   "Normal"},
        {Power_Pokemon_Types::fire,     "fire",     "Fire"},
        {Power_Pokemon_Types::water,    "water",    "Water"},
        {Power_Pokemon_Types::electric, "electric", "Electric"},
        {Power_Pokemon_Types::grass,    "grass",    "Grass"},
        {Power_Pokemon_Types::ice,      "ice",      "Ice"},
        {Power_Pokemon_Types::fighting, "fighting", "Fighting"},
        {Power_Pokemon_Types::poison,   "poison",   "Poison"},
        {Power_Pokemon_Types::ground,   "ground",   "Ground"},
        {Power_Pokemon_Types::flying,   "flying",   "Flying"},
        {Power_Pokemon_Types::psychic,  "psychic",  "Psychic"},
        {Power_Pokemon_Types::bug,      "bug",      "Bug"},
        {Power_Pokemon_Types::rock,     "rock",     "Rock"},
        {Power_Pokemon_Types::ghost,    "ghost",    "Ghost"},
        {Power_Pokemon_Types::dragon,   "dragon",   "Dragon"},
        {Power_Pokemon_Types::dark,     "dark",     "Dark"},
        {Power_Pokemon_Types::steel,    "steel",    "Steel"},
        {Power_Pokemon_Types::fairy,    "fairy",    "Fairy"},
    };
    return database;
}
const EnumDropdownDatabase<Power_Item_Types>& item_power_enum_database(){
    static EnumDropdownDatabase<Power_Item_Types> database{
        {Power_Item_Types::berries,     "berries",      "Berries"},
        {Power_Item_Types::candies,     "candies",      "Candies"},
        {Power_Item_Types::treasure,    "treasure",     "Treasure"},
        {Power_Item_Types::pokeballs,   "pokeballs",    "Poke Balls"},
        {Power_Item_Types::special,     "special",      "Special"},
        {Power_Item_Types::coins,       "coins",        "Coins"},
    };
    return database;
}
const EnumDropdownDatabase<Power_Level>& power_level_enum_database(){
    static EnumDropdownDatabase<Power_Level> database{
        {Power_Level::one,      "one",      "Lv. 1"},
        {Power_Level::two,      "two",      "Lv. 2"},
        {Power_Level::three,    "three",    "Lv. 3"},
    };
    return database;
}

std::string FlavorPowerTableEntry::to_str() const{
    std::string selected_power;

    switch (power){
    case Flavor_Powers::alpha:
        selected_power += "alpha-power-";
        break;
    case Flavor_Powers::humungo:
        selected_power += "humungo-power-";
        break;
    case Flavor_Powers::teensy:
        selected_power += "teensy-power-";
        break;
    case Flavor_Powers::sparkling:
        selected_power += "sparkling-power-";
        break;
    case Flavor_Powers::atk:
        selected_power += "attack-power-";
        break;
    case Flavor_Powers::spatk:
        selected_power += "sp-atk-power-";
        break;
    case Flavor_Powers::move:
        selected_power += "move-power-";
        break;
    case Flavor_Powers::speed:
        selected_power += "speed-power-";
        break;
    case Flavor_Powers::bighaul:
        selected_power += "big-haul-power-";
        break;
    case Flavor_Powers::item:
        selected_power += "item-power-";
        break;
    case Flavor_Powers::megacharge:
        selected_power += "mega-power-charging-";
        break;
    case Flavor_Powers::megaconserve:
        selected_power += "mega-power-conservation-";
        break;
    case Flavor_Powers::def:
        selected_power += "defense-power-";
        break;
    case Flavor_Powers::spdef:
        selected_power += "sp-def-power-";
        break;
    case Flavor_Powers::resistance:
        selected_power += "resistance-power-";
        break;
    case Flavor_Powers::encounter:
        selected_power += "encounter-power-";
        break;
    case Flavor_Powers::catching:
        selected_power += "catching-power-";
        break;
    }

    if (power == Flavor_Powers::catching || power == Flavor_Powers::sparkling || power == Flavor_Powers::move || power == Flavor_Powers::resistance) {
        switch (pokemon_type) {
        case Power_Pokemon_Types::all:
            if (power == Flavor_Powers::move || power == Flavor_Powers::resistance) {
                selected_power += "all-types-INVALID-SELECTION-";
                break;
            }
            selected_power += "all-types-";
            break;
        case Power_Pokemon_Types::normal:
            selected_power += "normal-";
            break;
        case Power_Pokemon_Types::fire:
            selected_power += "fire-";
            break;
        case Power_Pokemon_Types::water:
            selected_power += "water-";
            break;
        case Power_Pokemon_Types::electric:
            selected_power += "electric-";
            break;
        case Power_Pokemon_Types::grass:
            selected_power += "grass-";
            break;
        case Power_Pokemon_Types::ice:
            selected_power += "ice-";
            break;
        case Power_Pokemon_Types::fighting:
            selected_power += "fighting-";
            break;
        case Power_Pokemon_Types::poison:
            selected_power += "poison-";
            break;
        case Power_Pokemon_Types::ground:
            selected_power += "ground-";
            break;
        case Power_Pokemon_Types::flying:
            selected_power += "flying-";
            break;
        case Power_Pokemon_Types::psychic:
            selected_power += "psychic-";
            break;
        case Power_Pokemon_Types::bug:
            selected_power += "bug-";
            break;
        case Power_Pokemon_Types::rock:
            selected_power += "rock-";
            break;
        case Power_Pokemon_Types::ghost:
            selected_power += "ghost-";
            break;
        case Power_Pokemon_Types::dragon:
            selected_power += "dragon-";
            break;
        case Power_Pokemon_Types::dark:
            selected_power += "dark-";
            break;
        case Power_Pokemon_Types::steel:
            selected_power += "steel-";
            break;
        case Power_Pokemon_Types::fairy:
            selected_power += "fairy-";
            break;
        }
    }
    else if (power == Flavor_Powers::item) {
        switch (item_type) {
        case Power_Item_Types::berries:
            selected_power += "berries-";
            break;
        case Power_Item_Types::candies:
            selected_power += "candies-";
            break;
        case Power_Item_Types::treasure:
            selected_power += "treasure-";
            break;
        case Power_Item_Types::pokeballs:
            selected_power += "poké-balls-";
            break;
        case Power_Item_Types::special:
            selected_power += "special-";
            break;
        case Power_Item_Types::coins:
            selected_power += "coins-";
            break;
        }
    }

    switch (level) {
    case Power_Level::one:
        selected_power += "1";
        break;
    case Power_Level::two:
        selected_power += "2";
        break;
    case Power_Level::three:
        selected_power += "3";
        break;
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




FlavorPowerTableRow::~FlavorPowerTableRow(){
    power.remove_listener(*this);
}
FlavorPowerTableRow::FlavorPowerTableRow(EditableTableOption& parent_table)
    : EditableTableRow(parent_table)
    , power(flavor_power_enum_database(), LockMode::LOCK_WHILE_RUNNING, Flavor_Powers::alpha)
    , type_pokemon(pokemon_power_enum_database(), LockMode::LOCK_WHILE_RUNNING, Power_Pokemon_Types::all)
    , type_item(item_power_enum_database(), LockMode::LOCK_WHILE_RUNNING, Power_Item_Types::berries)
    , level(power_level_enum_database(), LockMode::LOCK_WHILE_RUNNING, Power_Level::three)
{
    PA_ADD_OPTION(power);
    PA_ADD_OPTION(type_pokemon);
    PA_ADD_OPTION(type_item);
    PA_ADD_OPTION(level);

    FlavorPowerTableRow::on_config_value_changed(this);
    power.add_listener(*this);
}
std::unique_ptr<EditableTableRow> FlavorPowerTableRow::clone() const{
    std::unique_ptr<FlavorPowerTableRow> ret(new FlavorPowerTableRow(parent()));
    ret->power.set(power);
    ret->type_pokemon.set(type_pokemon);
    ret->type_item.set(type_item);
    ret->level.set(level);
    return ret;
}
FlavorPowerTableEntry FlavorPowerTableRow::snapshot() const{
    return FlavorPowerTableEntry{power, type_pokemon, type_item, level};
}
void FlavorPowerTableRow::on_config_value_changed(void* object){
    Flavor_Powers power = this->power;

    type_item.set_visibility(
        power == Flavor_Powers::item
        ? ConfigOptionState::ENABLED
        : ConfigOptionState::HIDDEN
    );

    bool req_poke_types =
        power == Flavor_Powers::sparkling ||
        power == Flavor_Powers::catching ||
        power == Flavor_Powers::move ||
        power == Flavor_Powers::resistance;

    type_pokemon.set_visibility(
        req_poke_types
        ? ConfigOptionState::ENABLED
        : ConfigOptionState::HIDDEN
    );
}

FlavorPowerTable::FlavorPowerTable()
    : EditableTableOption_t<FlavorPowerTableRow>(
        "<b>Flavor Powers Table:</b><br>"
        "Add all desired flavor powers to this table. "
        "The program will check the powers of any baked donut and compare them against the selected items in the table. "
        "Be sure to set the correct Number of Powers to Match above."
        "<br>Note: \"All Types\" means the All Types Power in-game.",
        LockMode::LOCK_WHILE_RUNNING,
        make_defaults()
    )
{}
std::vector<FlavorPowerTableEntry> FlavorPowerTable::snapshot(){
    return EditableTableOption_t<FlavorPowerTableRow>::snapshot<FlavorPowerTableEntry>();
}
std::vector<std::string> FlavorPowerTable::make_header() const{
    return {
        "Flavor Power     ",
        "Pokemon Type     ",
        "Item Type      ",
        "Level    ",
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
