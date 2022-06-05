/*  Floating-Point Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *      This option is thread-safe.
 *
 */

#include <QHBoxLayout>
#include <QLabel>
//#include <QDoubleValidator>
#include "FloatingPointBaseWidget.h"

namespace PokemonAutomation{


FloatingPointBaseWidget::FloatingPointBaseWidget(QWidget& parent, FloatingPointBaseOption& value)
    : QWidget(&parent)
    , m_value(value)
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    QLabel* text = new QLabel(value.label(), this);
    text->setWordWrap(true);
    layout->addWidget(text, 1);
    m_box = new QLineEdit(QString::number(m_value, 'f', 2), this);
//    box->setInputMask("999999999");
//    QDoubleValidator* validator = new QDoubleValidator(value.min_value(), value.max_value(), 2, this);
//    m_box->setValidator(validator);
    layout->addWidget(m_box, 1);
    connect(
        m_box, &QLineEdit::textChanged,
        this, [=](const QString& text){
            bool ok;
            double current = text.toDouble(&ok);
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
            m_box->setText(QString::number(m_value, 'f', 2));
        }
    );
}
void FloatingPointBaseWidget::restore_defaults(){
    m_value.restore_defaults();
    update_ui();
}
void FloatingPointBaseWidget::update_ui(){
    m_box->setText(QString::number(m_value, 'f', 2));
}


}
