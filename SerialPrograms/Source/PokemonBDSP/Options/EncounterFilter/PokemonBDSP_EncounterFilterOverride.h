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
#include "PokemonSwSh/Options/PokemonSwSh_NameSelectOption.h"
#include "PokemonBDSP_EncounterFilterEnums.h"

namespace PokemonAutomation{
namespace Pokemon{
    class BallSelectWidget;
}
namespace NintendoSwitch{
namespace PokemonBDSP{

using namespace Pokemon;


struct EncounterActionFull{
    EncounterAction action;
    std::string pokeball_slug;

    bool operator==(const EncounterActionFull& x) const;
    bool operator!=(const EncounterActionFull& x) const;
    std::string to_str() const;
};


class EncounterFilterOverride : public EditableTableRow{
public:
    EncounterFilterOverride(bool allow_autocatch);

    virtual void load_json(const JsonValue& json) override;
    virtual JsonValue to_json() const override;
    virtual std::unique_ptr<EditableTableRow> clone() const override;
    virtual std::vector<QWidget*> make_widgets(QWidget& parent) override;

private:
    QWidget* make_action_box(QWidget& parent);
    BallSelectWidget* make_ball_select(QWidget& parent);
    QWidget* make_species_select(QWidget& parent);
    QWidget* make_shiny_box(QWidget& parent);

    void update_ball_select();

private:
    bool m_allow_autocatch;
    BallSelectWidget* m_ball_select;
public:
    EncounterAction action = EncounterAction::StopProgram;
    std::string pokeball_slug = "poke-ball";

    std::string pokemon_slug;
    ShinyFilter shininess = ShinyFilter::ANYTHING;
};

class EncounterFilterOptionFactory : public EditableTableFactory{
public:
    EncounterFilterOptionFactory(bool allow_autocatch);
    virtual std::vector<std::string> make_header() const override;
    virtual std::unique_ptr<EditableTableRow> make_row() const override;
private:
    bool m_allow_autocatch;
};







class EncounterFilterOverride2 : public EditableTableRow2, private ConfigOption::Listener{
public:
    ~EncounterFilterOverride2();
    EncounterFilterOverride2();
    virtual void load_json(const JsonValue& json) override;
    virtual std::unique_ptr<EditableTableRow2> clone() const override;

    virtual void value_changed() override;

public:
    EncounterActionCell action;
    PokemonSwSh::PokemonBallSelectCell pokeball;
    PokemonSwSh::PokemonNameSelectCell pokemon;
    ShinyFilterCell shininess;
};

class EncounterFilterTable : public EditableTableOption2<EncounterFilterOverride2>{
public:
    EncounterFilterTable();
    virtual std::vector<std::string> make_header() const override;
};







}
}
}
#endif
