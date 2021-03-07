/*  Divider
 *
 *  From: https://github.com/Mysticial/Pokemon-Automation-SwSh-Arduino-Scripts
 *
 */

#include <QJsonObject>
#include <QHBoxLayout>
#include <QLabel>
#include <QCheckBox>
#include "Tools/Tools.h"
#include "Divider.h"

namespace PokemonAutomation{


const QString Divider::OPTION_TYPE = "Divider";
const QString Divider::JSON_TEXT = "Text";

int Divider_init = register_option(
    Divider::OPTION_TYPE,
        [](const QJsonObject& obj){
        return std::unique_ptr<ConfigItem>(
            new Divider(obj)
        );
    }
);


Divider::Divider(const QJsonObject& obj)
    : ConfigItem(obj)
{}
QWidget* Divider::make_ui(QWidget& parent){
    return new DividerUI(parent, m_label);
}


DividerUI::DividerUI(QWidget& parent, const QString& label)
    : QWidget(&parent)
{
    QVBoxLayout* layout = new QVBoxLayout(this);

    QFrame* frame = new QFrame(this);
    layout->addWidget(frame);
    frame->setFrameShape(QFrame::HLine);

    QLabel* text = new QLabel(label, this);
    layout->addWidget(text);
    text->setWordWrap(true);
}
DividerUI::~DividerUI(){

}



}
