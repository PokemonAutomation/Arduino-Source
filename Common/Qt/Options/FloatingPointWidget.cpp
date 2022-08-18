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



ConfigWidget* FloatingPointCell::make_ui(QWidget& parent){
    return new FloatingPointCellWidget(parent, *this);
}
ConfigWidget* FloatingPointOption::make_ui(QWidget& parent){
    return new FloatingPointOptionWidget(parent, *this);
}




FloatingPointCellWidget::~FloatingPointCellWidget(){
    m_value.remove_listener(*this);
}
FloatingPointCellWidget::FloatingPointCellWidget(QWidget& parent, FloatingPointCell& value)
    : QLineEdit(QString::number(value, 'f'), &parent)
    , ConfigWidget(value, *this)
    , m_value(value)
{
    connect(
        this, &QLineEdit::textChanged,
        this, [=](const QString& text){
            bool ok;
            double current = text.toDouble(&ok);
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
        this, [=](){
            bool ok;
            double current = this->text().toDouble(&ok);
            QPalette palette;
            if (ok && m_value.check_validity(current).empty()){
                palette.setColor(QPalette::Text, Qt::black);
            }else{
                palette.setColor(QPalette::Text, Qt::red);
            }
            this->setPalette(palette);

            if (current == m_value){
                return;
            }

            m_value.set(current);
        }
    );
    value.add_listener(*this);
}
void FloatingPointCellWidget::update(){
    ConfigWidget::update();
    this->setText(QString::number(m_value, 'f'));
}
void FloatingPointCellWidget::value_changed(){
    QMetaObject::invokeMethod(this, [=]{
        update();
    }, Qt::QueuedConnection);
}





FloatingPointOptionWidget::FloatingPointOptionWidget(QWidget& parent, FloatingPointOption& value)
    : QWidget(&parent)
    , ConfigWidget(value, *this)
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    QLabel* text = new QLabel(QString::fromStdString(value.label()), this);
    text->setWordWrap(true);
    layout->addWidget(text, 1);
    m_cell = new FloatingPointCellWidget(*this, value);
    layout->addWidget(m_cell, 1);
}
void FloatingPointOptionWidget::update(){
    ConfigWidget::update();
    m_cell->update();
}




}
