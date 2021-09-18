/*  Batch Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QVBoxLayout>
#include <QGroupBox>
#include "Common/Qt/QtJsonTools.h"
#include "BatchOption.h"

namespace PokemonAutomation{

//BatchOption::BatchOption(){}
void BatchOption::add_option(ConfigOption& option, QString serialization_string){
    m_options.emplace_back(&option, std::move(serialization_string));
}

void BatchOption::load_json(const QJsonValue& json){
    const QJsonObject& obj = json.toObject();
    for (auto& item : m_options){
        if (!item.second.isEmpty()){
            item.first->load_json(json_get_value_nothrow(obj, item.second));
        }
    }
}
QJsonValue BatchOption::to_json() const{
    QJsonObject obj;
    for (auto& item : m_options){
        if (!item.second.isEmpty()){
            obj.insert(item.second, item.first->to_json());
        }
    }
    return obj;
}

bool BatchOption::is_valid() const{
    for (const auto& item : m_options){
        if (!item.first->is_valid()){
            return false;
        }
    }
    return true;
}
void BatchOption::restore_defaults(){
    for (const auto& item : m_options){
        item.first->restore_defaults();
    }
}


BatchOptionUI::BatchOptionUI(QWidget& parent, BatchOption& value)
    : QWidget(&parent)
    , m_value(value)
{
    QVBoxLayout* options_layout = new QVBoxLayout(this);
    options_layout->setAlignment(Qt::AlignTop);
    options_layout->setMargin(0);

    for (auto& item : m_value.m_options){
        m_options.emplace_back(item.first->make_ui(parent));
        options_layout->addWidget(m_options.back()->widget());
    }
}
bool BatchOptionUI::settings_valid() const{
    return m_value.is_valid();
}
void BatchOptionUI::restore_defaults(){
    for (ConfigOptionUI* item : m_options){
        item->restore_defaults();
    }
}





GroupOption::GroupOption(QString label)
    : m_label(std::move(label))
{}
GroupOptionUI::GroupOptionUI(QWidget& parent, GroupOption& value)
    : QWidget(&parent)
    , m_value(value)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
//    layout->setMargin(0);
    QGroupBox* group_box = new QGroupBox(value.m_label, this);
    layout->addWidget(group_box);

#if 0
    QVBoxLayout* mid_layout = new QVBoxLayout(group_box);
    QWidget* mid_widget = new QWidget(group_box);
    mid_layout->addWidget(mid_widget);
    mid_layout->setMargin(0);

    QFont font = group_box->font();
    mid_widget->setFont(font);
    font.setBold(true);
    group_box->setFont(font);
#endif

    m_options_layout = new QVBoxLayout(group_box);
    m_options_layout->setAlignment(Qt::AlignTop);
    m_options_layout->setMargin(0);

    for (auto& item : m_value.m_options){
        m_options.emplace_back(item.first->make_ui(parent));
        m_options_layout->addWidget(m_options.back()->widget());
    }
}
bool GroupOptionUI::settings_valid() const{
    return m_value.is_valid();
}
void GroupOptionUI::restore_defaults(){
    for (ConfigOptionUI* item : m_options){
        item->restore_defaults();
    }
}





}
