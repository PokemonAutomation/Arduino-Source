/*  ML Label Images
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Program to annotate images for training ML models.
 *
 *  This is a ConsolePanel (GameConsole/ConsolePanel.h). The panel infrastructure
 *  provides the entire UI: the console's video display (used here to show the
 *  image being labeled via the "Still Image" video source), plus the widgets for
 *  all the options declared below. As a result this program builds no Qt UI of
 *  its own:
 *    - Buttons are `ButtonOption`s. Presses arrive in `on_press()`.
 *    - Status text is `StaticTextOption`s that are updated with `set_text()`.
 *    - File paths are `PathOption`s (the option's widget owns the file dialog).
 *    - Mouse input arrives through `VideoDisplayHidListener` on the console's
 *      video overlay.
 *    - Keyboard input arrives through `ControllerInputListener` as HID key codes,
 *      which is the Qt-free view of the keyboard that the console UI feeds.
 *
 *  The one exception is `open_annotation_folder()`, which needs Qt to hand a
 *  folder to the system's file browser. It is written the same way as
 *  `GlobalSettings::on_press()`: a single guarded call, no widgets.
 */

#ifndef PokemonAutomation_ML_LabelImages_H
#define PokemonAutomation_ML_LabelImages_H

#include <atomic>
#include <memory>
#include <mutex>
#include <optional>
#include <set>
#include <string>
#include <vector>
#include "Common/Cpp/Color.h"
#include "Common/Cpp/Time.h"
#include "Common/Cpp/Options/ButtonOption.h"
#include "Common/Cpp/Options/EnumDropdownDatabase.h"
#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "Common/Cpp/Options/FloatingPointOption.h"
#include "Common/Cpp/Options/GroupOption.h"
#include "Common/Cpp/Options/PathOption.h"
#include "Common/Cpp/Options/StaticTextOption.h"
#include "Common/Cpp/Options/StringOption.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "CommonFramework/VideoPipeline/VideoSession.h"
#include "ControllerInput/ControllerInput.h"
#include "ControllerInput/Keyboard/KeyboardHidButtons.h"
#include "GameConsole/ConsolePanel.h"
#include "Pokemon/Options/Pokemon_HomeSpriteSelectOption.h"
#include "ML/DataLabeling/ML_ObjectAnnotation.h"
#include "ML/DataLabeling/ML_SegmentAnythingModel.h"

