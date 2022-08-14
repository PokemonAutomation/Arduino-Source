/*  String Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include "StringWidget.h"

namespace PokemonAutomation{


ConfigWidget* StringOption::make_ui(QWidget& parent){
    return new StringWidget(parent, *this);
}



StringWidget::~StringWidget(){
    m_value.remove_listener(*this);
}
StringWidget::StringWidget(QWidget& parent, StringOption& value)
    : QWidget(&parent)
    , ConfigWidget(value, *this)
    , m_value(value)
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    QLabel* text = new QLabel(QString::fromStdString(value.label()), this);
    text->setWordWrap(true);
    layout->addWidget(text, 1);

    m_box = new QLineEdit(QString::fromStdString(m_value));
    m_box->setPlaceholderText(QString::fromStdString(value.placeholder_text()));
    layout->addWidget(m_box, 1);

    if (m_value.is_password()){
        m_box->setEchoMode(QLineEdit::PasswordEchoOnEdit);
    }

    connect(
        m_box, &QLineEdit::editingFinished,
        this, [=](){
            m_value.set(m_box->text().toStdString());
        }
    );

    m_value.add_listener(*this);
}
void StringWidget::restore_defaults(){
    m_value.restore_defaults();
}
void StringWidget::update_ui(){
    m_box->setText(QString::fromStdString(m_value));
}
void StringWidget::value_changed(){
    QMetaObject::invokeMethod(m_box, [=]{
        update_ui();
    }, Qt::QueuedConnection);
}






}
