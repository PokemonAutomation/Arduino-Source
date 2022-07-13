/*  Max Lair Boss Action
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QVBoxLayout>
#include <QLabel>
#include <QHeaderView>
#include "Common/Compiler.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonArray.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "CommonFramework/Globals.h"
#include "Pokemon/Pokemon_Strings.h"
#include "Pokemon/Resources/Pokemon_PokemonNames.h"
#include "Pokemon/Resources/Pokemon_PokeballNames.h"
#include "PokemonSwSh/Resources/PokemonSwSh_PokemonSprites.h"
#include "PokemonSwSh/Resources/PokemonSwSh_MaxLairDatabase.h"
#include "PokemonSwSh_MaxLair_Options_BossAction.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{




const std::string BossAction_NAMES[] = {
    "Always stop program.",
    "Stop if shiny.",
};
const std::map<std::string, BossAction> BossAction_MAP{
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

void BossActionOption::load_json(const JsonValue& json){
    const JsonArray* array = json.get_array();
    if (array == nullptr){
        return;
    }
    std::map<std::string, BossFilter> map;
    for (const auto& item : *array){
        const JsonObject* obj = item.get_object();
        if (obj == nullptr){
            continue;
        }
        const std::string* slug = obj->get_string("Slug");
        if (slug == nullptr){
            continue;
        }
        const std::string* action_str = obj->get_string("Action");
        if (action_str == nullptr){
            continue;
        }
        const std::string* ball = obj->get_string("Ball");
        if (ball == nullptr){
            continue;
        }
        BossAction action = BossAction::CATCH_AND_STOP_IF_SHINY;
        auto iter = BossAction_MAP.find(*action_str);
        if (iter != BossAction_MAP.end()){
            action = iter->second;
        }
        map.emplace(
            *slug,
            BossFilter{*slug, action, *ball}
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
JsonValue BossActionOption::to_json() const{
    JsonArray array;
    for (const auto& item : m_list){
        JsonObject obj;
        obj["Slug"] = item.slug;
        obj["Action"] = BossAction_NAMES[(size_t)item.action];
        obj["Ball"] = item.ball;
        array.push_back(std::move(obj));
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
    QLabel* label = new QLabel(m_value.m_label, this);
    label->setWordWrap(true);
    layout->addWidget(label);
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
    header << QString::fromStdString(STRING_POKEMON) << "Action" << QString::fromStdString(STRING_POKEBALL);
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
        const QIcon& icon = ALL_POKEMON_SPRITES().get_throw(sprite_slug).icon;
        const std::string& display_name = get_pokemon_name(name_slug).display_name();
        QTableWidgetItem* icon_item = new QTableWidgetItem(icon, QString::fromStdString(display_name));
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
    for (const std::string& name : BossAction_NAMES){
        box->addItem(QString::fromStdString(name));
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
