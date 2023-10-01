/*  Stats Filter
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Pokemon_StatsFilter_H
#define PokemonAutomation_Pokemon_StatsFilter_H

#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "Common/Cpp/Options/EditableTableOption.h"
//#include "Pokemon/Pokemon_IVChecker.h"
#include "Pokemon/Pokemon_NatureChecker.h"
#include "Pokemon/Options/Pokemon_IvJudgeOption.h"
#include "Pokemon/Inference/Pokemon_IvJudgeReader.h"
#include "Pokemon/Inference/Pokemon_NatureReader.h"

namespace PokemonAutomation{
namespace Pokemon{


enum class StatsHuntAction{
    StopProgram,
    Keep,
    Discard,
};

enum class StatsHuntShinyFilter{
    Anything,
    NotShiny,
    Shiny,
};

enum class StatsHuntGenderFilter{
    Any,
    Male,
    Female,
    Genderless
};

#if 0
enum class StatsHuntNatureFilter{
    Any,
    Adamant,
    Bashful,
    Bold,
    Brave,
    Calm,
    Careful,
    Docile,
    Gentle,
    Hardy,
    Hasty,
    Impish,
    Jolly,
    Lax,
    Lonely,
    Mild,
    Modest,
    Naive,
    Naughty,
    Quiet,
    Quirky,
    Rash,
    Relaxed,
    Sassy,
    Serious,
    Timid
};
#endif

std::string gender_to_string(StatsHuntGenderFilter gender);


class StatsHuntFilterRow : public EditableTableRow{
public:
    StatsHuntFilterRow();
    StatsHuntFilterRow(StatsHuntShinyFilter p_shiny);
    virtual std::unique_ptr<EditableTableRow> clone() const override;

public:
    EnumDropdownCell<StatsHuntAction> action;
    EnumDropdownCell<StatsHuntShinyFilter> shiny;
    EnumDropdownCell<StatsHuntGenderFilter> gender;
    EnumDropdownCell<NatureCheckerFilter> nature;
    IVJudgeFilterCell iv_hp;
    IVJudgeFilterCell iv_atk;
    IVJudgeFilterCell iv_def;
    IVJudgeFilterCell iv_spatk;
    IVJudgeFilterCell iv_spdef;
    IVJudgeFilterCell iv_speed;
};

class StatsHuntFilterTable : public EditableTableOption_t<StatsHuntFilterRow>{
public:
    StatsHuntFilterTable();
    virtual std::vector<std::string> make_header() const override;
    static std::vector<std::unique_ptr<EditableTableRow>> make_defaults();
    
    StatsHuntAction get_action(
        bool shiny,
        const IvJudgeReader::Results& IVs,
        StatsHuntGenderFilter gender,
        NatureReader::Results nature
    ) const;
};










}
}
#endif
