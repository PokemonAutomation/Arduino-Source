/*  Egg Hatch Filter
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

//#include "Common/Compiler.h"
// #include "PokemonBDSP/Inference/BoxSystem/PokemonBDSP_BoxGenderDetector.h"
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


const EnumDatabase<StatsHuntAction>& EggHatchAction_Database(){
    static const EnumDatabase<StatsHuntAction> database({
        {StatsHuntAction::StopProgram,   "stop",     "Stop Program"},
        {StatsHuntAction::Keep,          "keep",     "Keep"},
//        {EggHatchAction::Release,       "release",  "Release"},
    });
    return database;
}
const EnumDatabase<StatsHuntShinyFilter>& EggHatchShinyFilter_Database(){
    static const EnumDatabase<StatsHuntShinyFilter> database({
        {StatsHuntShinyFilter::Anything, "anything",     "Anything"},
        {StatsHuntShinyFilter::NotShiny, "not-shiny",    "Not Shiny"},
        {StatsHuntShinyFilter::Shiny,    "shiny",        "Shiny"},
    });
    return database;
}
const EnumDatabase<StatsHuntGenderFilter>& EggHatchGenderFilter_Database(){
    static const EnumDatabase<StatsHuntGenderFilter> database({
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
    , action(EggHatchAction_Database(), LockWhileRunning::LOCKED, StatsHuntAction::Keep)
    , shiny(EggHatchShinyFilter_Database(), LockWhileRunning::LOCKED, StatsHuntShinyFilter::Anything)
    , gender(EggHatchGenderFilter_Database(), LockWhileRunning::LOCKED, StatsHuntGenderFilter::Any)
    , nature(NatureCheckerFilter_Database(), LockWhileRunning::LOCKED, NatureCheckerFilter::Any)
{}
void StatsHuntRowMisc::set(const StatsHuntRowMisc& x){
    action.set(x.action);
    shiny.set(x.shiny);
    gender.set(x.gender);
    nature.set(x.nature);
}


StatsHuntIvJudgeFilterRow::StatsHuntIvJudgeFilterRow(const StatsHuntMiscFeatureFlags& feature_flags)
    : misc(feature_flags)
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
StatsHuntIvJudgeFilterRow::StatsHuntIvJudgeFilterRow(const EditableTableOption* table)
    : StatsHuntIvJudgeFilterRow(static_cast<const StatsHuntIvJudgeFilterTable&>(*table).feature_flags)
{}
StatsHuntIvJudgeFilterRow::StatsHuntIvJudgeFilterRow(const StatsHuntMiscFeatureFlags& feature_flags, StatsHuntShinyFilter p_shiny)
    : StatsHuntIvJudgeFilterRow(feature_flags)
{
    misc.shiny.set(p_shiny);
}
std::unique_ptr<EditableTableRow> StatsHuntIvJudgeFilterRow::clone() const{
    std::unique_ptr<StatsHuntIvJudgeFilterRow> ret(new StatsHuntIvJudgeFilterRow(misc.feature_flags));
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
    const IvJudgeReader::Results& IVs,
    StatsHuntGenderFilter gender,
    NatureReader::Results nature
) const{
    //  Check the shiny filter.
    switch (misc.shiny){
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

    //  Check all the IV filters.
    if (!IvJudge_filter_match(iv_hp, IVs.hp)) return false;
    if (!IvJudge_filter_match(iv_atk, IVs.attack)) return false;
    if (!IvJudge_filter_match(iv_def, IVs.defense)) return false;
    if (!IvJudge_filter_match(iv_spatk, IVs.spatk)) return false;
    if (!IvJudge_filter_match(iv_spdef, IVs.spdef)) return false;
    if (!IvJudge_filter_match(iv_speed, IVs.speed)) return false;

    StatsHuntGenderFilter filter_gender = misc.gender;
    if (filter_gender != gender && filter_gender != StatsHuntGenderFilter::Any){
        return false;
    }

    if (!NatureChecker_filter_match(misc.nature, nature.nature)){
        return false;
    }

    return true;
}

StatsHuntIvJudgeFilterTable::StatsHuntIvJudgeFilterTable(
    const std::string& label,
    const StatsHuntMiscFeatureFlags& p_feature_flags
)
    : EditableTableOption_t<StatsHuntIvJudgeFilterRow>(label, LockWhileRunning::LOCKED)
    , feature_flags(p_feature_flags)
{
    set_default(make_defaults());
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
std::vector<std::unique_ptr<EditableTableRow>> StatsHuntIvJudgeFilterTable::make_defaults() const{
    std::vector<std::unique_ptr<EditableTableRow>> ret;
    if (feature_flags.shiny){
        ret.emplace_back(new StatsHuntIvJudgeFilterRow(this));
    }
    return ret;
}
StatsHuntAction StatsHuntIvJudgeFilterTable::get_action(
    bool shiny,
    const IvJudgeReader::Results& IVs,
    StatsHuntGenderFilter gender,
    NatureReader::Results nature
) const{
    StatsHuntAction action = StatsHuntAction::Discard;
    std::vector<std::unique_ptr<StatsHuntIvJudgeFilterRow>> list = copy_snapshot();
    for (size_t c = 0; c < list.size(); c++){
        const StatsHuntIvJudgeFilterRow& filter = *list[c];

        if (!filter.matches(shiny, IVs, gender, nature)){
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




StatsHuntIvRangeFilterRow::StatsHuntIvRangeFilterRow(const StatsHuntMiscFeatureFlags& feature_flags)
    : misc(feature_flags)
    , iv_lo_hp(LockWhileRunning::UNLOCKED, 0, 0, 31)
    , iv_hi_hp(LockWhileRunning::UNLOCKED, 31, 0, 31)
    , iv_lo_atk(LockWhileRunning::UNLOCKED, 0, 0, 31)
    , iv_hi_atk(LockWhileRunning::UNLOCKED, 31, 0, 31)
    , iv_lo_def(LockWhileRunning::UNLOCKED, 0, 0, 31)
    , iv_hi_def(LockWhileRunning::UNLOCKED, 31, 0, 31)
    , iv_lo_spatk(LockWhileRunning::UNLOCKED, 0, 0, 31)
    , iv_hi_spatk(LockWhileRunning::UNLOCKED, 31, 0, 31)
    , iv_lo_spdef(LockWhileRunning::UNLOCKED, 0, 0, 31)
    , iv_hi_spdef(LockWhileRunning::UNLOCKED, 31, 0, 31)
    , iv_lo_speed(LockWhileRunning::UNLOCKED, 0, 0, 31)
    , iv_hi_speed(LockWhileRunning::UNLOCKED, 31, 0, 31)
{
    if (misc.feature_flags.action)  PA_ADD_OPTION(misc.action);
    if (misc.feature_flags.shiny)   PA_ADD_OPTION(misc.shiny);
    if (misc.feature_flags.gender)  PA_ADD_OPTION(misc.gender);
    if (misc.feature_flags.nature)  PA_ADD_OPTION(misc.nature);
    PA_ADD_OPTION(iv_lo_hp);
    PA_ADD_OPTION(iv_hi_hp);
    PA_ADD_OPTION(iv_lo_atk);
    PA_ADD_OPTION(iv_hi_atk);
    PA_ADD_OPTION(iv_lo_def);
    PA_ADD_OPTION(iv_hi_def);
    PA_ADD_OPTION(iv_lo_spatk);
    PA_ADD_OPTION(iv_hi_spatk);
    PA_ADD_OPTION(iv_lo_spdef);
    PA_ADD_OPTION(iv_hi_spdef);
    PA_ADD_OPTION(iv_lo_speed);
    PA_ADD_OPTION(iv_hi_speed);
}
StatsHuntIvRangeFilterRow::StatsHuntIvRangeFilterRow(const EditableTableOption* table)
    : StatsHuntIvRangeFilterRow(static_cast<const StatsHuntIvJudgeFilterTable&>(*table).feature_flags)
{}
std::unique_ptr<EditableTableRow> StatsHuntIvRangeFilterRow::clone() const{
    std::unique_ptr<StatsHuntIvRangeFilterRow> ret(new StatsHuntIvRangeFilterRow(misc.feature_flags));
    ret->misc.set(misc);
    ret->iv_lo_hp.set(iv_lo_hp);
    ret->iv_hi_hp.set(iv_hi_hp);
    ret->iv_lo_atk.set(iv_lo_atk);
    ret->iv_hi_atk.set(iv_hi_atk);
    ret->iv_lo_def.set(iv_lo_def);
    ret->iv_hi_def.set(iv_hi_def);
    ret->iv_lo_spatk.set(iv_lo_spatk);
    ret->iv_hi_spatk.set(iv_hi_spatk);
    ret->iv_lo_spdef.set(iv_lo_spdef);
    ret->iv_hi_spdef.set(iv_hi_spdef);
    ret->iv_lo_speed.set(iv_lo_speed);
    ret->iv_hi_speed.set(iv_hi_speed);
    return ret;
}

StatsHuntIvRangeFilterTable::StatsHuntIvRangeFilterTable(
    const std::string& label,
    const StatsHuntMiscFeatureFlags& p_feature_flags
)
    : EditableTableOption_t<StatsHuntIvRangeFilterRow>(label, LockWhileRunning::LOCKED)
    , feature_flags(p_feature_flags)
{
//    set_default(make_defaults());
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

    ret.emplace_back("HP (low)");
    ret.emplace_back("HP (high)");
    ret.emplace_back("Atk (low)");
    ret.emplace_back("Atk (high)");
    ret.emplace_back("Def (low)");
    ret.emplace_back("Def (high)");
    ret.emplace_back("SpAtk (low)");
    ret.emplace_back("SpAtk (high)");
    ret.emplace_back("SpDef (low)");
    ret.emplace_back("SpDef (high)");
    ret.emplace_back("Spd (low)");
    ret.emplace_back("Spd (high)");

    return ret;
}






}
}
