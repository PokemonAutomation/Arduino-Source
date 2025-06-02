/*  Tera Move Table
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_TeraMoveTable_H
#define PokemonAutomation_PokemonSV_TeraMoveTable_H

#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/StringOption.h"
#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "Common/Cpp/Options/EditableTableOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{



enum class TeraMoveType{
    Wait,
    Move1,
    Move2,
    Move3,
    Move4,
    Cheer_AllOut,
    Cheer_HangTough,
    Cheer_HealUp,
};
const EnumDropdownDatabase<TeraMoveType>& tera_move_enum_database();

enum class TeraTarget{
    Opponent,
    Player0,    //  Yourself
    Player1,
    Player2,
    Player3,
};
const EnumDropdownDatabase<TeraTarget>& tera_target_enum_database();




struct TeraMoveEntry{
    TeraMoveType type;
    uint8_t seconds;
    TeraTarget target;

    std::string to_str() const;
};


class TeraMoveTableRow : public EditableTableRow, public ConfigOption::Listener{
public:
    ~TeraMoveTableRow();
    TeraMoveTableRow(EditableTableOption& parent_table);
    virtual std::unique_ptr<EditableTableRow> clone() const override;

    TeraMoveEntry snapshot() const;

private:
    virtual void on_config_value_changed(void* object) override;

private:
    EnumDropdownCell<TeraMoveType> type;
    SimpleIntegerCell<uint8_t> seconds;
    EnumDropdownCell<TeraTarget> target;
    StringCell notes;
};


class TeraMoveTable : public EditableTableOption_t<TeraMoveTableRow>{
public:
    TeraMoveTable();

    std::vector<TeraMoveEntry> snapshot();

    virtual std::vector<std::string> make_header() const;

    std::vector<std::unique_ptr<EditableTableRow>> make_defaults();

};




}
}
}
#endif
