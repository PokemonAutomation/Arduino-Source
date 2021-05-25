/*  String Select
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QHBoxLayout>
#include <QLabel>
#include <QCompleter>
#include "StringSelect.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{


StringSelect::StringSelect(
    QString label,
    std::vector<QString> cases,
    size_t default_index
)
    : ConfigOption(std::move(label))
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
    return QJsonValue(m_case_list[m_current]);
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
    m_box = new QComboBox(&parent);
    layout->addWidget(m_box);

    m_box->setEditable(true);
    m_box->setInsertPolicy(QComboBox::NoInsert);
    m_box->completer()->setCompletionMode(QCompleter::PopupCompletion);
    m_box->completer()->setFilterMode(Qt::MatchContains);

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
            cout << "index = " << index << endl;
        }
    );
}


void StringSelectUI::restore_defaults(){
    m_value.restore_defaults();
    m_box->setCurrentIndex((int)m_value);
}





}
