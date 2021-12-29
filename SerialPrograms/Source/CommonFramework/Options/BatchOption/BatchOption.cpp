/*  Batch Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QVBoxLayout>
#include "Common/Compiler.h"
#include "Common/Qt/QtJsonTools.h"
#include "BatchOption.h"
#include "BatchWidget.h"

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

QString BatchOption::check_validity() const{
    for (const auto& item : m_options){
        QString error = item.first->check_validity();
        if (!error.isEmpty()){
            return error;
        }
    }
    return QString();
}
void BatchOption::restore_defaults(){
    for (const auto& item : m_options){
        item.first->restore_defaults();
    }
}
void BatchOption::reset_state(){
    for (const auto& item : m_options){
        item.first->reset_state();
    }
}
ConfigWidget* BatchOption::make_ui(QWidget& parent){
    return new BatchWidget(parent, *this);
}



BatchWidget::BatchWidget(QWidget& parent, BatchOption& value)
    : QWidget(&parent)
    , ConfigWidget(value, *this)
    , m_value(value)
{
    QVBoxLayout* options_layout = new QVBoxLayout(this);
    options_layout->setAlignment(Qt::AlignTop);
    options_layout->setContentsMargins(0, 0, 0, 0);

    for (auto& item : m_value.m_options){
        m_options.emplace_back(item.first->make_ui(parent));
        options_layout->addWidget(&m_options.back()->widget(), 0);
    }
}
void BatchWidget::restore_defaults(){
    for (ConfigWidget* item : m_options){
        item->restore_defaults();
    }
}
void BatchWidget::update_visibility(){
    ConfigWidget::update_visibility();
    for (ConfigWidget* item : m_options){
        item->update_visibility();
    }
}





}
