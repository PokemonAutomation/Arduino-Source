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
        {Flavor_Powers::mega,       "mega",         "Mega"},
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
