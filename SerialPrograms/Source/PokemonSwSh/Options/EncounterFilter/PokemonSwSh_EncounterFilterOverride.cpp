/*  Encounter Filter
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "Common/Qt/NoWheelComboBox.h"
#include "Pokemon/Resources/Pokemon_PokeballNames.h"
#include "Pokemon/Resources/Pokemon_PokemonSlugs.h"
#include "Pokemon/Options/Pokemon_BallSelectWidget.h"
#include "Pokemon/Options/Pokemon_NameSelectWidget.h"
#include "PokemonSwSh/Resources/PokemonSwSh_PokemonIcons.h"
#include "PokemonSwSh_EncounterFilterEnums.h"
#include "PokemonSwSh_EncounterFilterOverride.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{



EncounterFilterOverride::EncounterFilterOverride(bool rare_stars)
    : m_rare_stars(rare_stars)
//    , shininess(rare_stars ? ShinyFilter::SQUARE_ONLY : ShinyFilter::STAR_ONLY)
{}

void EncounterFilterOverride::load_json(const JsonValue& json){
    const JsonObject* obj = json.get_object();
    if (obj == nullptr){
        return;
    }
    const std::string* str;
    str = obj->get_string("Action");
    if (str != nullptr){
        auto iter = EncounterAction_MAP.find(*str);
        if (iter != EncounterAction_MAP.end()){
            action = iter->second;
        }
    }
    obj->read_string(pokeball_slug, "Ball");
    obj->read_string(pokemon_slug, "Species");
    str = obj->get_string("ShinyFilter");
    if (str != nullptr){
        auto iter = ShinyFilter_MAP.find(*str);
        if (iter != ShinyFilter_MAP.end()){
            shininess = iter->second;
        }
    }
}
JsonValue EncounterFilterOverride::to_json() const{
    JsonObject obj;
    obj["Action"] = EncounterAction_NAMES[(size_t)action];
    obj["Ball"] = pokeball_slug;
    obj["Species"] = pokemon_slug;
    obj["ShinyFilter"] = ShinyFilter_NAMES[(size_t)shininess];
    return obj;
}
std::unique_ptr<EditableTableRow> EncounterFilterOverride::clone() const{
    return std::unique_ptr<EditableTableRow>(new EncounterFilterOverride(*this));
}
std::vector<QWidget*> EncounterFilterOverride::make_widgets(QWidget& parent){
    std::vector<QWidget*> widgets;
    BallSelectWidget* ball_select = make_ball_select(parent);
    widgets.emplace_back(make_action_box(parent, *ball_select));
    widgets.emplace_back(ball_select);
    widgets.emplace_back(make_species_select(parent));
    widgets.emplace_back(make_shiny_box(parent));
    return widgets;
}
QWidget* EncounterFilterOverride::make_action_box(QWidget& parent, BallSelectWidget& ball_select){
    QComboBox* box = new NoWheelComboBox(&parent);
    for (const std::string& action : EncounterAction_NAMES){
        box->addItem(QString::fromStdString(action));
    }
    box->setCurrentIndex((int)action);

    switch (action){
    case EncounterAction::StopProgram:
    case EncounterAction::RunAway:
        ball_select.setEnabled(false);
        break;
    case EncounterAction::ThrowBalls:
    case EncounterAction::ThrowBallsAndSave:
        ball_select.setEnabled(true);
        break;
    }

    box->connect(
        box, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
        box, [&](int index){
            if (index < 0){
                index = 0;
            }
            action = (EncounterAction)index;
            switch ((EncounterAction)index){
            case EncounterAction::StopProgram:
            case EncounterAction::RunAway:
                ball_select.setEnabled(false);
                break;
            case EncounterAction::ThrowBalls:
            case EncounterAction::ThrowBallsAndSave:
                ball_select.setEnabled(true);
                break;
            }
        }
    );
    return box;
}
BallSelectWidget* EncounterFilterOverride::make_ball_select(QWidget& parent){
    using namespace Pokemon;
    BallSelectWidget* box = new BallSelectWidget(parent, POKEBALL_SLUGS(), pokeball_slug);
    box->connect(
        box, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
        box, [&, box](int index){
            pokeball_slug = box->slug();
        }
    );
    return box;
}
QWidget* EncounterFilterOverride::make_species_select(QWidget& parent){
    using namespace Pokemon;
    NameSelectWidget* box = new NameSelectWidget(
        parent,
        PokemonSwSh::ALL_POKEMON_ICONS(),
        NATIONAL_DEX_SLUGS(),
        pokemon_slug
    );
    box->connect(
        box, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
        box, [&, box](int index){
            pokemon_slug = box->slug();
        }
    );
    return box;
}
QWidget* EncounterFilterOverride::make_shiny_box(QWidget& parent){
    QComboBox* box = new NoWheelComboBox(&parent);
    if (m_rare_stars){
        box->addItem(QString::fromStdString(ShinyFilter_NAMES[(int)ShinyFilter::NOT_SHINY]));
        box->addItem(QString::fromStdString(ShinyFilter_NAMES[(int)ShinyFilter::SQUARE_ONLY]));
    }else{
        box->addItem(QString::fromStdString(ShinyFilter_NAMES[(int)ShinyFilter::ANYTHING]));
        box->addItem(QString::fromStdString(ShinyFilter_NAMES[(int)ShinyFilter::NOT_SHINY]));
        box->addItem(QString::fromStdString(ShinyFilter_NAMES[(int)ShinyFilter::ANY_SHINY]));
        box->addItem(QString::fromStdString(ShinyFilter_NAMES[(int)ShinyFilter::STAR_ONLY]));
        box->addItem(QString::fromStdString(ShinyFilter_NAMES[(int)ShinyFilter::SQUARE_ONLY]));
        box->addItem(QString::fromStdString(ShinyFilter_NAMES[(int)ShinyFilter::NOTHING]));
    }
    ShinyFilter current = shininess;
    for (int c = 0; c < box->count(); c++){
        if (box->itemText(c).toStdString() == ShinyFilter_NAMES[(int)current]){
            box->setCurrentIndex(c);
            break;
        }
    }
    box->connect(
        box, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
        box, [&, box](int index){
            if (index < 0){
                return;
            }
            std::string text = box->itemText(index).toStdString();
            auto iter = ShinyFilter_MAP.find(text);
            if (iter == ShinyFilter_MAP.end()){
                throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Invalid option: " + text);
            }
            shininess = iter->second;
        }
    );
    return box;
}




}
}
}
