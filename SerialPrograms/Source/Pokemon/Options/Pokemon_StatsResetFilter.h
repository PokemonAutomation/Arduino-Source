/*  Stats Reset Filter
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Pokemon_StatsResetFilter_H
#define PokemonAutomation_Pokemon_StatsResetFilter_H

#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "Common/Cpp/Options/EditableTableOption.h"
#include "Pokemon/Pokemon_IVChecker.h"
#include "Pokemon/Options/Pokemon_IVCheckerOption.h"
#include "Pokemon/Inference/Pokemon_IVCheckerReader.h"
#include "Pokemon_EggHatchFilter.h"

namespace PokemonAutomation{
namespace Pokemon{

class StatsResetFilterRow : public EditableTableRow{
public:
    StatsResetFilterRow();
    virtual std::unique_ptr<EditableTableRow> clone() const override;

public:
    EggHatchAction action;
    IVCheckerFilterCell iv_hp;
    IVCheckerFilterCell iv_atk;
    IVCheckerFilterCell iv_def;
    IVCheckerFilterCell iv_spatk;
    IVCheckerFilterCell iv_spdef;
    IVCheckerFilterCell iv_speed;
};

class StatsResetFilterTable : public EditableTableOption_t<StatsResetFilterRow>{
public:
    StatsResetFilterTable();
    virtual std::vector<std::string> make_header() const override;
    static std::vector<std::unique_ptr<EditableTableRow>> make_defaults();

    EggHatchAction get_action(bool shiny, const IVCheckerReader::Results& IVs) const;
};

}
}
#endif
