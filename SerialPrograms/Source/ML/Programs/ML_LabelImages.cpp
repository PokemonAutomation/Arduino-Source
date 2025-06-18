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
#include <QMessageBox>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <cmath>
#include "CommonFramework/Globals.h"
#include "Common/Cpp/Json/JsonArray.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonTools.h"
#include "Common/Qt/CollapsibleGroupBox.h"
#include "Pokemon/Resources/Pokemon_PokemonForms.h"
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


ObjectAnnotation::ObjectAnnotation(): user_box(0,0,0,0), mask_box(0,0,0,0) {}

// if failed to pass, will throw JsonParseException
ObjectAnnotation json_to_object_annotation(const JsonValue& value){
    ObjectAnnotation anno_obj;

    const JsonObject& json_obj = value.to_object_throw();
    const JsonArray& user_box_array = json_obj.get_array_throw("UserBox");
    anno_obj.user_box = ImagePixelBox(
        size_t(user_box_array[0].to_integer_throw()),
        size_t(user_box_array[1].to_integer_throw()),
        size_t(user_box_array[2].to_integer_throw()),
        size_t(user_box_array[3].to_integer_throw())
    );
    const JsonArray& mask_box_array = json_obj.get_array_throw("MaskBox");
    anno_obj.mask_box = ImagePixelBox(
        size_t(mask_box_array[0].to_integer_throw()),
        size_t(mask_box_array[1].to_integer_throw()),
        size_t(mask_box_array[2].to_integer_throw()),
        size_t(mask_box_array[3].to_integer_throw())
    );
    size_t mask_width = anno_obj.mask_box.width(), mask_height = anno_obj.mask_box.height();
    anno_obj.mask.resize(mask_width * mask_height);
    const JsonArray& mask_values = json_obj.get_array_throw("Mask");
    for(size_t i = 0; i < anno_obj.mask.size(); i++){
        anno_obj.mask[i] = bool(mask_values[i].to_integer_throw());
    }

    anno_obj.label = json_obj.get_string_throw("Label");
    
    return anno_obj;
}

JsonObject object_annotation_to_json(const ObjectAnnotation& object_annotation){
    JsonObject json_obj;
    JsonArray user_box_arr;
    user_box_arr.push_back(int64_t(object_annotation.user_box.min_x));
    user_box_arr.push_back(int64_t(object_annotation.user_box.min_y));
    user_box_arr.push_back(int64_t(object_annotation.user_box.max_x));
    user_box_arr.push_back(int64_t(object_annotation.user_box.max_y));
    json_obj["UserBox"] = std::move(user_box_arr);

    JsonArray mask_box_arr;
    mask_box_arr.push_back(int64_t(object_annotation.mask_box.min_x));
    mask_box_arr.push_back(int64_t(object_annotation.mask_box.min_y));
    mask_box_arr.push_back(int64_t(object_annotation.mask_box.max_x));
    mask_box_arr.push_back(int64_t(object_annotation.mask_box.max_y));
    json_obj["MaskBox"] = std::move(mask_box_arr);

    JsonArray mask_arr;
    for(size_t i = 0; i < object_annotation.mask.size(); i++){
        mask_arr.push_back(int64_t(object_annotation.mask[i]));
    }
    json_obj["Mask"] = std::move(mask_arr);

    json_obj["Label"] = object_annotation.label;

    return json_obj;
}


DrawnBoundingBox::DrawnBoundingBox(LabelImages_Widget& widget, VideoOverlay& overlay)
    : m_widget(widget)
    , m_overlay(overlay)
{
    auto& program = m_widget.m_program;
    program.X.add_listener(*this);
    program.Y.add_listener(*this);
    program.WIDTH.add_listener(*this);
    program.HEIGHT.add_listener(*this);
    overlay.add_listener(*this);
}

DrawnBoundingBox::~DrawnBoundingBox(){
    detach();
}

