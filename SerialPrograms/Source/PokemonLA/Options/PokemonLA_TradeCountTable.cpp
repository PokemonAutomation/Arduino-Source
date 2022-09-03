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
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonArray.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "CommonFramework/Globals.h"
#include "Pokemon/Pokemon_Strings.h"
#include "Pokemon/Resources/Pokemon_PokemonNames.h"
#include "PokemonLA/Resources/PokemonLA_AvailablePokemon.h"
#include "PokemonLA/Resources/PokemonLA_PokemonSprites.h"
#include "PokemonLA_TradeCountTable.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{
    using namespace Pokemon;




std::map<std::string, int> make_research_catch_count_map(){
    std::string path = RESOURCE_PATH() + "PokemonLA/ResearchMaxCatches.json";
    JsonValue json = load_json_file(path);
    JsonObject& root = json.get_object_throw(path);

    std::map<std::string, int> map;
    for (auto& item : root){
        map.emplace(item.first, item.second.get_integer_throw(path));
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
void TradeCountTableOption::load_json(const JsonValue& json){
    const JsonArray* array = json.get_array();
    if (array == nullptr){
        return;
    }
    std::map<std::string, int> map;
    for (const auto& item : *array){
        const JsonObject* obj = item.get_object();
        if (obj == nullptr){
            continue;
        }
        const std::string* slug = obj->get_string("Slug");
        if (slug == nullptr){
            continue;
        }
        int count;
        if (!obj->read_integer(count, "Count", 0, 25)){
            continue;
        }
        map.emplace(*slug, count);
    }
    for (std::pair<std::string, int>& filter : m_list){
        auto iter = map.find(filter.first);
        if (iter == map.end()){
            continue;
        }
        filter.second = iter->second;
    }
}
JsonValue TradeCountTableOption::to_json() const{
    JsonArray array;
    for (const auto& item : m_list){
        JsonObject obj;
        obj["Slug"] = item.first;
        obj["Count"] = item.second;
        array.push_back(std::move(obj));
    }
    return array;
}
void TradeCountTableOption::restore_defaults(){
    for (auto& item : m_list){
        item.second = research_catch_count(item.first);
    }
}
ConfigWidget* TradeCountTableOption::make_ui(QWidget& parent){
    return new TradeCountTableWidget(parent, *this);
}





TradeCountTableWidget::TradeCountTableWidget(QWidget& parent, TradeCountTableOption& value)
    : QWidget(&parent)
    , ConfigWidget(value, *this)
    , m_value(value)
    , m_table(nullptr)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    QLabel* label = new QLabel(QString::fromStdString(m_value.m_label), this);
    label->setWordWrap(true);
    layout->addWidget(label);

    m_table = new AutoHeightTableWidget(this);
    layout->addWidget(m_table);
    m_table->setColumnCount(3);

    QStringList header;
    header << QString::fromStdString(STRING_POKEMON) << "Trades" << "Default";
    m_table->setHorizontalHeaderLabels(header);
    QFont font;
    font.setBold(true);
    m_table->horizontalHeader()->setFont(font);
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    const SpriteDatabase& ICONS = ALL_POKEMON_SPRITES();

    size_t rows = m_value.m_list.size();
    m_table->setRowCount((int)rows);
    int stop = (int)m_value.m_list.size();
    for (int c = 0; c < stop; c++){
        std::pair<std::string, int>& entry = m_value.m_list[c];
        const std::string& display_name = Pokemon::get_pokemon_name(entry.first).display_name();
        QTableWidgetItem* icon_item = new QTableWidgetItem(QString::fromStdString(display_name));

        const SpriteDatabase::Sprite* sprite = ICONS.get_nothrow(entry.first);
        if (sprite != nullptr){
            QPixmap pixmap = QPixmap::fromImage(sprite->icon.to_QImage_ref());
            icon_item->setIcon(pixmap);
        }

        m_table->setItem(c, 0, icon_item);
        m_table->setCellWidget(c, 1, make_count_box(*m_table, c, entry.second));
        QLabel* research_catch_count_label = new QLabel(QString::number(research_catch_count(entry.first)), m_table);
        research_catch_count_label->setAlignment(Qt::AlignCenter);
        m_table->setCellWidget(c, 2, research_catch_count_label);
    }

    m_table->resizeColumnsToContents();

}
void TradeCountTableWidget::update(){
    ConfigWidget::update();
    for (size_t c = 0; c < m_value.m_list.size(); c++){
        m_entries[c]->setText(QString::number(m_value.m_list[0].second));
    }
}
QLineEdit* TradeCountTableWidget::make_count_box(QWidget& parent, int row, int count){
    QLineEdit* box = new QLineEdit(&parent);
    QIntValidator* validator = new QIntValidator(0, 25, box);
    box->setValidator(validator);
    box->setText(QString::number(count));
    box->setMaxLength(2);
    box->setMaximumWidth(50);
    box->setAlignment(Qt::AlignHCenter);
    box->connect(
        box, &QLineEdit::textChanged,
        box, [this, box, row](const QString& text){
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
