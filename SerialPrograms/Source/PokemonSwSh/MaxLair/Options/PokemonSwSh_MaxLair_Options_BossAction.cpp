/*  Max Lair Boss Action
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QVBoxLayout>
#include <QLabel>
#include <QJsonArray>
#include <QJsonObject>
#include <QHeaderView>
#include "Common/Compiler.h"
#include "Common/Qt/QtJsonTools.h"
#include "CommonFramework/Globals.h"
#include "Pokemon/Resources/Pokemon_PokemonNames.h"
#include "Pokemon/Resources/Pokemon_PokeballNames.h"
#include "PokemonSwSh/Resources/PokemonSwSh_PokemonSprites.h"
#include "PokemonSwSh/Resources/PokemonSwSh_MaxLairDatabase.h"
#include "PokemonSwSh_MaxLair_Options_BossAction.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{




const QString BossAction_NAMES[] = {
    "Always stop program.",
    "Stop if shiny.",
};
const std::map<QString, BossAction> BossAction_MAP{
    {BossAction_NAMES[0], BossAction::CATCH_AND_STOP_PROGRAM},
    {BossAction_NAMES[1], BossAction::CATCH_AND_STOP_IF_SHINY},
};



BossActionOption::BossActionOption()
    : m_label("<b>Boss Actions:</b>")
{
    for (const auto& item : all_bosses_by_dex()){
        m_list.emplace_back(BossFilter{item.second});
    }
}

void BossActionOption::load_json(const QJsonValue& json){
    std::map<std::string, BossFilter> map;
    for (const auto& item : json.toArray()){
        QJsonObject obj = item.toObject();
        QString slug;
        json_get_string(slug, obj, "Slug");
        QString action_str;
        BossAction action = BossAction::CATCH_AND_STOP_IF_SHINY;
        json_get_string(action_str, obj, "Action");
        auto iter = BossAction_MAP.find(action_str);
        if (iter != BossAction_MAP.end()){
            action = iter->second;
        }
        QString ball;
        json_get_string(ball, obj, "Ball");

        std::string slug_str = slug.toStdString();
        map.emplace(
            slug_str,
            BossFilter{
                slug_str,
                action,
                ball.toStdString()
            }
        );
    }
    for (BossFilter& filter : m_list){
        auto iter = map.find(filter.slug);
        if (iter == map.end()){
            continue;
        }
        filter = iter->second;
    }
}
QJsonValue BossActionOption::to_json() const{
    QJsonArray array;
    for (const auto& item : m_list){
        QJsonObject obj;
        obj.insert("Slug", QString::fromStdString(item.slug));
        obj.insert("Action", BossAction_NAMES[(size_t)item.action]);
        obj.insert("Ball", QString::fromStdString(item.ball));
        array.append(obj);
    }
    return array;
}

void BossActionOption::restore_defaults(){
    for (auto& item : m_list){
        item.restore_defaults();
    }
}

ConfigWidget* BossActionOption::make_ui(QWidget& parent){
    return new BossActionWidget(parent, *this);
}




BossActionWidget::BossActionWidget(QWidget& parent, BossActionOption& value)
    : QWidget(&parent)
    , ConfigWidget(value, *this)
    , m_value(value)
    , m_table(nullptr)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(new QLabel(m_value.m_label));
    redraw_table();
}
void BossActionWidget::restore_defaults(){
    m_value.restore_defaults();
    update_ui();
}
void BossActionWidget::update_ui(){
    redraw_table();
}
void BossActionWidget::redraw_table(){
    delete m_table;
    m_table = new AutoHeightTableWidget(this);
    layout()->addWidget(m_table);
    m_table->setColumnCount(3);

    QStringList header;
    header << STRING_POKEMON << "Action" << STRING_POKEBALL;
    m_table->setHorizontalHeaderLabels(header);
    QFont font;
    font.setBold(true);
    m_table->horizontalHeader()->setFont(font);
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    size_t rows = m_value.m_list.size();
    m_table->setRowCount((int)rows);
    int stop = (int)m_value.m_list.size();
    for (int c = 0; c < stop; c++){
        BossFilter& filter = m_value.m_list[c];
        const MaxLairSlugs& slugs = get_maxlair_slugs(filter.slug);

        const std::string& sprite_slug = *slugs.sprite_slugs.begin();
        const std::string& name_slug = slugs.name_slug;
        const QIcon& icon = get_pokemon_sprite(sprite_slug).icon();
        const QString& display_name = get_pokemon_name(name_slug).display_name();
        QTableWidgetItem* icon_item = new QTableWidgetItem(icon, display_name);
//        icon_item->setIcon(icon);
        m_table->setItem(c, 0, icon_item);

        BallSelectWidget* ball_select = make_ball_select(*m_table, c, m_value.m_list[c].ball);
        m_table->setCellWidget(c, 1, make_action_box(*m_table, c, *ball_select, m_value.m_list[c].action));
        m_table->setCellWidget(c, 2, ball_select);
    }

    m_table->resizeColumnsToContents();
}


QComboBox* BossActionWidget::make_action_box(QWidget& parent, int row, BallSelectWidget& ball_select, BossAction action){
    QComboBox* box = new NoWheelComboBox(&parent);
    for (const QString& name : BossAction_NAMES){
        box->addItem(name);
    }
    box->setCurrentIndex((int)action);

    switch (action){
    case BossAction::CATCH_AND_STOP_PROGRAM:
    case BossAction::CATCH_AND_STOP_IF_SHINY:
        break;
    }

    connect(
        box, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
        this, [&, row](int index){
            if (index < 0){
                index = 0;
            }
            m_value.m_list[row].action = (BossAction)index;
        }
    );
    return box;
}
BallSelectWidget* BossActionWidget::make_ball_select(QWidget& parent, int row, const std::string& slug){
    using namespace Pokemon;
    BallSelectWidget* box = new BallSelectWidget(parent, POKEBALL_SLUGS(), slug);
    connect(
        box, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
        this, [&, row, box](int index){
            m_value.m_list[row].ball = box->slug();
//            cout << box->slug() << endl;
        }
    );
    return box;
}



}
}
}
}
