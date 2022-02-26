/*  Encounter Filter
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QtGlobal>
#include "Common/Cpp/Exception.h"
#include "Common/Qt/QtJsonTools.h"
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

void EncounterFilterOverride::load_json(const QJsonValue& json){
    QJsonObject obj = json.toObject();
    {
        QString value;
        if (json_get_string(value, obj, "Action")){
            auto iter = EncounterAction_MAP.find(value);
            if (iter != EncounterAction_MAP.end()){
                action = iter->second;
            }
        }
    }
    {
        QString value;
        json_get_string(value, obj, "Ball");
        pokeball_slug = value.toUtf8().data();
    }
    {
        QString value;
        json_get_string(value, obj, "Species");
        pokemon_slug = value.toStdString();
    }
    {
        QString value;
        if (json_get_string(value, obj, "ShinyFilter")){
            auto iter = ShinyFilter_MAP.find(value);
            if (iter != ShinyFilter_MAP.end()){
                shininess = iter->second;
            }
        }
    }
}
QJsonValue EncounterFilterOverride::to_json() const{
    QJsonObject obj;
    obj.insert("Action", EncounterAction_NAMES[(size_t)action]);
    obj.insert("Ball", QString::fromStdString(pokeball_slug));
    obj.insert("Species", QString::fromStdString(pokemon_slug));
    obj.insert("ShinyFilter", ShinyFilter_NAMES[(size_t)shininess]);
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
    for (const QString& action : EncounterAction_NAMES){
        box->addItem(action);
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
        box->addItem(ShinyFilter_NAMES[(int)ShinyFilter::NOT_SHINY]);
        box->addItem(ShinyFilter_NAMES[(int)ShinyFilter::SQUARE_ONLY]);
    }else{
        box->addItem(ShinyFilter_NAMES[(int)ShinyFilter::ANYTHING]);
        box->addItem(ShinyFilter_NAMES[(int)ShinyFilter::NOT_SHINY]);
        box->addItem(ShinyFilter_NAMES[(int)ShinyFilter::ANY_SHINY]);
        box->addItem(ShinyFilter_NAMES[(int)ShinyFilter::STAR_ONLY]);
        box->addItem(ShinyFilter_NAMES[(int)ShinyFilter::SQUARE_ONLY]);
        box->addItem(ShinyFilter_NAMES[(int)ShinyFilter::NOTHING]);
    }
    ShinyFilter current = shininess;
    for (int c = 0; c < box->count(); c++){
        if (box->itemText(c) == ShinyFilter_NAMES[(int)current]){
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
            QString text = box->itemText(index);
            auto iter = ShinyFilter_MAP.find(text);
            if (iter == ShinyFilter_MAP.end()){
                PA_THROW_StringException("Invalid option: " + text);
            }
            shininess = iter->second;
        }
    );
    return box;
}




}
}
}
