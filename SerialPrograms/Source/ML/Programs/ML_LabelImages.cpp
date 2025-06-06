/*  ML Label Images
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
#include <fstream>
#include <filesystem>
#include <cmath>
#include "CommonFramework/Globals.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Qt/CollapsibleGroupBox.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "NintendoSwitch/Framework/UI/NintendoSwitch_SwitchSystemWidget.h"
#include "CommonFramework/VideoPipeline/Backends/CameraWidgetQt6.5.h"
#include "CommonFramework/VideoPipeline/VideoSources/VideoSource_StillImage.h"
#include "ML_LabelImages.h"
#include "Pokemon/Pokemon_Strings.h"
#include "Common/Qt/Options/ConfigWidget.h"
#include "ML/DataLabeling/SegmentAnythingModel.h"


using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace ML{


DrawnBoundingBox::DrawnBoundingBox(LabelImages& label_program, VideoOverlay& overlay)
    : m_program(label_program)
    , m_overlay(overlay)
    , m_overlay_set(overlay)
{
    m_program.X.add_listener(*this);
    m_program.Y.add_listener(*this);
    m_program.WIDTH.add_listener(*this);
    m_program.HEIGHT.add_listener(*this);
    overlay.add_listener(*this);
}

DrawnBoundingBox::~DrawnBoundingBox(){
    detach();
}

void DrawnBoundingBox::on_config_value_changed(void* object){
    std::lock_guard<std::mutex> lg(m_lock);
    m_overlay_set.clear();
    m_overlay_set.add(COLOR_RED, {m_program.X, m_program.Y, m_program.WIDTH, m_program.HEIGHT}, "Unknown");
}
void DrawnBoundingBox::on_mouse_press(double x, double y){
    m_program.WIDTH.set(0);
    m_program.HEIGHT.set(0);
    m_program.X.set(x);
    m_program.Y.set(y);
    m_mouse_start.emplace();
    m_mouse_start->first = x;
    m_mouse_start->second = y;
}
void DrawnBoundingBox::on_mouse_release(double, double){
    m_mouse_start.reset();

    const size_t source_width = m_program.source_image_width;
    const size_t source_height = m_program.source_image_height;

    const int box_x = int(m_program.X * source_width + 0.5);
    const int box_y = int(m_program.Y * source_height + 0.5);
    const int box_width = int(m_program.WIDTH * source_width + 0.5);
    const int box_height = int(m_program.HEIGHT * source_height + 0.5);
    if (box_width == 0 || box_height == 0){
        return;
    }

    m_program.m_sam_session.run(
        m_program.m_image_embedding,
        (int)source_height, (int)source_width, {}, {},
        {box_x, box_y, box_x + box_width, box_y + box_height},
        m_program.m_output_boolean_mask
    );

    for (size_t y = 0; y < source_height; y++){
        for (size_t x = 0; x < source_width; x++){
            uint32_t& pixel = m_program.m_mask_image.pixel(x, y);
            // if the pixel's mask value is true, set a semi-transparent 45-degree blue strip color
            // otherwise: fully transparent (alpha = 0)
            uint32_t color = 0;
            if (m_program.m_output_boolean_mask[y*source_width + x]){
                color = (std::abs(int(x) - int(y)) % 4 <= 1) ? combine_argb(150, 30, 144, 255) : combine_argb(150, 0, 0, 60);
            }
            pixel = color;
        }
    }
    if (m_program.m_overlay_image){
        m_overlay.remove_image(*m_program.m_overlay_image);
    }
    m_program.m_overlay_image = std::make_unique<OverlayImage>(m_program.m_mask_image, 0.0, 0.0, 1.0, 1.0);
    m_overlay.add_image(*m_program.m_overlay_image);
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

    m_program.X.set(xl);
    m_program.Y.set(yl);
    m_program.WIDTH.set(xh - xl);
    m_program.HEIGHT.set(yh - yl);
}

void DrawnBoundingBox::detach(){
    m_overlay.remove_listener(*this);
    m_program.X.remove_listener(*this);
    m_program.Y.remove_listener(*this);
    m_program.WIDTH.remove_listener(*this);
    m_program.HEIGHT.remove_listener(*this);
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
    , m_sam_session{RESOURCE_PATH() + "ML/sam_cpu.onnx"}
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
    , m_program(instance)
    , m_session(instance.m_switch_control_option, 0, 0)
    , m_drawn_box(instance, m_session.overlay())
{
    std::cout << &m_program << std::endl;

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
    connect(button, &QPushButton::clicked, this, [&instance](bool){
        cout << "Button clicked!" << endl;
        const VideoSourceDescriptor* video_source = instance.m_switch_control_option.m_video.descriptor().get();
        auto image_source = dynamic_cast<const VideoSourceDescriptor_StillImage*>(video_source);
        if (image_source != nullptr){
            cout << "Image source: " << image_source->path() << endl;
        }
    });

    m_option_widget = instance.m_options.make_QtWidget(*scroll_inner);
    scroll_layout->addWidget(&m_option_widget->widget());

    const VideoSourceDescriptor* video_source_desc = instance.m_switch_control_option.m_video.descriptor().get();
    auto image_source_desc = dynamic_cast<const VideoSourceDescriptor_StillImage*>(video_source_desc);
    if (image_source_desc != nullptr){
        const std::string image_path = image_source_desc->path();
        const size_t source_image_height = image_source_desc->source_image_height();
        const size_t source_image_width = image_source_desc->source_image_width();
        m_program.source_image_height = source_image_height;
        m_program.source_image_width = source_image_width;
        m_program.m_mask_image = ImageRGB32(source_image_width, source_image_height);
        cout << "Image source: " << image_path << ", " << source_image_width << " x " << source_image_height << endl;
        // if no such embedding file, m_iamge_embedding will be empty
        load_image_embedding(image_path, m_program.m_image_embedding);
    }

    // m_overlay_image = std::make_unique<OverlayImage>(*m_image_mask, 0.0, 0.2, 0.5, 0.5);
    // m_session.overlay().add_image(*m_overlay_image);    
    cout << "LabelImages_Widget built" << endl;
}




}
}

