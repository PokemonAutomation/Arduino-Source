/*  Stats Reset Filter
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Compiler.h"
// #include "PokemonBDSP/Inference/BoxSystem/PokemonBDSP_BoxGenderDetector.h"
#include "Pokemon_StatsResetFilter.h"
#include "Pokemon_EggHatchFilter.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace Pokemon{

const EnumDatabase<EggHatchAction>& StatsResetAction_Database() {
    static const EnumDatabase<EggHatchAction> database({
        {EggHatchAction::StopProgram,   "stop",     "Stop Program"},
        {EggHatchAction::Keep,          "keep",     "Keep"},
        });
    return database;
}
const EnumDatabase<EggHatchShinyFilter>& StatsResetShinyFilter_Database() {
    static const EnumDatabase<EggHatchShinyFilter> database({
        {EggHatchShinyFilter::Anything, "anything",     "Anything"},
        {EggHatchShinyFilter::NotShiny, "not-shiny",    "Not Shiny"},
        {EggHatchShinyFilter::Shiny,    "shiny",        "Shiny"},
        });
    return database;
}
const EnumDatabase<EggHatchGenderFilter>& StatsResetGenderFilter_Database() {
    static const EnumDatabase<EggHatchGenderFilter> database({
        {EggHatchGenderFilter::Any,         "any",          "Any"},
        {EggHatchGenderFilter::Male,        "male",         "Male"},
        {EggHatchGenderFilter::Female,      "female",       "Female"},
        });
    return database;
}

StatsResetFilterRow::StatsResetFilterRow()
    : action(StatsResetAction_Database(), LockWhileRunning::LOCKED, EggHatchAction::Keep)
    , shiny(StatsResetShinyFilter_Database(), LockWhileRunning::LOCKED, EggHatchShinyFilter::Anything)
    , gender(StatsResetGenderFilter_Database(), LockWhileRunning::LOCKED, EggHatchGenderFilter::Any)
    , iv_hp(IVCheckerFilter::Anything)
    , iv_atk(IVCheckerFilter::Anything)
    , iv_def(IVCheckerFilter::Anything)
    , iv_spatk(IVCheckerFilter::Anything)
    , iv_spdef(IVCheckerFilter::Anything)
    , iv_speed(IVCheckerFilter::Anything)
{
    //PA_ADD_OPTION(action);
    //PA_ADD_OPTION(shiny);
    //PA_ADD_OPTION(gender);
    PA_ADD_OPTION(iv_hp);
    PA_ADD_OPTION(iv_atk);
    PA_ADD_OPTION(iv_def);
    PA_ADD_OPTION(iv_spatk);
    PA_ADD_OPTION(iv_spdef);
    PA_ADD_OPTION(iv_speed);
}
StatsResetFilterRow::StatsResetFilterRow(EggHatchShinyFilter p_shiny)
    : StatsResetFilterRow()
{
    shiny.set(p_shiny);
}
std::unique_ptr<EditableTableRow> StatsResetFilterRow::clone() const{
    std::unique_ptr<StatsResetFilterRow> ret(new StatsResetFilterRow());
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

StatsResetFilterTable::StatsResetFilterTable()
    : EditableTableOption_t<StatsResetFilterRow>(
        "<b>Actions Table:</b><br>"
        "If the caught Pokemon matches one of these filters, the program will stop. "
        "IV checking requires that your right panel be set to the IV Judge and that you have selected the correct game language above.",
        LockWhileRunning::LOCKED,
        make_defaults()
    )
{}
std::vector<std::string> StatsResetFilterTable::make_header() const{
    return std::vector<std::string>{
        //"Action",
        //"Shininess",
        //"Gender",
        "HP",
        "Attack",
        "Defense",
        "Sp. Attack",
        "Sp. Defense",
        "Speed",
    };
}
std::vector<std::unique_ptr<EditableTableRow>> StatsResetFilterTable::make_defaults(){
    std::vector<std::unique_ptr<EditableTableRow>> ret;
    ret.emplace_back(new StatsResetFilterRow(EggHatchShinyFilter::Shiny));
    return ret;
}


EggHatchAction StatsResetFilterTable::get_action(bool shiny, const IVCheckerReader::Results& IVs, EggHatchGenderFilter gender) const{
    EggHatchAction action = EggHatchAction::Release;
    std::vector<std::unique_ptr<StatsResetFilterRow>> list = copy_snapshot();
    for (size_t c = 0; c < list.size(); c++){
        const StatsResetFilterRow& filter = *list[c];

        //  Check all the IV filters.
        if (!IVChecker_filter_match(filter.iv_hp, IVs.hp)) continue;
        if (!IVChecker_filter_match(filter.iv_atk, IVs.attack)) continue;
        if (!IVChecker_filter_match(filter.iv_def, IVs.defense)) continue;
        if (!IVChecker_filter_match(filter.iv_spatk, IVs.spatk)) continue;
        if (!IVChecker_filter_match(filter.iv_spdef, IVs.spdef)) continue;
        if (!IVChecker_filter_match(filter.iv_speed, IVs.speed)) continue;

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
