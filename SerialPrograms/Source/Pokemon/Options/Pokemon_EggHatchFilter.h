/*  Egg Hatch Filter
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Pokemon_EggHatchFilter_H
#define PokemonAutomation_Pokemon_EggHatchFilter_H

#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "Common/Cpp/Options/EditableTableOption.h"
//#include "Pokemon/Pokemon_IVChecker.h"
#include "Pokemon/Pokemon_NatureChecker.h"
#include "Pokemon/Options/Pokemon_IvJudgeOption.h"
#include "Pokemon/Inference/Pokemon_IvJudgeReader.h"
#include "Pokemon/Inference/Pokemon_NatureReader.h"

namespace PokemonAutomation{
namespace Pokemon{


enum class EggHatchAction{
    StopProgram,
    Keep,
    Release,
};

enum class EggHatchShinyFilter{
    Anything,
    NotShiny,
    Shiny,
};

enum class EggHatchGenderFilter{
    Any,
    Male,
    Female,
    Genderless
};

enum class EggHatchNatureFilter {
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

std::string gender_to_string(EggHatchGenderFilter gender);


class EggHatchFilterRow : public EditableTableRow{
public:
    EggHatchFilterRow();
    EggHatchFilterRow(EggHatchShinyFilter p_shiny);
    virtual std::unique_ptr<EditableTableRow> clone() const override;

public:
    EnumDropdownCell<EggHatchAction> action;
    EnumDropdownCell<EggHatchShinyFilter> shiny;
    EnumDropdownCell<EggHatchGenderFilter> gender;
    EnumDropdownCell<NatureCheckerFilter> nature;
    IVJudgeFilterCell iv_hp;
    IVJudgeFilterCell iv_atk;
    IVJudgeFilterCell iv_def;
    IVJudgeFilterCell iv_spatk;
    IVJudgeFilterCell iv_spdef;
    IVJudgeFilterCell iv_speed;
};

class EggHatchFilterTable : public EditableTableOption_t<EggHatchFilterRow>{
public:
    EggHatchFilterTable();
    virtual std::vector<std::string> make_header() const override;
    static std::vector<std::unique_ptr<EditableTableRow>> make_defaults();
    
    EggHatchAction get_action(bool shiny, const IvJudgeReader::Results& IVs, EggHatchGenderFilter gender, NatureReader::Results nature) const;
};










}
}
#endif
