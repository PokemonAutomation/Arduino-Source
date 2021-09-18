/*  Text Edit
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QVBoxLayout>
#include <QLabel>
#include "TextEditOption.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{


TextEditOption::TextEditOption(QString label, QString default_value)
    : m_label(std::move(label))
    , m_default(std::move(default_value))
    , m_current(m_backing)
    , m_backing(m_default)
{}
TextEditOption::TextEditOption(QString& backing, QString label, QString default_value)
    : m_label(std::move(label))
    , m_default(std::move(default_value))
    , m_current(backing)
{}


void TextEditOption::load_json(const QJsonValue& json){
    m_current = json.toString();
}
QJsonValue TextEditOption::to_json() const{
    return m_current;
}
void TextEditOption::restore_defaults(){
    m_current = m_default;
}
ConfigOptionUI* TextEditOption::make_ui(QWidget& parent){
    return new TextEditOptionUI(parent, *this);
}






class TextEditOptionUI::Box : public QTextEdit{
public:
    Box(TextEditOptionUI& parent)
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
    TextEditOptionUI& m_parent;
};







TextEditOptionUI::TextEditOptionUI(QWidget& parent, TextEditOption& value)
    : QWidget(&parent)
    , m_value(value)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel(value.m_label, this));
    m_box = new Box(*this);
    m_box->setText(value.m_current);
    layout->addWidget(m_box);
}
void TextEditOptionUI::restore_defaults(){
    m_value.restore_defaults();
}









}
