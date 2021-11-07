/*  Enum Dropdown Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QHBoxLayout>
#include <QLabel>
#include "Common/Qt/NoWheelComboBox.h"
#include "EnumDropdownOption.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{


EnumDropdownOption::EnumDropdownOption(
    QString label,
    std::vector<QString> cases,
    size_t default_index
)
    : m_label(std::move(label))
    , m_case_list(std::move(cases))
    , m_default(default_index)
    , m_current(default_index)
{
    if (default_index >= m_case_list.size()){
        throw "Index is too large.";
    }

    for (size_t index = 0; index < m_case_list.size(); index++){
        const QString& item = m_case_list[index];
        auto ret = m_case_map.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(item),
            std::forward_as_tuple(index)
        );
        if (!ret.second){
            throw "Duplicate enum label.";
        }
    }
}


void EnumDropdownOption::load_json(const QJsonValue& json){
    if (!json.isString()){
        return;
    }
    QString str = json.toString();
    auto iter = m_case_map.find(str);
    if (iter != m_case_map.end()){
        m_current.store(iter->second, std::memory_order_relaxed);
    }
}
QJsonValue EnumDropdownOption::to_json() const{
    return QJsonValue(m_case_list[m_current]);
}

void EnumDropdownOption::restore_defaults(){
    m_current.store(m_default, std::memory_order_relaxed);
}

ConfigOptionUI* EnumDropdownOption::make_ui(QWidget& parent){
    return new EnumDropdownOptionUI(parent, *this);
}



EnumDropdownOptionUI::EnumDropdownOptionUI(QWidget& parent, EnumDropdownOption& value)
    : QWidget(&parent)
    , ConfigOptionUI(value, *this)
    , m_value(value)
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    QLabel* text = new QLabel(m_value.label(), this);
    layout->addWidget(text, 1);
    text->setWordWrap(true);
    m_box = new NoWheelComboBox(&parent);
    layout->addWidget(m_box);

    for (const QString& item : m_value.case_list()){
        m_box->addItem(item);
    }
    m_box->setCurrentIndex((int)m_value);
    layout->addWidget(m_box, 1);

    connect(
        m_box, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
        this, [=](int index){
            if (index < 0){
                m_value.restore_defaults();
                return;
            }
            m_value.set(index);
            on_changed();
        }
    );
}


void EnumDropdownOptionUI::restore_defaults(){
    m_value.restore_defaults();
    m_box->setCurrentIndex((int)m_value);
}



}
