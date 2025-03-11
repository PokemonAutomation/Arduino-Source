/*  Egg Hatch Filter
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Logging/Logger.h"
#include "Pokemon_StatsHuntFilter.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace Pokemon{

std::string gender_to_string(StatsHuntGenderFilter gender){
    const char * names[] = {
        "Any",
        "Male",
        "Female",
        "Genderless",
    };
    return names[int(gender)];
}


const EnumDropdownDatabase<StatsHuntAction>& StatsHuntAction_Database(){
    static const EnumDropdownDatabase<StatsHuntAction> database({
        {StatsHuntAction::StopProgram,   "stop",     "Stop Program"},
        {StatsHuntAction::Keep,          "keep",     "Keep"},
//        {EggHatchAction::Release,       "release",  "Release"},
    });
    return database;
}
const EnumDropdownDatabase<StatsHuntShinyFilter>& StatsHuntShinyFilter_Database(){
    static const EnumDropdownDatabase<StatsHuntShinyFilter> database({
        {StatsHuntShinyFilter::Anything, "anything",     "Anything"},
        {StatsHuntShinyFilter::NotShiny, "not-shiny",    "Not Shiny"},
        {StatsHuntShinyFilter::Shiny,    "shiny",        "Shiny"},
    });
    return database;
}
const EnumDropdownDatabase<StatsHuntGenderFilter>& StatsHuntGenderFilter_Database(){
    static const EnumDropdownDatabase<StatsHuntGenderFilter> database({
        {StatsHuntGenderFilter::Any,         "any",          "Any"},
        {StatsHuntGenderFilter::Male,        "male",         "Male"},
        {StatsHuntGenderFilter::Female,      "female",       "Female"},
//        {EggHatchGenderFilter::Genderless,  "genderless",   "Genderless"},
    });
    return database;
}




const char* StatsHuntIvJudgeFilterTable_Label_Eggs =
    "<b>Actions Table:</b><br>"
    "If a hatchling matches one of these filters, the specified action will be performed. "
    "Otherwise, it will be released. "
    "If multiple entries apply and have conflicting actions, the program will stop.<br>"
    "IV checking requires that your right panel be set to the IV Judge and that you have selected the correct game language above.";

const char* StatsHuntIvJudgeFilterTable_Label_Regular =
    "<b>Stop Conditions:</b><br>"
    "If the Pok\u00e9mon matches one of these filters, the program will stop.<br>"
    "IV checking requires that your right panel be set to the IV Judge and that you have selected the correct game language above.";






StatsHuntRowMisc::StatsHuntRowMisc(const StatsHuntMiscFeatureFlags& p_feature_flags)
    : feature_flags(p_feature_flags)
    , action(
          StatsHuntAction_Database(),
        LockMode::UNLOCK_WHILE_RUNNING,
        feature_flags.action
            ? StatsHuntAction::Keep
            : StatsHuntAction::StopProgram
    )
    , shiny(StatsHuntShinyFilter_Database(), LockMode::UNLOCK_WHILE_RUNNING, StatsHuntShinyFilter::Anything)
    , gender(StatsHuntGenderFilter_Database(), LockMode::UNLOCK_WHILE_RUNNING, StatsHuntGenderFilter::Any)
    , nature(NatureCheckerFilter_Database(), LockMode::UNLOCK_WHILE_RUNNING, NatureCheckerFilter::Any)
{}
void StatsHuntRowMisc::set(const StatsHuntRowMisc& x){
    action.set(x.action);
    shiny.set(x.shiny);
    gender.set(x.gender);
    nature.set(x.nature);
}
bool StatsHuntRowMisc::matches(
    bool shiny,
    StatsHuntGenderFilter gender,
    NatureCheckerValue nature
) const{
    //  Check the shiny filter.
    switch (this->shiny){
    case StatsHuntShinyFilter::Anything:
        break;
    case StatsHuntShinyFilter::NotShiny:
        if (shiny){
            return false;
        }
        break;
    case StatsHuntShinyFilter::Shiny:
        if (!shiny){
            return false;
        }
        break;
    }

    StatsHuntGenderFilter filter_gender = this->gender;
    if (filter_gender != gender && filter_gender != StatsHuntGenderFilter::Any){
        return false;
    }

    if (!NatureChecker_filter_match(this->nature, nature)){
        return false;
    }

    return true;
}


StatsHuntIvJudgeFilterRow::StatsHuntIvJudgeFilterRow(EditableTableOption& parent_table)
    : EditableTableRow(parent_table)
    , misc(static_cast<const StatsHuntIvJudgeFilterTable&>(parent_table).feature_flags)
    , iv_hp(IvJudgeFilter::Anything)
    , iv_atk(IvJudgeFilter::Anything)
    , iv_def(IvJudgeFilter::Anything)
    , iv_spatk(IvJudgeFilter::Anything)
    , iv_spdef(IvJudgeFilter::Anything)
    , iv_speed(IvJudgeFilter::Anything)
{
    if (misc.feature_flags.action)  PA_ADD_OPTION(misc.action);
    if (misc.feature_flags.shiny)   PA_ADD_OPTION(misc.shiny);
    if (misc.feature_flags.gender)  PA_ADD_OPTION(misc.gender);
    if (misc.feature_flags.nature)  PA_ADD_OPTION(misc.nature);
    PA_ADD_OPTION(iv_hp);
    PA_ADD_OPTION(iv_atk);
    PA_ADD_OPTION(iv_def);
    PA_ADD_OPTION(iv_spatk);
    PA_ADD_OPTION(iv_spdef);
    PA_ADD_OPTION(iv_speed);
}
StatsHuntIvJudgeFilterRow::StatsHuntIvJudgeFilterRow(EditableTableOption& parent_table, StatsHuntShinyFilter p_shiny)
    : StatsHuntIvJudgeFilterRow(parent_table)
{
    misc.shiny.set(p_shiny);
}
std::unique_ptr<EditableTableRow> StatsHuntIvJudgeFilterRow::clone() const{
    std::unique_ptr<StatsHuntIvJudgeFilterRow> ret(new StatsHuntIvJudgeFilterRow(parent()));
    ret->misc.set(misc);
    ret->iv_hp.set(iv_hp);
    ret->iv_atk.set(iv_atk);
    ret->iv_def.set(iv_def);
    ret->iv_spatk.set(iv_spatk);
    ret->iv_spdef.set(iv_spdef);
    ret->iv_speed.set(iv_speed);
    return ret;
}
bool StatsHuntIvJudgeFilterRow::matches(
    bool shiny,
    StatsHuntGenderFilter gender,
    NatureCheckerValue nature,
    const IvJudgeReader::Results& IVs
) const{
    if (!misc.matches(shiny, gender, nature)){
        return false;
    }

    //  Check all the IV filters.
    if (!IvJudge_filter_match(iv_hp, IVs.hp)) return false;
    if (!IvJudge_filter_match(iv_atk, IVs.attack)) return false;
    if (!IvJudge_filter_match(iv_def, IVs.defense)) return false;
    if (!IvJudge_filter_match(iv_spatk, IVs.spatk)) return false;
    if (!IvJudge_filter_match(iv_spdef, IVs.spdef)) return false;
    if (!IvJudge_filter_match(iv_speed, IVs.speed)) return false;

    return true;
}

StatsHuntIvJudgeFilterTable::StatsHuntIvJudgeFilterTable(
    const std::string& label,
    const StatsHuntMiscFeatureFlags& p_feature_flags
)
    : EditableTableOption_t<StatsHuntIvJudgeFilterRow>(label, LockMode::UNLOCK_WHILE_RUNNING)
    , feature_flags(p_feature_flags)
{
    set_default(make_defaults());
    restore_defaults();
}
std::vector<std::string> StatsHuntIvJudgeFilterTable::make_header() const{
    std::vector<std::string> ret;
    if (feature_flags.action){
        ret.emplace_back("Action");
    }
    if (feature_flags.shiny){
        ret.emplace_back("Shininess");
    }
    if (feature_flags.gender){
        ret.emplace_back("Gender");
    }
    if (feature_flags.nature){
        ret.emplace_back("Nature");
    }

    ret.emplace_back("HP");
    ret.emplace_back("Attack");
    ret.emplace_back("Defense");
    ret.emplace_back("Sp. Attack");
    ret.emplace_back("Sp. Defense");
    ret.emplace_back("Speed");

    return ret;
}
std::vector<std::unique_ptr<EditableTableRow>> StatsHuntIvJudgeFilterTable::make_defaults(){
    std::vector<std::unique_ptr<EditableTableRow>> ret;
    if (feature_flags.shiny){
        ret.emplace_back(new StatsHuntIvJudgeFilterRow(*this, StatsHuntShinyFilter::Shiny));
    }
    return ret;
}
StatsHuntAction StatsHuntIvJudgeFilterTable::get_action(
    bool shiny,
    StatsHuntGenderFilter gender,
    NatureCheckerValue nature,
    const IvJudgeReader::Results& IVs
) const{
    StatsHuntAction action = StatsHuntAction::Discard;
    std::vector<std::unique_ptr<StatsHuntIvJudgeFilterRow>> list = copy_snapshot();
    for (size_t c = 0; c < list.size(); c++){
        const StatsHuntIvJudgeFilterRow& filter = *list[c];

        if (!filter.matches(shiny, gender, nature, IVs)){
            continue;
        }

        StatsHuntAction filter_action = filter.misc.action;

        //  No action matched so far. Take the current action and continue.
        if (action == StatsHuntAction::Discard){
            action = filter_action;
            continue;
        }

        //  Conflicting actions.
        if (action != filter_action){
            global_logger_tagged().log("Multiple filters matched with conflicting actions. Stopping program...", COLOR_RED);
            return StatsHuntAction::StopProgram;
        }
    }
    return action;
}





const char* StatsHuntIvRangeFilterTable_Label_Regular =
    "<b>Stop Conditions:</b><br>"
    "If the Pok\u00e9mon matches one of these filters, the program will stop.<br>"
    "Partially overlapping IV ranges will count as a match. So if a filter is for 0-1, but the IV calculation returns a range 1-2, it will count.";



StatsHuntIvRangeFilterRow::StatsHuntIvRangeFilterRow(EditableTableOption& parent_table)
    : EditableTableRow(parent_table)
    , misc(static_cast<const StatsHuntIvJudgeFilterTable&>(parent_table).feature_flags)
    , iv_hp(LockMode::UNLOCK_WHILE_RUNNING, 0, 31, 0, 0, 0, 31, 31, 31)
    , iv_atk(LockMode::UNLOCK_WHILE_RUNNING, 0, 31, 0, 0, 0, 31, 31, 31)
    , iv_def(LockMode::UNLOCK_WHILE_RUNNING, 0, 31, 0, 0, 0, 31, 31, 31)
    , iv_spatk(LockMode::UNLOCK_WHILE_RUNNING, 0, 31, 0, 0, 0, 31, 31, 31)
    , iv_spdef(LockMode::UNLOCK_WHILE_RUNNING, 0, 31, 0, 0, 0, 31, 31, 31)
    , iv_speed(LockMode::UNLOCK_WHILE_RUNNING, 0, 31, 0, 0, 0, 31, 31, 31)
{
    if (misc.feature_flags.action)  PA_ADD_OPTION(misc.action);
    if (misc.feature_flags.shiny)   PA_ADD_OPTION(misc.shiny);
    if (misc.feature_flags.gender)  PA_ADD_OPTION(misc.gender);
    if (misc.feature_flags.nature)  PA_ADD_OPTION(misc.nature);
    PA_ADD_OPTION(iv_hp);
    PA_ADD_OPTION(iv_atk);
    PA_ADD_OPTION(iv_def);
    PA_ADD_OPTION(iv_spatk);
    PA_ADD_OPTION(iv_spdef);
    PA_ADD_OPTION(iv_speed);
}
std::unique_ptr<EditableTableRow> StatsHuntIvRangeFilterRow::clone() const{
    std::unique_ptr<StatsHuntIvRangeFilterRow> ret(new StatsHuntIvRangeFilterRow(parent()));
    ret->misc.set(misc);
    ret->iv_hp.set(iv_hp);
    ret->iv_atk.set(iv_atk);
    ret->iv_def.set(iv_def);
    ret->iv_spatk.set(iv_spatk);
    ret->iv_spdef.set(iv_spdef);
    ret->iv_speed.set(iv_speed);
    return ret;
}
bool StatsHuntIvRangeFilterRow::match_iv(const IvRange& desired, const IvRange& actual){
    if (desired.low == 0 && desired.high == 31){
        return true;
    }
    if (actual.high < desired.low){
        return false;
    }
    if (desired.high < actual.low){
        return false;
    }
    return true;
}
bool StatsHuntIvRangeFilterRow::match_iv(const IntegerRangeCell<uint8_t>& desired, const IvRange& actual){
    uint8_t lo, hi;
    desired.current_values(lo, hi);
    return match_iv(IvRange{(int8_t)lo, (int8_t)hi}, actual);
}
bool StatsHuntIvRangeFilterRow::matches(
    bool shiny,
    StatsHuntGenderFilter gender,
    NatureCheckerValue nature,
    const IvRanges& IVs
) const{
    if (!misc.matches(shiny, gender, nature)){
        return false;
    }

    if (!match_iv(iv_hp, IVs.hp)) return false;
    if (!match_iv(iv_atk, IVs.attack)) return false;
    if (!match_iv(iv_def, IVs.defense)) return false;
    if (!match_iv(iv_spatk, IVs.spatk)) return false;
    if (!match_iv(iv_spdef, IVs.spdef)) return false;
    if (!match_iv(iv_speed, IVs.speed)) return false;

    return true;
}


StatsHuntIvRangeFilterTable::StatsHuntIvRangeFilterTable(
    const std::string& label,
    const StatsHuntMiscFeatureFlags& p_feature_flags
)
    : EditableTableOption_t<StatsHuntIvRangeFilterRow>(label, LockMode::UNLOCK_WHILE_RUNNING)
    , feature_flags(p_feature_flags)
{
//    set_default(make_defaults());
//    restore_defaults();
}
std::vector<std::string> StatsHuntIvRangeFilterTable::make_header() const{
    std::vector<std::string> ret;
    if (feature_flags.action){
        ret.emplace_back("Action");
    }
    if (feature_flags.shiny){
        ret.emplace_back("Shininess");
    }
    if (feature_flags.gender){
        ret.emplace_back("Gender");
    }
    if (feature_flags.nature){
        ret.emplace_back("Nature");
    }

    ret.emplace_back("HP");
    ret.emplace_back("Atk");
    ret.emplace_back("Def");
    ret.emplace_back("SpAtk");
    ret.emplace_back("SpDef");
    ret.emplace_back("Spd");

    return ret;
}
StatsHuntAction StatsHuntIvRangeFilterTable::get_action(
    bool shiny,
    StatsHuntGenderFilter gender,
    NatureCheckerValue nature,
    const IvRanges& IVs
) const{
    StatsHuntAction action = StatsHuntAction::Discard;
    std::vector<std::unique_ptr<StatsHuntIvRangeFilterRow>> list = copy_snapshot();
    for (size_t c = 0; c < list.size(); c++){
        const StatsHuntIvRangeFilterRow& filter = *list[c];

        if (!filter.matches(shiny, gender, nature, IVs)){
            continue;
        }

        StatsHuntAction filter_action = filter.misc.action;

        //  No action matched so far. Take the current action and continue.
        if (action == StatsHuntAction::Discard){
            action = filter_action;
            continue;
        }

        //  Conflicting actions.
        if (action != filter_action){
            global_logger_tagged().log("Multiple filters matched with conflicting actions. Stopping program...", COLOR_RED);
            return StatsHuntAction::StopProgram;
        }
    }
    return action;
}






}
}
