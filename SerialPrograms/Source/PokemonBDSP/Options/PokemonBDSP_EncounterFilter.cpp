/*  Encounter Filter
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QVBoxLayout>
#include <QLabel>
#include <QHeaderView>
#include <QStandardItemModel>
#include "Common/Cpp/Exception.h"
#include "Common/Qt/QtJsonTools.h"
#include "CommonFramework/Globals.h"
#include "Pokemon/Resources/Pokemon_PokeballNames.h"
#include "Pokemon/Resources/Pokemon_PokemonSlugs.h"
#include "PokemonBDSP_EncounterFilter.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


const QString ShinyFilter_NAMES[] = {
    "Anything",
    "Not Shiny",
    "Shiny",
    "Nothing",
};
const std::map<QString, ShinyFilter> ShinyFilter_MAP{
    {ShinyFilter_NAMES[0], ShinyFilter::ANYTHING},
    {ShinyFilter_NAMES[1], ShinyFilter::NOT_SHINY},
    {ShinyFilter_NAMES[2], ShinyFilter::SHINY},
    {ShinyFilter_NAMES[3], ShinyFilter::NOTHING},
};

const QString EncounterAction_NAMES[] = {
    "Stop Program",
    "Run Away",
    "Throw balls.",
    "Throw balls. Save if caught.",
};
const std::map<QString, EncounterAction> EncounterAction_MAP{
    {EncounterAction_NAMES[0], EncounterAction::StopProgram},
    {EncounterAction_NAMES[1], EncounterAction::RunAway},
    {EncounterAction_NAMES[2], EncounterAction::ThrowBalls},
    {EncounterAction_NAMES[3], EncounterAction::ThrowBallsAndSave},
};


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
QComboBox* EncounterFilterOverride::make_action_box(QWidget& parent){
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
NameSelectWidget* EncounterFilterOverride::make_species_select(QWidget& parent){
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
QComboBox* EncounterFilterOverride::make_shiny_box(QWidget& parent){
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





EncounterFilterOption::EncounterFilterOption(bool enable_overrides, bool allow_autocatch)
    : m_label("<b>Encounter Filter:</b>")
    , m_enable_overrides(enable_overrides)
    , m_allow_autocatch(allow_autocatch)
    , m_shiny_filter_default(ShinyFilter::SHINY)
    , m_shiny_filter_current(m_shiny_filter_default)
    , m_factory(allow_autocatch)
    , m_table(
        "<b>Overrides:</b><br>"
        "The game language must be properly set to read " + STRING_POKEMON + " names. "
        "If multiple overrides apply and are conflicting, the program will stop.",
        m_factory, false
    )
{}
std::vector<EncounterFilterOverride> EncounterFilterOption::overrides() const{
    std::vector<EncounterFilterOverride> ret;
    for (size_t c = 0; c < m_table.size(); c++){
        ret.emplace_back(static_cast<const EncounterFilterOverride&>(m_table[c]));
    }
    return ret;
}
void EncounterFilterOption::load_json(const QJsonValue& json){
    using namespace Pokemon;

    QJsonObject obj = json.toObject();

    QString shiny_filter;
    if (json_get_string(shiny_filter, obj, "ShinyFilter")){
        auto iter = ShinyFilter_MAP.find(shiny_filter);
        if (iter != ShinyFilter_MAP.end()){
            m_shiny_filter_current = iter->second;
        }
    }

    if (m_enable_overrides){
        m_table.load_json(json_get_array_nothrow(obj, "Overrides"));
    }
}
QJsonValue EncounterFilterOption::to_json() const{
    QJsonObject obj;
    obj.insert("ShinyFilter", ShinyFilter_NAMES[(size_t)m_shiny_filter_current]);

    if (m_enable_overrides){
        obj.insert("Overrides", m_table.to_json());
    }

    return obj;
}
void EncounterFilterOption::restore_defaults(){
    m_shiny_filter_current = m_shiny_filter_default;
    m_table.restore_defaults();
}
ConfigOptionUI* EncounterFilterOption::make_ui(QWidget& parent){
    return new EncounterFilterOptionUI(parent, *this);
}


EncounterFilterOptionUI::EncounterFilterOptionUI(QWidget& parent, EncounterFilterOption& value)
    : QWidget(&parent)
    , ConfigOptionUI(value, *this)
    , m_value(value)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
//    QLabel* text = new QLabel(value.label(), this);
//    layout->addWidget(text);

    {
//        QWidget* widget = new QWidget(this);

        QHBoxLayout* hbox = new QHBoxLayout();
        layout->addLayout(hbox);
        hbox->addWidget(new QLabel("<b>Stop on:</b>"));

        m_shininess = new NoWheelComboBox(this);
        hbox->addWidget(m_shininess);
        for (const QString& item : ShinyFilter_NAMES){
            m_shininess->addItem(item);
        }
        ShinyFilter current = m_value.m_shiny_filter_current;
        for (int c = 0; c < m_shininess->count(); c++){
            if (m_shininess->itemText(c) == ShinyFilter_NAMES[(int)current]){
                m_shininess->setCurrentIndex(c);
                break;
            }
        }
        connect(
            m_shininess, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, [=](int index){
                if (index < 0){
                    return;
                }

                QString text = m_shininess->itemText(index);
                auto iter = ShinyFilter_MAP.find(text);
                if (iter == ShinyFilter_MAP.end()){
                    PA_THROW_StringException("Invalid option: " + text);
                }
                m_value.m_shiny_filter_current = iter->second;
            }
        );
    }

    if (m_value.m_enable_overrides){
        layout->addSpacing(5);
        m_table = value.m_table.make_ui(*this);
        layout->addWidget(&m_table->widget());
    }
}
void EncounterFilterOptionUI::restore_defaults(){
    m_value.restore_defaults();
    ShinyFilter current = m_value.m_shiny_filter_current;
    for (int c = 0; c < m_shininess->count(); c++){
        if (m_shininess->itemText(c) == ShinyFilter_NAMES[(int)current]){
            m_shininess->setCurrentIndex(c);
            break;
        }
    }
    if (m_table){
        m_table->restore_defaults();
    }
}



}
}
}
