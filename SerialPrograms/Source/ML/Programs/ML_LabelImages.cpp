/*  ML Label Images
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */


#include <QMessageBox>
#include <cfloat>
#include <iostream>
#include <filesystem>
#include <cmath>
#include "Common/Cpp/Json/JsonArray.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonTools.h"
#include "Pokemon/Pokemon_Strings.h"
#include "Pokemon/Resources/Pokemon_PokemonForms.h"
#include "ML/DataLabeling/ML_SegmentAnythingModel.h"
#include "ML/DataLabeling/ML_AnnotationIO.h"
#include "ML_LabelImages.h"



using std::cout;
using std::endl;

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



#define ADD_OPTION(x)    m_options.add_option(x, #x)

IntegerEnumDropdownDatabase create_label_type_database(){
    IntegerEnumDropdownDatabase database;
    database.add(0, "pokemon-form", Pokemon::STRING_POKEMON + " Forms");
    database.add(1, "custom-set", "Custom Set");
    database.add(2, "manual-input", "Manual Input");
    return database;
}

LabelImages::LabelImages(const LabelImages_Descriptor& descriptor)
    : PanelInstance(descriptor)
    , m_display_session(m_display_option)
    , m_overlay_set(m_display_session.overlay())
    , m_options(LockMode::UNLOCK_WHILE_RUNNING)
    , X("<b>X Coordinate:</b>", LockMode::UNLOCK_WHILE_RUNNING, 0.3, 0.0, 1.0)
    , Y("<b>Y Coordinate:</b>", LockMode::UNLOCK_WHILE_RUNNING, 0.3, 0.0, 1.0)
    , WIDTH("<b>Width:</b>", LockMode::UNLOCK_WHILE_RUNNING, 0.4, 0.0, 1.0)
    , HEIGHT("<b>Height:</b>", LockMode::UNLOCK_WHILE_RUNNING, 0.4, 0.0, 1.0)
    , LABEL_TYPE_DATABASE(create_label_type_database())
    , LABEL_TYPE("<b>Select Label:</b>", LABEL_TYPE_DATABASE, LockMode::UNLOCK_WHILE_RUNNING, 0)
    , FORM_LABEL("bulbasaur")
    , CUSTOM_LABEL_DATABASE(create_string_select_database({"mc"})) // mc for "main character"
    , CUSTOM_SET_LABEL(CUSTOM_LABEL_DATABASE, LockMode::UNLOCK_WHILE_RUNNING, 0)
    , MANUAL_LABEL(false, LockMode::UNLOCK_WHILE_RUNNING, "", "Custom Label", true)
{
    ADD_OPTION(X);
    ADD_OPTION(Y);
    ADD_OPTION(WIDTH);
    ADD_OPTION(HEIGHT);
    ADD_OPTION(LABEL_TYPE);
    ADD_OPTION(FORM_LABEL);
    ADD_OPTION(CUSTOM_SET_LABEL);
    ADD_OPTION(MANUAL_LABEL);

    X.add_listener(*this);
    Y.add_listener(*this);
    WIDTH.add_listener(*this);
    HEIGHT.add_listener(*this);
    LABEL_TYPE.add_listener(*this);
    FORM_LABEL.add_listener(*this);
    CUSTOM_SET_LABEL.add_listener(*this);
    MANUAL_LABEL.add_listener(*this);

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
LabelImages::~LabelImages(){
    X.remove_listener(*this);
    Y.remove_listener(*this);
    WIDTH.remove_listener(*this);
    HEIGHT.remove_listener(*this);
    LABEL_TYPE.remove_listener(*this);
    FORM_LABEL.remove_listener(*this);
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
    const std::string* file_path = obj->get_string("CUSTOM_LABEL_SET_FILE_PATH");
    if (file_path){
        load_custom_label_set(*file_path);
    }
}
JsonValue LabelImages::to_json() const{
    JsonObject obj = std::move(*m_options.to_json().to_object());
    obj["ImageSetup"] = m_display_option.to_json();
    obj["CUSTOM_LABEL_SET_FILE_PATH"] = m_custom_label_set_file_path;

    save_annotation_to_file();
    return obj;
}

void LabelImages::save_annotation_to_file() const{
    // m_annotation_file_path
    if (m_annotation_file_path.size() > 0 && !m_fail_to_load_annotation_file){
        JsonArray anno_json_arr;
        for(const auto& anno_obj: m_annotations){
            anno_json_arr.push_back(anno_obj.to_json());
        }
        cout << "Saving annotation to " << m_annotation_file_path << endl;
        anno_json_arr.dump(m_annotation_file_path);
    }
}

void LabelImages::clear_for_new_image(){
    m_overlay_set.clear();
    source_image_width = source_image_height = 0;
    m_image_embedding.clear();
    m_output_boolean_mask.clear();
    m_mask_image = ImageRGB32();
    m_annotations.clear();
    m_selected_obj_idx = 0;
    m_annotation_file_path = "";
    m_fail_to_load_annotation_file = false;
}


// assuming clear_for_new_image() is already called
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
            ObjectAnnotation anno_obj = ObjectAnnotation::from_json((*json_array)[i]);
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
    update_rendered_objects();
    cout << "Loaded existing annotation file " << m_annotation_file_path << endl;
}

