/*  Stats Filter
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Pokemon_StatsFilter_H
#define PokemonAutomation_Pokemon_StatsFilter_H

#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/IntegerRangeOption.h"
#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "Common/Cpp/Options/EditableTableOption.h"
//#include "Pokemon/Pokemon_IVChecker.h"
#include "Pokemon/Pokemon_NatureChecker.h"
#include "Pokemon/Pokemon_StatsCalculation.h"
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
std::string gender_to_string(StatsHuntGenderFilter gender);



//  Preset labels.
extern const char* StatsHuntIvJudgeFilterTable_Label_Eggs;
extern const char* StatsHuntIvJudgeFilterTable_Label_Regular;




struct StatsHuntMiscFeatureFlags{
    bool action = false;
    bool shiny = false;
    bool gender = false;
    bool nature = false;
};
struct StatsHuntRowMisc{
    StatsHuntRowMisc(const StatsHuntMiscFeatureFlags& p_feature_flags);
    void set(const StatsHuntRowMisc& x);

    const StatsHuntMiscFeatureFlags& feature_flags;

    EnumDropdownCell<StatsHuntAction> action;
    EnumDropdownCell<StatsHuntShinyFilter> shiny;
    EnumDropdownCell<StatsHuntGenderFilter> gender;
    EnumDropdownCell<NatureCheckerFilter> nature;
};




class StatsHuntIvJudgeFilterTable;
class StatsHuntIvJudgeFilterRow : public EditableTableRow{
public:
    StatsHuntIvJudgeFilterRow(const StatsHuntMiscFeatureFlags& feature_flags);
    StatsHuntIvJudgeFilterRow(const EditableTableOption* table);
    StatsHuntIvJudgeFilterRow(const StatsHuntMiscFeatureFlags& feature_flags, StatsHuntShinyFilter p_shiny);
    virtual std::unique_ptr<EditableTableRow> clone() const override;

    bool matches(
        bool shiny,
        const IvJudgeReader::Results& IVs,
        StatsHuntGenderFilter gender,
        NatureReader::Results nature
    ) const;

public:
    StatsHuntRowMisc misc;

    IVJudgeFilterCell iv_hp;
    IVJudgeFilterCell iv_atk;
    IVJudgeFilterCell iv_def;
    IVJudgeFilterCell iv_spatk;
    IVJudgeFilterCell iv_spdef;
    IVJudgeFilterCell iv_speed;
};
class StatsHuntIvJudgeFilterTable : public EditableTableOption_t<StatsHuntIvJudgeFilterRow>{
public:
    StatsHuntIvJudgeFilterTable(
        const std::string& label,
        const StatsHuntMiscFeatureFlags& p_feature_flags
    );
    virtual std::vector<std::string> make_header() const override;
    std::vector<std::unique_ptr<EditableTableRow>> make_defaults() const;

    StatsHuntAction get_action(
        bool shiny,
        const IvJudgeReader::Results& IVs,
        StatsHuntGenderFilter gender,
        NatureReader::Results nature
    ) const;

public:
    const StatsHuntMiscFeatureFlags feature_flags;
};







class StatsHuntIvRangeFilterTable;
class StatsHuntIvRangeFilterRow : public EditableTableRow{
public:
    StatsHuntIvRangeFilterRow(const StatsHuntMiscFeatureFlags& feature_flags);
    StatsHuntIvRangeFilterRow(const EditableTableOption* table);
    virtual std::unique_ptr<EditableTableRow> clone() const override;

    bool matches(
        bool shiny,
        const IvRanges& IVs,
        StatsHuntGenderFilter gender,
        NatureReader::Results nature
    ) const;

public:
    StatsHuntRowMisc misc;

    IntegerRangeCell<uint8_t> iv_hp;
    IntegerRangeCell<uint8_t> iv_atk;
    IntegerRangeCell<uint8_t> iv_def;
    IntegerRangeCell<uint8_t> iv_spatk;
    IntegerRangeCell<uint8_t> iv_spdef;
    IntegerRangeCell<uint8_t> iv_speed;
};
class StatsHuntIvRangeFilterTable : public EditableTableOption_t<StatsHuntIvRangeFilterRow>{
public:
    StatsHuntIvRangeFilterTable(
        const std::string& label,
        const StatsHuntMiscFeatureFlags& p_feature_flags
    );
    virtual std::vector<std::string> make_header() const override;
//    std::vector<std::unique_ptr<EditableTableRow>> make_defaults() const;

    StatsHuntAction get_action(
        bool shiny,
        const IvRanges& IVs,
        StatsHuntGenderFilter gender,
        NatureReader::Results nature
    ) const;

public:
    const StatsHuntMiscFeatureFlags feature_flags;
};





}
}
#endif
