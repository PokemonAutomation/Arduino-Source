/*  Encounter Filter
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <map>
#include <QVBoxLayout>
#include <QLabel>
#include <QHeaderView>
#include <QStandardItemModel>
#include "Common/Cpp/Exception.h"
#include "Common/Qt/QtJsonTools.h"
#include "Common/Qt/NoWheelComboBox.h"
#include "CommonFramework/Globals.h"
#include "Pokemon/Resources/Pokemon_PokeballNames.h"
#include "Pokemon/Resources/Pokemon_PokemonSlugs.h"
#include "PokemonSwSh_EncounterFilter.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


const QString ShinyFilter_NAMES[] = {
    "Anything",
    "Not Shiny",
    "Any Shiny",
    "Star Shiny",
    "Square Shiny",
    "Nothing",
};
const std::map<QString, ShinyFilter> ShinyFilter_MAP{
    {ShinyFilter_NAMES[0], ShinyFilter::ANYTHING},
    {ShinyFilter_NAMES[1], ShinyFilter::NOT_SHINY},
    {ShinyFilter_NAMES[2], ShinyFilter::ANY_SHINY},
    {ShinyFilter_NAMES[3], ShinyFilter::STAR_ONLY},
    {ShinyFilter_NAMES[4], ShinyFilter::SQUARE_ONLY},
    {ShinyFilter_NAMES[5], ShinyFilter::NOTHING},
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



EncounterFilterOverride::EncounterFilterOverride(bool rare_stars)
    : m_rare_stars(rare_stars)
    , shininess(rare_stars ? ShinyFilter::SQUARE_ONLY : ShinyFilter::STAR_ONLY)
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
QComboBox* EncounterFilterOverride::make_action_box(QWidget& parent, BallSelectWidget& ball_select){
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






EncounterFilterOptionFactory::EncounterFilterOptionFactory(bool rare_stars)
    : m_rare_stars(rare_stars)
{}
QStringList EncounterFilterOptionFactory::make_header() const{
    QStringList list;
    list << "Action" << STRING_POKEBALL << STRING_POKEMON << "Shininess";
    return list;
}
std::unique_ptr<EditableTableRow> EncounterFilterOptionFactory::make_row() const{
    return std::unique_ptr<EditableTableRow>(new EncounterFilterOverride(m_rare_stars));
}




EncounterFilterOption::EncounterFilterOption(bool rare_stars, bool enable_overrides)
    : m_label("<b>Encounter Filter:</b>")
    , m_rare_stars(rare_stars)
    , m_enable_overrides(enable_overrides)
    , m_shiny_filter_default(ShinyFilter::ANY_SHINY)
    , m_shiny_filter_current(m_shiny_filter_default)
    , m_factory(rare_stars)
    , m_table(
        rare_stars
            ?   "<b>Overrides:</b><br>"
                "The game language must be properly set to read " + STRING_POKEMON + " names. "
                "If more than one override applies, the last one will be chosen.<br>"
                "<font color=\"red\">Due to the extreme rarity of star shinies (1 in 6 million), "
                "the filters here will not allow you to run from them. "
                "If you get a star shiny, catch it and cherish it.</font>"
            :
                "<b>Overrides:</b><br>"
                "The game language must be properly set to read " + STRING_POKEMON + " names.<br>"
                "If more than one override applies, the last one will be chosen.",
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
        if (m_value.m_rare_stars){
            m_shininess->addItem(ShinyFilter_NAMES[(int)ShinyFilter::ANYTHING]);
            m_shininess->addItem(ShinyFilter_NAMES[(int)ShinyFilter::ANY_SHINY]);
            m_shininess->addItem(ShinyFilter_NAMES[(int)ShinyFilter::STAR_ONLY]);
        }else{
            m_shininess->addItem(ShinyFilter_NAMES[(int)ShinyFilter::ANYTHING]);
            m_shininess->addItem(ShinyFilter_NAMES[(int)ShinyFilter::NOT_SHINY]);
            m_shininess->addItem(ShinyFilter_NAMES[(int)ShinyFilter::ANY_SHINY]);
            m_shininess->addItem(ShinyFilter_NAMES[(int)ShinyFilter::STAR_ONLY]);
            m_shininess->addItem(ShinyFilter_NAMES[(int)ShinyFilter::SQUARE_ONLY]);
            m_shininess->addItem(ShinyFilter_NAMES[(int)ShinyFilter::NOTHING]);
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
        layout->addWidget(m_table->widget());
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
