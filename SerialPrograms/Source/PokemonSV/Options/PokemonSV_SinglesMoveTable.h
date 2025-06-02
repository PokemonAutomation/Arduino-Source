/*  Singles Move Table
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_SinglesMoveTable_H
#define PokemonAutomation_PokemonSV_SinglesMoveTable_H

#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/StringOption.h"
#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "Common/Cpp/Options/EditableTableOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


enum class SinglesMoveType{
    Move1,
    Move2,
    Move3,
    Move4,
    Run,
};
const EnumDropdownDatabase<SinglesMoveType>& singles_move_enum_database_wild();
const EnumDropdownDatabase<SinglesMoveType>& singles_move_enum_database_trainer();


struct SinglesMoveEntry{
    SinglesMoveType type;
    bool terastallize;

    std::string to_str() const;
};



class SinglesMoveTableRow : public EditableTableRow, public ConfigOption::Listener{
public:
    ~SinglesMoveTableRow();
    SinglesMoveTableRow(EditableTableOption& parent_table);
    virtual std::unique_ptr<EditableTableRow> clone() const override;

    SinglesMoveEntry snapshot() const;

private:
    virtual void on_config_value_changed(void* object) override;

private:
    EnumDropdownCell<SinglesMoveType> type;
    BooleanCheckBoxCell terastallize;
    StringCell notes;
};


class SinglesMoveTable : public EditableTableOption_t<SinglesMoveTableRow>{
public:
    SinglesMoveTable(std::string label, bool trainer_battle);

    std::vector<SinglesMoveEntry> snapshot();

    virtual std::vector<std::string> make_header() const;

    std::vector<std::unique_ptr<EditableTableRow>> make_defaults();

private:
    friend class SinglesMoveTableRow;

    bool m_trainer_battle;
};



}
}
}
#endif
