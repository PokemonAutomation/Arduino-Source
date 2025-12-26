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

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{


template class RegisterConfigWidget<FloatingPointCellWidget>;
template class RegisterConfigWidget<FloatingPointOptionWidget>;




FloatingPointCellWidget::~FloatingPointCellWidget(){
    m_value.remove_listener(*this);
}
FloatingPointCellWidget::FloatingPointCellWidget(QWidget& parent, FloatingPointCell& value, bool sanitize)
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
        this, [this, sanitize](){
            bool ok;
            double current = this->text().toDouble(&ok);
            QPalette palette;
            palette.setColor(QPalette::Text, Qt::black);
            if (sanitize){
                m_value.set_and_sanitize(current);
            }else{
                if (!ok || !m_value.check_validity(current).empty()){
                    palette.setColor(QPalette::Text, Qt::red);
                }
                m_value.set(current);
            }
            this->setPalette(palette);

            //  Always update the UI.
            on_config_value_changed(this);
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
    m_cell = new FloatingPointCellWidget(*this, value, false);
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
