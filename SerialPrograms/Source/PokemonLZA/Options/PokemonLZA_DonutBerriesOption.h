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
    // Subjective ordering by group
    any,

    sparkling_any,
    sparkling_all,
    sparkling_normal,
    sparkling_fire,
    sparkling_water,
    sparkling_electric,
    sparkling_grass,
    sparkling_ice,
    sparkling_fighting,
    sparkling_poison,
    sparkling_ground,
    sparkling_flying,
    sparkling_psychic,
    sparkling_bug,
    sparkling_rock,
    sparkling_ghost,
    sparkling_dragon,
    sparkling_dark,
    sparkling_steel,
    sparkling_fairy,

    alpha,
    humungo,
    teensy,

    item_berries,
    item_candies,
    item_treasure,
    item_pokeballs,
    item_special,
    item_coins,
    bighaul,
    megacharge,
    megaconserve,

    catching_any,
    catching_all,
    catching_normal,
    catching_fire,
    catching_water,
    catching_electric,
    catching_grass,
    catching_ice,
    catching_fighting,
    catching_poison,
    catching_ground,
    catching_flying,
    catching_psychic,
    catching_bug,
    catching_rock,
    catching_ghost,
    catching_dragon,
    catching_dark,
    catching_steel,
    catching_fairy,
    encounter,

    move_any,
    move_normal,
    move_fire,
    move_water,
    move_electric,
    move_grass,
    move_ice,
    move_fighting,
    move_poison,
    move_ground,
    move_flying,
    move_psychic,
    move_bug,
    move_rock,
    move_ghost,
    move_dragon,
    move_dark,
    move_steel,
    move_fairy,
    atk,
    spatk,
    speed,

    resistance_any,
    resistance_normal,
    resistance_fire,
    resistance_water,
    resistance_electric,
    resistance_grass,
    resistance_ice,
    resistance_fighting,
    resistance_poison,
    resistance_ground,
    resistance_flying,
    resistance_psychic,
    resistance_bug,
    resistance_rock,
    resistance_ghost,
    resistance_dragon,
    resistance_dark,
    resistance_steel,
    resistance_fairy,
    def,
    spdef,
};

enum class Power_Level{
    any,
    one,
    two,
    three,
    one_two,
    two_three,
};

struct FlavorPowerTableEntry{
    uint16_t limit;
    Flavor_Powers power_1;
    Power_Level level_1;
    Flavor_Powers power_2;
    Power_Level level_2;
    Flavor_Powers power_3;
    Power_Level level_3;

    std::string to_str() const;
    std::vector<std::string> get_entry_flavor_power_strings() const;
    std::string get_single_flavor_power_string(int power_index) const;
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

    FlavorPowerTableEntry snapshot() const;

private:
    SimpleIntegerCell<uint16_t> limit;
    EnumDropdownCell<Flavor_Powers> power_1;
    EnumDropdownCell<Power_Level> level_1;
    EnumDropdownCell<Flavor_Powers> power_2;
    EnumDropdownCell<Power_Level> level_2;
    EnumDropdownCell<Flavor_Powers> power_3;
    EnumDropdownCell<Power_Level> level_3;
};


class FlavorPowerTable : public EditableTableOption_t<FlavorPowerTableRow>{
public:
    FlavorPowerTable();

    std::vector<FlavorPowerTableEntry> snapshot();

    virtual std::vector<std::string> make_header() const;

    std::vector<std::unique_ptr<EditableTableRow>> make_defaults();

};


}
}
}
#endif