// called when drawn bounding box changed
void DrawnBoundingBox::on_config_value_changed(void* object){
    auto& program = m_widget.m_program;
    std::lock_guard<std::mutex> lg(m_lock);
    program.update_rendered_objects(m_widget.m_overlay_set);
}
void DrawnBoundingBox::on_mouse_press(double x, double y){
    auto& program = m_widget.m_program;
    program.WIDTH.set(0);
    program.HEIGHT.set(0);
    program.X.set(x);
    program.Y.set(y);
    m_mouse_start.emplace();
    m_mouse_start->first = x;
    m_mouse_start->second = y;
}
void DrawnBoundingBox::on_mouse_release(double, double){
    m_mouse_start.reset();
    auto& m_program = m_widget.m_program;
    auto& m_overlay_set = m_widget.m_overlay_set;

    m_program.compute_mask(m_overlay_set);
}

void DrawnBoundingBox::on_mouse_move(double x, double y){
    auto& program = m_widget.m_program;
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

    program.X.set(xl);
    program.Y.set(yl);
    program.WIDTH.set(xh - xl);
    program.HEIGHT.set(yh - yl);
}

void DrawnBoundingBox::detach(){
    auto& program = m_widget.m_program;
    m_overlay.remove_listener(*this);
    program.X.remove_listener(*this);
    program.Y.remove_listener(*this);
    program.WIDTH.remove_listener(*this);
    program.HEIGHT.remove_listener(*this);
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
    , FORM_LABEL("bulbasaur")
    , m_sam_session{RESOURCE_PATH() + "ML/sam_cpu.onnx"}
{
    ADD_OPTION(X);
    ADD_OPTION(Y);
    ADD_OPTION(WIDTH);
    ADD_OPTION(HEIGHT);
    ADD_OPTION(FORM_LABEL);
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

    // m_annotation_file_path
    if (m_annotation_file_path.size() > 0 && !m_fail_to_load_annotation_file){
        JsonArray anno_json_arr;
        for(const auto& anno_obj: m_annotated_objects){
            anno_json_arr.push_back(object_annotation_to_json(anno_obj));
        }
        cout << "Saving annotation to " << m_annotation_file_path << endl;
        anno_json_arr.dump(m_annotation_file_path);
    }
    return obj;
}
QWidget* LabelImages::make_widget(QWidget& parent, PanelHolder& holder){
    return new LabelImages_Widget(parent, *this, holder);
}

void LabelImages::load_image_related_data(const std::string& image_path, size_t source_image_width, size_t source_image_height){
    this->source_image_height = source_image_height;
    this->source_image_width = source_image_width;

    m_mask_image = ImageRGB32(source_image_width, source_image_height);
    cout << "Image source: " << image_path << ", " << source_image_width << " x " << source_image_height << endl;
    // if no such embedding file, m_iamge_embedding will be empty
    const bool embedding_loaded = load_image_embedding(image_path, m_image_embedding);
    if (!embedding_loaded){
        return; // no embedding, then no way for us to annotate
    }
    // see if we can load the previously created labels
    const std::string anno_filename = std::filesystem::path(image_path).filename().replace_extension(".json").string();

    // ensure the folder exists
    std::filesystem::create_directory(ML_ANNOTATION_PATH());
    m_annotation_file_path = ML_ANNOTATION_PATH() + anno_filename;
    if (!std::filesystem::exists(m_annotation_file_path)){
        cout << "Annotataion output path, " << m_annotation_file_path << " does not exist yet" << endl;
        return;
    }
    std::string json_content;
    const bool anno_loaded = file_to_string(m_annotation_file_path, json_content);
    if (!anno_loaded){
        m_fail_to_load_annotation_file = true;
        QMessageBox box;
        box.warning(nullptr, "Unable to Load Annotation",
            QString::fromStdString("Cannot open annotation file " + m_annotation_file_path + ". Probably wrong permission?"));
        return;
    }

    JsonValue loaded_json = parse_json(json_content);
    const JsonArray* json_array = loaded_json.to_array();
    if (json_array == nullptr){
        m_fail_to_load_annotation_file = true;
        QMessageBox box;
        box.warning(nullptr, "Unable to Load Annotation",
            QString::fromStdString("Cannot load annotation file " + m_annotation_file_path + ". Loaded json is not an array"));
        return;
    }

    for(size_t i = 0; i < json_array->size(); i++){
        try{
            ObjectAnnotation anno_obj = json_to_object_annotation((*json_array)[i]);
            m_annotated_objects.emplace_back(std::move(anno_obj));
        } catch([[maybe_unused]] JsonParseException & e){
            m_fail_to_load_annotation_file = true;
            QMessageBox box;
            box.warning(nullptr, "Unable to Load Annotation",
                QString::fromStdString("Cannot load annotation file " + m_annotation_file_path + 
                    ". Parsing object " + std::to_string(i) + " failed."));
        }
    }
    m_last_object_idx = m_annotated_objects.size(); 
    cout << "Loaded existing annotation file " << m_annotation_file_path << endl;
}

void LabelImages::update_rendered_objects(VideoOverlaySet& overlay_set){
    overlay_set.clear();
    overlay_set.add(COLOR_RED, {X, Y, WIDTH, HEIGHT});

    for(size_t i_obj = 0; i_obj < m_annotated_objects.size(); i_obj++){
        const auto& obj = m_annotated_objects[i_obj];
        // overlayset.add(COLOR_RED, pixelbox_to_floatbox(source_image_width, source_image_height, obj.user_box));
        const auto mask_float_box = pixelbox_to_floatbox(source_image_width, source_image_height, obj.mask_box);
        std::string label = obj.label;
        const Pokemon::PokemonForm* form = Pokemon::get_pokemon_form(label);
        if (form != nullptr){
            label = form->display_name();
        }
        Color mask_box_color = (i_obj == m_last_object_idx) ? COLOR_BLACK : COLOR_BLUE;
        overlay_set.add(mask_box_color, mask_float_box, label);
        size_t mask_width = obj.mask_box.width();
        size_t mask_height = obj.mask_box.height();
        ImageRGB32 mask_image(mask_width, mask_height);
        // cout << "in render, mask_box " << obj.mask_box.min_x << " " << obj.mask_box.min_y << " " << obj.mask_box.max_x << " " << obj.mask_box.max_y << endl;

        for (size_t y = 0; y < mask_height; y++){
            for (size_t x = 0; x < mask_width; x++){
                const bool mask = obj.mask[y*mask_width + x];
                uint32_t& pixel = mask_image.pixel(x, y);
                // if the pixel's mask value is true, set a semi-transparent 45-degree blue strip color
                // otherwise: fully transparent (alpha = 0)
                uint32_t color = 0;
                if (mask){
                    color = (std::abs(int(x) - int(y)) % 4 <= 1) ? combine_argb(150, 30, 144, 255) : combine_argb(150, 0, 0, 60);
                }
                pixel = color;
            }
        }
        // cout << " count " << count << endl;
        overlay_set.add(std::move(mask_image), mask_float_box);
    }
}

void LabelImages::compute_mask(VideoOverlaySet& overlay_set){
    const size_t source_width = source_image_width;
    const size_t source_height = source_image_height;
    
    const int box_x = int(X * source_width + 0.5);
    const int box_y = int(Y * source_height + 0.5);
    const int box_width = int(WIDTH * source_width + 0.5);
    const int box_height = int(HEIGHT * source_height + 0.5);
    if (box_width == 0 || box_height == 0){
        return;
    }

    if (m_image_embedding.size() == 0){
        // no embedding file loaded
        return;
    }
    m_sam_session.run(
        m_image_embedding,
        (int)source_height, (int)source_width, {}, {},
        {box_x, box_y, box_x + box_width, box_y + box_height},
        m_output_boolean_mask
    );

    size_t min_mask_x = INT_MAX, max_mask_x = 0;
    size_t min_mask_y = INT_MAX, max_mask_y = 0;
    for (size_t y = 0; y < source_height; y++){
        for (size_t x = 0; x < source_width; x++){
            bool mask = m_output_boolean_mask[y*source_width + x];
            uint32_t& pixel = m_mask_image.pixel(x, y);
            // if the pixel's mask value is true, set a semi-transparent 45-degree blue strip color
            // otherwise: fully transparent (alpha = 0)
            uint32_t color = 0;
            if (mask){
                color = (std::abs(int(x) - int(y)) % 4 <= 1) ? combine_argb(150, 30, 144, 255) : combine_argb(150, 0, 0, 60);
                min_mask_x = std::min(x, min_mask_x);
                max_mask_x = std::max(x, max_mask_x);
                min_mask_y = std::min(y, min_mask_y);
                max_mask_y = std::max(y, max_mask_y);
            }
            pixel = color;
        }
    }
    if (min_mask_x < INT_MAX && max_mask_x > min_mask_x && min_mask_y < INT_MAX && max_mask_y > min_mask_y){
        const size_t mask_width = max_mask_x - min_mask_x + 1;
        const size_t mask_height = max_mask_y - min_mask_y + 1;
        ImageFloatBox mask_box(
            min_mask_x/double(source_width), min_mask_y/double(source_height),
            mask_width/double(source_width), mask_height/double(source_height));
        const std::string label = FORM_LABEL.slug();
        

        ObjectAnnotation annotation;
        annotation.user_box = ImagePixelBox(box_x, box_y, box_x + box_width + 1, box_y + box_height + 1);
        annotation.mask_box = ImagePixelBox(min_mask_x, min_mask_y, max_mask_x+1, max_mask_y+1);
        annotation.mask.resize(mask_width * mask_height);
        for(size_t row = 0; row < mask_height; row++){
            auto it = m_output_boolean_mask.begin() + (min_mask_y + row) * source_width + min_mask_x;
            auto it2 = annotation.mask.begin() + row * mask_width;
            std::copy(it, it + mask_width, it2);
        }

        annotation.label = label;
        m_last_object_idx = m_annotated_objects.size();
        m_annotated_objects.emplace_back(std::move(annotation));

        update_rendered_objects(overlay_set);
    }
}

LabelImages_Widget::~LabelImages_Widget(){
    m_program.FORM_LABEL.remove_listener(*this);
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
    , m_overlay_set(m_session.overlay())
    , m_drawn_box(*this, m_session.overlay())
{
    m_program.FORM_LABEL.add_listener(*this);

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

    QPushButton* button = new QPushButton("Delete Last Mask", scroll_inner);
    scroll_layout->addWidget(button);
    connect(button, &QPushButton::clicked, this, [this](bool){
        auto& program = this->m_program;
        if (program.m_annotated_objects.size() > 0){
            program.m_annotated_objects.pop_back();
        }
        if (program.m_annotated_objects.size() > 0){
            program.m_last_object_idx = program.m_annotated_objects.size() - 1;
        }
        program.update_rendered_objects(this->m_overlay_set);
    });

    m_option_widget = instance.m_options.make_QtWidget(*scroll_inner);
    scroll_layout->addWidget(&m_option_widget->widget());

    const VideoSourceDescriptor* video_source_desc = instance.m_switch_control_option.m_video.descriptor().get();
    auto image_source_desc = dynamic_cast<const VideoSourceDescriptor_StillImage*>(video_source_desc);
    if (image_source_desc != nullptr){
        const std::string image_path = image_source_desc->path();
        const size_t source_image_height = image_source_desc->source_image_height();
        const size_t source_image_width = image_source_desc->source_image_width();
        m_program.load_image_related_data(image_path, source_image_width, source_image_height);
        m_program.update_rendered_objects(m_overlay_set);
    }

    cout << "LabelImages_Widget built" << endl;
}

void LabelImages_Widget::on_config_value_changed(void* object){
    if (m_program.m_annotated_objects.size() > 0 && m_program.m_last_object_idx < m_program.m_annotated_objects.size()){
        std::string& cur_label = m_program.m_annotated_objects[m_program.m_last_object_idx].label;
        cur_label = m_program.FORM_LABEL.slug();
        m_program.update_rendered_objects(m_overlay_set);
    }
}



}
}

