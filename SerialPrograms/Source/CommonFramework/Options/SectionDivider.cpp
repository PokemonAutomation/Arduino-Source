/*  Section Divider
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QVBoxLayout>
#include <QFrame>
#include <QLabel>
#include "SectionDivider.h"


SectionDivider::SectionDivider(QString label)
    : m_label(std::move(label))
{}

ConfigOptionUI* SectionDivider::make_ui(QWidget& parent){
    return new SectionDividerUI(parent, *this);
}

SectionDividerUI::SectionDividerUI(QWidget& parent, SectionDivider& value)
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

