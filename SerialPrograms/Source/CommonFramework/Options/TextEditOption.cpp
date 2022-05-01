/*  Text Edit
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QJsonValue>
#include <QVBoxLayout>
#include <QLabel>
#include <QTextEdit>
#include "TextEditOption.h"

namespace PokemonAutomation{




class TextEditWidget : public QWidget, public ConfigWidget{
public:
    TextEditWidget(QWidget& parent, TextEditOption& value);

    virtual void update_ui() override;
    virtual void restore_defaults() override;

private:
    class Box;

    TextEditOption& m_value;
    QTextEdit* m_box;
};



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
ConfigWidget* TextEditOption::make_ui(QWidget& parent){
    return new TextEditWidget(parent, *this);
}






class TextEditWidget::Box : public QTextEdit{
public:
    Box(TextEditWidget& parent)
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
    TextEditWidget& m_parent;
};







TextEditWidget::TextEditWidget(QWidget& parent, TextEditOption& value)
    : QWidget(&parent)
    , ConfigWidget(value, *this)
    , m_value(value)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(new QLabel(value.label(), this));
    m_box = new Box(*this);
    m_box->setText(value);
    layout->addWidget(m_box);
}
void TextEditWidget::update_ui(){
    m_box->setText(m_value);
}
void TextEditWidget::restore_defaults(){
    m_value.restore_defaults();
    update_ui();
}









}
