/*  Cram-o-matic Table
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_CramomaticTable_H
#define PokemonAutomation_PokemonSwSh_CramomaticTable_H

#include "Common/Cpp/Options/EditableTableOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/EnumDropdownOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

enum class CramomaticBallType
{
    Poke,
    Great,
    Shop1,
    Shop2,
    Apricorn,
    Safari,
    Sport
};

struct CramomaticSelection
{
    CramomaticBallType ball_type;
    bool is_bonus;
    uint16_t priority;
};

class CramomaticRow : public EditableTableRow{
public:
    CramomaticRow(EditableTableOption& parent_table);
    CramomaticRow(EditableTableOption& parent_table, CramomaticBallType p_ball_type, bool p_is_bonus, uint16_t p_priority);
    virtual std::unique_ptr<EditableTableRow> clone() const;

public:
    EnumDropdownCell<CramomaticBallType> ball_type;
    BooleanCheckBoxCell is_bonus;
    SimpleIntegerCell<uint16_t> priority;
};



class CramomaticTable : public EditableTableOption_t<CramomaticRow>{
public:
    CramomaticTable(std::string label);

    std::vector<CramomaticSelection> selected_balls() const;

    virtual std::vector<std::string> make_header() const;

    std::vector<std::unique_ptr<EditableTableRow>> make_defaults();
};





}
}
}
#endif
