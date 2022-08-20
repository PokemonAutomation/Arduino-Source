/*  Berry Selector, UI component to select multiple berries
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_BerrySelector_H
#define PokemonAutomation_PokemonBDSP_BerrySelector_H

#include "Common/Cpp/Options/EditableTableOption2.h"
#include "PokemonBDSP_BerrySelectOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{



class BerrySelectorRow2 : public EditableTableRow2{
public:
    BerrySelectorRow2();
    virtual std::unique_ptr<EditableTableRow2> clone() const;

public:
    BerrySelectCell berry;
};



class BerryTable : public EditableTableOption_t<BerrySelectorRow2>{
public:
    BerryTable(std::string label);


    // Whether berry_slug is among the selected berries.
    bool find_berry(const std::string& berry_slug) const;
    // Return the berry slugs that the user has selected via the berry table UI.
    std::vector<std::string> selected_berries() const;

    virtual std::vector<std::string> make_header() const;

    static std::vector<std::unique_ptr<EditableTableRow2>> make_defaults();
};





}
}
}
#endif
