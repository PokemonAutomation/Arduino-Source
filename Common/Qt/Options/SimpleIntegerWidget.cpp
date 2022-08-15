/*  Simple Integer Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include "ConfigWidget.h"
#include "SimpleIntegerWidget.h"

namespace PokemonAutomation{


template <typename Type>
SimpleIntegerWidget<Type>::~SimpleIntegerWidget(){
    m_value.remove_listener(*this);
}
template <typename Type>
SimpleIntegerWidget<Type>::SimpleIntegerWidget(QWidget& parent, SimpleIntegerOption<Type>& value)
    : QWidget(&parent)
    , ConfigWidget(value, *this)
    , m_value(value)
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    QLabel* text = new QLabel(QString::fromStdString(m_value.label()), this);
    text->setWordWrap(true);
    layout->addWidget(text, 1);
    m_box = new QLineEdit(QString::number(m_value), this);
//    box->setInputMask("999999999");
//    QIntValidator* validator = new QIntValidator(value.m_min_value, value.m_max_value, box);
//    box->setValidator(validator);
    layout->addWidget(m_box, 1);
    connect(
        m_box, &QLineEdit::textChanged,
        this, [=](const QString& text){
            bool ok;
            Type current = (Type)text.toLong(&ok);
            QPalette palette;
            if (ok && m_value.check_validity(current).empty()){
                palette.setColor(QPalette::Text, Qt::black);
            }else{
                palette.setColor(QPalette::Text, Qt::red);
            }
            m_box->setPalette(palette);
        }
    );
    connect(
        m_box, &QLineEdit::editingFinished,
        this, [=](){
            bool ok;
            Type current = (Type)m_box->text().toLong(&ok);
            QPalette palette;
            if (ok && m_value.check_validity(current).empty()){
                palette.setColor(QPalette::Text, Qt::black);
            }else{
                palette.setColor(QPalette::Text, Qt::red);
            }
            m_box->setPalette(palette);

            if (current == m_value){
                return;
            }

            m_value.set(current);
        }
    );
    value.add_listener(*this);
}
template <typename Type>
void SimpleIntegerWidget<Type>::update(){
    ConfigWidget::update();
    m_box->setText(QString::number(m_value));
}
template <typename Type>
void SimpleIntegerWidget<Type>::value_changed(){
    QMetaObject::invokeMethod(m_box, [=]{
        update();
    }, Qt::QueuedConnection);
}




template class SimpleIntegerWidget<uint8_t>;
template class SimpleIntegerWidget<uint16_t>;
template class SimpleIntegerWidget<uint32_t>;
template class SimpleIntegerWidget<uint64_t>;
template class SimpleIntegerWidget<int8_t>;


}
