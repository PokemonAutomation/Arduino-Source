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

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{




template <typename Type>
SimpleIntegerCellWidget<Type>::~SimpleIntegerCellWidget(){
    m_value.remove_listener(*this);
}
template <typename Type>
SimpleIntegerCellWidget<Type>::SimpleIntegerCellWidget(QWidget& parent, SimpleIntegerCell<Type>& value)
    : QLineEdit(QString::number(value), &parent)
    , ConfigWidget(value, *this)
    , m_value(value)
{
    connect(
        this, &QLineEdit::textChanged,
        this, [this](const QString& text){
            bool ok;
            Type current = (Type)text.toLong(&ok);
            QPalette palette;
            if (ok && m_value.check_validity(current).empty()){
                palette.setColor(QPalette::Text, Qt::black);
            }else{
                palette.setColor(QPalette::Text, Qt::red);
            }
            this->setPalette(palette);
        }
    );
    connect(
        this, &QLineEdit::editingFinished,
        this, [this](){
            bool ok;
            Type current = (Type)this->text().toLong(&ok);
            QPalette palette;
            if (ok && m_value.check_validity(current).empty()){
                palette.setColor(QPalette::Text, Qt::black);
            }else{
                palette.setColor(QPalette::Text, Qt::red);
            }
            this->setPalette(palette);
            m_value.set(current);
        }
    );
    value.add_listener(*this);
}
template <typename Type>
void SimpleIntegerCellWidget<Type>::update_value(){
    this->setText(QString::number(m_value));
}
template <typename Type>
void SimpleIntegerCellWidget<Type>::update_visibility(bool program_is_running){
    ConfigWidget::update_visibility(program_is_running);
    update_value();
}
template <typename Type>
void SimpleIntegerCellWidget<Type>::value_changed(){
    QMetaObject::invokeMethod(this, [this]{
        update_value();
    }, Qt::QueuedConnection);
}







template <typename Type>
SimpleIntegerOptionWidget<Type>::~SimpleIntegerOptionWidget(){
    m_value.remove_listener(*this);
}
template <typename Type>
SimpleIntegerOptionWidget<Type>::SimpleIntegerOptionWidget(QWidget& parent, SimpleIntegerOption<Type>& value)
    : QWidget(&parent)
    , ConfigWidget(value, *this)
    , m_cell(new SimpleIntegerCellWidget<Type>(*this, value))
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    QLabel* text = new QLabel(QString::fromStdString(value.label()), this);
    text->setWordWrap(true);
    layout->addWidget(text, 1);
    layout->addWidget(m_cell, 1);
    value.add_listener(*this);
}
template <typename Type>
void SimpleIntegerOptionWidget<Type>::update_visibility(bool program_is_running){
    ConfigWidget::update_visibility(program_is_running);
//    m_cell->update_value();
}
template <typename Type>
void SimpleIntegerOptionWidget<Type>::value_changed(){
//    QMetaObject::invokeMethod(this, [this]{
//        update_value();
//    }, Qt::QueuedConnection);
}



template class SimpleIntegerCellWidget<uint8_t>;
template class SimpleIntegerCellWidget<uint16_t>;
template class SimpleIntegerCellWidget<uint32_t>;
template class SimpleIntegerCellWidget<uint64_t>;
template class SimpleIntegerCellWidget<int8_t>;

template class SimpleIntegerOptionWidget<uint8_t>;
template class SimpleIntegerOptionWidget<uint16_t>;
template class SimpleIntegerOptionWidget<uint32_t>;
template class SimpleIntegerOptionWidget<uint64_t>;
template class SimpleIntegerOptionWidget<int8_t>;


}
