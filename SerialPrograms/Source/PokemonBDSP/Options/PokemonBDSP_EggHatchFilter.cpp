/*  Egg Hatch Filter
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
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




EggHatchFilterRow::EggHatchFilterRow()
    : action({"Stop Program", "Keep"}, 1)
    , shiny({"Anything", "Not Shiny", "Shiny"}, 0)
    , gender({"Any", "Male", "Female"}, 0)
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
    shiny.set_index((size_t)p_shiny);
}
std::unique_ptr<EditableTableRow2> EggHatchFilterRow::clone() const{
    std::unique_ptr<EggHatchFilterRow> ret(new EggHatchFilterRow());
    ret->action.set_index(action);
    ret->shiny.set_index(shiny);
    ret->gender.set_index(gender);
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
std::vector<std::unique_ptr<EditableTableRow2>> EggHatchFilterTable::make_defaults(){
    std::vector<std::unique_ptr<EditableTableRow2>> ret;
    ret.emplace_back(new EggHatchFilterRow(EggHatchShinyFilter::Shiny));
    return ret;
}


EggHatchAction EggHatchFilterTable::get_action(bool shiny, const IVCheckerReader::Results& IVs, EggHatchGenderFilter gender) const{
    EggHatchAction action = EggHatchAction::Release;
    std::vector<std::unique_ptr<EggHatchFilterRow>> list = copy_snapshot();
    for (size_t c = 0; c < list.size(); c++){
        const EggHatchFilterRow& filter = *list[c];

        //  Check the shiny filter.
        switch ((EggHatchShinyFilter)(size_t)filter.shiny){
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

        EggHatchGenderFilter filter_gender = (EggHatchGenderFilter)(size_t)filter.gender;
        if(filter_gender != gender && filter_gender != EggHatchGenderFilter::Any){
            continue;
        }

        EggHatchAction filter_action = (EggHatchAction)(size_t)filter.action;

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
