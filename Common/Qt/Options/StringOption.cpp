/*  String Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "StringOption.h"

#include <QHBoxLayout>
#include <QLabel>

namespace PokemonAutomation{


StringOption::StringOption(
    QString& backing,
    QString label,
    QString default_value
)
    : m_label(std::move(label))
    , m_default(default_value)
    , m_current(backing)
{}
StringOption::StringOption(
    QString label,
    QString default_value
)
    : m_label(std::move(label))
    , m_default(default_value)
    , m_current(m_backing)
    , m_backing(default_value)
{}
void StringOption::load_default(const QJsonValue& json){
    if (!json.isString()) {
        return;
    }
    m_default = json.toString();
}
void StringOption::load_current(const QJsonValue& json){
    if (!json.isString()) {
        return;
    }
    m_current = json.toString();
}
QJsonValue StringOption::write_default() const{
    return QJsonValue(m_default);
}
QJsonValue StringOption::write_current() const{
    return QJsonValue(m_current);
}

void StringOption::restore_defaults(){
    m_current = m_default;
}

StringOptionUI::StringOptionUI(QWidget& parent, StringOption& value)
    : QWidget(&parent)
    , m_value(value)
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    QLabel* text = new QLabel(value.m_label, this);
    layout->addWidget(text, 1);
    text->setWordWrap(true);

    m_line_edit = new QLineEdit(m_value.m_current);
    layout->addWidget(m_line_edit, 1);

    connect(
        m_line_edit, &QLineEdit::textChanged,
        this, [=](const QString& line){
            m_value.m_current = line;
        }
    );
}
void StringOptionUI::restore_defaults(){
    m_value.restore_defaults();
    m_line_edit->setText(m_value.m_current);
}
    

}

