/*  Encounter Filter
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_EncounterFilterOverride_H
#define PokemonAutomation_PokemonBDSP_EncounterFilterOverride_H

#include "Common/Cpp/Options/EditableTableOption2.h"
#include "CommonFramework/Options/EditableTableOption.h"
#include "PokemonSwSh/Options/PokemonSwSh_BallSelectOption.h"
#include "PokemonBDSP_EncounterFilterEnums.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


struct EncounterActionFull{
    EncounterAction action;
    std::string pokeball_slug;

    bool operator==(const EncounterActionFull& x) const;
    bool operator!=(const EncounterActionFull& x) const;
    std::string to_str() const;
};




class EncounterFilterOverride : public EditableTableRow2, private ConfigOption::Listener{
public:
    ~EncounterFilterOverride();
    EncounterFilterOverride();
    virtual void load_json(const JsonValue& json) override;
    virtual std::unique_ptr<EditableTableRow2> clone() const override;

    virtual void value_changed() override;

public:
    EncounterActionCell action;
    PokemonSwSh::PokemonBallSelectCell pokeball;
    StringSelectCell pokemon;
    ShinyFilterCell shininess;
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
