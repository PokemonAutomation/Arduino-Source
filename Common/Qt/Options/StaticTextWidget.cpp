/*  Static Text Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QVBoxLayout>
#include <QFrame>
#include <QLabel>
#include "StaticTextWidget.h"

namespace PokemonAutomation{



ConfigWidget* StaticTextOption::make_QtWidget(QWidget& parent){
    return new StaticTextWidget(parent, *this);
}
ConfigWidget* SectionDividerOption::make_QtWidget(QWidget& parent){
    return new SectionDividerWidget(parent, *this);
}




StaticTextWidget::~StaticTextWidget(){
    m_value.remove_listener(*this);
}
StaticTextWidget::StaticTextWidget(QWidget& parent, StaticTextOption& value)
    : QWidget(&parent)
    , ConfigWidget(value, *this)
    , m_value(value)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    m_text = new QLabel(QString::fromStdString(value.text()), this);
    m_text->setWordWrap(value.text_wrapping());
    layout->addWidget(m_text);
//    text->setTextInteractionFlags(Qt::TextBrowserInteraction);
    m_text->setOpenExternalLinks(true);

    m_value.add_listener(*this);
}
void StaticTextWidget::update_value(){
    m_text->setText(QString::fromStdString(m_value.text()));
}
void StaticTextWidget::on_config_value_changed(void* object){
    QMetaObject::invokeMethod(m_text, [this]{
        update_value();
    }, Qt::QueuedConnection);
}


SectionDividerWidget::~SectionDividerWidget(){
    m_value.remove_listener(*this);
}
SectionDividerWidget::SectionDividerWidget(QWidget& parent, SectionDividerOption& value)
    : QWidget(&parent)
    , ConfigWidget(value, *this)
    , m_value(value)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 10, 0, 0);

    QFrame* frame = new QFrame(this);
    layout->addWidget(frame);
    frame->setFrameShape(QFrame::HLine);

    m_text = new QLabel(QString::fromStdString(value.text()), this);
    m_text->setWordWrap(value.text_wrapping());
    layout->addWidget(m_text);
//    text->setTextInteractionFlags(Qt::TextBrowserInteraction);
    m_text->setOpenExternalLinks(true);

    m_value.add_listener(*this);
}
void SectionDividerWidget::update_value(){
    m_text->setText(QString::fromStdString(m_value.text()));
}
void SectionDividerWidget::on_config_value_changed(void* object){
    QMetaObject::invokeMethod(m_text, [this]{
        update_value();
    }, Qt::QueuedConnection);
}





}
