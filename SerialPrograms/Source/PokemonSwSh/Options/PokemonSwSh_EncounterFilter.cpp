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



void EncounterFilterOverrides::load_json(const QJsonValue& json){
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
        pokemon_slug = value.toUtf8().data();
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
QJsonValue EncounterFilterOverrides::to_json() const{
    QJsonObject obj;
    obj.insert("Action", EncounterAction_NAMES[(size_t)action]);
    obj.insert("Ball", QString::fromStdString(pokeball_slug));
    obj.insert("Species", QString::fromStdString(pokemon_slug));
    obj.insert("ShinyFilter", ShinyFilter_NAMES[(size_t)shininess]);
    return obj;
}



EncounterFilterOption::EncounterFilterOption(bool rare_stars, bool enable_overrides)
    : m_label("<b>Encounter Filter:</b>")
    , m_rare_stars(rare_stars)
    , m_enable_overrides(enable_overrides)
    , m_shiny_filter_default(ShinyFilter::ANY_SHINY)
    , m_shiny_filter_current(m_shiny_filter_default)
//    , m_blacklist_default(true)
//    , m_blacklist_current(m_blacklist_default)
{}

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
        QJsonArray species_list = json_get_array_nothrow(obj, "Overrides");
        m_overrides.clear();
        for (const auto& item : species_list){
            m_overrides.emplace_back(m_rare_stars);
            m_overrides.back().load_json(item);
        }
    }

}
QJsonValue EncounterFilterOption::to_json() const{
    QJsonObject obj;
    obj.insert("ShinyFilter", ShinyFilter_NAMES[(size_t)m_shiny_filter_current]);

    if (m_enable_overrides){
        QJsonArray overrides;
        for (const EncounterFilterOverrides& item : m_overrides){
            overrides.append(item.to_json());
        }
        obj.insert("Overrides", overrides);
    }

    return obj;
}

void EncounterFilterOption::restore_defaults(){
    m_shiny_filter_current = m_shiny_filter_default;
    m_overrides.clear();
}
ConfigOptionUI* EncounterFilterOption::make_ui(QWidget& parent){
    return new EncounterFilterOptionUI(parent, *this);
}




EncounterFilterOptionUI::EncounterFilterOptionUI(QWidget& parent, EncounterFilterOption& value)
    : QWidget(&parent)
    , m_value(value)
    , m_table(nullptr)
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

