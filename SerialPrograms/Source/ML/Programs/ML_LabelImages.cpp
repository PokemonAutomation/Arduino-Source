/*  ML Label Images
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QFileDialog>
#include <QLabel>
#include <QDir>
#include <cfloat>
#include <QDirIterator>
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
#include <iomanip>
#include "CommonFramework/Globals.h"
#include "Common/Cpp/BitmapConversion.h"
#include "Common/Cpp/Json/JsonArray.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonTools.h"
#include "Common/Qt/CollapsibleGroupBox.h"
#include "Pokemon/Resources/Pokemon_PokemonForms.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "ML/UI/ML_ImageAnnotationDisplayWidget.h"
#include "CommonFramework/VideoPipeline/Backends/CameraWidgetQt6.5.h"
#include "CommonFramework/VideoPipeline/VideoSources/VideoSource_StillImage.h"
#include "ML_LabelImages.h"
#include "Pokemon/Pokemon_Strings.h"
#include "Common/Qt/Options/ConfigWidget.h"
#include "ML/DataLabeling/ML_SegmentAnythingModel.h"
#include "ML/DataLabeling/ML_AnnotationIO.h"



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
    const size_t mask_width = anno_obj.mask_box.width(), mask_height = anno_obj.mask_box.height();
    const size_t num_mask_ele = mask_width * mask_height;
    
    const std::string mask_base64 = json_obj.get_string_throw("Mask");
    anno_obj.mask = unpack_bit_vector_from_base64(mask_base64, num_mask_ele);
    if (anno_obj.mask.size() != num_mask_ele){
        std::string err_msg = "wrong decoded object annotation mask size: decoded " + std::to_string(anno_obj.mask.size())
            + " but should be " + std::to_string(num_mask_ele);
        throw ParseException(err_msg);
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

    json_obj["Mask"] = pack_bit_vector_to_base64(object_annotation.mask);

    json_obj["Label"] = object_annotation.label;

    return json_obj;
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
    , m_display_session(m_display_option)
    , m_options(LockMode::UNLOCK_WHILE_RUNNING)
    , X("<b>X Coordinate:</b>", LockMode::UNLOCK_WHILE_RUNNING, 0.3, 0.0, 1.0)
    , Y("<b>Y Coordinate:</b>", LockMode::UNLOCK_WHILE_RUNNING, 0.3, 0.0, 1.0)
    , WIDTH("<b>Width:</b>", LockMode::UNLOCK_WHILE_RUNNING, 0.4, 0.0, 1.0)
    , HEIGHT("<b>Height:</b>", LockMode::UNLOCK_WHILE_RUNNING, 0.4, 0.0, 1.0)
    , FORM_LABEL("bulbasaur")
{
    ADD_OPTION(X);
    ADD_OPTION(Y);
    ADD_OPTION(WIDTH);
    ADD_OPTION(HEIGHT);
    ADD_OPTION(FORM_LABEL);

    // , m_sam_session{RESOURCE_PATH() + "ML/sam_cpu.onnx"}
    const std::string sam_model_path = RESOURCE_PATH() + "ML/sam_cpu.onnx";
    if (std::filesystem::exists(sam_model_path)){
        m_sam_session = std::make_unique<SAMSession>(sam_model_path);
    } else{
        std::cerr << "Error: no such SAM model path " << sam_model_path << "." << std::endl;
        QMessageBox box;
        box.critical(nullptr, "SAM Model Does Not Exist",
            QString::fromStdString("SAM model path" + sam_model_path + " does not exist."));
    }
}

void LabelImages::from_json(const JsonValue& json){
    const JsonObject* obj = json.to_object();
    if (obj == nullptr){
        return;
    }
    const JsonValue* value = obj->get_value("ImageSetup");
    if (value){
        m_display_option.load_json(*value);
    }
    m_options.load_json(json);
}
JsonValue LabelImages::to_json() const{
    JsonObject obj = std::move(*m_options.to_json().to_object());
    obj["ImageSetup"] = m_display_option.to_json();

    save_annotation_to_file();
    return obj;
}

void LabelImages::save_annotation_to_file() const{
    // m_annotation_file_path
    if (m_annotation_file_path.size() > 0 && !m_fail_to_load_annotation_file){
        JsonArray anno_json_arr;
        for(const auto& anno_obj: m_annotations){
            anno_json_arr.push_back(object_annotation_to_json(anno_obj));
        }
        cout << "Saving annotation to " << m_annotation_file_path << endl;
        anno_json_arr.dump(m_annotation_file_path);
    }
}

void LabelImages::clear_for_new_image(){
    source_image_width = source_image_height = 0;
    m_image_embedding.clear();
    m_output_boolean_mask.clear();
    m_mask_image = ImageRGB32();
    m_annotations.clear();
    m_selected_obj_idx = 0;
    m_annotation_file_path = "";
    m_fail_to_load_annotation_file = false;
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

    const JsonValue loaded_json = parse_json(json_content);
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
            m_annotations.emplace_back(std::move(anno_obj));
        } catch(JsonParseException&){
            m_fail_to_load_annotation_file = true;
            QMessageBox box;
            box.warning(nullptr, "Unable to Load Annotation",
                QString::fromStdString(
                    "Cannot load annotation file " + m_annotation_file_path + 
                    ". Parsing object " + std::to_string(i) + " failed."
                )
            );
        }
    }
    m_selected_obj_idx = m_annotations.size(); 
    cout << "Loaded existing annotation file " << m_annotation_file_path << endl;
}

void LabelImages::update_rendered_objects(VideoOverlaySet& overlay_set){
    overlay_set.clear();
    overlay_set.add(COLOR_RED, {X, Y, WIDTH, HEIGHT});

    for(size_t i_obj = 0; i_obj < m_annotations.size(); i_obj++){
        const auto& obj = m_annotations[i_obj];
        // overlayset.add(COLOR_RED, pixelbox_to_floatbox(source_image_width, source_image_height, obj.user_box));
        const auto mask_float_box = pixelbox_to_floatbox(source_image_width, source_image_height, obj.mask_box);
        std::string label = obj.label;
        const Pokemon::PokemonForm* form = Pokemon::get_pokemon_form(label);
        if (form != nullptr){
            label = form->display_name();
        }
        Color mask_box_color = (i_obj == m_selected_obj_idx) ? COLOR_BLACK : COLOR_BLUE;
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

    if (!m_sam_session || m_image_embedding.size() == 0){
        // no embedding file loaded
        return;
    }
    m_sam_session->run(
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
        m_selected_obj_idx = m_annotations.size();
        m_annotations.emplace_back(std::move(annotation));

        update_rendered_objects(overlay_set);
    }
}

void LabelImages::compute_embeddings_for_folder(const std::string& image_folder_path){
    std::string embedding_model_path = RESOURCE_PATH() + "ML/sam_embedder_cpu.onnx";
    std::cout << "Use SAM Embedding model " << embedding_model_path << std::endl;
    ML::compute_embeddings_for_folder(embedding_model_path, image_folder_path);
}

void LabelImages::delete_selected_annotation(){
    if (m_annotations.size() == 0 || m_selected_obj_idx >= m_annotations.size()){
        return;
    }

    m_annotations.erase(m_annotations.begin() + m_selected_obj_idx);

    if (m_annotations.size() == 0){
        m_selected_obj_idx = 0;
        return;
    }

    if (m_selected_obj_idx >= m_annotations.size()){
        m_selected_obj_idx = m_annotations.size() - 1;
    } else{
        // no change to the currently selected index
    }

    std::string& cur_label = m_annotations[m_selected_obj_idx].label;
    FORM_LABEL.set_by_slug(cur_label);
}

void LabelImages::change_annotation_selection_by_mouse(double x, double y){
    // no image or no annotation
    if (source_image_width == 0 || source_image_height == 0 || m_annotations.size() == 0){
        return;
    }
    
    double closest_distance = DBL_MAX;
    for(size_t i = 0; i < m_annotations.size(); i++){
        ImageFloatBox float_box = pixelbox_to_floatbox(source_image_width, source_image_height, m_annotations[i].mask_box);
        double center_x = float_box.x + float_box.width/2.0;
        double center_y = float_box.y + float_box.height/2.0;
        double d2 = (center_x-x)*(center_x-x) + (center_y-y)*(center_y-y);
        if (d2 < closest_distance){
            closest_distance = d2;
            m_selected_obj_idx = i;
        }
    }

    auto new_label = m_annotations[m_selected_obj_idx].label;
    if (FORM_LABEL.slug() != new_label){
        FORM_LABEL.set_by_slug(new_label);
    }
}

void LabelImages::select_prev_annotation(){
    // no image or no annotation
    if (source_image_width == 0 || source_image_height == 0 || m_annotations.size() == 0){
        return;
    }
    
    if (m_selected_obj_idx >= m_annotations.size()){
        m_selected_obj_idx = m_annotations.size() - 1;
    } else if (m_selected_obj_idx == 0){
        m_selected_obj_idx = m_annotations.size() - 1;
    } else {
        m_selected_obj_idx--;
    }

    auto new_label = m_annotations[m_selected_obj_idx].label;
    if (FORM_LABEL.slug() != new_label){
        FORM_LABEL.set_by_slug(new_label);
    }
}
void LabelImages::select_next_annotation(){
    // no image or no annotation
    if (source_image_width == 0 || source_image_height == 0 || m_annotations.size() == 0){
        return;
    }
    
    if (m_selected_obj_idx >= m_annotations.size()){
        m_selected_obj_idx = 0;
    } else if (m_selected_obj_idx + 1 == m_annotations.size()){
        m_selected_obj_idx = 0;
    } else {
        m_selected_obj_idx++;
    }

    auto new_label = m_annotations[m_selected_obj_idx].label;
    if (FORM_LABEL.slug() != new_label){
        FORM_LABEL.set_by_slug(new_label);
    }
}


LabelImages_Widget::~LabelImages_Widget(){
    m_program.X.remove_listener(*this);
    m_program.Y.remove_listener(*this);
    m_program.WIDTH.remove_listener(*this);
    m_program.HEIGHT.remove_listener(*this);
    m_program.FORM_LABEL.remove_listener(*this);

    m_display_session.overlay().remove_listener(*this);
    m_display_session.video_session().remove_state_listener(*this);

    delete m_image_display_widget;
}
LabelImages_Widget::LabelImages_Widget(
    QWidget& parent,
    LabelImages& program,
    PanelHolder& holder
)
    : PanelWidget(parent, program, holder)
    , m_program(program)
    , m_display_session(m_program.m_display_session)
    , m_overlay_set(m_display_session.overlay())
{
    m_program.X.add_listener(*this);
    m_program.Y.add_listener(*this);
    m_program.WIDTH.add_listener(*this);
    m_program.HEIGHT.add_listener(*this);
    m_program.FORM_LABEL.add_listener(*this);

    m_display_session.overlay().add_listener(*this);
    m_display_session.video_session().add_state_listener(*this);

    m_embedding_info_label = new QLabel(this);

    // Main layout
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

    m_image_display_widget = new ImageAnnotationDisplayWidget(*this, m_display_session, this);
    scroll_layout->addWidget(m_image_display_widget);

    QHBoxLayout* embedding_info_row = new QHBoxLayout();
    scroll_layout->addLayout(embedding_info_row);
    embedding_info_row->addWidget(new QLabel("<b>Image Embedding File:</b> ", this));    
    embedding_info_row->addWidget(m_embedding_info_label);

    // add a row for buttons

    QHBoxLayout* button_row = new QHBoxLayout();
    scroll_layout->addLayout(button_row);

    QPushButton* delete_anno_button = new QPushButton("Delete Selected Annotation", scroll_inner);
    button_row->addWidget(delete_anno_button, 1);

    QPushButton* pre_anno_button = new QPushButton("Prev Annotation", scroll_inner);
    button_row->addWidget(pre_anno_button, 1);

    QPushButton* next_anno_button = new QPushButton("Next Annotation", scroll_inner);
    button_row->addWidget(next_anno_button, 1);

    // add a row for user annotation
    QHBoxLayout* annotation_row = new QHBoxLayout();
    scroll_layout->addLayout(annotation_row);
    annotation_row->addWidget(new QLabel("<b>Select Label:</b>", scroll_inner), 0);

    ConfigWidget* option_widget = program.FORM_LABEL.make_QtWidget(*scroll_inner);
    annotation_row->addWidget(&option_widget->widget(), 5);

    // add compute embedding button

    QPushButton* compute_embedding_button = new QPushButton("Compute Image Embeddings (SLOW!)", scroll_inner);
    scroll_layout->addWidget(compute_embedding_button);

    connect(delete_anno_button, &QPushButton::clicked, this, [this](bool){
        auto& program = this->m_program;
        program.delete_selected_annotation();
        program.update_rendered_objects(this->m_overlay_set);
    });

    connect(pre_anno_button, &QPushButton::clicked, this, [this](bool){
        auto& program = this->m_program;
        program.select_prev_annotation();
        program.update_rendered_objects(this->m_overlay_set);
    });
    connect(next_anno_button, &QPushButton::clicked, this, [this](bool){
        auto& program = this->m_program;
        program.select_next_annotation();
        program.update_rendered_objects(this->m_overlay_set);
    });

    connect(compute_embedding_button, &QPushButton::clicked, this, [this](bool){
        std::string folder_path = QFileDialog::getExistingDirectory(
            nullptr, "Open image folder", ".").toStdString();

        if (folder_path.size() > 0){
            this->m_program.compute_embeddings_for_folder(folder_path);
        }
    });

    cout << "LabelImages_Widget built" << endl;
}

void LabelImages_Widget::clear_for_new_image(){
    m_overlay_set.clear();
    m_program.clear_for_new_image();
}

void LabelImages_Widget::on_config_value_changed(void* object){
    // TODO: the logic here should be part of LabelImage program
    if (m_program.m_annotations.size() > 0 && m_program.m_selected_obj_idx < m_program.m_annotations.size()){
        std::string& cur_label = m_program.m_annotations[m_program.m_selected_obj_idx].label;
        const std::string& ui_slug = m_program.FORM_LABEL.slug();
        if (ui_slug != cur_label){
            cur_label = m_program.FORM_LABEL.slug();    
        }
    }
    m_program.update_rendered_objects(m_overlay_set);
}

// This callback function will be called whenever the display source (the image source) is loaded or reloaded:
void LabelImages_Widget::post_startup(VideoSource* source){
    const std::string& image_path = m_display_session.option().m_image_path;

    m_program.save_annotation_to_file();  // save the current annotation file
    clear_for_new_image();
    if (image_path.size() == 0){
        m_embedding_info_label->setText("");
        return;
    }

    const std::string embedding_path = image_path + ".embedding";
    const std::string embedding_path_display = "<IMAGE_FOLDER>/" + std::filesystem::path(embedding_path).filename().string();
    if (!std::filesystem::exists(embedding_path)){
        m_embedding_info_label->setText(QString::fromStdString(embedding_path_display + " Dose Not Exist. Cannot Annotate The Image!"));
        m_embedding_info_label->setStyleSheet("color: red");
        return;
    }
        
    m_embedding_info_label->setText(QString::fromStdString(embedding_path_display));
    m_embedding_info_label->setStyleSheet("color: green");

    const auto cur_res = m_display_session.video_session().current_resolution();
    if (cur_res.width == 0 || cur_res.height == 0){
        QMessageBox box;
        box.warning(nullptr, "Invalid Image Dimension",
            QString::fromStdString("Loaded image " + image_path + " has invalid dimension: " + cur_res.to_string()));
        return;
    }
    
    m_program.load_image_related_data(image_path, cur_res.width, cur_res.height);
    m_program.update_rendered_objects(m_overlay_set);
}

void LabelImages_Widget::key_release(QKeyEvent* event){
    const auto key = Qt::Key(event->key());
    if (key == Qt::Key::Key_Delete || key == Qt::Key::Key_Backspace){
        m_program.delete_selected_annotation();
        m_program.update_rendered_objects(m_overlay_set);
    }
}

void LabelImages_Widget::on_mouse_press(double x, double y){
    m_program.WIDTH.set(0);
    m_program.HEIGHT.set(0);
    m_program.X.set(x);
    m_program.Y.set(y);
    m_mouse_start.emplace();
    m_mouse_end.emplace();
    m_mouse_start->first = m_mouse_end->first = x;
    m_mouse_start->second = m_mouse_end->second = y;
}

void LabelImages_Widget::on_mouse_release(double x, double y){
    if (m_mouse_start == m_mouse_end){
        // process mouse clicking
        // change currently selected annotation
        // also change the option values in the UI
        m_program.change_annotation_selection_by_mouse(x, y);
    }
    m_mouse_start.reset();
    m_mouse_end.reset();
    m_program.compute_mask(m_overlay_set);
}

void LabelImages_Widget::on_mouse_move(double x, double y){
    if (!m_mouse_start){
        return;
    }

    m_mouse_end->first = x;
    m_mouse_end->second = y;

    double xl = m_mouse_start->first;
    double yl = m_mouse_start->second;
    double xh = x;
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

}
}

