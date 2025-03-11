/*  Stats Reset Filter
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Pokemon_StatsResetFilter_H
#define PokemonAutomation_Pokemon_StatsResetFilter_H

#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "Common/Cpp/Options/EditableTableOption.h"
//#include "Pokemon/Pokemon_IvJudge.h"
#include "Pokemon/Options/Pokemon_IvJudgeOption.h"
#include "Pokemon/Inference/Pokemon_IvJudgeReader.h"
#include "Pokemon_StatsHuntFilter.h"
#include "Pokemon/Inference/Pokemon_NatureReader.h"

namespace PokemonAutomation{
namespace Pokemon{

class StatsResetFilterRow : public EditableTableRow{
public:
    StatsResetFilterRow();
    virtual std::unique_ptr<EditableTableRow> clone() const override;

public:
    StatsHuntAction action;
    EnumDropdownCell<NatureCheckerFilter> nature;
    IVJudgeFilterCell iv_hp;
    IVJudgeFilterCell iv_atk;
    IVJudgeFilterCell iv_def;
    IVJudgeFilterCell iv_spatk;
    IVJudgeFilterCell iv_spdef;
    IVJudgeFilterCell iv_speed;
};

class StatsResetFilterTable : public EditableTableOption_t<StatsResetFilterRow>{
public:
    StatsResetFilterTable();
    virtual std::vector<std::string> make_header() const override;
    static std::vector<std::unique_ptr<EditableTableRow>> make_defaults();
    
    StatsHuntAction get_action(bool shiny, const IvJudgeReader::Results& IVs, NatureReader::Results nature) const;
};

}
}
#endif