//        layout->addWidget(widget);
    }

    if (m_value.m_enable_overrides){
        layout->addSpacing(5);
        if (m_value.m_rare_stars){
            layout->addWidget(new QLabel(
                "<b>Overrides:</b><br>"
                "The game language must be properly set to read " + STRING_POKEMON + " names. "
                "If more than one override applies, the last one will be chosen.<br>"
                "<font color=\"red\">Due to the extreme rarity of star shinies (1 in 6 million), "
                "the filters here will not allow you to run from them. "
                "If you get a star shiny, catch it and cherish it.</font>"
            ));
        }else{
            layout->addWidget(new QLabel(
                "<b>Overrides:</b><br>"
                "The game language must be properly set to read " + STRING_POKEMON + " names.<br>"
                "If more than one override applies, the last one will be chosen."
            ));
        }
        replace_table();
    }


}
void EncounterFilterOptionUI::replace_table(){
    if (!m_value.m_enable_overrides){
        return;
    }
    QVBoxLayout* layout = static_cast<QVBoxLayout*>(this->layout());
    if (m_table != nullptr){
        layout->removeWidget(m_table);
        delete m_table;
        m_index_table.clear();
        m_table = nullptr;
    }

    m_table = new QTableWidget(this);
    layout->addWidget(m_table);
    m_table->setColumnCount(6);

    QStringList header;
    header << "Action" << STRING_POKEBALL << STRING_POKEMON << "Shininess" << "" << "";
    m_table->setHorizontalHeaderLabels(header);
    QFont font;
    font.setBold(true);
    m_table->horizontalHeader()->setFont(font);
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    size_t rows = m_value.m_overrides.size();
    m_table->setRowCount((int)(m_value.m_overrides.size() + 1));
    for (size_t c = 0; c < rows; c++){
        const auto& item = m_value.m_overrides[c];
        m_index_table.emplace_back(new int);
        add_row((int)c, item, *m_index_table.back());
    }

    QPushButton* button = new QPushButton(m_table);
    button->setText("Add Row");
    m_table->setCellWidget((int)m_value.m_overrides.size(), 0, button);
    connect(
        button, &QPushButton::clicked,
        this, [&](bool){
            int index = (int)m_index_table.size();

            //  Update data vector.
            m_value.m_overrides.emplace_back(m_value.m_rare_stars);
            const auto& item = m_value.m_overrides.back();

            //  Update index vector.
            m_index_table.emplace_back(new int);

            //  Update UI.
            m_table->insertRow(index);
            add_row(index, item, *m_index_table.back());
        }
    );

}
void EncounterFilterOptionUI::add_row(int row, const EncounterFilterOverrides& game, int& index_ref){
    index_ref = row;
    BallSelectWidget* ball_select = make_ball_select(*m_table, index_ref, m_value.m_overrides[row].pokeball_slug);
    m_table->setCellWidget(row, 0, make_action_box(*m_table, index_ref, *ball_select, m_value.m_overrides[row].action));
    m_table->setCellWidget(row, 1, ball_select);
    m_table->setCellWidget(row, 2, make_species_select(*m_table, index_ref, m_value.m_overrides[row].pokemon_slug));
    m_table->setCellWidget(row, 3, make_shiny_box(*m_table, index_ref, m_value.m_overrides[row].shininess));
    m_table->setCellWidget(row, 4, make_insert_button(*m_table, index_ref));
    m_table->setCellWidget(row, 5, make_remove_button(*m_table, index_ref));
    m_table->resizeColumnsToContents();
}
QComboBox* EncounterFilterOptionUI::make_action_box(QWidget& parent, int& row, BallSelectWidget& ball_select, EncounterAction action){
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

    connect(
        box, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
        this, [&](int index){
            if (index < 0){
                index = 0;
            }
            m_value.m_overrides[row].action = (EncounterAction)index;
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
BallSelectWidget* EncounterFilterOptionUI::make_ball_select(QWidget& parent, int& row, const std::string& slug){
    using namespace Pokemon;
    BallSelectWidget* box = new BallSelectWidget(parent, POKEBALL_SLUGS(), slug);
    connect(
        box, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
        this, [&, box](int index){
            m_value.m_overrides[row].pokeball_slug = box->slug();
//            cout << box->slug() << endl;
        }
    );
    return box;
}
NameSelectWidget* EncounterFilterOptionUI::make_species_select(QWidget& parent, int& row, const std::string& slug){
    using namespace Pokemon;
    NameSelectWidget* box = new NameSelectWidget(parent, NATIONAL_DEX_SLUGS(), slug);
    connect(
        box, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
        this, [&, box](int index){
            m_value.m_overrides[row].pokemon_slug = box->slug();
//            cout << box->slug() << endl;
        }
    );
    return box;
}
QComboBox* EncounterFilterOptionUI::make_shiny_box(QWidget& parent, int& row, ShinyFilter shiniess){
    QComboBox* box = new NoWheelComboBox(&parent);
    if (m_value.m_rare_stars){
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
    ShinyFilter current = m_value.m_overrides[row].shininess;
    for (int c = 0; c < box->count(); c++){
        if (box->itemText(c) == ShinyFilter_NAMES[(int)current]){
            box->setCurrentIndex(c);
            break;
        }
    }
    connect(
        box, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
        this, [&, box](int index){
            if (index < 0){
                return;
            }
            QString text = box->itemText(index);
            auto iter = ShinyFilter_MAP.find(text);
            if (iter == ShinyFilter_MAP.end()){
                PA_THROW_StringException("Invalid option: " + text);
            }
            m_value.m_overrides[row].shininess = iter->second;
        }
    );
    return box;
}
QPushButton* EncounterFilterOptionUI::make_insert_button(QWidget& parent, int& row){
    QPushButton* button = new QPushButton(&parent);
    QFont font;
    font.setBold(true);
    button->setFont(font);
    button->setText("Insert");
    button->setMaximumWidth(60);
    connect(
        button, &QPushButton::clicked,
        this, [&](bool){
            int index = row;
            EncounterFilterOverrides item(m_value.m_rare_stars);

            //  Update data vector.
            m_value.m_overrides.insert(m_value.m_overrides.begin() + index, item);

            //  Update index vector.
            m_index_table.insert(m_index_table.begin() + index, std::unique_ptr<int>(new int(index)));
            int& new_row = *m_index_table[index];
            for (size_t c = 0; c < m_index_table.size(); c++){
                *m_index_table[c] = (int)c;
            }

            //  Update UI.
            m_table->insertRow(index);
            add_row(index, item, new_row);
        }
    );
    return button;
}
QPushButton* EncounterFilterOptionUI::make_remove_button(QWidget& parent, int& row){
    QPushButton* button = new QPushButton(&parent);
    QFont font;
    font.setBold(true);
    button->setFont(font);
    button->setText("Delete");
    button->setMaximumWidth(60);
    connect(
        button, &QPushButton::clicked,
        this, [&](bool){
            int index = row;

            //  Update data vector.
            m_index_table.erase(m_index_table.begin() + index);

            //  Update index vector.
            for (size_t c = 0; c < m_index_table.size(); c++){
                *m_index_table[c] = (int)c;
            }
            m_value.m_overrides.erase(m_value.m_overrides.begin() + index);

            //  Update UI.
            m_table->removeRow(index);
        }
    );
    return button;
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
//    m_shininess->setCurrentIndex((int)m_value.m_shiny_filter_default);
    replace_table();
}


















}
}
}
