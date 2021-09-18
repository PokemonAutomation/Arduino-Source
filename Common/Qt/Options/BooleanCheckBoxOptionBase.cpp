/*  Boolean Check Box Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QHBoxLayout>
#include <QLabel>
#include "BooleanCheckBoxOptionBase.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{


BooleanCheckBoxOptionBase::BooleanCheckBoxOptionBase(
    bool& backing,
    QString label,
    bool default_value
)
    : m_label(std::move(label))
    , m_default(default_value)
    , m_current(backing)
{
//    cout << "Backing: " << m_label.toUtf8().data() << endl;
//    cout << "&m_current = " << &m_current << endl;
}
BooleanCheckBoxOptionBase::BooleanCheckBoxOptionBase(
    QString label,
    bool default_value
)
    : m_label(std::move(label))
    , m_default(default_value)
    , m_current(m_backing)
    , m_backing(default_value)
{}

void BooleanCheckBoxOptionBase::load_default(const QJsonValue& json){
    if (!json.isBool()){
        return;
    }
    m_default = json.toBool();
}
void BooleanCheckBoxOptionBase::load_current(const QJsonValue& json){
    if (!json.isBool()){
        return;
    }
    m_current = json.toBool();
}
QJsonValue BooleanCheckBoxOptionBase::write_default() const{
    return QJsonValue(m_default);
}
QJsonValue BooleanCheckBoxOptionBase::write_current() const{
    return QJsonValue(m_current);
}

void BooleanCheckBoxOptionBase::restore_defaults(){
    m_current = m_default;
}


BooleanCheckBoxOptionBaseUI::BooleanCheckBoxOptionBaseUI(QWidget& parent, BooleanCheckBoxOptionBase& value)
    : QWidget(&parent)
    , m_value(value)
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    QLabel* text = new QLabel(m_value.m_label, this);
    layout->addWidget(text, 3);
    text->setWordWrap(true);
    m_box = new QCheckBox(this);
    m_box->setChecked(m_value);
    layout->addWidget(m_box, 1);
    connect(
        m_box, &QCheckBox::stateChanged,
        this, [=](int){
            m_value.m_current = m_box->isChecked();
//            cout << "m_value.m_current = " << m_value.m_current << endl;
//            cout << "&m_value.m_current = " << &m_value.m_current << endl;
        }
    );
}
void BooleanCheckBoxOptionBaseUI::restore_defaults(){
    m_value.restore_defaults();
    m_box->setChecked(m_value);
}



}

