/*  Time Expression Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include "TimeExpressionWidget.h"

namespace PokemonAutomation{



template <typename Type>
TimeExpressionCellWidget<Type>::~TimeExpressionCellWidget(){
    m_value.remove_listener(*this);
}
template <typename Type>
TimeExpressionCellWidget<Type>::TimeExpressionCellWidget(QWidget& parent, TimeExpressionCell<Type>& value)
    : QLineEdit(QString::fromStdString(value.current_text()), &parent)
    , ConfigWidget(value, *this)
    , m_value(value)
{
    connect(
        this, &QLineEdit::textChanged,
        this, [this](const QString& text){
            std::string error = m_value.set(text.toStdString());
        }
    );

    value.add_listener(*this);
}
template <typename Type>
void TimeExpressionCellWidget<Type>::update_value(){
    this->setText(QString::fromStdString(m_value.current_text()));
}
template <typename Type>
void TimeExpressionCellWidget<Type>::on_config_value_changed(void* object){
    QMetaObject::invokeMethod(this, [this]{
        this->update_value();
    }, Qt::QueuedConnection);
}





template <typename Type>
TimeExpressionOptionWidget<Type>::~TimeExpressionOptionWidget(){
    m_value.remove_listener(*this);
}
template <typename Type>
TimeExpressionOptionWidget<Type>::TimeExpressionOptionWidget(QWidget& parent, TimeExpressionOption<Type>& value)
    : QWidget(&parent)
    , ConfigWidget(value, *this)
    , m_value(value)
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    QLabel* text = new QLabel(QString::fromStdString(m_value.label()), this);
    text->setWordWrap(true);
    text->setTextFormat(Qt::RichText);
    text->setTextInteractionFlags(Qt::TextBrowserInteraction);
    text->setOpenExternalLinks(true);
    layout->addWidget(text, 1);
    QVBoxLayout* rows = new QVBoxLayout();
    layout->addLayout(rows, 1);

    QHBoxLayout* row0 = new QHBoxLayout();
    rows->addLayout(row0);
    QHBoxLayout* row1 = new QHBoxLayout();
    rows->addLayout(row1);

    m_box = new QLineEdit(QString::fromStdString(m_value.current_text()), this);
    row0->addWidget(m_box);
    row0->addWidget(new QLabel("ticks", this));

    QLabel* seconds = new QLabel(QString::fromStdString(m_value.time_string()), this);
    seconds->setAlignment(Qt::AlignHCenter);
    row1->addWidget(seconds);

    connect(
        m_box, &QLineEdit::editingFinished,
        this, [this, seconds](){
            std::string error = m_value.set(m_box->text().toStdString());
            if (error.empty()){
                seconds->setText(QString::fromStdString(m_value.time_string()));
            }else{
                seconds->setText(QString::fromStdString("<font color=\"red\">" + error + "</font>"));
            }
        }
    );

    value.add_listener(*this);
}
template <typename Type>
void TimeExpressionOptionWidget<Type>::update_value(){
    m_box->setText(QString::fromStdString(m_value.current_text()));
}
template <typename Type>
void TimeExpressionOptionWidget<Type>::on_config_value_changed(void* object){
    QMetaObject::invokeMethod(m_box, [this]{
        this->update_value();
    }, Qt::QueuedConnection);
}





template class TimeExpressionCellWidget<uint8_t>;
template class TimeExpressionCellWidget<uint16_t>;
template class TimeExpressionCellWidget<uint32_t>;
template class TimeExpressionCellWidget<int16_t>;
template class TimeExpressionCellWidget<int32_t>;

template class TimeExpressionOptionWidget<uint8_t>;
template class TimeExpressionOptionWidget<uint16_t>;
template class TimeExpressionOptionWidget<uint32_t>;
template class TimeExpressionOptionWidget<int16_t>;
template class TimeExpressionOptionWidget<int32_t>;




}
