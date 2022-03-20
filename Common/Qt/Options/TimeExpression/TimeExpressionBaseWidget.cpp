/*  Time Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QHBoxLayout>
#include <QLabel>
#include "TimeExpressionBaseWidget.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


template <typename Type>
TimeExpressionBaseWidget<Type>::TimeExpressionBaseWidget(QWidget& parent, TimeExpressionBaseOption<Type>& value)
    : QWidget(&parent)
    , m_value(value)
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    QLabel* text = new QLabel(m_value.label(), this);
    layout->addWidget(text, 1);
    text->setWordWrap(true);
    QVBoxLayout* rows = new QVBoxLayout();
    layout->addLayout(rows, 1);

    QHBoxLayout* row0 = new QHBoxLayout();
    rows->addLayout(row0);
    QHBoxLayout* row1 = new QHBoxLayout();
    rows->addLayout(row1);

    m_box = new QLineEdit(m_value.text(), this);
    row0->addWidget(m_box);
    row0->addWidget(new QLabel("ticks", this));

    QLabel* seconds = new QLabel(m_value.time_string(), this);
    seconds->setAlignment(Qt::AlignHCenter);
    row1->addWidget(seconds);

    connect(
        m_box, &QLineEdit::textChanged,
        this, [=](const QString& text){
//           cout << text.toStdString() << endl;
            QString error = m_value.set(text);
            if (error.isEmpty()){
                seconds->setText(m_value.time_string());
            }else{
                seconds->setText("<font color=\"red\">" + error + "</font>");
            }
        }
    );
    connect(
        m_box, &QLineEdit::editingFinished,
        this, [=](){
            m_box->setText(m_value.text());
            seconds->setText(m_value.time_string());
        }
    );
}
template <typename Type>
void TimeExpressionBaseWidget<Type>::restore_defaults(){
    m_value.restore_defaults();
    m_box->setText(m_value.text());
}



template class TimeExpressionBaseWidget<uint16_t>;
template class TimeExpressionBaseWidget<uint32_t>;
template class TimeExpressionBaseWidget<int16_t>;
template class TimeExpressionBaseWidget<int32_t>;



}
}
