/*  Stats Reset Filter
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Compiler.h"
#include "Pokemon_StatsResetFilter.h"
#include "Pokemon_StatsHuntFilter.h"

namespace PokemonAutomation{
namespace Pokemon{

StatsResetFilterRow::StatsResetFilterRow()
    : action(StatsHuntAction::StopProgram)
    , nature(NatureCheckerFilter_Database(), LockMode::LOCK_WHILE_RUNNING, NatureCheckerFilter::Any)
    , iv_hp(IvJudgeFilter::Anything)
    , iv_atk(IvJudgeFilter::Anything)
    , iv_def(IvJudgeFilter::Anything)
    , iv_spatk(IvJudgeFilter::Anything)
    , iv_spdef(IvJudgeFilter::Anything)
    , iv_speed(IvJudgeFilter::Anything)
{
    PA_ADD_OPTION(nature);
    PA_ADD_OPTION(iv_hp);
    PA_ADD_OPTION(iv_atk);
    PA_ADD_OPTION(iv_def);
    PA_ADD_OPTION(iv_spatk);
    PA_ADD_OPTION(iv_spdef);
    PA_ADD_OPTION(iv_speed);
}

std::unique_ptr<EditableTableRow> StatsResetFilterRow::clone() const{
    std::unique_ptr<StatsResetFilterRow> ret(new StatsResetFilterRow());
    ret->nature.set(nature);
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
        LockMode::LOCK_WHILE_RUNNING,
        make_defaults()
    )
{}
std::vector<std::string> StatsResetFilterTable::make_header() const{
    return std::vector<std::string>{
        "Nature",
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
    ret.emplace_back(new StatsResetFilterRow());
    return ret;
}

StatsHuntAction StatsResetFilterTable::get_action(bool shiny, const IvJudgeReader::Results& IVs, NatureReader::Results nature) const{
    StatsHuntAction action = StatsHuntAction::Discard;
    std::vector<std::unique_ptr<StatsResetFilterRow>> list = copy_snapshot();
    for (size_t c = 0; c < list.size(); c++){
        const StatsResetFilterRow& filter = *list[c];

        //  Check all the IV filters.
        if (!IvJudge_filter_match(filter.iv_hp, IVs.hp)) continue;
        if (!IvJudge_filter_match(filter.iv_atk, IVs.attack)) continue;
        if (!IvJudge_filter_match(filter.iv_def, IVs.defense)) continue;
        if (!IvJudge_filter_match(filter.iv_spatk, IVs.spatk)) continue;
        if (!IvJudge_filter_match(filter.iv_spdef, IVs.spdef)) continue;
        if (!IvJudge_filter_match(filter.iv_speed, IVs.speed)) continue;

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
