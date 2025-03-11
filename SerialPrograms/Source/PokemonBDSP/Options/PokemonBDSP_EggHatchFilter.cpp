/*  Egg Hatch Filter
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Compiler.h"
#include "PokemonBDSP/Inference/BoxSystem/PokemonBDSP_BoxGenderDetector.h"
#include "PokemonBDSP_EggHatchFilter.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{




const EnumDatabase<EggHatchAction>& EggHatchAction_Database(){
    static const EnumDatabase<EggHatchAction> database({
        {EggHatchAction::StopProgram,   "stop",     "Stop Program"},
        {EggHatchAction::Keep,          "keep",     "Keep"},
//        {EggHatchAction::Release,       "release",  "Release"},
    });
    return database;
}
const EnumDatabase<EggHatchShinyFilter>& EggHatchShinyFilter_Database(){
    static const EnumDatabase<EggHatchShinyFilter> database({
        {EggHatchShinyFilter::Anything, "anything",     "Anything"},
        {EggHatchShinyFilter::NotShiny, "not-shiny",    "Not Shiny"},
        {EggHatchShinyFilter::Shiny,    "shiny",        "Shiny"},
    });
    return database;
}
const EnumDatabase<EggHatchGenderFilter>& EggHatchGenderFilter_Database(){
    static const EnumDatabase<EggHatchGenderFilter> database({
        {EggHatchGenderFilter::Any,         "any",          "Any"},
        {EggHatchGenderFilter::Male,        "male",         "Male"},
        {EggHatchGenderFilter::Female,      "female",       "Female"},
//        {EggHatchGenderFilter::Genderless,  "genderless",   "Genderless"},
    });
    return database;
}




EggHatchFilterRow::EggHatchFilterRow()
    : action(EggHatchAction_Database(), EggHatchAction::Keep)
    , shiny(EggHatchShinyFilter_Database(), EggHatchShinyFilter::Anything)
    , gender(EggHatchGenderFilter_Database(), EggHatchGenderFilter::Any)
    , iv_hp(IVCheckerFilter::Anything)
    , iv_atk(IVCheckerFilter::Anything)
    , iv_def(IVCheckerFilter::Anything)
    , iv_spatk(IVCheckerFilter::Anything)
    , iv_spdef(IVCheckerFilter::Anything)
    , iv_speed(IVCheckerFilter::Anything)
{
    PA_ADD_OPTION(action);
    PA_ADD_OPTION(shiny);
    PA_ADD_OPTION(gender);
    PA_ADD_OPTION(iv_hp);
    PA_ADD_OPTION(iv_atk);
    PA_ADD_OPTION(iv_def);
    PA_ADD_OPTION(iv_spatk);
    PA_ADD_OPTION(iv_spdef);
    PA_ADD_OPTION(iv_speed);
}
EggHatchFilterRow::EggHatchFilterRow(EggHatchShinyFilter p_shiny)
    : EggHatchFilterRow()
{
    shiny.set(p_shiny);
}
std::unique_ptr<EditableTableRow> EggHatchFilterRow::clone() const{
    std::unique_ptr<EggHatchFilterRow> ret(new EggHatchFilterRow());
    ret->action.set(action);
    ret->shiny.set(shiny);
    ret->gender.set(gender);
    ret->iv_hp.set(iv_hp);
    ret->iv_atk.set(iv_atk);
    ret->iv_def.set(iv_def);
    ret->iv_spatk.set(iv_spatk);
    ret->iv_spdef.set(iv_spdef);
    ret->iv_speed.set(iv_speed);
    return ret;
}





EggHatchFilterTable::EggHatchFilterTable()
    : EditableTableOption_t<EggHatchFilterRow>(
        "<b>Actions Table:</b><br>"
        "If a hatchling matches one of these filters, the specified action will be performed. "
        "Otherwise, it will be released. "
        "If multiple entries apply and have conflicting actions, the program will stop.<br>"
        "IV checking requires that your right panel be set to the IV Judge and that you have selected the correct game language above.",
        make_defaults()
    )
{}
std::vector<std::string> EggHatchFilterTable::make_header() const{
    return std::vector<std::string>{
        "Action",
        "Shininess",
        "Gender",
        "HP",
        "Attack",
        "Defense",
        "Sp. Attack",
        "Sp. Defense",
        "Speed",
    };
}
std::vector<std::unique_ptr<EditableTableRow>> EggHatchFilterTable::make_defaults(){
    std::vector<std::unique_ptr<EditableTableRow>> ret;
    ret.emplace_back(new EggHatchFilterRow(EggHatchShinyFilter::Shiny));
    return ret;
}


EggHatchAction EggHatchFilterTable::get_action(bool shiny, const IVCheckerReader::Results& IVs, EggHatchGenderFilter gender) const{
    EggHatchAction action = EggHatchAction::Release;
    std::vector<std::unique_ptr<EggHatchFilterRow>> list = copy_snapshot();
    for (size_t c = 0; c < list.size(); c++){
        const EggHatchFilterRow& filter = *list[c];

        //  Check the shiny filter.
        switch (filter.shiny){
        case EggHatchShinyFilter::Anything:
            break;
        case EggHatchShinyFilter::NotShiny:
            if (shiny){
                continue;
            }
            break;
        case EggHatchShinyFilter::Shiny:
            if (!shiny){
                continue;
            }
            break;
        }

        //  Check all the IV filters.
        if (!IVChecker_filter_match(filter.iv_hp, IVs.hp)) continue;
        if (!IVChecker_filter_match(filter.iv_atk, IVs.attack)) continue;
        if (!IVChecker_filter_match(filter.iv_def, IVs.defense)) continue;
        if (!IVChecker_filter_match(filter.iv_spatk, IVs.spatk)) continue;
        if (!IVChecker_filter_match(filter.iv_spdef, IVs.spdef)) continue;
        if (!IVChecker_filter_match(filter.iv_speed, IVs.speed)) continue;

        EggHatchGenderFilter filter_gender = filter.gender;
        if(filter_gender != gender && filter_gender != EggHatchGenderFilter::Any){
            continue;
        }

        EggHatchAction filter_action = filter.action;

        //  No action matched so far. Take the current action and continue.
        if (action == EggHatchAction::Release){
            action = filter_action;
            continue;
        }

        //  Conflicting actions.
        if (action != filter_action){
            global_logger_tagged().log("Multiple filters matched with conflicting actions. Stopping program...", COLOR_RED);
            return EggHatchAction::StopProgram;
        }
    }
    return action;
}





}
}
}
