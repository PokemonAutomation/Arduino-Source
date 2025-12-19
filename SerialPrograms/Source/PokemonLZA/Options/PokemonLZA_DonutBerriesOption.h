/*  Donut Berries Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLZA_DonutBerriesOption_H
#define PokemonAutomation_PokemonLZA_DonutBerriesOption_H

#include "CommonTools/Options/StringSelectOption.h"
#include "Common/Cpp/Options/EditableTableOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{


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



}
}
}
#endif
