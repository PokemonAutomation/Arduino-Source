/*  Stats Filter
 *
 *  From: https://github.com/PokemonAutomation/
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

    bool matches(
        bool shiny,
        StatsHuntGenderFilter gender,
        NatureCheckerValue nature
    ) const;

    const StatsHuntMiscFeatureFlags& feature_flags;

    EnumDropdownCell<StatsHuntAction> action;
    EnumDropdownCell<StatsHuntShinyFilter> shiny;
    EnumDropdownCell<StatsHuntGenderFilter> gender;
    EnumDropdownCell<NatureCheckerFilter> nature;
};




class StatsHuntIvJudgeFilterTable;
class StatsHuntIvJudgeFilterRow : public EditableTableRow{
public:
    StatsHuntIvJudgeFilterRow(EditableTableOption& parent_table);
    StatsHuntIvJudgeFilterRow(EditableTableOption& parent_table, StatsHuntShinyFilter p_shiny);
    virtual std::unique_ptr<EditableTableRow> clone() const override;

    bool matches(
        bool shiny,
        StatsHuntGenderFilter gender,
        NatureCheckerValue nature,
        const IvJudgeReader::Results& IVs
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
    std::vector<std::unique_ptr<EditableTableRow>> make_defaults();

    StatsHuntAction get_action(
        bool shiny,
        StatsHuntGenderFilter gender,
        NatureCheckerValue nature,
        const IvJudgeReader::Results& IVs
    ) const;

public:
    const StatsHuntMiscFeatureFlags feature_flags;
};








//  Preset labels.
extern const char* StatsHuntIvRangeFilterTable_Label_Regular;


class StatsHuntIvRangeFilterRow : public EditableTableRow{
public:
    StatsHuntIvRangeFilterRow(EditableTableOption& parent_table);
    virtual std::unique_ptr<EditableTableRow> clone() const override;

    bool matches(
        bool shiny,
        StatsHuntGenderFilter gender,
        NatureCheckerValue nature,
        const IvRanges& IVs
    ) const;

private:
    static bool match_iv(const IvRange& desired, const IvRange& actual);
    static bool match_iv(const IntegerRangeCell<uint8_t>& desired, const IvRange& actual);

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

    StatsHuntAction get_action(
        bool shiny,
        StatsHuntGenderFilter gender,
        NatureCheckerValue nature,
        const IvRanges& IVs
    ) const;

public:
    const StatsHuntMiscFeatureFlags feature_flags;
};





}
}
#endif
