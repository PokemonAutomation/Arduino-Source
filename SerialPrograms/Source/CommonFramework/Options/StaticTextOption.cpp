/*  Static Text
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QVBoxLayout>
#include <QFrame>
#include <QLabel>
#include "StaticTextOption.h"

namespace PokemonAutomation{


StaticTextOption::StaticTextOption(QString label)
    : m_label(std::move(label))
{}

ConfigOptionUI* StaticTextOption::make_ui(QWidget& parent){
    return new StaticTextOptionUI(parent, *this);
}
StaticTextOptionUI::StaticTextOptionUI(QWidget& parent, StaticTextOption& value)
    : QWidget(&parent)
    , ConfigOptionUI(value, *this)
{
    QVBoxLayout* layout = new QVBoxLayout(this);

    QLabel* text = new QLabel(value.m_label, this);
    layout->addWidget(text);
    text->setWordWrap(true);
//    text->setTextInteractionFlags(Qt::TextBrowserInteraction);
    text->setOpenExternalLinks(true);
}


SectionDividerOption::SectionDividerOption(QString label)
    : m_label(std::move(label))
{}

ConfigOptionUI* SectionDividerOption::make_ui(QWidget& parent){
    return new SectionDividerOptionUI(parent, *this);
}
SectionDividerOptionUI::SectionDividerOptionUI(QWidget& parent, SectionDividerOption& value)
    : QWidget(&parent)
    , ConfigOptionUI(value, *this)
{
    QVBoxLayout* layout = new QVBoxLayout(this);

    QFrame* frame = new QFrame(this);
    layout->addWidget(frame);
    frame->setFrameShape(QFrame::HLine);

    QLabel* text = new QLabel(value.m_label, this);
    layout->addWidget(text);
    text->setWordWrap(true);
//    text->setTextInteractionFlags(Qt::TextBrowserInteraction);
    text->setOpenExternalLinks(true);
}



}
