/*  Simple Integer Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QHBoxLayout>
#include <QLabel>
//#include <QIntValidator>
#include "SimpleIntegerBaseWidget.h"

namespace PokemonAutomation{


template <typename Type>
SimpleIntegerBaseWidget<Type>::SimpleIntegerBaseWidget(QWidget& parent, SimpleIntegerBaseOption<Type>& value)
    : QWidget(&parent)
    , m_value(value)
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    QLabel* text = new QLabel(m_value.label(), this);
    layout->addWidget(text, 1);
    text->setWordWrap(true);
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
            if (ok && m_value.set(current).isEmpty()){
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
            m_box->setText(QString::number(m_value));
        }
    );
}
template <typename Type>
void SimpleIntegerBaseWidget<Type>::restore_defaults(){
    m_value.restore_defaults();
    update_ui();
}
template <typename Type>
void SimpleIntegerBaseWidget<Type>::update_ui(){
    m_box->setText(QString::number(m_value));
}




template class SimpleIntegerBaseWidget<uint8_t>;
template class SimpleIntegerBaseWidget<uint16_t>;
template class SimpleIntegerBaseWidget<uint32_t>;
template class SimpleIntegerBaseWidget<uint64_t>;
template class SimpleIntegerBaseWidget<int8_t>;


}
