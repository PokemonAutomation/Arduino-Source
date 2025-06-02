/*  Encounter Filter
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_EncounterFilterOverride_H
#define PokemonAutomation_PokemonSwSh_EncounterFilterOverride_H

#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/EditableTableOption.h"
#include "PokemonSwSh/Options/PokemonSwSh_BallSelectOption.h"
#include "PokemonSwSh_EncounterFilterEnums.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


struct EncounterActionFull{
    EncounterAction action;
    std::string pokeball_slug;
    uint16_t ball_limit = 999;
};


class EncounterFilterOverride : public EditableTableRow, private ConfigOption::Listener{
public:
    ~EncounterFilterOverride();
    EncounterFilterOverride(EditableTableOption& parent_table);
    virtual void load_json(const JsonValue& json) override;
    virtual std::unique_ptr<EditableTableRow> clone() const override;

    virtual void on_config_value_changed(void* object) override;

public:
    StringSelectCell pokemon;
    ShinyFilterCell shininess;
    EncounterActionCell action;
    PokemonBallSelectCell pokeball;
    SimpleIntegerCell<uint16_t> ball_limit;
};


//  Cannot use "EditableTableOption_t<>" since the row class takes an extra
//  parameter.
class EncounterFilterTable : public EditableTableOption{
public:
    EncounterFilterTable(bool rare_stars);
    std::vector<std::unique_ptr<EncounterFilterOverride>> copy_snapshot() const;
    virtual std::vector<std::string> make_header() const override;
    virtual std::unique_ptr<EditableTableRow> make_row() override;

public:
    const bool rare_stars;
};





}
}
}
#endif
