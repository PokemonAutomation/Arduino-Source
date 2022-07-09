/*  Enum Dropdown Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QStandardItemModel>
#include <QHBoxLayout>
#include <QLabel>
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Qt/NoWheelComboBox.h"
#include "EnumDropdownOption.h"
#include "EnumDropdownWidget.h"

namespace PokemonAutomation{


EnumDropdownOption::EnumDropdownOption(
    QString label,
    std::initializer_list<Option> cases,
    size_t default_index
)
    : m_label(std::move(label))
    , m_default(default_index)
    , m_current(default_index)
{
    if (default_index >= cases.size()){
        throw "Index is too large.";
    }
    size_t index = 0;
    for (auto iter = cases.begin(); iter != cases.end(); ++iter){
        m_case_list.emplace_back(*iter);
        const std::string& item = m_case_list.back().name;
        auto ret = m_case_map.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(item),
            std::forward_as_tuple(index++)
        );
        if (!ret.second){
            throw "Duplicate enum label.";
        }
    }
}


void EnumDropdownOption::load_json(const JsonValue2& json){
    const std::string* str = json.get_string();
    if (str == nullptr){
        return;
    }
    auto iter = m_case_map.find(*str);
    if (iter != m_case_map.end() && m_case_list[iter->second].enabled){
        m_current.store(iter->second, std::memory_order_relaxed);
    }
}
JsonValue2 EnumDropdownOption::to_json() const{
    return m_case_list[m_current].name;
}

void EnumDropdownOption::restore_defaults(){
    m_current.store(m_default, std::memory_order_relaxed);
}

ConfigWidget* EnumDropdownOption::make_ui(QWidget& parent){
    return new EnumDropdownWidget(parent, *this);
}



EnumDropdownWidget::EnumDropdownWidget(QWidget& parent, EnumDropdownOption& value)
    : QWidget(&parent)
    , ConfigWidget(value, *this)
    , m_value(value)
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    QLabel* text = new QLabel(m_value.label(), this);
    text->setWordWrap(true);
    layout->addWidget(text, 1);
    m_box = new NoWheelComboBox(&parent);
    layout->addWidget(m_box);

    for (const auto& item : m_value.m_case_list){
        m_box->addItem(QString::fromStdString(item.name));
        if (item.enabled){
            continue;
        }
        auto* model = qobject_cast<QStandardItemModel*>(m_box->model());
        if (model == nullptr){
            continue;
        }
        QStandardItem* line_handle = model->item(m_box->count() - 1);
        if (line_handle != nullptr){
            line_handle->setEnabled(false);
        }
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
            emit on_changed();
        }
    );
}


void EnumDropdownWidget::restore_defaults(){
    m_value.restore_defaults();
    update_ui();
}
void EnumDropdownWidget::update_ui(){
    m_box->setCurrentIndex((int)m_value);
}



}
