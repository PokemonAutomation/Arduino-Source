/*  BDSP RNG Filter
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Logging/Logger.h"
#include "PokemonBDSP_RngFilter.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{

using namespace Pokemon;


const uint8_t BDSP_HEIGHT_MIN = 0;
const uint8_t BDSP_HEIGHT_MAX = 255;


BdspRngFilterRow::BdspRngFilterRow(EditableTableOption& parent_table)
    : EditableTableRow(parent_table)
    , misc(static_cast<const BdspRngFilterTable&>(parent_table).feature_flags)
    , height(
        LockMode::UNLOCK_WHILE_RUNNING,
        BDSP_HEIGHT_MIN, BDSP_HEIGHT_MAX, BDSP_HEIGHT_MIN, BDSP_HEIGHT_MIN,
        BDSP_HEIGHT_MIN, BDSP_HEIGHT_MAX, BDSP_HEIGHT_MAX, BDSP_HEIGHT_MAX
    )
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
    PA_ADD_OPTION(height);
    PA_ADD_OPTION(iv_hp);
    PA_ADD_OPTION(iv_atk);
    PA_ADD_OPTION(iv_def);
    PA_ADD_OPTION(iv_spatk);
    PA_ADD_OPTION(iv_spdef);
    PA_ADD_OPTION(iv_speed);
}
std::unique_ptr<EditableTableRow> BdspRngFilterRow::clone() const{
    std::unique_ptr<BdspRngFilterRow> ret(new BdspRngFilterRow(parent()));
    ret->misc.set(misc);
    ret->height.set(height);
    ret->iv_hp.set(iv_hp);
    ret->iv_atk.set(iv_atk);
    ret->iv_def.set(iv_def);
    ret->iv_spatk.set(iv_spatk);
    ret->iv_spdef.set(iv_spdef);
    ret->iv_speed.set(iv_speed);
    return ret;
}
bool BdspRngFilterRow::match_iv(
    const IntegerRangeCell<uint8_t>& desired, const IvRange& actual
){
    uint8_t lo, hi;
    desired.current_values(lo, hi);
    if (lo == 0 && hi == 31){
        return true;
    }
    if (actual.high < (int8_t)lo){
        return false;
    }
    if ((int8_t)hi < actual.low){
        return false;
    }
    return true;
}
bool BdspRngFilterRow::matches(
    bool shiny,
    StatsHuntGenderFilter gender,
    NatureCheckerValue nature,
    uint8_t height,
    const IvRanges& ivs
) const{
    if (!misc.matches(shiny, gender, nature)){
        return false;
    }

    uint8_t height_lo, height_hi;
    this->height.current_values(height_lo, height_hi);
    if (height < height_lo || height_hi < height){
        return false;
    }

    if (!match_iv(iv_hp, ivs.hp)) return false;
    if (!match_iv(iv_atk, ivs.attack)) return false;
    if (!match_iv(iv_def, ivs.defense)) return false;
    if (!match_iv(iv_spatk, ivs.spatk)) return false;
    if (!match_iv(iv_spdef, ivs.spdef)) return false;
    if (!match_iv(iv_speed, ivs.speed)) return false;

    return true;
}


BdspRngFilterTable::BdspRngFilterTable(
    const std::string& label,
    const StatsHuntMiscFeatureFlags& p_feature_flags
)
    : EditableTableOption_t<BdspRngFilterRow>(label, LockMode::UNLOCK_WHILE_RUNNING)
    , feature_flags(p_feature_flags)
{}
std::vector<std::string> BdspRngFilterTable::make_header() const{
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

    ret.emplace_back("Height");

    ret.emplace_back("HP");
    ret.emplace_back("Atk");
    ret.emplace_back("Def");
    ret.emplace_back("SpAtk");
    ret.emplace_back("SpDef");
    ret.emplace_back("Spd");

    return ret;
}
BdspRngFilterSnapshot BdspRngFilterTable::make_snapshot() const{
    return BdspRngFilterSnapshot(copy_snapshot());
}
StatsHuntAction BdspRngFilterTable::get_action(
    bool shiny,
    StatsHuntGenderFilter gender,
    NatureCheckerValue nature,
    uint8_t height,
    const IvRanges& ivs
) const{
    return make_snapshot().get_action(shiny, gender, nature, height, ivs);
}


BdspRngFilterSnapshot::BdspRngFilterSnapshot(std::vector<std::unique_ptr<BdspRngFilterRow>> rows)
    : m_rows(std::move(rows))
{}
StatsHuntAction BdspRngFilterSnapshot::get_action(
    bool shiny,
    StatsHuntGenderFilter gender,
    NatureCheckerValue nature,
    uint8_t height,
    const IvRanges& ivs
) const{
    StatsHuntAction action = StatsHuntAction::Discard;
    for (size_t c = 0; c < m_rows.size(); c++){
        const BdspRngFilterRow& filter = *m_rows[c];

        if (!filter.matches(shiny, gender, nature, height, ivs)){
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
            global_logger_tagged().log(
                "Multiple filters matched with conflicting actions. Stopping program...",
                COLOR_RED
            );
            return StatsHuntAction::StopProgram;
        }
    }
    return action;
}


}
}
}
