/*  Encounter Filter
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QtGlobal>
#include "Common/Cpp/Exception.h"
#include "Common/Qt/QtJsonTools.h"
#include "CommonFramework/Globals.h"
#include "Pokemon/Resources/Pokemon_PokeballNames.h"
#include "Pokemon/Resources/Pokemon_PokemonSlugs.h"
#include "Pokemon/Options/Pokemon_BallSelectWidget.h"
#include "Pokemon/Options/Pokemon_NameSelectWidget.h"
#include "PokemonBDSP_EncounterFilterEnums.h"
#include "PokemonBDSP_EncounterFilterOverride.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{



bool EncounterActionFull::operator==(const EncounterActionFull& x) const{
    switch (action){
    case EncounterAction::StopProgram:
    case EncounterAction::RunAway:
        return action == x.action;
    case EncounterAction::ThrowBalls:
    case EncounterAction::ThrowBallsAndSave:
        return action == x.action && pokeball_slug == x.pokeball_slug;
    }
}
bool EncounterActionFull::operator!=(const EncounterActionFull& x) const{
    return !(*this == x);
}
QString EncounterActionFull::to_str() const{
    QString str;
    str += EncounterAction_NAMES[(size_t)action];
    if (action == EncounterAction::ThrowBalls || action == EncounterAction::ThrowBallsAndSave){
        str += " (";
        str += get_pokeball_name(pokeball_slug).display_name();
        str += ")";
    }
    return str;
}



EncounterFilterOverride::EncounterFilterOverride(bool allow_autocatch)
    : m_allow_autocatch(allow_autocatch)
    , m_ball_select(nullptr)
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
        if (!m_allow_autocatch && action >= EncounterAction::ThrowBalls){
            action = EncounterAction::StopProgram;
        }
    }
    if (m_allow_autocatch){
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
    if (m_allow_autocatch){
        obj.insert("Ball", QString::fromStdString(pokeball_slug));
    }
    obj.insert("Species", QString::fromStdString(pokemon_slug));
    obj.insert("ShinyFilter", ShinyFilter_NAMES[(size_t)shininess]);
    return obj;
}
std::unique_ptr<EditableTableRow> EncounterFilterOverride::clone() const{
    return std::unique_ptr<EditableTableRow>(new EncounterFilterOverride(*this));
}
std::vector<QWidget*> EncounterFilterOverride::make_widgets(QWidget& parent){
    std::vector<QWidget*> widgets;
    if (m_allow_autocatch){
        m_ball_select = make_ball_select(parent);
        widgets.emplace_back(make_action_box(parent));
        widgets.emplace_back(m_ball_select);
    }else{
        widgets.emplace_back(make_action_box(parent));
    }
    widgets.emplace_back(make_species_select(parent));
    widgets.emplace_back(make_shiny_box(parent));
    return widgets;
}
void EncounterFilterOverride::update_ball_select(){
    if (!m_allow_autocatch){
        return;
    }
    switch (action){
    case EncounterAction::StopProgram:
    case EncounterAction::RunAway:
        m_ball_select->setEnabled(false);
        break;
    case EncounterAction::ThrowBalls:
    case EncounterAction::ThrowBallsAndSave:
        m_ball_select->setEnabled(true);
        break;
    }
}
QWidget* EncounterFilterOverride::make_action_box(QWidget& parent){
    QComboBox* box = new NoWheelComboBox(&parent);
    box->addItem(EncounterAction_NAMES[0]);
    box->addItem(EncounterAction_NAMES[1]);
    if (m_allow_autocatch){
        box->addItem(EncounterAction_NAMES[2]);
        box->addItem(EncounterAction_NAMES[3]);
    }
    box->setCurrentIndex((int)action);

    if (m_allow_autocatch){
        switch (action){
        case EncounterAction::StopProgram:
        case EncounterAction::RunAway:
            m_ball_select->setEnabled(false);
            break;
        case EncounterAction::ThrowBalls:
        case EncounterAction::ThrowBallsAndSave:
            m_ball_select->setEnabled(true);
            break;
        }
    }

    box->connect(
        box, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
        box, [&](int index){
            if (index < 0){
                index = 0;
            }
            action = (EncounterAction)index;
            if (m_allow_autocatch){
                switch ((EncounterAction)index){
                case EncounterAction::StopProgram:
                case EncounterAction::RunAway:
                    m_ball_select->setEnabled(false);
                    break;
                case EncounterAction::ThrowBalls:
                case EncounterAction::ThrowBallsAndSave:
                    m_ball_select->setEnabled(true);
                    break;
                }
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
    NameSelectWidget* box = new NameSelectWidget(parent, NATIONAL_DEX_SLUGS(), pokemon_slug);
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
    for (const QString& action : ShinyFilter_NAMES){
        box->addItem(action);
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



EncounterFilterOptionFactory::EncounterFilterOptionFactory(bool allow_autocatch)
    : m_allow_autocatch(allow_autocatch)
{}
QStringList EncounterFilterOptionFactory::make_header() const{
    QStringList list;
    if (m_allow_autocatch){
        list << "Action" << STRING_POKEBALL << STRING_POKEMON << "Shininess";
    }else{
        list << "Action" << STRING_POKEMON << "Shininess";
    }
    return list;
}
std::unique_ptr<EditableTableRow> EncounterFilterOptionFactory::make_row() const{
    return std::unique_ptr<EditableTableRow>(new EncounterFilterOverride(m_allow_autocatch));
}




}
}
}
