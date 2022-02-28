/*  Trade Count Table
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QHeaderView>
#include "Common/Cpp/Exceptions.h"
#include "Common/Qt/QtJsonTools.h"
#include "CommonFramework/Globals.h"
#include "Pokemon/Resources/Pokemon_PokemonNames.h"
#include "PokemonLA/Resources/PokemonLA_AvailablePokemon.h"
#include "PokemonLA/Resources/PokemonLA_PokemonIcons.h"
#include "PokemonLA_TradeCountTable.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{




std::map<std::string, int> make_research_catch_count_map(){
    QJsonObject json = read_json_file(
        RESOURCE_PATH() + "PokemonLA/ResearchMaxCatches.json"
    ).object();

    std::map<std::string, int> map;
    for (auto iter = json.begin(); iter != json.end(); ++iter){
        map.emplace(iter.key().toStdString(), iter.value().toInt());
    }

    return map;
}
int research_catch_count(const std::string& slug){
    static const std::map<std::string, int> database = make_research_catch_count_map();
    auto iter = database.find(slug);
    if (iter == database.end()){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "No research count for: " + slug);
    }
    return iter->second;
}



TradeCountTableOption::TradeCountTableOption()
    : m_label(
        "<b>Trade Counts:</b>"
        "<br>Trade each " + STRING_POKEMON + " of this species this many times.<br>"
        "The defaults here are the # of catches needed to max out research. "
        "Maxing out catches is sufficient to reach level 10 for everything except Unown, Spritomb, and legendaries. "
        "Note that gen1 trade evolutions cannot be touch traded. The program will skip them. This applies to Kadabra, Haunter, Graveler, and Machoke."
    )
{
    for (const std::string& slug : HISUI_DEX_SLUGS()){
        m_list.emplace_back(slug, research_catch_count(slug));
    }
}
void TradeCountTableOption::load_json(const QJsonValue& json){
    std::map<std::string, int> map;
    for (const auto& item : json.toArray()){
        QJsonObject obj = item.toObject();
        QString slug;
        json_get_string(slug, obj, "Slug");
        int count;
        json_get_int(count, obj, "Count", 0, 25);

        std::string slug_str = slug.toStdString();
        map.emplace(slug_str, count);
    }
    for (std::pair<std::string, int>& filter : m_list){
        auto iter = map.find(filter.first);
        if (iter == map.end()){
            continue;
        }
        filter.second = iter->second;
    }
}
QJsonValue TradeCountTableOption::to_json() const{
    QJsonArray array;
    for (const auto& item : m_list){
        QJsonObject obj;
        obj.insert("Slug", QString::fromStdString(item.first));
        obj.insert("Count", item.second);
        array.append(obj);
    }
    return array;
}
void TradeCountTableOption::restore_defaults(){
    for (auto& item : m_list){
        item.second = research_catch_count(item.first);
    }
}
ConfigWidget* TradeCountTableOption::make_ui(QWidget& parent){
    return new TradeCountTableOptionUI(parent, *this);
}





TradeCountTableOptionUI::TradeCountTableOptionUI(QWidget& parent, TradeCountTableOption& value)
    : QWidget(&parent)
    , ConfigWidget(value, *this)
    , m_value(value)
    , m_table(nullptr)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel(m_value.m_label));

    m_table = new AutoHeightTableWidget(this);
    layout->addWidget(m_table);
    m_table->setColumnCount(3);

    QStringList header;
    header << STRING_POKEMON << "Trades" << "Default";
    m_table->setHorizontalHeaderLabels(header);
    QFont font;
    font.setBold(true);
    m_table->horizontalHeader()->setFont(font);
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    const std::map<std::string, QIcon>& ICONS = ALL_POKEMON_ICONS();

    size_t rows = m_value.m_list.size();
    m_table->setRowCount((int)rows);
    int stop = (int)m_value.m_list.size();
    for (int c = 0; c < stop; c++){
        std::pair<std::string, int>& entry = m_value.m_list[c];
        const QString& display_name = Pokemon::get_pokemon_name(entry.first).display_name();
        QTableWidgetItem* icon_item = new QTableWidgetItem(display_name);

        auto iter = ICONS.find(entry.first);
        if (iter != ICONS.end()){
            icon_item->setIcon(iter->second);
        }

        m_table->setItem(c, 0, icon_item);
        m_table->setCellWidget(c, 1, make_count_box(*m_table, c, entry.second));
        QLabel* label = new QLabel(QString::number(research_catch_count(entry.first)), m_table);
        label->setAlignment(Qt::AlignCenter);
        m_table->setCellWidget(c, 2, label);
    }

    m_table->resizeColumnsToContents();

}
void TradeCountTableOptionUI::restore_defaults(){
    m_value.restore_defaults();
    for (size_t c = 0; c < m_value.m_list.size(); c++){
        m_entries[c]->setText(QString::number(m_value.m_list[0].second));
    }
}
QLineEdit* TradeCountTableOptionUI::make_count_box(QWidget& parent, int row, int count){
    QLineEdit* box = new QLineEdit(&parent);
    QIntValidator* validator = new QIntValidator(0, 25, box);
    box->setValidator(validator);
    box->setText(QString::number(count));
    box->setMaxLength(2);
    box->setMaximumWidth(50);
    box->setAlignment(Qt::AlignHCenter);
    box->connect(
        box, &QLineEdit::textChanged,
        box, [=](const QString& text){
            int raw = text.toInt();
            int fixed = raw;
            fixed = std::max(fixed, 0);
            fixed = std::min(fixed, 25);
            if (raw != fixed){
                box->setText(QString::number(fixed));
            }
            m_value.m_list[row].second = fixed;
        }
    );
    m_entries.emplace_back(box);
    return box;
}




}
}
}
