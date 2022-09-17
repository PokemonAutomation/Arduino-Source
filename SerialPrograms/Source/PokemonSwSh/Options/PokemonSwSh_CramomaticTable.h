/*  Cram-o-matic Table
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_CramomaticTable_H
#define PokemonAutomation_PokemonSwSh_CramomaticTable_H

#include "Common/Cpp/Options/EditableTableOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "CommonFramework/Options/StringSelectOption.h"
#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "PokemonSwSh_CramomaticTable.h"

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
    CramomaticRow();
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

    static std::vector<std::unique_ptr<EditableTableRow>> make_defaults();
};





}
}
}
#endif
