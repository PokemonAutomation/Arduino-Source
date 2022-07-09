/*  Encounter Filter
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_EncounterFilterOverride_H
#define PokemonAutomation_PokemonSwSh_EncounterFilterOverride_H

#include "CommonFramework/Options/EditableTableOption.h"

namespace PokemonAutomation{
namespace Pokemon{
    class BallSelectWidget;
}
namespace NintendoSwitch{
namespace PokemonSwSh{

using namespace Pokemon;



enum class ShinyFilter{
    ANYTHING,
    NOT_SHINY,
    ANY_SHINY,
    STAR_ONLY,
    SQUARE_ONLY,
    NOTHING,
};

enum class EncounterAction{
    StopProgram,
    RunAway,
    ThrowBalls,
    ThrowBallsAndSave,
};



class EncounterFilterOverride : public EditableTableRow{
public:
    EncounterFilterOverride(bool rare_stars);

    virtual void load_json(const JsonValue& json) override;
    virtual JsonValue to_json() const override;
    virtual std::unique_ptr<EditableTableRow> clone() const override;
    virtual std::vector<QWidget*> make_widgets(QWidget& parent) override;

private:
    QWidget* make_action_box(QWidget& parent, BallSelectWidget& ball_select);
    BallSelectWidget* make_ball_select(QWidget& parent);
    QWidget* make_species_select(QWidget& parent);
    QWidget* make_shiny_box(QWidget& parent);

private:
    bool m_rare_stars;
public:
    EncounterAction action = EncounterAction::StopProgram;
    std::string pokeball_slug = "poke-ball";

    std::string pokemon_slug;
    ShinyFilter shininess = ShinyFilter::NOT_SHINY;
};




}
}
}
#endif
