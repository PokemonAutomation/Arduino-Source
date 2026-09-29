/*  Battle Move Table
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_BattleMoveTable_H
#define PokemonAutomation_PokemonSV_BattleMoveTable_H

#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/StringOption.h"
#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "Common/Cpp/Options/EditableTableOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


enum class BattleMoveType{
    Move1,
    Move2,
    Move3,
    Move4,
};
const EnumDropdownDatabase<BattleMoveType>& Battle_move_enum_database();

class BattleMoveTableRow : public EditableTableRow{
public:
    BattleMoveTableRow(EditableTableOption& parent_table);
    virtual std::unique_ptr<EditableTableRow> clone() const override;

public:
    EnumDropdownCell<BattleMoveType> type;
    StringCell notes;
};

class BattleMoveTable : public EditableTableOption_t<BattleMoveTableRow>{
public:
    BattleMoveTable();

    virtual std::vector<std::string> make_header() const;
    std::vector<std::unique_ptr<EditableTableRow>> make_defaults();
};


}
}
}
#endif
