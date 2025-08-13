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
#include "ML/DataLabeling/ML_SegmentAnythingModel.h"
#include "ML/DataLabeling/ML_AnnotationIO.h"
#include "ML_LabelImages.h"
#include "ML_LabelImagesOverlayManager.h"



using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace ML{


LabelImages_Descriptor::LabelImages_Descriptor()
    : PanelDescriptor(
        Color(),
        "ML:LabelImages",
        "ML", "Label Images",
        "ComputerControl/blob/master/Wiki/Programs/ML/LabelImages.md",
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

    m_overlay_manager = new LabelImages_OverlayManager(*this);
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

    file_path = obj->get_string("YOLO_CONFIG_FILE_PATH");
    if (file_path){
        m_yolo_config_file_path = *file_path;
    }
}
JsonValue LabelImages::to_json() const{
    JsonObject obj = std::move(*m_options.to_json().to_object());
    obj["ImageSetup"] = m_display_option.to_json();
    obj["CUSTOM_LABEL_SET_FILE_PATH"] = m_custom_label_set_file_path;
    obj["YOLO_CONFIG_FILE_PATH"] = m_yolo_config_file_path;

    save_annotation_to_file();
    return obj;
}

void LabelImages::save_annotation_to_file() const{
    if (m_annotation_file_path.size() == 0 || m_fail_to_load_annotation_file){
        return;
    }
    JsonObject json;
    json["IMAGE_WIDTH"] = source_image_width;
    json["IMAGE_HEIGHT"] = source_image_height;

    JsonArray anno_json_arr;
    for(const auto& anno_obj: m_annotations){
        anno_json_arr.push_back(anno_obj.to_json());
    }
    json["ANNOTATION"] = std::move(anno_json_arr);

    cout << "Saving annotation to " << m_annotation_file_path << endl;
    json.dump(m_annotation_file_path);
}

void LabelImages::clear_for_new_image(){
    m_overlay_manager->clear();
    source_image_width = source_image_height = 0;
    m_image_embedding.clear();
    m_output_boolean_mask.clear();
    m_annotations.clear();
    m_selected_obj_idx = 0;
    m_annotation_file_path = "";
    m_fail_to_load_annotation_file = false;
}


// assuming clear_for_new_image() is already called
void LabelImages::load_image_related_data(const std::string& image_path, size_t source_image_width, size_t source_image_height){
    cout << "Image source: " << image_path << ", " << source_image_width << " x " << source_image_height << endl;

    this->source_image_height = source_image_height;
    this->source_image_width = source_image_width;

    m_overlay_manager->set_image_size();

    // if no such embedding file, m_image_embedding will be empty
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
    const JsonObject* json_obj = loaded_json.to_object();
    const JsonArray* json_array = nullptr;
    if (json_obj == nullptr){
        // legacy format, load as an array
        json_array = loaded_json.to_array();
    } else{
        json_array = json_obj->get_array("ANNOTATION");
    }
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
    m_overlay_manager->update_rendered_annotations();
}

void LabelImages::add_new_annotation_from_user_box(){
    ImageFloatBox user_float_box(X, Y, WIDTH, HEIGHT);
    ImagePixelBox user_box = floatbox_to_pixelbox(source_image_width, source_image_height, user_float_box);

    ImagePixelBox mask_box;
    std::vector<bool> mask;
    const bool mask_computed = run_sam_to_create_annotation(user_box, {}, {}, mask_box, mask);

    if (mask_computed){
        ObjectAnnotation annotation;
        annotation.user_box = user_box;
        annotation.mask_box = mask_box;
        annotation.mask = std::move(mask);
        annotation.label = this->selected_label();;
        m_selected_obj_idx = m_annotations.size();
        m_annotations.emplace_back(std::move(annotation));
        update_rendered_objects();
    }
}

void LabelImages::update_mask_for_selected_annotation(){
    if (m_selected_obj_idx >= m_annotations.size()){
        return;
    }

    auto& anno = m_annotations[m_selected_obj_idx];
    run_sam_to_create_annotation(anno.user_box, anno.inclusion_points, anno.exclusion_points, anno.mask_box, anno.mask);
}

bool LabelImages::run_sam_to_create_annotation(
    const ImagePixelBox& user_box,
    const std::vector<std::pair<size_t, size_t>>& inclusion_points,
    const std::vector<std::pair<size_t, size_t>>& exclusion_points,
    ImagePixelBox& mask_box,
    std::vector<bool>& mask
){
    const size_t source_width = source_image_width;
    const size_t source_height = source_image_height;
    if (!m_sam_session || m_image_embedding.size() == 0){
        // no embedding file loaded
        return false;
    }
    if (user_box.width() == 0 || user_box.height() == 0){
        return false;
    }
    
    // input_points: input point coordinates (x, y) in pixel units. [p0_x, p0_y, p1_x, p1_y, p2_x, ...].
    //     Vector size: 2*num_points
    // input_point_labels: if a point is part of the object to segment, its corresponding label value is 1.
    //     if a point is outside of the object, value is 0. Vector size: num_points.

    // input_box: if not empty, the two corner points (in pixel units) of a bounding box for the object to segment.
    //     [p0_x, p0_y, p1_x, p1_y], where p0 is the top-left corner and p1 is the lower right corner.
    const size_t num_points = inclusion_points.size() + exclusion_points.size();
    std::vector<int> input_points(2*num_points), input_point_labels(num_points);
    for(size_t i = 0; i < inclusion_points.size(); i++){
        input_points[2*i] = static_cast<int>(inclusion_points[i].first);
        input_points[2*i+1] = static_cast<int>(inclusion_points[i].second);
        input_point_labels[i] = 1;
    }
    for(size_t i = 0; i < exclusion_points.size(); i++){
        input_points[2*inclusion_points.size() + 2*i] = static_cast<int>(exclusion_points[i].first);
        input_points[2*inclusion_points.size() + 2*i+1] = static_cast<int>(exclusion_points[i].second);
        input_point_labels[inclusion_points.size() + i] = 0;
    }

    m_sam_session->run(
        m_image_embedding,
        (int)source_height, (int)source_width, input_points, input_point_labels,
        {static_cast<int>(user_box.min_x), static_cast<int>(user_box.min_y), static_cast<int>(user_box.max_x)-1, static_cast<int>(user_box.max_y)-1},
        m_output_boolean_mask
    );

    size_t min_mask_x = INT_MAX, max_mask_x = 0;
    size_t min_mask_y = INT_MAX, max_mask_y = 0;
    for (size_t y = 0; y < source_height; y++){
        for (size_t x = 0; x < source_width; x++){
            if (m_output_boolean_mask[y*source_width + x]){
                min_mask_x = std::min(x, min_mask_x);
                max_mask_x = std::max(x, max_mask_x);
                min_mask_y = std::min(y, min_mask_y);
                max_mask_y = std::max(y, max_mask_y);
            }
        }
    }
    if (min_mask_x >= INT_MAX || max_mask_x < min_mask_x || min_mask_y >= INT_MAX || max_mask_y <= min_mask_y){
        return false;
    }
    
    const size_t mask_width = max_mask_x - min_mask_x + 1;
    const size_t mask_height = max_mask_y - min_mask_y + 1;

    mask_box = ImagePixelBox(min_mask_x, min_mask_y, max_mask_x+1, max_mask_y+1);
    mask.resize(mask_width * mask_height);
    for(size_t row = 0; row < mask_height; row++){
        auto it = m_output_boolean_mask.begin() + (min_mask_y + row) * source_width + min_mask_x;
        auto it2 = mask.begin() + row * mask_width;
        std::copy(it, it + mask_width, it2);
    }
    return true;
}

void LabelImages::add_segmentation_inclusion_point(double x, double y){
    if (source_image_width == 0 || source_image_height == 0 || m_annotations.size() == 0
        || m_selected_obj_idx >= m_annotations.size()){
        return;
    }
    auto& cur_anno = m_annotations[m_selected_obj_idx];
    if (cur_anno.mask.size() == 0){
        return;
    }
    cur_anno.inclusion_points.push_back(float_to_pixel(x, y));
    update_mask_for_selected_annotation();
    update_rendered_objects();
}

void LabelImages::add_segmentation_exclusion_point(double x, double y){
    if (source_image_width == 0 || source_image_height == 0 || m_annotations.size() == 0
        || m_selected_obj_idx >= m_annotations.size()){
        return;
    }
    auto& cur_anno = m_annotations[m_selected_obj_idx];
    if (cur_anno.mask.size() == 0){
        return;
    }
    cur_anno.exclusion_points.push_back(float_to_pixel(x, y));
    update_mask_for_selected_annotation();
    update_rendered_objects();
}

void LabelImages::remove_closest_point(std::vector<std::pair<size_t, size_t>>& points, double x, double y){
    if (points.size() == 0){
        return;
    }

    std::pair<size_t, size_t> tp = float_to_pixel(x, y);

    size_t min_dist = SIZE_MAX;
    size_t target_point = 0;
    for(size_t i = 0; i < points.size(); i++){
        const auto& ip = points[i];
        size_t d_x = ip.first > tp.first ? ip.first - tp.first : tp.first - ip.first;
        size_t d_y = ip.second > tp.second ? ip.second - tp.second : tp.second - ip.second;
        size_t d2 = d_x * d_x + d_y * d_y;
        if (d2 < min_dist){
            min_dist = d2;
            target_point = i;
        }
    }
    points.erase(points.begin() + target_point);

}
    
void LabelImages::remove_segmentation_inclusion_point(double x, double y){
    if (source_image_width == 0 || source_image_height == 0 || m_annotations.size() == 0
        || m_selected_obj_idx >= m_annotations.size()){
        return;
    }
    auto& points = m_annotations[m_selected_obj_idx].inclusion_points;
    remove_closest_point(points, x, y);
    update_mask_for_selected_annotation();
    update_rendered_objects();
}

void LabelImages::remove_segmentation_exclusion_point(double x, double y){
    if (source_image_width == 0 || source_image_height == 0 || m_annotations.size() == 0
        || m_selected_obj_idx >= m_annotations.size()){
        return;
    }
    auto& points = m_annotations[m_selected_obj_idx].exclusion_points;
    remove_closest_point(points, x, y);
    update_mask_for_selected_annotation();
    update_rendered_objects();
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

    if (m_annotations.size() == 0){ // no more annotations
        m_selected_obj_idx = 0;
        update_rendered_objects();
        return;
    }

    if (m_selected_obj_idx >= m_annotations.size()){
        m_selected_obj_idx = m_annotations.size() - 1;
    } else{
        // no change to the currently selected index
    }

    std::string cur_label = m_annotations[m_selected_obj_idx].label;
    set_selected_label(cur_label);
    update_rendered_objects();
}

void LabelImages::change_annotation_selection_by_mouse(double x, double y){
    // no image or no annotation
    if (source_image_width == 0 || source_image_height == 0 || m_annotations.size() == 0){
        return;
    }

    std::pair<size_t, size_t> p = float_to_pixel(x, y);

    const size_t old_selected_idx = m_selected_obj_idx;
    
    size_t closest_distance = SIZE_MAX;
    std::vector<size_t> zero_distance_annotations;
    for(size_t i = 0; i < m_annotations.size(); i++){
        const size_t dx = m_annotations[i].mask_box.distance_to_point_x(p.first);
        const size_t dy = m_annotations[i].mask_box.distance_to_point_y(p.second);
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
        closest_distance = SIZE_MAX;
        for(size_t i : zero_distance_annotations){
            const size_t dx = m_annotations[i].mask_box.center_distance_to_point_x(p.first);
            const size_t dy = m_annotations[i].mask_box.center_distance_to_point_y(p.second);
            const size_t d2 = dx*dx + dy*dy;
            if (d2 < closest_distance){
                closest_distance = d2;
                m_selected_obj_idx = i;
            }
        }
    }

    if (old_selected_idx != m_selected_obj_idx){
        std::string new_label = m_annotations[m_selected_obj_idx].label;
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

    std::string new_label = m_annotations[m_selected_obj_idx].label;
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

    std::string new_label = m_annotations[m_selected_obj_idx].label;
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
        // label changed by user: modify internal annotation data
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


std::pair<size_t, size_t> LabelImages::float_to_pixel(double x, double y) const{
    const size_t px = (size_t)std::max<double>(source_image_width * x + 0.5, 0);
    const size_t py = (size_t)std::max<double>(source_image_height * y + 0.5, 0);
    return std::make_pair(px, py);
}


std::pair<double, double> LabelImages::pixel_to_float(size_t x, size_t y) const{
    return std::make_pair(x / (double)source_image_width, y / (double)source_image_height);
}


void LabelImages::export_to_yolov5_dataset(const std::string& image_folder_path, const std::string& dataset_path){
    m_yolo_config_file_path = dataset_path;

    export_image_annotations_to_yolo_dataset(image_folder_path, ML_ANNOTATION_PATH(), dataset_path);
}


}
}

