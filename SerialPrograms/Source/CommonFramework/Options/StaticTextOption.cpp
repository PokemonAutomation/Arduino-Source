/*  Static Text
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QVBoxLayout>
#include <QFrame>
#include <QLabel>
#include "Common/Cpp/Json/JsonValue.h"
#include "StaticTextOption.h"

namespace PokemonAutomation{



class StaticTextWidget : public QWidget, public ConfigWidget{
public:
    StaticTextWidget(QWidget& parent, StaticTextOption& value);
    virtual void update_ui() override{}
    virtual void restore_defaults() override{}
};


class SectionDividerWidget : public QWidget, public ConfigWidget{
public:
    SectionDividerWidget(QWidget& parent, SectionDividerOption& value);
    virtual void update_ui() override{}
    virtual void restore_defaults() override{}
};




StaticTextOption::StaticTextOption(QString label)
    : m_label(std::move(label))
{}
void StaticTextOption::load_json(const JsonValue&){
}
JsonValue StaticTextOption::to_json() const{
    return JsonValue();
}
ConfigWidget* StaticTextOption::make_ui(QWidget& parent){
    return new StaticTextWidget(parent, *this);
}



SectionDividerOption::SectionDividerOption(QString label)
    : m_label(std::move(label))
{}
void SectionDividerOption::load_json(const JsonValue&){
}
JsonValue SectionDividerOption::to_json() const{
    return JsonValue();
}

ConfigWidget* SectionDividerOption::make_ui(QWidget& parent){
    return new SectionDividerWidget(parent, *this);
}




StaticTextWidget::StaticTextWidget(QWidget& parent, StaticTextOption& value)
    : QWidget(&parent)
    , ConfigWidget(value, *this)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    QLabel* text = new QLabel(value.m_label, this);
    text->setWordWrap(true);
    layout->addWidget(text);
//    text->setTextInteractionFlags(Qt::TextBrowserInteraction);
    text->setOpenExternalLinks(true);
}



SectionDividerWidget::SectionDividerWidget(QWidget& parent, SectionDividerOption& value)
    : QWidget(&parent)
    , ConfigWidget(value, *this)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 10, 0, 0);

    QFrame* frame = new QFrame(this);
    layout->addWidget(frame);
    frame->setFrameShape(QFrame::HLine);

    QLabel* text = new QLabel(value.m_label, this);
    text->setWordWrap(true);
    layout->addWidget(text);
//    text->setTextInteractionFlags(Qt::TextBrowserInteraction);
    text->setOpenExternalLinks(true);
}






}
