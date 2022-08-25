/*  Encounter Filter
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_EncounterFilterOverride_H
#define PokemonAutomation_PokemonSwSh_EncounterFilterOverride_H

#include "Common/Cpp/Options/EditableTableOption.h"
#include "PokemonSwSh/Options/PokemonSwSh_BallSelectOption.h"
#include "PokemonSwSh_EncounterFilterEnums.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{



class EncounterFilterOverride : public EditableTableRow, private ConfigOption::Listener{
public:
    ~EncounterFilterOverride();
    EncounterFilterOverride(bool rare_stars);
    virtual void load_json(const JsonValue& json) override;
    virtual std::unique_ptr<EditableTableRow> clone() const override;

    virtual void value_changed() override;

private:
    const bool m_rare_stars;
public:
    EncounterActionCell action;
    PokemonBallSelectCell pokeball;
    StringSelectCell pokemon;
    ShinyFilterCell shininess;
};


//  Cannot use "EditableTableOption_t<>" since the row class takes an extra
//  parameter.
class EncounterFilterTable : public EditableTableOption{
public:
    EncounterFilterTable(bool rare_stars);
    std::vector<std::unique_ptr<EncounterFilterOverride>> copy_snapshot() const;
    virtual std::vector<std::string> make_header() const override;
    virtual std::unique_ptr<EditableTableRow> make_row() const override;

private:
    bool m_rare_stars;
};





}
}
}
#endif
