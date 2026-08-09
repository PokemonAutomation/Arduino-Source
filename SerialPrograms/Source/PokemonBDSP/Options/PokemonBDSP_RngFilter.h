/*  BDSP RNG Filter
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_RngFilter_H
#define PokemonAutomation_PokemonBDSP_RngFilter_H

#include <stdint.h>
#include <string>
#include <vector>
#include "Common/Cpp/Options/EditableTableOption.h"
#include "Common/Cpp/Options/IntegerRangeOption.h"
#include "Pokemon/Pokemon_NatureChecker.h"
#include "Pokemon/Pokemon_StatsCalculation.h"
#include "Pokemon/Options/Pokemon_StatsHuntFilter.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


class BdspRngFilterTable;
class BdspRngFilterRow : public EditableTableRow{
public:
    BdspRngFilterRow(EditableTableOption& parent_table);
    virtual std::unique_ptr<EditableTableRow> clone() const override;

    bool matches(
        bool shiny,
        Pokemon::StatsHuntGenderFilter gender,
        Pokemon::NatureCheckerValue nature,
        uint8_t height,
        const Pokemon::IvRanges& ivs
    ) const;

private:
    static bool match_iv(const IntegerRangeCell<uint8_t>& desired, const Pokemon::IvRange& actual);

public:
    Pokemon::StatsHuntRowMisc misc;

    IntegerRangeCell<uint8_t> height;

    IntegerRangeCell<uint8_t> iv_hp;
    IntegerRangeCell<uint8_t> iv_atk;
    IntegerRangeCell<uint8_t> iv_def;
    IntegerRangeCell<uint8_t> iv_spatk;
    IntegerRangeCell<uint8_t> iv_spdef;
    IntegerRangeCell<uint8_t> iv_speed;
};


//  The table copied once. Calling get_action() on the table itself clones every row on
//  every call, which is far too slow for a scan running over millions of advances.
class BdspRngFilterSnapshot{
public:
    explicit BdspRngFilterSnapshot(std::vector<std::unique_ptr<BdspRngFilterRow>> rows);

    Pokemon::StatsHuntAction get_action(
        bool shiny,
        Pokemon::StatsHuntGenderFilter gender,
        Pokemon::NatureCheckerValue nature,
        uint8_t height,
        const Pokemon::IvRanges& ivs
    ) const;

private:
    std::vector<std::unique_ptr<BdspRngFilterRow>> m_rows;
};


class BdspRngFilterTable : public EditableTableOption_t<BdspRngFilterRow>{
public:
    BdspRngFilterTable(
        const std::string& label,
        const Pokemon::StatsHuntMiscFeatureFlags& p_feature_flags
    );
    virtual std::vector<std::string> make_header() const override;

    BdspRngFilterSnapshot make_snapshot() const;

    Pokemon::StatsHuntAction get_action(
        bool shiny,
        Pokemon::StatsHuntGenderFilter gender,
        Pokemon::NatureCheckerValue nature,
        uint8_t height,
        const Pokemon::IvRanges& ivs
    ) const;

public:
    const Pokemon::StatsHuntMiscFeatureFlags feature_flags;
};


}
}
}
#endif
