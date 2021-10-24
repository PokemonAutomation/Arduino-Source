/*  Max Lair Options
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QJsonArray>
#include <QJsonObject>
#include <QHeaderView>
#include "Common/Qt/QtJsonTools.h"
#include "CommonFramework/Globals.h"
#include "Pokemon/Resources/Pokemon_PokemonNames.h"
#include "Pokemon/Resources/Pokemon_PokeballNames.h"
#include "PokemonSwSh/Resources/PokemonSwSh_PokemonSprites.h"
#include "PokemonSwSh/Resources/PokemonSwSh_MaxLairDatabase.h"
#include "PokemonSwSh_MaxLair_Options.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{



const QString MODULE_NAME = "Max Lair";
const std::chrono::milliseconds INFERENCE_RATE = std::chrono::milliseconds(200);



MaxLairConsoleOptions::MaxLairConsoleOptions(
    QString label,
    const LanguageSet& languages,
    bool enable_boss_ball
)
    : GroupOption(std::move(label))
    , language("<b>Game Language:</b>", languages, true)
    , normal_ball("<b>Normal Ball:</b> Ball for catching non-boss " + STRING_POKEMON + ".", "poke-ball")
    , boss_ball("<b>Boss Ball:</b> Ball for catching the boss " + STRING_POKEMON + ".", "poke-ball")
{
    PA_ADD_OPTION(language);
    PA_ADD_OPTION(normal_ball);
    if (enable_boss_ball){
        PA_ADD_OPTION(boss_ball);
    }
}

HostingSettings::HostingSettings()
    : GroupOption("Hosting Options")
    , MODE(
        "<b>Mode:</b>",
        {
            "Alone: Run by yourself using only Switches controlled by this program.",
            "Host Locally: Allow other local Switches to join.",
            "Host Online: Allow other people online to join.",
        }
        , 0
    )
    , RAID_CODE(
        "<b>Raid Code:</b><br>Required if using multiple Switches. "
        "Random code is strongly recommended when hosting to ensure your own Switches get in.",
        4, ""
    )
    , LOBBY_WAIT_DELAY(
        "<b>Lobby Wait Delay:</b><br>Wait this long before starting raid. Start time is 3 minutes minus this number.",
        "60 * TICKS_PER_SECOND"
    )
    , CONNECT_TO_INTERNET_DELAY(
        "<b>Connect to Internet Delay:</b><br>Time from \"Connect to Internet\" to when you're ready to start adventure.",
        "20 * TICKS_PER_SECOND"
    )
    , NOTIFICATIONS("Live-Hosting Announcements", true)
{
    PA_ADD_OPTION(MODE);
    PA_ADD_OPTION(RAID_CODE);
    PA_ADD_OPTION(LOBBY_WAIT_DELAY);
    PA_ADD_OPTION(CONNECT_TO_INTERNET_DELAY);
    PA_ADD_OPTION(NOTIFICATIONS);
}







const QString BossAction_NAMES[] = {
    "Catch and always stop program.",
    "Catch and stop if shiny.",
    "Do not catch and continue running.",
};
const std::map<QString, BossAction> BossAction_MAP{
    {BossAction_NAMES[0], BossAction::CATCH_AND_STOP_PROGRAM},
    {BossAction_NAMES[1], BossAction::CATCH_AND_STOP_IF_SHINY},
    {BossAction_NAMES[2], BossAction::DO_NOT_CATCH_AND_CONTINUE},
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
        json_get_string(action_str,obj, "Action");
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

ConfigOptionUI* BossActionOption::make_ui(QWidget& parent){
    return new BossActionOptionUI(parent, *this);
}




BossActionOptionUI::BossActionOptionUI(QWidget& parent, BossActionOption& value)
    : QWidget(&parent)
    , m_value(value)
    , m_table(nullptr)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel(m_value.m_label));

    m_table = new AutoHeightTableWidget(this);
    layout->addWidget(m_table);
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
        icon_item->setIcon(icon);
        m_table->setItem(c, 0, icon_item);

        BallSelectWidget* ball_select = make_ball_select(*m_table, c, m_value.m_list[c].ball);
        m_table->setCellWidget(c, 1, make_action_box(*m_table, c, *ball_select, m_value.m_list[c].action));
        m_table->setCellWidget(c, 2, ball_select);
    }

    m_table->resizeColumnsToContents();

}
void BossActionOptionUI::restore_defaults(){
    m_value.restore_defaults();
}


QComboBox* BossActionOptionUI::make_action_box(QWidget& parent, int row, BallSelectWidget& ball_select, BossAction action){
    QComboBox* box = new NoWheelComboBox(&parent);
    for (const QString& name : BossAction_NAMES){
        box->addItem(name);
    }
    box->setCurrentIndex((int)action);

    switch (action){
    case BossAction::CATCH_AND_STOP_PROGRAM:
    case BossAction::CATCH_AND_STOP_IF_SHINY:
        ball_select.setEnabled(true);
        break;
    case BossAction::DO_NOT_CATCH_AND_CONTINUE:
        ball_select.setEnabled(false);
        break;
    }

    connect(
        box, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
        this, [&, row](int index){
            if (index < 0){
                index = 0;
            }
            m_value.m_list[row].action = (BossAction)index;
            switch ((BossAction)index){
            case BossAction::CATCH_AND_STOP_PROGRAM:
            case BossAction::CATCH_AND_STOP_IF_SHINY:
                ball_select.setEnabled(true);
                break;
            case BossAction::DO_NOT_CATCH_AND_CONTINUE:
                ball_select.setEnabled(false);
                break;
            }
        }
    );
    return box;
}
BallSelectWidget* BossActionOptionUI::make_ball_select(QWidget& parent, int row, const std::string& slug){
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
