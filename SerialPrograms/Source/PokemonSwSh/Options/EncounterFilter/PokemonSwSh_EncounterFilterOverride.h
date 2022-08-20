/*  Encounter Filter
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_EncounterFilterOverride_H
#define PokemonAutomation_PokemonSwSh_EncounterFilterOverride_H

#include "Common/Cpp/Options/EditableTableOption2.h"
#include "PokemonSwSh/Options/PokemonSwSh_BallSelectOption.h"
#include "PokemonSwSh/Options/PokemonSwSh_NameSelectOption.h"
#include "PokemonSwSh_EncounterFilterEnums.h"

namespace PokemonAutomation{
namespace Pokemon{
    class BallSelectWidget;
}
namespace NintendoSwitch{
namespace PokemonSwSh{

using namespace Pokemon;



class EncounterFilterOverride : public EditableTableRow2, private ConfigOption::Listener{
public:
    ~EncounterFilterOverride();
    EncounterFilterOverride(bool rare_stars);
    virtual void load_json(const JsonValue& json) override;
    virtual std::unique_ptr<EditableTableRow2> clone() const override;

    virtual void value_changed() override;

private:
    const bool m_rare_stars;
public:
    EncounterActionCell action;
    PokemonBallSelectCell pokeball;
    PokemonNameSelectCell pokemon;
    ShinyFilterCell shininess;
};


//  Cannot use "EditableTableOption_t<>" since the row class takes an extra
//  parameter.
class EncounterFilterTable : public EditableTableOption2{
public:
    EncounterFilterTable(bool rare_stars);
    std::vector<std::unique_ptr<EncounterFilterOverride>> copy_snapshot() const;
    virtual std::vector<std::string> make_header() const override;
    virtual std::unique_ptr<EditableTableRow2> make_row() const override;

private:
    bool m_rare_stars;
};





}
}
}
#endif
