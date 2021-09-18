/*  String Box Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QVBoxLayout>
#include <QLabel>
#include <QTextEdit>
#include "StringBoxOption.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{


StringBoxOption::StringBoxOption(
    QString& backing,
    QString label,
    QString default_value
)
    : m_label(std::move(label))
    , m_default(default_value)
    , m_current(backing)
{}
StringBoxOption::StringBoxOption(
    QString label,
    QString default_value
)
    : m_label(std::move(label))
    , m_default(default_value)
    , m_current(m_backing)
    , m_backing(default_value)
{}


void StringBoxOption::load_json(const QJsonValue& json){
    if (!json.isString()) {
        return;
    }
    m_current = json.toString();
}
QJsonValue StringBoxOption::to_json() const{
    return QJsonValue(m_current);
}
void StringBoxOption::restore_defaults(){
    m_current = m_default;
}




class StringBoxOptionUI::Box : public QTextEdit{
public:
    Box(StringBoxOptionUI& parent)
        : QTextEdit(&parent)
        , m_parent(parent)
    {
        this->setAcceptRichText(false);
        this->setFocusPolicy(Qt::StrongFocus);
    }

    void update_backing(){
        m_parent.m_value.m_current = this->toPlainText();
    }
    virtual void focusOutEvent(QFocusEvent* event) override{
        QTextEdit::focusOutEvent(event);
        update_backing();
    }

private:
    StringBoxOptionUI& m_parent;
};



StringBoxOptionUI::StringBoxOptionUI(QWidget& parent, StringBoxOption& value)
    : QWidget(&parent)
    , m_value(value)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel(value.m_label, this));
    m_box = new Box(*this);
    m_box->setText(value.m_current);
    layout->addWidget(m_box);
}
void StringBoxOptionUI::restore_defaults(){
    return m_value.restore_defaults();
}



}
