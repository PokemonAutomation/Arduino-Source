/*  Floating-Point Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include "ConfigWidget.h"
#include "FloatingPointWidget.h"

namespace PokemonAutomation{



ConfigWidget* FloatingPointOption::make_ui(QWidget& parent){
    return new FloatingPointWidget(parent, *this);
}



FloatingPointWidget::~FloatingPointWidget(){
    m_value.remove_listener(*this);
}
FloatingPointWidget::FloatingPointWidget(QWidget& parent, FloatingPointOption& value)
    : QWidget(&parent)
    , ConfigWidget(value, *this)
    , m_value(value)
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    QLabel* text = new QLabel(QString::fromStdString(value.label()), this);
    text->setWordWrap(true);
    layout->addWidget(text, 1);
    m_box = new QLineEdit(QString::number(m_value, 'f'), this);
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
            double current = m_box->text().toDouble(&ok);
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
void FloatingPointWidget::restore_defaults(){
    m_value.restore_defaults();
}
void FloatingPointWidget::update_ui(){
    m_box->setText(QString::number(m_value, 'f'));
}
void FloatingPointWidget::value_changed(){
    QMetaObject::invokeMethod(m_box, [=]{
        update_ui();
    }, Qt::QueuedConnection);
}




}