void LabelImages::update_rendered_objects(){
    m_overlay_set.clear();
    if (WIDTH > 0.0 && HEIGHT > 0.0){
        m_overlay_set.add(COLOR_RED, {X, Y, WIDTH, HEIGHT});
    }

    auto create_overlay_for_index = [&](size_t i_obj){
        const auto& obj = m_annotations[i_obj];
        // overlayset.add(COLOR_RED, pixelbox_to_floatbox(source_image_width, source_image_height, obj.user_box));
        const auto mask_float_box = pixelbox_to_floatbox(source_image_width, source_image_height, obj.mask_box);
        std::string label = obj.label;
        const Pokemon::PokemonForm* form = Pokemon::get_pokemon_form(label);
        if (form != nullptr){
            label = form->display_name();
        }
        Color mask_box_color = (i_obj == m_selected_obj_idx) ? COLOR_BLACK : COLOR_BLUE;
        m_overlay_set.add(mask_box_color, mask_float_box, label);
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
        m_overlay_set.add(std::move(mask_image), mask_float_box);
    };
    for(size_t i_obj = 0; i_obj < m_annotations.size(); i_obj++){
        if (i_obj == m_selected_obj_idx){
            // skip current selected annotation because we want to render it last so that
            // it will not be occluded by other annotations
            continue;
        }
        create_overlay_for_index(i_obj);
    }
    if (m_selected_obj_idx < m_annotations.size()){
        create_overlay_for_index(m_selected_obj_idx);
    }
}

void LabelImages::compute_mask(){
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
        const std::string label = this->selected_label();
        

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

        update_rendered_objects();
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
        update_rendered_objects();
        return;
    }

    if (m_selected_obj_idx >= m_annotations.size()){
        m_selected_obj_idx = m_annotations.size() - 1;
    } else{
        // no change to the currently selected index
    }

    std::string& cur_label = m_annotations[m_selected_obj_idx].label;
    set_selected_label(cur_label);
    update_rendered_objects();
}

