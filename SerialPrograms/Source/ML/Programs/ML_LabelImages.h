/*  ML Label Images
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_ML_LabelImages_H
#define PokemonAutomation_ML_LabelImages_H

#include <memory>
#include "Common/Cpp/Options/BatchOption.h"
#include "Common/Cpp/Options/FloatingPointOption.h"
#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "Common/Cpp/Options/EnumDropdownDatabase.h"
#include "Common/Cpp/Options/StringOption.h"
#include "CommonFramework/Panels/PanelInstance.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "Pokemon/Options/Pokemon_HomeSpriteSelectOption.h"
#include "ML/DataLabeling/ML_ObjectAnnotation.h"
#include "ML/DataLabeling/ML_SegmentAnythingModel.h"
#include "ML/UI/ML_ImageAnnotationDisplayOption.h"
#include "ML/UI/ML_ImageAnnotationDisplaySession.h"

class QGraphicsView;
class QGraphicsPixmapItem;
class QLabel;

namespace PokemonAutomation{


class ConfigWidget;


namespace ML{


class ImageAnnotationDisplayWidget;
class LabelImages_Widget;
class LabelImages_OverlayManager;


class LabelImages_Descriptor : public PanelDescriptor{
public:
    LabelImages_Descriptor();
};


// Program to annoatation images for training ML models
class LabelImages : public PanelInstance, public ConfigOption::Listener {
public:
    LabelImages(const LabelImages_Descriptor& descriptor);
    virtual QWidget* make_widget(QWidget& parent, PanelHolder& holder) override;
    ~LabelImages();

public:
    // Serialization
    virtual void from_json(const JsonValue& json) override;
    virtual JsonValue to_json() const override;

    void save_annotation_to_file() const;

    // called after loading a new image, clean up all internal data 
    void clear_for_new_image();

    // Load image related data:
    // - Image SAM embedding data file, which has the same file path but with a name suffix ".embedding"
    // - Existing annotation file, which is stored in a pre-defined ML_ANNOTATION_PATH() and with the same filename as
    //   the image but with name extension replaced to be ".json".
    void load_image_related_data(const std::string& image_path, const size_t source_image_width, const size_t source_image_height);

    // Update rendering data reflect the current annotation
    void update_rendered_objects();

    // Use user currently drawn box to compute per-pixel masks on the image using SAM model
    void add_new_annotation_from_user_box();
    // update mask after user add or remove an inclusion or exclusion point on the currently selected annotation
    // rendered objects are not updated
    void update_mask_for_selected_annotation();
    // User adds an inclusion point to the current selected annotation. It will then re-compute
    // the per-pixel mask using the added inclusion point.
    void add_segmentation_inclusion_point(double x, double y);
    // Remove the closest user added segmentation inclusion point on the current selected annotation
    void remove_segmentation_inclusion_point(double x, double y);
    // User adds an exclusion point to the current selected annotation. It will then re-compute
    // the per-pixel mask using the added exclusion point.
    void add_segmentation_exclusion_point(double x, double y);
    // Remove the closest user added segmentation exclusion point on the current selected annotation
    void remove_segmentation_exclusion_point(double x, double y);

    // Compute embeddings for all images in a folder.
    // This can be very slow!
    void compute_embeddings_for_folder(const std::string& image_folder);

    // Delete the currently selected object annotation.
    void delete_selected_annotation();

    void change_annotation_selection_by_mouse(double x, double y);
    void select_prev_annotation();
    void select_next_annotation();

    // return the label selected on UI
    std::string selected_label() const;
    void set_selected_label(const std::string& label);

    void load_custom_label_set(const std::string& json_path);

    void export_to_yolov5_dataset(const std::string& image_folder_path, const std::string& dataset_path);

private:
    void on_config_value_changed(void* object) override;

    std::pair<size_t, size_t> float_to_pixel(double x, double y) const;
    // note! will have division by source_image_width/height!
    std::pair<double, double> pixel_to_float(size_t x, size_t y) const;

    void remove_closest_point(std::vector<std::pair<size_t, size_t>>& points, double x, double y);

    // call SAM model to compute mask from the given user box and inclusiong and exclusion points.
    // if SAM session failed or no SAM model loaded, return false
    // if user box is empty or no mask is created, return false
    // return true when the mask is created successfully and saved to `mask_box` and `mask`.
    // no change to `mask_box` and `mask` if the function returns false.
    bool run_sam_to_create_annotation(
        const ImagePixelBox& user_box,
        const std::vector<std::pair<size_t, size_t>>& inclusion_points,
        const std::vector<std::pair<size_t, size_t>>& exclusion_points,
        ImagePixelBox& mask_box,
        std::vector<bool>& mask
    );

    friend class LabelImages_Widget;
    friend class LabelImages_OverlayManager;

    // image display options like what image file is loaded
    ImageAnnotationDisplayOption m_display_option;
    // handles image display session, holding a reference to m_display_option
    ImageAnnotationDisplaySession m_display_session;
    // manages overlay rendering that shows annotations overlayed on the image
    LabelImages_OverlayManager* m_overlay_manager = nullptr;
    // the group option that holds rest of the options defined below:
    BatchOption m_options;

    FloatingPointOption X;
    FloatingPointOption Y;
    FloatingPointOption WIDTH;
    FloatingPointOption HEIGHT;

    // the database to initialize LABEL_TYPE
    IntegerEnumDropdownDatabase LABEL_TYPE_DATABASE;
    // a dropdown menu to choose which source below to set label from
    IntegerEnumDropdownOption LABEL_TYPE;
    // source 1: a dropdown menu for all pokemon forms
    Pokemon::HomeSpriteSelectCell FORM_LABEL;
    // the database to initialize CUSTOM_SET_LABEL
    StringSelectDatabase CUSTOM_LABEL_DATABASE;
    // source 2: a dropdown menu for custom labels
    StringSelectCell CUSTOM_SET_LABEL;
    // source 3: editable text input
    StringCell MANUAL_LABEL;

    size_t source_image_height = 0;
    size_t source_image_width = 0;
    std::vector<float> m_image_embedding;
    std::vector<bool> m_output_boolean_mask;

    std::unique_ptr<SAMSession> m_sam_session;
    std::vector<ObjectAnnotation> m_annotations;
    
    // currently selected annotated object's index
    // if this value == m_annotations.size(), it means the user is not selecting anything
    size_t m_selected_obj_idx = 0;
    std::string m_annotation_file_path;
    // if we find an annotation file that is supposed to be created by user in a previous session, but
    // we fail to load it, then we shouldn't overwrite this file to possibly erase the previous work.
    // so this flag is used to denote if we fail to load an annotation file
    bool m_fail_to_load_annotation_file = false;

    // the file path to load custom label set
    std::string m_custom_label_set_file_path;
    // the path to get the YOLOv5 YAML config file to export images and finished annotations to
    // YOLO dataset.
    std::string m_yolo_config_file_path;
};



}
}
#endif

