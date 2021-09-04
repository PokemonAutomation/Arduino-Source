/*  Enum Dropdown Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QHBoxLayout>
#include <QLabel>
#include "Common/Qt/NoWheelComboBox.h"
#include "EnumDropdown.h"

namespace PokemonAutomation{


EnumDropdown::EnumDropdown(
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


void EnumDropdown::load_json(const QJsonValue& json){
    if (!json.isString()){
        return;
    }
    QString str = json.toString();
    auto iter = m_case_map.find(str);
    if (iter != m_case_map.end()){
        m_current = iter->second;
    }
}
QJsonValue EnumDropdown::to_json() const{
    return QJsonValue(m_case_list[m_current]);
}

void EnumDropdown::restore_defaults(){
    m_current = m_default;
}

ConfigOptionUI* EnumDropdown::make_ui(QWidget& parent){
    return new EnumDropdownUI(parent, *this);
}



EnumDropdownUI::EnumDropdownUI(QWidget& parent, EnumDropdown& value)
    : QWidget(&parent)
    , m_value(value)
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    QLabel* text = new QLabel(m_value.m_label, this);
    layout->addWidget(text, 1);
    text->setWordWrap(true);
    m_box = new NoWheelComboBox(&parent);
    layout->addWidget(m_box);

    for (const QString& item : m_value.m_case_list){
        m_box->addItem(item);
    }
    m_box->setCurrentIndex((int)m_value.m_current);
    layout->addWidget(m_box, 1);

    connect(
        m_box, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
        this, [=](int index){
            if (index < 0){
                m_value.restore_defaults();
                return;
            }
            m_value.m_current = index;
        }
    );
}


void EnumDropdownUI::restore_defaults(){
    m_value.restore_defaults();
    m_box->setCurrentIndex((int)m_value);
}



}