void LabelImages::change_annotation_selection_by_mouse(double x, double y){
    // no image or no annotation
    if (source_image_width == 0 || source_image_height == 0 || m_annotations.size() == 0){
        return;
    }

    const size_t px = (size_t)std::max<double>(source_image_width * x + 0.5, 0);
    const size_t py = (size_t)std::max<double>(source_image_height * y + 0.5, 0);

    const size_t old_selected_idx = m_selected_obj_idx;
    
    double closest_distance = DBL_MAX;
    std::vector<size_t> zero_distance_annotations;
    for(size_t i = 0; i < m_annotations.size(); i++){
        const size_t dx = m_annotations[i].mask_box.distance_to_point_x(px);
        const size_t dy = m_annotations[i].mask_box.distance_to_point_y(py);
        const size_t d2 = dx*dx + dy*dy;
        if (d2 == 0){
            zero_distance_annotations.push_back(i);
        }
        if (d2 < closest_distance){
            closest_distance = d2;
            m_selected_obj_idx = i;
        }
    }

    if (zero_distance_annotations.size() > 1){
        // this point is inside multiple boxes, we then use the closest to the box center to determine
        closest_distance = DBL_MAX;
        for(size_t i : zero_distance_annotations){
            const size_t dx = m_annotations[i].mask_box.center_distance_to_point_x(px);
            const size_t dy = m_annotations[i].mask_box.center_distance_to_point_y(py);
            const size_t d2 = dx*dx + dy*dy;
            if (d2 < closest_distance){
                closest_distance = d2;
                m_selected_obj_idx = i;
            }
        }
    }

    if (old_selected_idx != m_selected_obj_idx){
        auto new_label = m_annotations[m_selected_obj_idx].label;
        set_selected_label(new_label);
        update_rendered_objects();
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
    set_selected_label(new_label);
    update_rendered_objects();
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
    set_selected_label(new_label);
    update_rendered_objects();
}

void LabelImages::on_config_value_changed(void* object){
    // cout << "LabelImages::on_config_value_changed" << endl;
    if (object == &LABEL_TYPE){
        const size_t value = LABEL_TYPE.current_value();
        // cout << "LABEL_TYPE value changed to " << value << endl;
        // label type changed
        if (value == 0){
            FORM_LABEL.set_visibility(ConfigOptionState::ENABLED);
            CUSTOM_SET_LABEL.set_visibility(ConfigOptionState::HIDDEN);
            MANUAL_LABEL.set_visibility(ConfigOptionState::HIDDEN);
        } else if (value == 1){
            FORM_LABEL.set_visibility(ConfigOptionState::HIDDEN);
            CUSTOM_SET_LABEL.set_visibility(ConfigOptionState::ENABLED);
            MANUAL_LABEL.set_visibility(ConfigOptionState::HIDDEN);
        } else { // value == 2
            FORM_LABEL.set_visibility(ConfigOptionState::HIDDEN);
            CUSTOM_SET_LABEL.set_visibility(ConfigOptionState::HIDDEN);
            MANUAL_LABEL.set_visibility(ConfigOptionState::ENABLED);
        }
    }

    if (object == &LABEL_TYPE || object == &FORM_LABEL || object == &CUSTOM_SET_LABEL || object == &MANUAL_LABEL){
        // label changed
        if (m_annotations.size() > 0 && m_selected_obj_idx < m_annotations.size()){
            std::string& cur_label = m_annotations[m_selected_obj_idx].label;
            const std::string ui_slug = this->selected_label();
            if (ui_slug != cur_label){
                cur_label = ui_slug;
            }
        }
        update_rendered_objects();
    }
}

std::string LabelImages::selected_label() const{
    const size_t label_type = LABEL_TYPE.current_value();
    if (label_type == 0){
        return FORM_LABEL.slug();
    }
    if (label_type == 1){
        return CUSTOM_SET_LABEL.slug();
    }
    return MANUAL_LABEL;
}

void LabelImages::set_selected_label(const std::string& slug){
    size_t index = FORM_LABEL.database().search_index_by_slug(slug);
    if (index != SIZE_MAX){
        LABEL_TYPE.set_value(0);
        FORM_LABEL.set_by_index(index);
        return;
    }
    index = CUSTOM_SET_LABEL.database().search_index_by_slug(slug);
    if (index != SIZE_MAX){
        LABEL_TYPE.set_value(1);
        CUSTOM_SET_LABEL.set_by_index(index);
        return;
    }
    LABEL_TYPE.set_value(2);
    MANUAL_LABEL.set(slug);
}

void LabelImages::load_custom_label_set(const std::string& json_path){
    StringSelectDatabase new_database;
    try{
        JsonValue value = load_json_file(json_path);
        const JsonArray& json_array = value.to_array_throw();
        for(size_t i = 0; i < json_array.size(); i++){
            const std::string& label_slug = json_array[i].to_string_throw();
            new_database.add_entry(StringSelectEntry(label_slug, label_slug));
        }
    } catch(FileException& e){
        std::cerr << "Error: File exception " << e.message() << std::endl;
        QMessageBox box;
        box.warning(nullptr, "Unable to Load Custom Label Set",
            QString::fromStdString("Cannot open JSON file " + json_path + " for the custom label set. Probably wrong permission?"));
        return;
    } catch(JsonParseException& e){
        std::cerr << "Error: JSON parse exception " << e.message() << std::endl;
        QMessageBox box;
        box.warning(nullptr, "Unable to Load Custom Label Set",
            QString::fromStdString("Cannot parse JSON file " + json_path + " for the custom label set. Probably wrong file content?"));
        return;
    }
    
    cout << "Loaded " << new_database.size() << " custom labels from " << json_path << endl;
    CUSTOM_LABEL_DATABASE = new_database;
    if (&json_path != &m_custom_label_set_file_path){
        m_custom_label_set_file_path = json_path;
    }

    // if the current label is set by MANUAL_LABEL but its value appears in the newly loaded custom set,
    // the label UI should switch the label to be shown as part of the custom set.
    // so call the following line to achieve that
    set_selected_label(selected_label());
}


}
}

