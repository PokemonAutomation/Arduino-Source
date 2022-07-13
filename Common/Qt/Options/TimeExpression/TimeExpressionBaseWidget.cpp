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
    layout->setContentsMargins(0, 0, 0, 0);
    QLabel* text = new QLabel(QString::fromStdString(m_value.label()), this);
    text->setWordWrap(true);
    layout->addWidget(text, 1);
    QVBoxLayout* rows = new QVBoxLayout();
    layout->addLayout(rows, 1);

    QHBoxLayout* row0 = new QHBoxLayout();
    rows->addLayout(row0);
    QHBoxLayout* row1 = new QHBoxLayout();
    rows->addLayout(row1);

    m_box = new QLineEdit(QString::fromStdString(m_value.text()), this);
    row0->addWidget(m_box);
    row0->addWidget(new QLabel("ticks", this));

    QLabel* seconds = new QLabel(QString::fromStdString(m_value.time_string()), this);
    seconds->setAlignment(Qt::AlignHCenter);
    row1->addWidget(seconds);

    connect(
        m_box, &QLineEdit::textChanged,
        this, [=](const QString& text){
//           cout << text.toStdString() << endl;
            std::string error = m_value.set(text.toStdString());
            if (error.empty()){
                seconds->setText(QString::fromStdString(m_value.time_string()));
            }else{
                seconds->setText(QString::fromStdString("<font color=\"red\">" + error + "</font>"));
            }
        }
    );
    connect(
        m_box, &QLineEdit::editingFinished,
        this, [=](){
            m_box->setText(QString::fromStdString(m_value.text()));
            seconds->setText(QString::fromStdString(m_value.time_string()));
        }
    );
}
template <typename Type>
void TimeExpressionBaseWidget<Type>::restore_defaults(){
    m_value.restore_defaults();
    update_ui();
}
template <typename Type>
void TimeExpressionBaseWidget<Type>::update_ui(){
    m_box->setText(QString::fromStdString(m_value.text()));
}



template class TimeExpressionBaseWidget<uint16_t>;
template class TimeExpressionBaseWidget<uint32_t>;
template class TimeExpressionBaseWidget<int16_t>;
template class TimeExpressionBaseWidget<int32_t>;



}
}
