/*  String Select
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QHBoxLayout>
#include <QLabel>
#include <QCompleter>
#include "Common/Cpp/Exception.h"
#include "Common/Qt/NoWheelComboBox.h"
#include "StringSelect.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{


StringSelect::StringSelect(
    QString label,
    const std::vector<QString>& cases,
    const QString& default_case
)
    : m_label(std::move(label))
//    , m_case_list(std::move(cases))
    , m_default(0)
    , m_current(0)
{
    for (size_t index = 0; index < cases.size(); index++){
        const QString& item = cases[index];
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
            PA_THROW_StringException("Duplicate enum label.");
        }
    }
    m_current = m_default;
}
StringSelect::StringSelect(
    QString label,
    std::vector<std::pair<QString, QIcon>> cases,
    const QString& default_case
)
    : m_label(std::move(label))
    , m_case_list(std::move(cases))
    , m_default(0)
    , m_current(0)
{
    for (size_t index = 0; index < m_case_list.size(); index++){
        const QString& item = m_case_list[index].first;
        if (item == default_case){
            m_default = index;
        }
        auto ret = m_case_map.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(item),
            std::forward_as_tuple(index)
        );
        if (!ret.second){
            PA_THROW_StringException("Duplicate enum label.");
        }
    }
    m_current = m_default;
}

void StringSelect::load_json(const QJsonValue& json){
    if (!json.isString()){
        return;
    }
    QString str = json.toString();
    auto iter = m_case_map.find(str);
    if (iter != m_case_map.end()){
        m_current = iter->second;
    }
}
QJsonValue StringSelect::to_json() const{
    return QJsonValue(m_case_list[m_current].first);
}

void StringSelect::restore_defaults(){
    m_current = m_default;
}

ConfigOptionUI* StringSelect::make_ui(QWidget& parent){
    return new StringSelectUI(parent, *this);
}



StringSelectUI::StringSelectUI(QWidget& parent, StringSelect& value)
    : QWidget(&parent)
    , m_value(value)
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    QLabel* text = new QLabel(m_value.m_label, this);
    layout->addWidget(text, 1);
    text->setWordWrap(true);
    m_box = new NoWheelComboBox(&parent);

    m_box->setEditable(true);
    m_box->setInsertPolicy(QComboBox::NoInsert);
    m_box->completer()->setCompletionMode(QCompleter::PopupCompletion);
    m_box->completer()->setFilterMode(Qt::MatchContains);

    for (const auto& item : m_value.m_case_list){
        m_box->addItem(item.second, item.first);
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
//            cout << "index = " << index << endl;
        }
    );
}


void StringSelectUI::restore_defaults(){
    m_value.restore_defaults();
    m_box->setCurrentIndex((int)m_value);
}





}
