/*  Floating-Point Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include "ConfigWidget.h"
#include "FloatingPointWidget.h"

namespace PokemonAutomation{



ConfigWidget* FloatingPointCell::make_QtWidget(QWidget& parent){
    return new FloatingPointCellWidget(parent, *this);
}
ConfigWidget* FloatingPointOption::make_QtWidget(QWidget& parent){
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
        this, [this](const QString& text){
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
        this, [this](){
            bool ok;
            double current = this->text().toDouble(&ok);
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
void FloatingPointCellWidget::update_value(){
    this->setText(QString::number(m_value, 'f'));
}
void FloatingPointCellWidget::on_config_value_changed(void* object){
    QMetaObject::invokeMethod(this, [this]{
        update_value();
    }, Qt::QueuedConnection);
}





FloatingPointOptionWidget::~FloatingPointOptionWidget(){
    m_value.remove_listener(*this);
}
FloatingPointOptionWidget::FloatingPointOptionWidget(QWidget& parent, FloatingPointOption& value)
    : QWidget(&parent)
    , ConfigWidget(value, *this)
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    QLabel* text = new QLabel(QString::fromStdString(value.label()), this);
    text->setWordWrap(true);
    text->setTextFormat(Qt::RichText);
    text->setTextInteractionFlags(Qt::TextBrowserInteraction);
    text->setOpenExternalLinks(true);
    layout->addWidget(text, 1);
    m_cell = new FloatingPointCellWidget(*this, value);
    layout->addWidget(m_cell, 1);
    value.add_listener(*this);
}
void FloatingPointOptionWidget::update_value(){
    m_cell->update_value();
}
void FloatingPointOptionWidget::on_config_value_changed(void* object){
    QMetaObject::invokeMethod(this, [this]{
        update_value();
    }, Qt::QueuedConnection);
}




}
