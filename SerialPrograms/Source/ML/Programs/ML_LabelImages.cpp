/*  Virtual Game Console
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QLabel>
#include <QDir>
#include <QVBoxLayout>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QScrollArea>
#include <QPushButton>
#include <QResizeEvent>
#include <iostream>
#include "Common/Cpp/Json/JsonObject.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Qt/CollapsibleGroupBox.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "NintendoSwitch/Framework/UI/NintendoSwitch_SwitchSystemWidget.h"
#include "CommonFramework/VideoPipeline/Backends/CameraWidgetQt6.5.h"
#include "ML_LabelImages.h"
#include "Pokemon/Pokemon_Strings.h"
#include "Common/Qt/Options/ConfigWidget.h"


using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace ML{


DrawnBoundingBox::DrawnBoundingBox(LabelImages& parent, VideoOverlay& overlay)
    : m_parent(parent)
    , m_overlay(overlay)
    , m_overlay_set(overlay)
{
    m_parent.X.add_listener(*this);
    m_parent.Y.add_listener(*this);
    m_parent.WIDTH.add_listener(*this);
    m_parent.HEIGHT.add_listener(*this);
    overlay.add_listener(*this);
}

DrawnBoundingBox::~DrawnBoundingBox(){
    detach();
}

void DrawnBoundingBox::on_config_value_changed(void* object){
    std::lock_guard<std::mutex> lg(m_lock);
    m_overlay_set.clear();
    m_overlay_set.add(COLOR_RED, {m_parent.X, m_parent.Y, m_parent.WIDTH, m_parent.HEIGHT});
}
void DrawnBoundingBox::on_mouse_press(double x, double y){
    m_parent.WIDTH.set(0);
    m_parent.HEIGHT.set(0);
    m_parent.X.set(x);
    m_parent.Y.set(y);
    m_mouse_start.emplace();
    m_mouse_start->first = x;
    m_mouse_start->second = y;
}
void DrawnBoundingBox::on_mouse_release(double x, double y){
    m_mouse_start.reset();
}
void DrawnBoundingBox::on_mouse_move(double x, double y){
    if (!m_mouse_start){
        return;
    }

    double xl = m_mouse_start->first;
    double xh = x;
    double yl = m_mouse_start->second;
    double yh = y;

    if (xl > xh){
        std::swap(xl, xh);
    }
    if (yl > yh){
        std::swap(yl, yh);
    }

    m_parent.X.set(xl);
    m_parent.Y.set(yl);
    m_parent.WIDTH.set(xh - xl);
    m_parent.HEIGHT.set(yh - yl);
}

void DrawnBoundingBox::detach(){
    m_overlay.remove_listener(*this);
    m_parent.X.remove_listener(*this);
    m_parent.Y.remove_listener(*this);
    m_parent.WIDTH.remove_listener(*this);
    m_parent.HEIGHT.remove_listener(*this);
}


LabelImages_Descriptor::LabelImages_Descriptor()
    : PanelDescriptor(
        Color(),
        "ML:LabelImages",
        "ML", "Label Images",
        "", // "ComputerControl/blob/master/Wiki/Programs/NintendoSwitch/LabelImages.md",
        "Label " + Pokemon::STRING_POKEMON + " on images" 
    )
{}


#define ADD_OPTION(x)    m_options.add_option(x, #x)

LabelImages::LabelImages(const LabelImages_Descriptor& descriptor)
    : PanelInstance(descriptor)
    , m_switch_control_option({}, false)
    , m_options(LockMode::UNLOCK_WHILE_RUNNING)
    , X("<b>X Coordinate:</b>", LockMode::UNLOCK_WHILE_RUNNING, 0.3, 0.0, 1.0)
    , Y("<b>Y Coordinate:</b>", LockMode::UNLOCK_WHILE_RUNNING, 0.3, 0.0, 1.0)
    , WIDTH("<b>Width:</b>", LockMode::UNLOCK_WHILE_RUNNING, 0.4, 0.0, 1.0)
    , HEIGHT("<b>Height:</b>", LockMode::UNLOCK_WHILE_RUNNING, 0.4, 0.0, 1.0)
{
    ADD_OPTION(X);
    ADD_OPTION(Y);
    ADD_OPTION(WIDTH);
    ADD_OPTION(HEIGHT);
}
void LabelImages::from_json(const JsonValue& json){
    const JsonObject* obj = json.to_object();
    if (obj == nullptr){
        return;
    }
    const JsonValue* value = obj->get_value("SwitchSetup");
    if (value){
        m_switch_control_option.load_json(*value);
    }
    m_options.load_json(json);
}
JsonValue LabelImages::to_json() const{
    JsonObject obj = std::move(*m_options.to_json().to_object());
    obj["SwitchSetup"] = m_switch_control_option.to_json();
    return obj;
}
QWidget* LabelImages::make_widget(QWidget& parent, PanelHolder& holder){
    return new LabelImages_Widget(parent, *this, holder);
}


LabelImages_Widget::~LabelImages_Widget(){
    delete m_switch_widget;
}
LabelImages_Widget::LabelImages_Widget(
    QWidget& parent,
    LabelImages& instance,
    PanelHolder& holder
)
    : PanelWidget(parent, instance, holder)
    , m_session(instance.m_switch_control_option, 0, 0)
    , m_drawn_box(instance, m_session.overlay())
{
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

    m_switch_widget = new NintendoSwitch::SwitchSystemWidget(*this, m_session, 0);
    scroll_layout->addWidget(m_switch_widget);

    QPushButton* button = new QPushButton("This is a button", scroll_inner);
    scroll_layout->addWidget(button);

    m_option_widget = instance.m_options.make_QtWidget(*scroll_inner);
    scroll_layout->addWidget(&m_option_widget->widget());
}




}
}

