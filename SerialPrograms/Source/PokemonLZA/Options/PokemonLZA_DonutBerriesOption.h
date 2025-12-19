/*  Donut Berries Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLZA_DonutBerriesOption_H
#define PokemonAutomation_PokemonLZA_DonutBerriesOption_H

#include "CommonTools/Options/StringSelectOption.h"
#include "Common/Cpp/Options/EditableTableOption.h"
#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{

enum class Flavor_Powers {
    alpha,
    humungo,
    teensy,
    sparkling,
    atk,
    spatk,
    move,
    speed,
    bighaul,
    item,
    mega,
    def,
    spdef,
    resistance,
    encounter,
    catching,
};

enum class Power_Pokemon_Types {
    any, //Accept any of the below options
    all, //Accept only the All type (ex. Sparkling Power: All Types (Lv. 3)) Applies to catching and sparkling, but not move or resist
    normal,
    fire,
    water,
    electric,
    grass,
    ice,
    fighting,
    poison,
    ground,
    flying,
    psychic,
    bug,
    rock,
    ghost,
    dragon,
    dark,
    steel,
    fairy,
};

enum class Power_Item_Types {
    berries,
    candies,
    treasure,
    pokeballs,
    special,
    coins,
};

enum class Power_Level{
    one,
    two,
    three,
};


//Berry + Hyperspace Berry selection
class DonutBerriesTableCell : public StringSelectCell{
public:
    DonutBerriesTableCell(const std::string& default_slug);
};

class DonutBerriesTableRow : public EditableTableRow{
public:
    DonutBerriesTableRow(EditableTableOption& parent_table);
    virtual std::unique_ptr<EditableTableRow> clone() const override;

public:
    DonutBerriesTableCell berry;
};


class DonutBerriesTable : public EditableTableOption_t<DonutBerriesTableRow>{
public:
    DonutBerriesTable(std::string label);

    virtual std::vector<std::string> make_header() const override;

    std::vector<std::unique_ptr<EditableTableRow>> make_defaults();
};



//Donut Flavor Power selection
class FlavorPowerTableRow : public EditableTableRow, public ConfigOption::Listener{
public:
    ~FlavorPowerTableRow();
    FlavorPowerTableRow(EditableTableOption& parent_table);
    virtual std::unique_ptr<EditableTableRow> clone() const override;

private:
    virtual void on_config_value_changed(void* object) override;

private:
    EnumDropdownCell<Flavor_Powers> power;
    EnumDropdownCell<Power_Pokemon_Types> type_pokemon;
    EnumDropdownCell<Power_Item_Types> type_item;
    EnumDropdownCell<Power_Level> level;
};


class FlavorPowerTable : public EditableTableOption_t<FlavorPowerTableRow>{
public:
    FlavorPowerTable();

    virtual std::vector<std::string> make_header() const;

    std::vector<std::unique_ptr<EditableTableRow>> make_defaults();

};


}
}
}
#endif
