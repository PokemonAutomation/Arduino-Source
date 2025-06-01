/*  Virtual Game Console
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QVBoxLayout>
#include <QScrollArea>
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Qt/CollapsibleGroupBox.h"
#include "ML_LabelImages.h"
#include "Pokemon/Pokemon_Strings.h"

namespace PokemonAutomation{
namespace ML{

LabelImages_Descriptor::LabelImages_Descriptor()
    : PanelDescriptor(
        Color(),
        "ML:LabelImages",
        "ML", "Label Images",
        "", // "ComputerControl/blob/master/Wiki/Programs/NintendoSwitch/LabelImages.md",
        "Label " + Pokemon::STRING_POKEMON + " on images" 
    )
{}



LabelImages::LabelImages(const LabelImages_Descriptor& descriptor)
    : PanelInstance(descriptor)
    // , m_switch({}, false)
{}
void LabelImages::from_json(const JsonValue& json){
    // m_switch.load_json(json);
}
JsonValue LabelImages::to_json() const{
    return {};
    // return m_switch.to_json();
}
QWidget* LabelImages::make_widget(QWidget& parent, PanelHolder& holder){
    return LabelImages_Widget::make(parent, *this, holder);
}



LabelImages_Widget* LabelImages_Widget::make(
    QWidget& parent,
    LabelImages& instance,
    PanelHolder& holder
){
    LabelImages_Widget* widget = new LabelImages_Widget(parent, instance, holder);
    widget->construct();
    return widget;
}
LabelImages_Widget::~LabelImages_Widget(){
    // delete m_switch;
}
LabelImages_Widget::LabelImages_Widget(
    QWidget& parent,
    LabelImages& instance,
    PanelHolder& holder
)
    : PanelWidget(parent, instance, holder)
    // , m_session(instance.m_switch, 0, 0)
{}
void LabelImages_Widget::construct(){
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(make_header(*this));

    QScrollArea* scroll_outer = new QScrollArea(this);
    layout->addWidget(scroll_outer);
    scroll_outer->setWidgetResizable(true);

    QWidget* scroll_inner = new QWidget(scroll_outer);
    scroll_outer->setWidget(scroll_inner);
    QVBoxLayout* scroll_layout = new QVBoxLayout(scroll_inner);
    scroll_layout->setAlignment(Qt::AlignTop);

    // m_switch = new SwitchSystemWidget(*this, m_session, 0);
    // scroll_layout->addWidget(m_switch);
}





}
}

