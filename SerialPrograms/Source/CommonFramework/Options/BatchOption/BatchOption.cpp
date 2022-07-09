/*  Batch Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QVBoxLayout>
#include "Common/Compiler.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "BatchOption.h"
#include "BatchWidget.h"

namespace PokemonAutomation{

//BatchOption::BatchOption(){}
void BatchOption::add_option(ConfigOption& option, std::string serialization_string){
    m_options.emplace_back(&option, std::move(serialization_string));
}

void BatchOption::load_json(const JsonValue2& json){
    const JsonObject2* obj = json.get_object();
    if (obj == nullptr){
        return;
    }
    for (auto& item : m_options){
        if (!item.second.empty()){
            const JsonValue2* value = obj->get_value(item.second);
            if (value){
                item.first->load_json(*value);
            }
        }
    }
}
JsonValue2 BatchOption::to_json() const{
    JsonObject2 obj;
    for (auto& item : m_options){
        if (!item.second.empty()){
            obj[item.second] = item.first->to_json();
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
        m_options.back()->widget().setContentsMargins(0, 5, 0, 5);
        options_layout->addWidget(&m_options.back()->widget(), 0);
    }
}
void BatchWidget::restore_defaults(){
    for (ConfigWidget* item : m_options){
        item->restore_defaults();
    }
}
void BatchWidget::update_ui(){
    for (ConfigWidget* item : m_options){
        item->update_ui();
    }
}
void BatchWidget::update_visibility(){
    ConfigWidget::update_visibility();
    for (ConfigWidget* item : m_options){
        item->update_visibility();
    }
}





}
