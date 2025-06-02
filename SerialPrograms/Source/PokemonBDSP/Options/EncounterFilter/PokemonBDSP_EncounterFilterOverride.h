/*  Encounter Filter
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_EncounterFilterOverride_H
#define PokemonAutomation_PokemonBDSP_EncounterFilterOverride_H

#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/EditableTableOption.h"
#include "PokemonSwSh/Options/PokemonSwSh_BallSelectOption.h"
#include "PokemonBDSP_EncounterFilterEnums.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


struct EncounterActionFull{
    EncounterAction action;
    std::string pokeball_slug;
    uint16_t ball_limit = 999;

    bool operator==(const EncounterActionFull& x) const;
    bool operator!=(const EncounterActionFull& x) const;
    std::string to_str() const;
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
    PokemonSwSh::PokemonBallSelectCell pokeball;
    SimpleIntegerCell<uint16_t> ball_limit;
};

class EncounterFilterTable : public EditableTableOption_t<EncounterFilterOverride>{
public:
    EncounterFilterTable();
    virtual std::vector<std::string> make_header() const override;
};







}
}
}
#endif
