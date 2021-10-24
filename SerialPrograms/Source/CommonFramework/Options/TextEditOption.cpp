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


TextEditOption::TextEditOption(
    QString label,
    QString default_value,
    QString placeholder_text
)
    : m_label(std::move(label))
    , m_default(std::move(default_value))
    , m_placeholder_text(std::move(placeholder_text))
    , m_current(m_default)
{}

TextEditOption::operator const QString&() const{
    SpinLockGuard lg(m_lock);
    return m_current;
}
void TextEditOption::set(QString x){
    SpinLockGuard lg(m_lock);
    m_current = std::move(x);
}


void TextEditOption::load_json(const QJsonValue& json){
    SpinLockGuard lg(m_lock);
    m_current = json.toString();
}
QJsonValue TextEditOption::to_json() const{
    SpinLockGuard lg(m_lock);
    return m_current;
}
void TextEditOption::restore_defaults(){
    SpinLockGuard lg(m_lock);
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
        this->setPlaceholderText(parent.m_value.placeholder_text());
//        this->hide();
    }

    void update_backing(){
        m_parent.m_value.set(this->toPlainText());
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
    layout->addWidget(new QLabel(value.label(), this));
    m_box = new Box(*this);
    m_box->setText(value);
    layout->addWidget(m_box);
}
void TextEditOptionUI::restore_defaults(){
    m_value.restore_defaults();
}









}
