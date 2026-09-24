/*  ML Label Images
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <cmath>
#include <iostream>
#ifdef QT_CORE_LIB
#include <QDesktopServices>
#include <QUrl>
#endif
#include "Common/Cpp/ColoredText.h"
#include "Common/Cpp/Logging/GlobalLogger.h"
#include "Common/Cpp/Filesystem/Filesystem.h"
#include "Common/Cpp/Filesystem/FileIO.h"
#include "Common/Cpp/Json/JsonArray.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "CommonFramework/GlobalAutoPaths.h"
#include "CommonFramework/Logging/Logger.h"
#include "CommonFramework/Options/Environment/PerformanceOptions.h"
#include "CommonFramework/VideoPipeline/VideoSources/VideoSource_StillImage.h"
#include "ControllerInput/Keyboard/KeyboardInput_State.h"
#include "GameConsole/Framework/ConsoleSystemSession.h"
#include "Pokemon/Pokemon_Strings.h"
#include "ML/DataLabeling/ML_AnnotationIO.h"
#include "ML/DataLabeling/ML_SegmentAnythingModel.h"
#include "ML_LabelImages.h"
#include "ML_LabelImagesOverlayManager.h"



using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace ML{


LabelImages_Descriptor::LabelImages_Descriptor()
    : ConsolePanelDescriptor(
        Color(),
        "ML:LabelImages",
        "ML", "Label Images",
        "Programs/ML/LabelImages.html",
        "Label " + Pokemon::STRING_POKEMON + " on images",
        PanelDeprecation::NOT_DEPRECATED,
        false
    )
{}



#define ADD_OPTION(x)                   add_option(x, #x)
#define ADD_GROUP_OPTION(group, x)      (group).add_option(x, #x)

IntegerEnumDropdownDatabase create_label_type_database(){
    IntegerEnumDropdownDatabase database;
    database.add(0, "pokemon-form", Pokemon::STRING_POKEMON + " Forms");
    database.add(1, "custom-set", "Custom Set");
    database.add(2, "manual-input", "Manual Input");
    return database;
}

//  The color dropdowns all offer the same choices. This is a shared database
//  because EnumDropdownCell (unlike EnumDropdownOption) cannot own an inlined
//  one: it holds a reference, so the database has to outlive the cells.
const EnumDropdownDatabase<ColorChoice>& color_choice_database(){
    static const EnumDropdownDatabase<ColorChoice> database{
        {ColorChoice::BLACK,        "black",        "Black"},
        {ColorChoice::RED,          "red",          "Red"},
        {ColorChoice::BLUE,         "blue",         "Blue"},
        {ColorChoice::YELLOW,       "yellow",       "Yellow"},
        {ColorChoice::GREEN,        "green",        "Green"},
        {ColorChoice::ORANGE,       "orange",       "Orange"},
        {ColorChoice::MAGENTA,      "magenta",      "Magenta"},
        {ColorChoice::PURPLE,       "purple",       "Purple"},
        {ColorChoice::CYAN,         "cyan",         "Cyan"},
    };
    return database;
}


LabelImages::LabelImages(GameConsole::ConsoleSystemSession& system)
    : m_system(system)
    , m_use_gpu_for_sam_anno(PerformanceOptions::instance().ONNX_OPTIONS.USE_GPU)
    , X("<b>X Coordinate:</b>", LockMode::UNLOCK_WHILE_RUNNING, 0.3, 0.0, 1.0)
    , Y("<b>Y Coordinate:</b>", LockMode::UNLOCK_WHILE_RUNNING, 0.3, 0.0, 1.0)
    , WIDTH("<b>Width:</b>", LockMode::UNLOCK_WHILE_RUNNING, 0.4, 0.0, 1.0)
    , HEIGHT("<b>Height:</b>", LockMode::UNLOCK_WHILE_RUNNING, 0.4, 0.0, 1.0)
    , IMAGE_INFO("")
    , EMBEDDING_INFO("")
    , IMAGE_BUTTONS(LockMode::UNLOCK_WHILE_RUNNING, true)
    , PREV_IMAGE("Prev Image in Folder (z/Z)")
    , NEXT_IMAGE("Next Image in Folder (x/X)")
    , LABEL_GROUP("Label of the Selected Annotation", LockMode::UNLOCK_WHILE_RUNNING)
    , LABEL_TYPE_DATABASE(create_label_type_database())
    , LABEL_TYPE("<b>Select Label:</b>", LABEL_TYPE_DATABASE, LockMode::UNLOCK_WHILE_RUNNING, 0)
    , FORM_LABEL("bulbasaur")
    , CUSTOM_LABEL_DATABASE(create_string_select_database({"mc"})) // mc for "main character"
    , CUSTOM_SET_LABEL(CUSTOM_LABEL_DATABASE, LockMode::UNLOCK_WHILE_RUNNING, 0)
    , MANUAL_LABEL(false, LockMode::UNLOCK_WHILE_RUNNING, "", "Custom Label", true)
    , CUSTOM_LABEL_SET(
        "<b>Custom Label Set:</b>",
        LockMode::UNLOCK_WHILE_RUNNING,
        "",
        "*.json",
        "JSON file holding the list of custom labels."
    )
    , ANNOTATION_GROUP("Annotations", LockMode::UNLOCK_WHILE_RUNNING)
    , ANNOTATION_BUTTONS(LockMode::UNLOCK_WHILE_RUNNING, true)
    , DELETE_ANNOTATION("Delete Selected Annotation (Delete)")
    , PREV_ANNOTATION("Prev Annotation")
    , NEXT_ANNOTATION("Next Annotation")
    , TOGGLE_ANNOTATIONS("Hide Annotations")
    , SAVE_ANNOTATIONS("Save Annotations")
    , OPEN_ANNOTATION_FOLDER("Open Saved Annotation Folder")
    , RENDERING_ROW(LockMode::UNLOCK_WHILE_RUNNING, true)
    , RENDERING_LABEL("<b>Annotation Colors:</b>", false)
    , SELECTED_ANNO_LABEL("Selected:", false)
    , SELECTED_ANNO_COLOR(color_choice_database(), LockMode::LOCK_WHILE_RUNNING, ColorChoice::BLACK)
    , UNSELECTED_ANNO_LABEL("Unselected:", false)
    , UNSELECTED_ANNO_COLOR(color_choice_database(), LockMode::LOCK_WHILE_RUNNING, ColorChoice::BLUE)
    , CURRENT_DRAWN_BOX_LABEL("Drawn Box:", false)
    , CURRENT_DRAWN_BOX(color_choice_database(), LockMode::LOCK_WHILE_RUNNING, ColorChoice::RED)
    , DATASET_ROW(LockMode::UNLOCK_WHILE_RUNNING, true)
    , YOLO_CONFIG_LABEL("<b>Dataset Tools:</b> YOLOv5 Config:", false)
    , YOLO_CONFIG(
        LockMode::UNLOCK_WHILE_RUNNING,
        "",
        "*.yaml",
        "YAML config file of the YOLOv5 dataset to export to."
    )
    , EXPORT_TO_YOLO("Export to YOLOv5")
    , COMPUTE_EMBEDDINGS("Compute Embeddings (SLOW!)")
    , m_cur_mouse_x(0.0)
    , m_cur_mouse_y(0.0)
    , m_shift_pressed(false)
    , m_control_pressed(false)
{
    ADD_OPTION(IMAGE_INFO);
    ADD_OPTION(EMBEDDING_INFO);
    IMAGE_BUTTONS.PA_ADD_STATIC(PREV_IMAGE);
    IMAGE_BUTTONS.PA_ADD_STATIC(NEXT_IMAGE);
    PA_ADD_STATIC(IMAGE_BUTTONS);

    ADD_GROUP_OPTION(LABEL_GROUP, LABEL_TYPE);
    ADD_GROUP_OPTION(LABEL_GROUP, FORM_LABEL);
    ADD_GROUP_OPTION(LABEL_GROUP, CUSTOM_SET_LABEL);
    ADD_GROUP_OPTION(LABEL_GROUP, MANUAL_LABEL);
    ADD_GROUP_OPTION(LABEL_GROUP, CUSTOM_LABEL_SET);
    add_option(LABEL_GROUP, "LABEL");

    ANNOTATION_BUTTONS.PA_ADD_STATIC(DELETE_ANNOTATION);
    ANNOTATION_BUTTONS.PA_ADD_STATIC(PREV_ANNOTATION);
    ANNOTATION_BUTTONS.PA_ADD_STATIC(NEXT_ANNOTATION);
    ANNOTATION_BUTTONS.PA_ADD_STATIC(TOGGLE_ANNOTATIONS);
    ANNOTATION_BUTTONS.PA_ADD_STATIC(SAVE_ANNOTATIONS);
    ANNOTATION_BUTTONS.PA_ADD_STATIC(OPEN_ANNOTATION_FOLDER);
    ANNOTATION_GROUP.PA_ADD_STATIC(ANNOTATION_BUTTONS);
    add_option(ANNOTATION_GROUP, "ANNOTATIONS");

    RENDERING_ROW.PA_ADD_STATIC(RENDERING_LABEL);
    RENDERING_ROW.PA_ADD_STATIC(SELECTED_ANNO_LABEL);
    RENDERING_ROW.PA_ADD_OPTION(SELECTED_ANNO_COLOR);
    RENDERING_ROW.PA_ADD_STATIC(UNSELECTED_ANNO_LABEL);
    RENDERING_ROW.PA_ADD_OPTION(UNSELECTED_ANNO_COLOR);
    RENDERING_ROW.PA_ADD_STATIC(CURRENT_DRAWN_BOX_LABEL);
    RENDERING_ROW.PA_ADD_OPTION(CURRENT_DRAWN_BOX);
    add_option(RENDERING_ROW, "RENDERING");

    DATASET_ROW.PA_ADD_STATIC(YOLO_CONFIG_LABEL);
    DATASET_ROW.PA_ADD_OPTION(YOLO_CONFIG);
    DATASET_ROW.PA_ADD_STATIC(EXPORT_TO_YOLO);
    DATASET_ROW.PA_ADD_STATIC(COMPUTE_EMBEDDINGS);
    add_option(DATASET_ROW, "DATASET");

    X.add_listener(*this);
    Y.add_listener(*this);
    WIDTH.add_listener(*this);
    HEIGHT.add_listener(*this);
    LABEL_TYPE.add_listener(*this);
    FORM_LABEL.add_listener(*this);
    CUSTOM_SET_LABEL.add_listener(*this);
    MANUAL_LABEL.add_listener(*this);
    CUSTOM_LABEL_SET.add_listener(*this);

    PREV_IMAGE.add_listener(static_cast<ButtonListener&>(*this));
    NEXT_IMAGE.add_listener(static_cast<ButtonListener&>(*this));
    DELETE_ANNOTATION.add_listener(static_cast<ButtonListener&>(*this));
    PREV_ANNOTATION.add_listener(static_cast<ButtonListener&>(*this));
    NEXT_ANNOTATION.add_listener(static_cast<ButtonListener&>(*this));
    TOGGLE_ANNOTATIONS.add_listener(static_cast<ButtonListener&>(*this));
    SAVE_ANNOTATIONS.add_listener(static_cast<ButtonListener&>(*this));
    OPEN_ANNOTATION_FOLDER.add_listener(static_cast<ButtonListener&>(*this));
    EXPORT_TO_YOLO.add_listener(static_cast<ButtonListener&>(*this));
    COMPUTE_EMBEDDINGS.add_listener(static_cast<ButtonListener&>(*this));

    init_sam_session(PerformanceOptions::instance().ONNX_OPTIONS.USE_GPU);

    m_overlay_manager.reset(new LabelImages_OverlayManager(*this, m_system.overlay()));

    //  The console's video display sends us the mouse and the focus changes.
    m_system.overlay().add_hid_listener(*this);
    //  The console's video session tells us when a different image is loaded.
    m_system.video().add_state_listener(*this);

    //  Apply the initial visibility of the three label sources.
    on_config_value_changed(&LABEL_TYPE);

    //  The video source may already be up and running (e.g. the image path was
    //  restored from the settings file), in which case we will not get a
    //  post_startup() for it.
    load_current_image_source();
}
LabelImages::~LabelImages(){
    m_system.video().remove_state_listener(*this);
    m_system.overlay().remove_hid_listener(*this);
    global_input_remove_listener(*this);

    COMPUTE_EMBEDDINGS.remove_listener(static_cast<ButtonListener&>(*this));
    EXPORT_TO_YOLO.remove_listener(static_cast<ButtonListener&>(*this));
    OPEN_ANNOTATION_FOLDER.remove_listener(static_cast<ButtonListener&>(*this));
    SAVE_ANNOTATIONS.remove_listener(static_cast<ButtonListener&>(*this));
    TOGGLE_ANNOTATIONS.remove_listener(static_cast<ButtonListener&>(*this));
    NEXT_ANNOTATION.remove_listener(static_cast<ButtonListener&>(*this));
    PREV_ANNOTATION.remove_listener(static_cast<ButtonListener&>(*this));
    DELETE_ANNOTATION.remove_listener(static_cast<ButtonListener&>(*this));
    NEXT_IMAGE.remove_listener(static_cast<ButtonListener&>(*this));
    PREV_IMAGE.remove_listener(static_cast<ButtonListener&>(*this));

    CUSTOM_LABEL_SET.remove_listener(*this);
    MANUAL_LABEL.remove_listener(*this);
    CUSTOM_SET_LABEL.remove_listener(*this);
    FORM_LABEL.remove_listener(*this);
    LABEL_TYPE.remove_listener(*this);
    HEIGHT.remove_listener(*this);
    WIDTH.remove_listener(*this);
    Y.remove_listener(*this);
    X.remove_listener(*this);
}

JsonValue LabelImages::to_json() const{
    std::lock_guard<std::recursive_mutex> lg(m_lock);
    save_annotation_to_file();
    return ConsolePanelInstance::to_json();
}

void LabelImages::init_sam_session(bool use_gpu){
    const std::string sam_model_path = RESOURCE_PATH() + "ML/sam_cpu.onnx";
    if (Filesystem::exists(sam_model_path)){
        m_sam_session = std::make_unique<SAMSession>(sam_model_path, use_gpu);
    }else{
        m_system.logger().log(
            "SAM model path " + sam_model_path + " does not exist. Cannot annotate images.",
            COLOR_RED
        );
    }
}

void LabelImages::save_annotation_to_file() const{
    std::lock_guard<std::recursive_mutex> lg(m_lock);
    if (m_annotation_file_path.size() == 0 || m_fail_to_load_annotation_file){
        return;
    }
    JsonObject json;
    json["IMAGE_WIDTH"] = source_image_width;
    json["IMAGE_HEIGHT"] = source_image_height;

    JsonArray anno_json_arr;
    for (const auto& anno_obj: m_annotations){
        anno_json_arr.push_back(anno_obj.to_json());
    }
    json["ANNOTATION"] = std::move(anno_json_arr);

    m_system.logger().log("Saving annotation to " + m_annotation_file_path);
    json.dump(m_annotation_file_path);
}

//  Opening a folder in the system's file browser is the one thing this program
//  cannot do without Qt, so it follows the same pattern as
//  `GlobalSettings::on_press()`: the call is guarded and the button does nothing
//  in a build without Qt.
void LabelImages::open_annotation_folder(){
    std::string folder_path;
    {
        std::lock_guard<std::recursive_mutex> lg(m_lock);
        save_annotation_to_file();
        folder_path = m_image_folder_path;
    }
    if (folder_path.empty()){
        m_system.logger().log("Cannot open the annotation folder: no image is loaded.", COLOR_RED);
        return;
    }
#ifdef QT_CORE_LIB
    QDesktopServices::openUrl(QUrl::fromLocalFile(QString::fromStdString(folder_path)));
#endif
}

void LabelImages::clear_for_new_image(){
    std::lock_guard<std::recursive_mutex> lg(m_lock);
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
    std::lock_guard<std::recursive_mutex> lg(m_lock);

    m_system.logger().log(
        "Image source: " + image_path + ", " +
        std::to_string(source_image_width) + " x " + std::to_string(source_image_height)
    );

    this->source_image_height = source_image_height;
    this->source_image_width = source_image_width;

    m_overlay_manager->set_image_size();

    // if no such embedding file, m_image_embedding will be empty
    const bool embedding_loaded = load_image_embedding(image_path, m_image_embedding);
    if (!embedding_loaded){
        return; // no embedding, then no way for us to annotate
    }

    // see if we can load the previously created labels
    const std::string anno_filename = Filesystem::Path(image_path).filename().replace_extension(".json").string();
    const std::string image_folder_path = Filesystem::Path(image_path).parent_path().string() + "/";

    m_annotation_file_path = image_folder_path + anno_filename;
    if (!Filesystem::exists(m_annotation_file_path)){
        m_system.logger().log("Annotataion output path, " + m_annotation_file_path + " does not exist yet");
        return;
    }
    std::string json_content;
    const bool anno_loaded = file_to_string(m_annotation_file_path, json_content);
    if (!anno_loaded){
        m_fail_to_load_annotation_file = true;
        m_system.logger().log(
            "Cannot open annotation file " + m_annotation_file_path + ". Probably wrong permission?",
            COLOR_RED
        );
        return;
    }

    const JsonValue loaded_json = parse_json(json_content);
    const JsonObject* json_obj = loaded_json.to_object();
    const JsonArray* json_array = nullptr;
    if (json_obj == nullptr){
        // legacy format, load as an array
        json_array = loaded_json.to_array();
    }else{
        json_array = json_obj->get_array("ANNOTATION");
    }
    if (json_array == nullptr){
        m_fail_to_load_annotation_file = true;
        m_system.logger().log(
            "Cannot load annotation file " + m_annotation_file_path + ". Loaded json is not an array",
            COLOR_RED
        );
        return;
    }

    for (size_t i = 0; i < json_array->size(); i++){
        try{
            ObjectAnnotation anno_obj = ObjectAnnotation::load_json((*json_array)[i]);
            m_annotations.emplace_back(std::move(anno_obj));
        }catch (JsonParseException&){
            m_fail_to_load_annotation_file = true;
            m_system.logger().log(
                "Cannot load annotation file " + m_annotation_file_path +
                ". Parsing object " + std::to_string(i) + " failed.",
                COLOR_RED
            );
        }
    }
    m_selected_obj_idx = m_annotations.size();
    update_rendered_objects();
    m_system.logger().log("Loaded existing annotation file " + m_annotation_file_path);
}

void LabelImages::update_rendered_objects(){
    std::lock_guard<std::recursive_mutex> lg(m_lock);
    m_overlay_manager->update_rendered_annotations();
}

void LabelImages::add_new_annotation_from_user_box(){
    std::lock_guard<std::recursive_mutex> lg(m_lock);

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
    std::lock_guard<std::recursive_mutex> lg(m_lock);
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
    for (size_t i = 0; i < inclusion_points.size(); i++){
        input_points[2*i] = static_cast<int>(inclusion_points[i].first);
        input_points[2*i+1] = static_cast<int>(inclusion_points[i].second);
        input_point_labels[i] = 1;
    }
    for (size_t i = 0; i < exclusion_points.size(); i++){
        input_points[2*inclusion_points.size() + 2*i] = static_cast<int>(exclusion_points[i].first);
        input_points[2*inclusion_points.size() + 2*i+1] = static_cast<int>(exclusion_points[i].second);
        input_point_labels[inclusion_points.size() + i] = 0;
    }

    // fall back to CPU if fails with GPU.
    for (size_t i = 0; i < 2; i++){
        try{
            // if (m_use_gpu_for_sam_anno){ throw Ort::Exception("Testing.", ORT_FAIL); }  // to simulate GPU/CPU failure
            m_sam_session->run(
                m_image_embedding,
                (int)source_height, (int)source_width, input_points, input_point_labels,
                {static_cast<int>(user_box.min_x), static_cast<int>(user_box.min_y), static_cast<int>(user_box.max_x)-1, static_cast<int>(user_box.max_y)-1},
                m_output_boolean_mask
            );
            break;
        }catch (Ort::Exception& e){
            if (m_use_gpu_for_sam_anno){
                m_system.logger().log(
                    std::string("Warning: SAM session failed using the GPU. Will reattempt with the CPU.\n") + e.what(),
                    COLOR_ORANGE
                );
                m_use_gpu_for_sam_anno = false;
                init_sam_session(m_use_gpu_for_sam_anno);
            }else{
                m_system.logger().log(
                    std::string("Error: SAM session failed even when using the CPU.\n") + e.what(),
                    COLOR_RED
                );
                return false;
            }
        }catch (...){
            m_system.logger().log("Error: Unknown error. SAM session failed.", COLOR_RED);
            return false;

        }

        if (i > 0){
            m_system.logger().log(
                "Internal Program Error: This section of code shouldn't be reachable.",
                COLOR_RED
            );
            return false;
        }

    }
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
    for (size_t row = 0; row < mask_height; row++){
        auto it = m_output_boolean_mask.begin() + (min_mask_y + row) * source_width + min_mask_x;
        auto it2 = mask.begin() + row * mask_width;
        std::copy(it, it + mask_width, it2);
    }
    return true;
}

void LabelImages::add_segmentation_inclusion_point(double x, double y){
    std::lock_guard<std::recursive_mutex> lg(m_lock);
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
    std::lock_guard<std::recursive_mutex> lg(m_lock);
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
    for (size_t i = 0; i < points.size(); i++){
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
    std::lock_guard<std::recursive_mutex> lg(m_lock);
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
    std::lock_guard<std::recursive_mutex> lg(m_lock);
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
    m_system.logger().log("Use SAM Embedding model " + embedding_model_path);
    ML::compute_embeddings_for_folder(
        embedding_model_path,
        image_folder_path,
        PerformanceOptions::instance().ONNX_OPTIONS.USE_GPU
    );
}

void LabelImages::delete_selected_annotation(){
    std::lock_guard<std::recursive_mutex> lg(m_lock);
    if (m_annotations.size() == 0 || m_selected_obj_idx >= m_annotations.size()){
        return;
    }

    m_annotations.erase(m_annotations.begin() + m_selected_obj_idx);

    if (m_annotations.size() == 0){ // no more annotations
        m_selected_obj_idx = 0;
        update_rendered_objects();
        return;
    }

    m_selected_obj_idx = m_annotations.size();  // don't select anything after deleting an object, but keep the old selected label
    std::string old_label = selected_label();
    set_selected_label(old_label);
    update_rendered_objects();
}

void LabelImages::change_annotation_selection_by_mouse(double x, double y){
    std::lock_guard<std::recursive_mutex> lg(m_lock);

    // no image or no annotation
    if (source_image_width == 0 || source_image_height == 0 || m_annotations.size() == 0){
        return;
    }
    const size_t screen_normalize_factor = (source_image_height/1080);

    std::pair<size_t, size_t> p = float_to_pixel(x, y);

    const size_t old_selected_idx = m_selected_obj_idx;

    m_selected_obj_idx = m_annotations.size();  // de-select annotations by default, when clicking the screen

    size_t closest_distance = SIZE_MAX;
    std::vector<size_t> zero_distance_annotations;
    for (size_t i = 0; i < m_annotations.size(); i++){
        const size_t dx = m_annotations[i].mask_box.distance_to_point_x(p.first);
        const size_t dy = m_annotations[i].mask_box.distance_to_point_y(p.second);
        const size_t d2 = dx*dx + dy*dy;
        if (d2 == 0){
            zero_distance_annotations.push_back(i);
        }
        if (d2 < closest_distance){
            closest_distance = d2;
            if (d2 < (500 * screen_normalize_factor * screen_normalize_factor)){
                m_selected_obj_idx = i; // only select the object if the mouse click is very close to the object box, or within it.
            }

        }
    }

    if (zero_distance_annotations.size() > 1){
        // this point is inside multiple boxes, we then use the closest to the box center to determine
        closest_distance = SIZE_MAX;
        for (size_t i : zero_distance_annotations){
            const size_t dx = m_annotations[i].mask_box.center_distance_to_point_x(p.first);
            const size_t dy = m_annotations[i].mask_box.center_distance_to_point_y(p.second);
            const size_t d2 = dx*dx + dy*dy;
            if (d2 < closest_distance){
                closest_distance = d2;
                m_selected_obj_idx = i;
            }
        }
    }

    if (m_selected_obj_idx == m_annotations.size()){ // no annotation selected
        std::string old_label = selected_label();
        set_selected_label(old_label);
        update_rendered_objects();
    }else if (old_selected_idx != m_selected_obj_idx){ // different object selected
        std::string new_label = m_annotations[m_selected_obj_idx].label;
        set_selected_label(new_label);
        update_rendered_objects();
    }
}

void LabelImages::select_prev_annotation(){
    std::lock_guard<std::recursive_mutex> lg(m_lock);

    // no image or no annotation
    if (source_image_width == 0 || source_image_height == 0 || m_annotations.size() == 0){
        return;
    }

    if (m_selected_obj_idx >= m_annotations.size()){
        m_selected_obj_idx = m_annotations.size() - 1;
    } else if (m_selected_obj_idx == 0){
        m_selected_obj_idx = m_annotations.size() - 1;
    }else{
        m_selected_obj_idx--;
    }

    std::string new_label = m_annotations[m_selected_obj_idx].label;
    set_selected_label(new_label);
    update_rendered_objects();
}
void LabelImages::select_next_annotation(){
    std::lock_guard<std::recursive_mutex> lg(m_lock);

    // no image or no annotation
    if (source_image_width == 0 || source_image_height == 0 || m_annotations.size() == 0){
        return;
    }

    if (m_selected_obj_idx >= m_annotations.size()){
        m_selected_obj_idx = 0;
    } else if (m_selected_obj_idx + 1 == m_annotations.size()){
        m_selected_obj_idx = 0;
    }else{
        m_selected_obj_idx++;
    }

    std::string new_label = m_annotations[m_selected_obj_idx].label;
    set_selected_label(new_label);
    update_rendered_objects();
}

void LabelImages::on_config_value_changed(void* object){
    std::lock_guard<std::recursive_mutex> lg(m_lock);

    if (object == &CUSTOM_LABEL_SET){
        const std::string path = CUSTOM_LABEL_SET;
        if (!path.empty() && path != m_loaded_custom_label_set_path){
            load_custom_label_set(path);
        }
        return;
    }

    if (object == &LABEL_TYPE){
        const size_t value = LABEL_TYPE.current_value();
        // label type changed
        if (value == 0){
            FORM_LABEL.set_visibility(ConfigOptionState::ENABLED);
            CUSTOM_SET_LABEL.set_visibility(ConfigOptionState::HIDDEN);
            MANUAL_LABEL.set_visibility(ConfigOptionState::HIDDEN);
        } else if (value == 1){
            FORM_LABEL.set_visibility(ConfigOptionState::HIDDEN);
            CUSTOM_SET_LABEL.set_visibility(ConfigOptionState::ENABLED);
            MANUAL_LABEL.set_visibility(ConfigOptionState::HIDDEN);
        }else{ // value == 2
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

//  All the buttons of this panel land here. `button` is the ButtonCell base of
//  the ButtonOption that was pressed, so the address comparisons below identify
//  which one it was.
void LabelImages::on_press(ButtonCell& button){
    if (&button == &PREV_IMAGE){
        go_to_image_by_index_offset(-1);
        return;
    }
    if (&button == &NEXT_IMAGE){
        go_to_image_by_index_offset(1);
        return;
    }
    if (&button == &DELETE_ANNOTATION){
        delete_selected_annotation();
        return;
    }
    if (&button == &PREV_ANNOTATION){
        select_prev_annotation();
        return;
    }
    if (&button == &NEXT_ANNOTATION){
        select_next_annotation();
        return;
    }
    if (&button == &TOGGLE_ANNOTATIONS){
        std::lock_guard<std::recursive_mutex> lg(m_lock);
        m_annotations_hidden = !m_annotations_hidden;
        TOGGLE_ANNOTATIONS.set_text(m_annotations_hidden ? "Show Annotations" : "Hide Annotations");
        update_rendered_objects();
        return;
    }
    if (&button == &SAVE_ANNOTATIONS){
        save_annotation_to_file();
        return;
    }
    if (&button == &OPEN_ANNOTATION_FOLDER){
        open_annotation_folder();
        return;
    }
    if (&button == &EXPORT_TO_YOLO){
        std::lock_guard<std::recursive_mutex> lg(m_lock);
        const std::string dataset_path = YOLO_CONFIG;
        if (m_image_folder_path.empty()){
            m_system.logger().log("Cannot export: no image is loaded.", COLOR_RED);
            return;
        }
        if (dataset_path.empty()){
            m_system.logger().log("Cannot export: no YOLOv5 dataset config file is set.", COLOR_RED);
            return;
        }
        save_annotation_to_file();
        export_to_yolov5_dataset(m_image_folder_path, dataset_path);
        return;
    }
    if (&button == &COMPUTE_EMBEDDINGS){
        std::string image_folder_path;
        {
            std::lock_guard<std::recursive_mutex> lg(m_lock);
            image_folder_path = m_image_folder_path;
        }
        if (image_folder_path.empty()){
            m_system.logger().log("Cannot compute embeddings: no image is loaded.", COLOR_RED);
            return;
        }
        compute_embeddings_for_folder(image_folder_path);
        return;
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
    std::lock_guard<std::recursive_mutex> lg(m_lock);

    StringSelectDatabase new_database;
    try{
        JsonValue value = load_json_file(json_path);
        const JsonArray& json_array = value.to_array_throw();
        for (size_t i = 0; i < json_array.size(); i++){
            const std::string& label_slug = json_array[i].to_string_throw();
            new_database.add_entry(StringSelectEntry(label_slug, label_slug));
        }
    }catch (FileException& e){
        m_system.logger().log(
            "Cannot open JSON file " + json_path + " for the custom label set. Probably wrong permission? " + e.message(),
            COLOR_RED
        );
        return;
    }catch (JsonParseException& e){
        m_system.logger().log(
            "Cannot parse JSON file " + json_path + " for the custom label set. Probably wrong file content? " + e.message(),
            COLOR_RED
        );
        return;
    }

    m_system.logger().log(
        "Loaded " + std::to_string(new_database.size()) + " custom labels from " + json_path
    );
    CUSTOM_LABEL_DATABASE = new_database;
    m_loaded_custom_label_set_path = json_path;

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
    m_system.logger().log("Exporting " + image_folder_path + " to YOLOv5 dataset " + dataset_path);
    export_image_annotations_to_yolo_dataset(image_folder_path, dataset_path);
}



//
//  Image source
//

void LabelImages::post_startup(VideoSource* source){
    load_current_image_source();
}

//  Read the image path out of the console's video source and reload everything
//  that depends on it. If the console is not displaying a still image (e.g. it
//  is set to a camera or to nothing), we have no image to label.
void LabelImages::load_current_image_source(){
    std::lock_guard<std::recursive_mutex> lg(m_lock);

    save_annotation_to_file();  // save the annotation of the previously loaded image
    clear_for_new_image();

    std::string image_path;
    std::shared_ptr<const VideoSourceDescriptor> descriptor = m_system.video().descriptor();
    if (descriptor != nullptr && descriptor->type == VideoSourceType::StillImage){
        image_path = static_cast<const VideoSourceDescriptor_StillImage&>(*descriptor).path();
    }

    refresh_image_folder_listing(image_path);

    if (image_path.empty()){
        IMAGE_INFO.set_text(
            "<b>Image:</b> (none) Set the console's \"Video Input\" to \"Still Image\" to load one."
        );
        EMBEDDING_INFO.set_text("");
        return;
    }

    std::string image_info = "<b>Image:</b> " + image_path;
    if (m_cur_image_file_idx_in_folder == SIZE_MAX){
        image_info += " (?\?/" + std::to_string(m_image_paths_in_folder.size()) + " in folder)";
    }else{
        image_info +=
            " (" + std::to_string(m_cur_image_file_idx_in_folder + 1) +
            "/" + std::to_string(m_image_paths_in_folder.size()) + " in folder)";
    }
    IMAGE_INFO.set_text(std::move(image_info));

    const std::string embedding_path = image_path + ".embedding";
    const std::string embedding_path_display = "<IMAGE_FOLDER>/" + Filesystem::Path(embedding_path).filename().string();
    if (!Filesystem::exists(embedding_path)){
        set_embedding_status(embedding_path_display + " Does Not Exist. Cannot Annotate The Image!", true);
        return;
    }
    set_embedding_status(embedding_path_display, false);

    const Resolution cur_res = m_system.video().current_resolution();
    if (cur_res.width == 0 || cur_res.height == 0){
        m_system.logger().log(
            "Loaded image " + image_path + " has invalid dimension: " + cur_res.to_string(),
            COLOR_RED
        );
        return;
    }

    load_image_related_data(image_path, cur_res.width, cur_res.height);
}

void LabelImages::set_embedding_status(std::string message, bool is_error){
    if (is_error){
        m_system.logger().log(message, COLOR_RED);
    }
    EMBEDDING_INFO.set_text(
        "<b>Image Embedding File:</b> " +
        html_color_text(message, is_error ? COLOR_RED : COLOR_DARKGREEN)
    );
}

void LabelImages::refresh_image_folder_listing(const std::string& image_path){
    m_image_folder_path.clear();
    m_image_paths_in_folder.clear();
    m_cur_image_file_idx_in_folder = SIZE_MAX;
    if (image_path.empty()){
        return;
    }

    const Filesystem::Path path(image_path);
    const auto filename = path.filename();
    const bool recursive_search = false;
    m_image_folder_path = path.parent_path().string();
    m_image_paths_in_folder = find_images_in_folder(m_image_folder_path, recursive_search);
    for (size_t i = 0; i < m_image_paths_in_folder.size(); i++){
        if (Filesystem::Path(m_image_paths_in_folder[i]).filename() == filename){
            m_cur_image_file_idx_in_folder = i;
        }
    }
    if (m_cur_image_file_idx_in_folder == SIZE_MAX){
        m_system.logger().log(
            "When searching the loaded image folder " + m_image_folder_path +
            ", cannot find the loaded image filename " + filename.string() + ".",
            COLOR_RED
        );
    }
}

void LabelImages::go_to_image_by_index_offset(int offset){
    //  Don't hold the lock while setting the video source: the video session
    //  runs the source change on the calling thread and some of its listeners
    //  block on the UI thread, which may itself be waiting on this lock.
    std::string path;
    {
        std::lock_guard<std::recursive_mutex> lg(m_lock);
        if (m_cur_image_file_idx_in_folder == SIZE_MAX || m_image_paths_in_folder.empty()){
            return;
        }

        const int folder_size = static_cast<int>(m_image_paths_in_folder.size());

        //  Calculate new index with wrap-around behavior.
        //  For negative indices, we need to add multiples of folder_size until positive.
        int new_idx = (static_cast<int>(m_cur_image_file_idx_in_folder) + offset) % folder_size;
        if (new_idx < 0){
            new_idx += folder_size;
        }

        //  Only reload if the index actually changed.
        if (static_cast<size_t>(new_idx) == m_cur_image_file_idx_in_folder){
            return;
        }
        path = m_image_paths_in_folder[new_idx];
    }

    //  Update the still image descriptor that the console is already holding so
    //  that the console's video source selector stays in sync with us.
    VideoSourceOption option;
    m_system.video().save(option);
    std::shared_ptr<VideoSourceDescriptor> descriptor = option.get_descriptor_from_cache(VideoSourceType::StillImage);
    static_cast<VideoSourceDescriptor_StillImage&>(*descriptor).set_path(std::move(path));
    m_system.video().set_source(descriptor);
}



//
//  Mouse
//

void LabelImages::on_mouse_press(double x, double y){
    std::lock_guard<std::recursive_mutex> lg(m_lock);

    m_cur_mouse_x.store(x, std::memory_order_relaxed);
    m_cur_mouse_y.store(y, std::memory_order_relaxed);

    WIDTH.set(0);
    HEIGHT.set(0);
    X.set(x);
    Y.set(y);
    m_mouse_start.emplace();
    m_mouse_end.emplace();
    m_mouse_start->first = m_mouse_end->first = x;
    m_mouse_start->second = m_mouse_end->second = y;
    m_mouse_start_time = current_time();
}

void LabelImages::on_mouse_release(double x, double y){
    std::lock_guard<std::recursive_mutex> lg(m_lock);

    m_cur_mouse_x.store(x, std::memory_order_relaxed);
    m_cur_mouse_y.store(y, std::memory_order_relaxed);

    if (!m_mouse_start || !m_mouse_end){
        return;
    }

    const auto duration = current_time() - m_mouse_start_time;
    const double rel_x = std::fabs(m_mouse_start->first - m_mouse_end->first);
    const double rel_y = std::fabs(m_mouse_start->second - m_mouse_end->second);

    m_mouse_start.reset();
    m_mouse_end.reset();

    // user may have very small movement while doing quick clicking. To register this as a simple click, use relative
    // screen distance threshold 0.0015 and click duration threshold 0.15 second:
    if ((rel_x == 0 && rel_y == 0) || (rel_x < 0.0015 && rel_y < 0.0015 && duration < std::chrono::milliseconds(150))){
        if (m_control_pressed.load(std::memory_order_relaxed)){
            add_segmentation_inclusion_point(x, y);
        } else if (m_shift_pressed.load(std::memory_order_relaxed)){
            add_segmentation_exclusion_point(x, y);
        }else{
            // normal mouse clicking
            // change currently selected annotation
            // also change the option values in the UI
            change_annotation_selection_by_mouse(x, y);
        }
        return;
    }

    // not mouse clicking. So user draw a box:
    add_new_annotation_from_user_box();
}

void LabelImages::on_mouse_move(double x, double y){
    std::lock_guard<std::recursive_mutex> lg(m_lock);

    m_cur_mouse_x.store(x, std::memory_order_relaxed);
    m_cur_mouse_y.store(y, std::memory_order_relaxed);

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

    X.set(xl);
    Y.set(yl);
    WIDTH.set(xh - xl);
    HEIGHT.set(yh - yl);

    update_rendered_objects();
}



//
//  Keyboard
//
//  The console UI forwards key events to the global input sources, which turn
//  them into HID key codes and report the set of keys that are currently held
//  down. Subscribing to that (instead of to the raw key events) is what lets
//  this program handle the keyboard without any Qt of its own.
//

void LabelImages::on_focus_in(){
    global_input_add_listener(*this);
}
void LabelImages::on_focus_out(){
    global_input_remove_listener(*this);
    {
        std::lock_guard<std::mutex> lg(m_keyboard_lock);
        m_held_keys.clear();
    }
    m_shift_pressed.store(false, std::memory_order_relaxed);
    m_control_pressed.store(false, std::memory_order_relaxed);
}

void LabelImages::run_controller_input(ControllerInputState& state){
    //  Called on the keyboard tracker thread.
    if (state.type() != ControllerInputType::HID_Keyboard){
        return;
    }
    const std::set<KeyboardKey>& keys = static_cast<const KeyboardInputState&>(state).keys();

    auto held = [&](KeyboardKey key){
        return keys.find(key) != keys.end();
    };

    //  Qt maps the Mac "command" key to control, so accept either as the
    //  inclusion point modifier on all platforms.
    const bool shift = held(KeyboardKey::KEY_LEFT_SHIFT) || held(KeyboardKey::KEY_RIGHT_SHIFT);
    const bool control =
        held(KeyboardKey::KEY_LEFT_CTRL) || held(KeyboardKey::KEY_RIGHT_CTRL) ||
        held(KeyboardKey::KEY_LEFT_META) || held(KeyboardKey::KEY_RIGHT_META);
    m_shift_pressed.store(shift, std::memory_order_relaxed);
    m_control_pressed.store(control, std::memory_order_relaxed);

    bool delete_annotation, remove_point, prev_image, next_image;
    {
        std::lock_guard<std::mutex> lg(m_keyboard_lock);

        //  A key counts as pressed the first time we see it in the held set.
        auto pressed = [&](KeyboardKey key){
            return held(key) && m_held_keys.find(key) == m_held_keys.end();
        };
        delete_annotation = pressed(KeyboardKey::KEY_DELETE) || pressed(KeyboardKey::KEY_BACKSPACE);
        remove_point = pressed(KeyboardKey::KEY_D);
        prev_image = pressed(KeyboardKey::KEY_Z);
        next_image = pressed(KeyboardKey::KEY_X);

        m_held_keys = keys;
    }

    if (delete_annotation){
        delete_selected_annotation();
    }
    if (remove_point){
        const double x = m_cur_mouse_x.load(std::memory_order_relaxed);
        const double y = m_cur_mouse_y.load(std::memory_order_relaxed);
        if (control){
            remove_segmentation_inclusion_point(x, y);
        }else if (shift){
            remove_segmentation_exclusion_point(x, y);
        }
    }
    if (prev_image){
        go_to_image_by_index_offset(shift ? -10 : -1);
    }
    if (next_image){
        go_to_image_by_index_offset(shift ? 10 : 1);
    }
}



Color enum_to_color(ColorChoice color_choice){
    switch(color_choice){
    case ColorChoice::BLACK:
        return COLOR_BLACK;
    case ColorChoice::RED:
        return COLOR_RED;
    case ColorChoice::BLUE:
        return COLOR_BLUE;
    case ColorChoice::YELLOW:
        return COLOR_YELLOW;
    case ColorChoice::GREEN:
        return COLOR_GREEN;
    case ColorChoice::ORANGE:
        return COLOR_ORANGE;
    case ColorChoice::MAGENTA:
        return COLOR_MAGENTA;
    case ColorChoice::PURPLE:
        return COLOR_PURPLE;
    case ColorChoice::CYAN:
        return COLOR_CYAN;
    default:
        global_logger_tagged().log("Error: Unknown color selected.", COLOR_RED);
        return COLOR_BLACK;
    }
}

}
}
