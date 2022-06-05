/*  Switch Date Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QHBoxLayout>
#include <QLabel>
#include "SwitchDateBaseWidget.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


SwitchDateBaseWidget::SwitchDateBaseWidget(QWidget& parent, SwitchDateBaseOption& value)
    : QWidget(&parent)
    , m_value(value)
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    QLabel* text = new QLabel(value.label(), this);
    text->setWordWrap(true);
    layout->addWidget(text, 1);

    m_date_edit = new QDateEdit(m_value.get());
    layout->addWidget(m_date_edit, 1);
    m_date_edit->setDisplayFormat("MMMM d, yyyy");
    m_date_edit->setMinimumDate(QDate(2000, 1, 1));
    m_date_edit->setMaximumDate(QDate(2060, 12, 31));

    connect(
        m_date_edit, &QDateEdit::dateChanged,
        this, [=](const QDate& date){
            m_value.set(date);
        }
    );
}
void SwitchDateBaseWidget::restore_defaults(){
    m_value.restore_defaults();
    update_ui();
}
void SwitchDateBaseWidget::update_ui(){
    m_date_edit->setDate(m_value.get());
}



}
}