namespace PokemonAutomation{
namespace ML{


class LabelImages_OverlayManager;


enum class ColorChoice{
    BLACK,
    RED,
    BLUE,
    YELLOW,
    GREEN,
    ORANGE,
    MAGENTA,
    PURPLE,
    CYAN,
};

Color enum_to_color(ColorChoice color_choice);


class LabelImages_Descriptor : public GameConsole::ConsolePanelDescriptor{
public:
    LabelImages_Descriptor();
};


//  Program to annotate images for training ML models.
//
//  The image to label is the console's video source: pick "Still Image" in the
//  console's "Video Input" dropdown and it will ask for the image file. The
//  annotations are drawn on the console's video overlay.
//
//  Mouse/keyboard interaction on the image display:
//    - Drag a box:            run SAM on the box to create a new annotation.
//    - Click:                 select/deselect the annotation under the cursor.
//    - Ctrl(Cmd)+click:       add a segmentation inclusion point.
//    - Shift+click:           add a segmentation exclusion point.
//    - Ctrl(Cmd)+D / Shift+D: remove the inclusion/exclusion point closest to
//                             the cursor.
//    - Delete/Backspace:      delete the selected annotation.
//    - z/x, Shift+z/Shift+x:  move 1/10 images backwards/forwards in the folder.
class LabelImages
    : public GameConsole::ConsolePanelInstance
    , public ConfigOption::Listener
    , public ButtonListener
    , public VideoSession::StateListener
    , public VideoDisplayHidListener
    , public ControllerInputListener
{
public:
    //  Required by `make_ConsolePanel<>()` to find this program's descriptor.
    using Descriptor = LabelImages_Descriptor;

    ~LabelImages();
    LabelImages(GameConsole::ConsoleSystemSession& system);

public:
    //  Serialization.
    //  Saving the panel also flushes the current annotation to its .json file.
    virtual JsonValue to_json() const override;

    void init_sam_session(bool use_gpu);

    void save_annotation_to_file() const;

    //  Save the current annotation, then show the folder it was saved into in
    //  the system's file browser. The annotation .json files are written next
    //  to the images, so this is the folder of the currently loaded image.
    void open_annotation_folder();

    // called after loading a new image, clean up all internal data
    void clear_for_new_image();

    // Load image related data:
    // - Image SAM embedding data file, which has the same file path but with a name suffix ".embedding"
    // - Existing annotation file, which is stored in the same folder as the image and with the same filename as
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
    //  Overwrites ConfigOption::Listener::on_config_value_changed().
    virtual void on_config_value_changed(void* object) override;

    //  Overwrites ButtonListener::on_press().
    //  All the buttons of this panel are dispatched from here.
    virtual void on_press(ButtonCell& button) override;

    //  Overwrites VideoSession::StateListener::post_startup().
    //  Called whenever the console's video source is (re)started, which for this
    //  panel means the user loaded a different image.
    virtual void post_startup(VideoSource* source) override;

    //  Overwrites VideoDisplayHidListener::on_focus_in()/on_focus_out().
    //  Keyboard input is only routed to us while the console display has focus,
    //  so we subscribe to the global input sources only while focused.
    virtual void on_focus_in() override;
    virtual void on_focus_out() override;

    //  Overwrites VideoDisplayHidListener::on_mouse_*().
    //  These are called on the UI thread with coordinates relative to the image
    //  ([0.0, 1.0] on each axis).
    virtual void on_mouse_press(double x, double y) override;
    virtual void on_mouse_release(double x, double y) override;
    virtual void on_mouse_move(double x, double y) override;

    //  Overwrites ControllerInputListener::run_controller_input().
    //  Called on the keyboard tracker thread with the set of keys currently held
    //  down. We diff it against the previous state to get key presses.
    virtual void run_controller_input(ControllerInputState& state) override;


private:
    //  Reload everything that depends on which image is loaded. Called on startup
    //  and whenever the console's video source changes.
    void load_current_image_source();

    //  Refresh the list of images that live in the same folder as the currently
    //  loaded image, and remember where in that list the current image is. This
    //  is what the "Prev/Next Image in Folder" buttons walk through.
    void refresh_image_folder_listing(const std::string& image_path);

    //  Load the image that is `offset` positions away from the current image in
    //  its folder. Wraps around on both ends. Negative offset moves backwards.
    void go_to_image_by_index_offset(int offset);

    //  Show `message` in the embedding status text, and log it if it is an error.
    void set_embedding_status(std::string message, bool is_error);

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

    friend class LabelImages_OverlayManager;


private:
    GameConsole::ConsoleSystemSession& m_system;

    //  Annotation state is touched from the UI thread (mouse, buttons, options)
    //  and from the keyboard tracker thread (`run_controller_input()`), so all
    //  the entry points that touch it take this lock. It is recursive because
    //  changing an option value re-enters us through `on_config_value_changed()`.
    mutable std::recursive_mutex m_lock;

    // manages overlay rendering that shows annotations overlayed on the image
    std::unique_ptr<LabelImages_OverlayManager> m_overlay_manager;

    bool m_use_gpu_for_sam_anno;

    //  The box the user is currently dragging on the image. Not serialized: it
    //  only lives as long as the drag that produced it.
    FloatingPointOption X;
    FloatingPointOption Y;
    FloatingPointOption WIDTH;
    FloatingPointOption HEIGHT;

    //  Which image is loaded and whether it has a SAM embedding file.
    StaticTextOption IMAGE_INFO;
    StaticTextOption EMBEDDING_INFO;
    //  A horizontal BatchOption lays its children out in one row. The buttons
    //  inside are ButtonCells rather than ButtonOptions because a ButtonOption
    //  also renders a label column, which would split the row.
    BatchOption IMAGE_BUTTONS;
    ButtonCell PREV_IMAGE;
    ButtonCell NEXT_IMAGE;

    GroupOption LABEL_GROUP;
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
    // the file path to load custom label set
    PathOption CUSTOM_LABEL_SET;

    GroupOption ANNOTATION_GROUP;
    BatchOption ANNOTATION_BUTTONS;
    ButtonCell DELETE_ANNOTATION;
    ButtonCell PREV_ANNOTATION;
    ButtonCell NEXT_ANNOTATION;
    ButtonCell TOGGLE_ANNOTATIONS;
    ButtonCell SAVE_ANNOTATIONS;
    ButtonCell OPEN_ANNOTATION_FOLDER;

    //  The three colors are one compact row rather than a titled group of
    //  full-width dropdowns. Each dropdown is an EnumDropdownCell (no label
    //  column) preceded by its own short StaticTextOption label.
    BatchOption RENDERING_ROW;
    StaticTextOption RENDERING_LABEL;
    StaticTextOption SELECTED_ANNO_LABEL;
    EnumDropdownCell<ColorChoice> SELECTED_ANNO_COLOR;
    StaticTextOption UNSELECTED_ANNO_LABEL;
    EnumDropdownCell<ColorChoice> UNSELECTED_ANNO_COLOR;
    StaticTextOption CURRENT_DRAWN_BOX_LABEL;
    EnumDropdownCell<ColorChoice> CURRENT_DRAWN_BOX;

    //  Also one compact row: a PathCell and two ButtonCells, none of which
    //  render a label column of their own.
    BatchOption DATASET_ROW;
    StaticTextOption YOLO_CONFIG_LABEL;
    // the path to get the YOLOv5 YAML config file to export images and finished annotations to
    // YOLO dataset.
    PathCell YOLO_CONFIG;
    ButtonCell EXPORT_TO_YOLO;
    ButtonCell COMPUTE_EMBEDDINGS;

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

    // track whether annotations are currently hidden
    bool m_annotations_hidden = false;

    //  The custom label set that is currently loaded into CUSTOM_LABEL_DATABASE.
    //  Used to ignore the change notifications that CUSTOM_LABEL_SET
    //  sends while we are reacting to it.
    std::string m_loaded_custom_label_set_path;

    //  The folder of the currently loaded image and the images found in it, used
    //  by the "Prev/Next Image in Folder" buttons and the z/x hotkeys.
    std::string m_image_folder_path;
    std::vector<std::string> m_image_paths_in_folder;
    //  SIZE_MAX if the loaded image was not found in its own folder listing.
    size_t m_cur_image_file_idx_in_folder = SIZE_MAX;

    //  Mouse drag state. Only touched on the UI thread except for the cursor
    //  position, which the keyboard thread reads for the "remove point" hotkeys.
    std::optional<std::pair<double, double>> m_mouse_start;
    std::optional<std::pair<double, double>> m_mouse_end;
    WallClock m_mouse_start_time;
    std::atomic<double> m_cur_mouse_x;
    std::atomic<double> m_cur_mouse_y;

    //  Keyboard state. `m_held_keys` is written by the keyboard tracker thread
    //  and cleared by the UI thread when focus is lost, so it has its own lock.
    //  It is never held while calling into anything else, so it cannot deadlock
    //  against `m_lock`.
    std::mutex m_keyboard_lock;
    std::set<KeyboardKey> m_held_keys;
    std::atomic<bool> m_shift_pressed;
    std::atomic<bool> m_control_pressed;
};



}
}
#endif
