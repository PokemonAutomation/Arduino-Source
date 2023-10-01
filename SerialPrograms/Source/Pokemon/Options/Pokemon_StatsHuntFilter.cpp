/*  Egg Hatch Filter
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Compiler.h"
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

StatsHuntFilterRow::StatsHuntFilterRow()
    : action(EggHatchAction_Database(), LockWhileRunning::LOCKED, StatsHuntAction::Keep)
    , shiny(EggHatchShinyFilter_Database(), LockWhileRunning::LOCKED, StatsHuntShinyFilter::Anything)
    , gender(EggHatchGenderFilter_Database(), LockWhileRunning::LOCKED, StatsHuntGenderFilter::Any)
    , nature(NatureCheckerFilter_Database(), LockWhileRunning::LOCKED, NatureCheckerFilter::Any)
    , iv_hp(IvJudgeFilter::Anything)
    , iv_atk(IvJudgeFilter::Anything)
    , iv_def(IvJudgeFilter::Anything)
    , iv_spatk(IvJudgeFilter::Anything)
    , iv_spdef(IvJudgeFilter::Anything)
    , iv_speed(IvJudgeFilter::Anything)
{
    PA_ADD_OPTION(action);
    PA_ADD_OPTION(shiny);
    PA_ADD_OPTION(gender);
    PA_ADD_OPTION(nature);
    PA_ADD_OPTION(iv_hp);
    PA_ADD_OPTION(iv_atk);
    PA_ADD_OPTION(iv_def);
    PA_ADD_OPTION(iv_spatk);
    PA_ADD_OPTION(iv_spdef);
    PA_ADD_OPTION(iv_speed);
}
StatsHuntFilterRow::StatsHuntFilterRow(StatsHuntShinyFilter p_shiny)
    : StatsHuntFilterRow()
{
    shiny.set(p_shiny);
}
std::unique_ptr<EditableTableRow> StatsHuntFilterRow::clone() const{
    std::unique_ptr<StatsHuntFilterRow> ret(new StatsHuntFilterRow());
    ret->action.set(action);
    ret->shiny.set(shiny);
    ret->gender.set(gender);
    ret->nature.set(nature);
    ret->iv_hp.set(iv_hp);
    ret->iv_atk.set(iv_atk);
    ret->iv_def.set(iv_def);
    ret->iv_spatk.set(iv_spatk);
    ret->iv_spdef.set(iv_spdef);
    ret->iv_speed.set(iv_speed);
    return ret;
}





StatsHuntFilterTable::StatsHuntFilterTable()
    : EditableTableOption_t<StatsHuntFilterRow>(
        "<b>Actions Table:</b><br>"
        "If a hatchling matches one of these filters, the specified action will be performed. "
        "Otherwise, it will be released. "
        "If multiple entries apply and have conflicting actions, the program will stop.<br>"
        "IV checking requires that your right panel be set to the IV Judge and that you have selected the correct game language above.",
        LockWhileRunning::LOCKED,
        make_defaults()
    )
{}
std::vector<std::string> StatsHuntFilterTable::make_header() const{
    return std::vector<std::string>{
        "Action",
        "Shininess",
        "Gender",
        "Nature",
        "HP",
        "Attack",
        "Defense",
        "Sp. Attack",
        "Sp. Defense",
        "Speed",
    };
}
std::vector<std::unique_ptr<EditableTableRow>> StatsHuntFilterTable::make_defaults(){
    std::vector<std::unique_ptr<EditableTableRow>> ret;
    ret.emplace_back(new StatsHuntFilterRow(StatsHuntShinyFilter::Shiny));
    return ret;
}

StatsHuntAction StatsHuntFilterTable::get_action(
    bool shiny,
    const IvJudgeReader::Results& IVs,
    StatsHuntGenderFilter gender,
    NatureReader::Results nature
) const{
    StatsHuntAction action = StatsHuntAction::Discard;
    std::vector<std::unique_ptr<StatsHuntFilterRow>> list = copy_snapshot();
    for (size_t c = 0; c < list.size(); c++){
        const StatsHuntFilterRow& filter = *list[c];

        //  Check the shiny filter.
        switch (filter.shiny){
        case StatsHuntShinyFilter::Anything:
            break;
        case StatsHuntShinyFilter::NotShiny:
            if (shiny){
                continue;
            }
            break;
        case StatsHuntShinyFilter::Shiny:
            if (!shiny){
                continue;
            }
            break;
        }

        //  Check all the IV filters.
        if (!IvJudge_filter_match(filter.iv_hp, IVs.hp)) continue;
        if (!IvJudge_filter_match(filter.iv_atk, IVs.attack)) continue;
        if (!IvJudge_filter_match(filter.iv_def, IVs.defense)) continue;
        if (!IvJudge_filter_match(filter.iv_spatk, IVs.spatk)) continue;
        if (!IvJudge_filter_match(filter.iv_spdef, IVs.spdef)) continue;
        if (!IvJudge_filter_match(filter.iv_speed, IVs.speed)) continue;
        
        StatsHuntGenderFilter filter_gender = filter.gender;
        if (filter_gender != gender && filter_gender != StatsHuntGenderFilter::Any){
            continue;
        }

        if (!NatureChecker_filter_match(filter.nature, nature.nature)){
            continue;
        }

        StatsHuntAction filter_action = filter.action;

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
