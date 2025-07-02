/*  Simple Integer Option
 *
 *  From: https://github.com/PokemonAutomation/
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
//    cout << "sizeHint() = " << this->sizeHint().width() << endl;

    this->setReadOnly(value.lock_mode() == LockMode::READ_ONLY);

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
            if (std::is_unsigned_v<Type>){
                uint64_t current = this->text().toULongLong(&ok);
                current = std::max<uint64_t>(current, m_value.min_value());
                current = std::min<uint64_t>(current, m_value.max_value());
                this->setText(QString::number(current));
                m_value.set((Type)current);
            }else{
                int64_t current = this->text().toLongLong(&ok);
                current = std::max<int64_t>(current, m_value.min_value());
                current = std::min<int64_t>(current, m_value.max_value());
                this->setText(QString::number(current));
                m_value.set((Type)current);
            }
        }
    );
    value.add_listener(*this);
}
template <typename Type>
void SimpleIntegerCellWidget<Type>::update_value(){
    this->setText(QString::number(m_value));
}
template <typename Type>
void SimpleIntegerCellWidget<Type>::on_config_value_changed(void* object){
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
    text->setTextFormat(Qt::RichText);
    text->setTextInteractionFlags(Qt::TextBrowserInteraction);
    text->setOpenExternalLinks(true);
    layout->addWidget(text, 1);
    layout->addWidget(m_cell, 1);
    value.add_listener(*this);
}
template <typename Type>
void SimpleIntegerOptionWidget<Type>::update_value(){
    m_cell->update_value();
}
template <typename Type>
void SimpleIntegerOptionWidget<Type>::on_config_value_changed(void* object){
    m_cell->on_config_value_changed(object);
}



template class SimpleIntegerCellWidget<uint8_t>;
template class SimpleIntegerCellWidget<uint16_t>;
template class SimpleIntegerCellWidget<uint32_t>;
template class SimpleIntegerCellWidget<uint64_t>;
template class SimpleIntegerCellWidget<int8_t>;
template class SimpleIntegerCellWidget<int16_t>;
template class SimpleIntegerCellWidget<int32_t>;
template class SimpleIntegerCellWidget<int64_t>;

template class SimpleIntegerOptionWidget<uint8_t>;
template class SimpleIntegerOptionWidget<uint16_t>;
template class SimpleIntegerOptionWidget<uint32_t>;
template class SimpleIntegerOptionWidget<uint64_t>;
template class SimpleIntegerOptionWidget<int8_t>;
template class SimpleIntegerOptionWidget<int16_t>;
template class SimpleIntegerOptionWidget<int32_t>;
template class SimpleIntegerOptionWidget<int64_t>;


//  This is stupid.
#ifdef __APPLE__
template class SimpleIntegerCellWidget<size_t>;
template class SimpleIntegerOptionWidget<size_t>;
#endif


}
