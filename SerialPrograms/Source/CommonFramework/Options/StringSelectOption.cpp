/*  String Select
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QCompleter>
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Qt/NoWheelComboBox.h"
#include "Common/Qt/Options/ConfigWidget.h"
#include "StringSelectOption.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{



class StringSelectWidget : public QWidget, public ConfigWidget{
public:
    StringSelectWidget(QWidget& parent, StringSelectOption& value);

    virtual void update() override;

private:
    StringSelectOption& m_value;
    NoWheelComboBox* m_box;
//    bool m_updating = false;
};





StringSelectOption::StringSelectOption(
    std::string label,
    const std::vector<std::string>& cases,
    const std::string& default_case
)
    : m_label(std::move(label))
//    , m_case_list(std::move(cases))
    , m_default(0)
    , m_current(0)
{
    for (size_t index = 0; index < cases.size(); index++){
        const std::string& item = cases[index];
        if (item == default_case){
            m_default = index;
        }
        m_case_list.emplace_back(item, QIcon());
        auto ret = m_case_map.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(item),
            std::forward_as_tuple(index)
        );
        if (!ret.second){
            throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Duplicate enum label.");
        }
    }
    m_current.store(m_default, std::memory_order_relaxed);
}
StringSelectOption::StringSelectOption(
    std::string label,
    std::vector<std::pair<std::string, QIcon>> cases,
    const std::string& default_case
)
    : m_label(std::move(label))
    , m_case_list(std::move(cases))
    , m_default(0)
    , m_current(0)
{
    for (size_t index = 0; index < m_case_list.size(); index++){
        const std::string& item = m_case_list[index].first;
        if (item == default_case){
            m_default = index;
        }
        auto ret = m_case_map.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(item),
            std::forward_as_tuple(index)
        );
        if (!ret.second){
            throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Duplicate enum label.");
        }
    }
    m_current.store(m_default, std::memory_order_relaxed);
}

void StringSelectOption::load_json(const JsonValue& json){
    const std::string* str = json.get_string();
    if (str == nullptr){
        return;
    }
    auto iter = m_case_map.find(*str);
    if (iter != m_case_map.end()){
        m_current.store(iter->second, std::memory_order_relaxed);
    }
}
JsonValue StringSelectOption::to_json() const{
    return m_case_list[(size_t)*this].first;
}

void StringSelectOption::restore_defaults(){
    m_current.store(m_default, std::memory_order_relaxed);
}

ConfigWidget* StringSelectOption::make_ui(QWidget& parent){
    return new StringSelectWidget(parent, *this);
}



StringSelectWidget::StringSelectWidget(QWidget& parent, StringSelectOption& value)
    : QWidget(&parent)
    , ConfigWidget(value, *this)
    , m_value(value)
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    QLabel* text = new QLabel(QString::fromStdString(m_value.m_label), this);
    text->setWordWrap(true);
    layout->addWidget(text, 1);
    m_box = new NoWheelComboBox(&parent);

    m_box->setEditable(true);
    m_box->setInsertPolicy(QComboBox::NoInsert);
    m_box->completer()->setCompletionMode(QCompleter::PopupCompletion);
    m_box->completer()->setFilterMode(Qt::MatchContains);

    for (const auto& item : m_value.m_case_list){
        m_box->addItem(item.second, QString::fromStdString(item.first));
    }
    m_box->setCurrentIndex((int)(size_t)m_value);
    m_box->update_size_cache();

    layout->addWidget(m_box, 1);

    connect(
        m_box, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
        this, [=](int index){
            if (index < 0){
                m_value.restore_defaults();
                return;
            }
            m_value.m_current.store(index, std::memory_order_relaxed);
//            cout << "index = " << index << endl;
        }
    );
}


void StringSelectWidget::update(){
    ConfigWidget::update();
    m_box->setCurrentIndex((int)(size_t)m_value);
}











size_t StringSelectDatabase::search_index_by_slug(const std::string& slug) const{
    auto iter = m_slug_to_entry.find(slug);
    if (iter == m_slug_to_entry.end()){
        return (size_t)0 - 1;
    }
    return iter->second;
}
size_t StringSelectDatabase::search_index_by_name(const std::string& display_name) const{
    auto iter = m_display_name_to_entry.find(display_name);
    if (iter == m_display_name_to_entry.end()){
        return (size_t)0 - 1;
    }
    return iter->second;
}
void StringSelectDatabase::add_entry(StringSelectEntry entry){
    size_t index = m_list.size();
    StringSelectEntry& item = m_list.emplace_back(std::move(entry));

    std::map<std::string, size_t>::const_iterator iter0 = m_slug_to_entry.end();
    std::map<std::string, size_t>::const_iterator iter1 = m_display_name_to_entry.end();

    try{
        auto ret0 = m_slug_to_entry.emplace(item.slug, index);
        if (ret0.second){
            iter0 = ret0.first;
        }else{
            throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Duplicate Slug: " + item.slug);
        }
        auto ret1 = m_display_name_to_entry.emplace(item.display_name, index);
        if (ret1.second){
            iter1 = ret1.first;
        }else{
            throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Duplicate Display Name: " + item.display_name);
        }
    }catch (...){
        m_list.pop_back();
        if (iter0 != m_slug_to_entry.end()){
            m_slug_to_entry.erase(iter0);
        }
        if (iter1 != m_display_name_to_entry.end()){
            m_slug_to_entry.erase(iter1);
        }
        throw;
    }
}


StringSelectCell::StringSelectCell(const StringSelectDatabase& database, size_t default_index)
    : m_database(database)
    , m_default(default_index)
    , m_index(default_index)
{
    if (default_index >= database.case_list().size()){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Invalid Default Index: " + std::to_string(default_index));
    }
}
StringSelectCell::StringSelectCell(const StringSelectDatabase& database, const std::string& default_slug)
    : m_database(database)
    , m_default(default_slug == "" ? 0 : database.search_index_by_slug(default_slug))
    , m_index(m_default)
{
    if (m_default == (size_t)0 - 1){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Invalid Default Slug: " + default_slug);
    }
}
std::string StringSelectCell::set_by_slug(const std::string& slug){
    size_t index = m_database.search_index_by_slug(slug);
    if (index == (size_t)0 - 1){
        return "Invalid Slug: " + slug;
    }
    m_index.store(index, std::memory_order_relaxed);
    push_update();
    return "";
}
std::string StringSelectCell::set_by_name(const std::string& display_name){
    size_t index = m_database.search_index_by_name(display_name);
    if (index == (size_t)0 - 1){
        return "Invalid Name: " + display_name;
    }
    m_index.store(index, std::memory_order_relaxed);
    push_update();
    return "";
}
void StringSelectCell::load_json(const JsonValue& json){
    const std::string* str = json.get_string();
    if (str == nullptr){
        return;
    }
    size_t index = m_database.search_index_by_slug(*str);
    if (index == (size_t)0 - 1){
        return;
    }
    m_index.store(index, std::memory_order_relaxed);
    push_update();
}
JsonValue StringSelectCell::to_json() const{
    return m_database.case_list()[m_index.load(std::memory_order_relaxed)].slug;
}
void StringSelectCell::restore_defaults(){
    m_index.store(m_default, std::memory_order_relaxed);
    push_update();
}





















}
