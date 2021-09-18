/*  Section Divider
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QVBoxLayout>
#include <QFrame>
#include <QLabel>
#include "SectionDivider.h"

namespace PokemonAutomation{


SectionDividerOption::SectionDividerOption(QString label)
    : m_label(std::move(label))
{}

ConfigOptionUI* SectionDividerOption::make_ui(QWidget& parent){
    return new SectionDividerOptionUI(parent, *this);
}

SectionDividerOptionUI::SectionDividerOptionUI(QWidget& parent, SectionDividerOption& value)
    : QWidget(&parent)
{
    QVBoxLayout* layout = new QVBoxLayout(this);

    QFrame* frame = new QFrame(this);
    layout->addWidget(frame);
    frame->setFrameShape(QFrame::HLine);

    QLabel* text = new QLabel(value.m_label, this);
    layout->addWidget(text);
    text->setWordWrap(true);
}



}
