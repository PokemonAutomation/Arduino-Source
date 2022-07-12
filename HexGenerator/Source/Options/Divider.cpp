/*  Divider
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QHBoxLayout>
#include <QLabel>
#include <QCheckBox>
#include "Tools/Tools.h"
#include "Divider.h"

namespace PokemonAutomation{


const std::string Divider::OPTION_TYPE = "Divider";
const std::string Divider::JSON_TEXT = "Text";

int Divider_init = register_option(
    Divider::OPTION_TYPE,
        [](const JsonObject& obj){
        return std::unique_ptr<ConfigItem>(
            new Divider(obj)
        );
    }
);


Divider::Divider(const JsonObject& obj)
    : ConfigItem(obj)
{}
QWidget* Divider::make_ui(QWidget& parent){
    return new DividerUI(parent, m_label);
}


DividerUI::DividerUI(QWidget& parent, const std::string& label)
    : QWidget(&parent)
{
    QVBoxLayout* layout = new QVBoxLayout(this);

    QFrame* frame = new QFrame(this);
    layout->addWidget(frame);
    frame->setFrameShape(QFrame::HLine);

    QLabel* text = new QLabel(QString::fromStdString(label), this);
    layout->addWidget(text);
    text->setWordWrap(true);
}
DividerUI::~DividerUI(){

}



}
