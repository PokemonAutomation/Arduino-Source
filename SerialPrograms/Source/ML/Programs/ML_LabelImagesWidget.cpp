/*  ML Label Images Widget
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Derived class of PanelWidget as the UI for program LabelImages.
 */

#include <QDesktopServices>
#include <QUrl>
#include <QFileDialog>
#include <QLabel>
#include <QMessageBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QPushButton>
#include <QKeyEvent>
#include <QWidget>

#include "Common/Qt/Options/ConfigWidget.h"
#include "Common/Qt/CollapsibleGroupBox.h"

#include "ML_LabelImages.h"
#include "ML_LabelImagesWidget.h"
#include "ML/UI/ML_ImageAnnotationDisplayWidget.h"


#include <iostream>
using std::cout;
using std::endl;


namespace PokemonAutomation{
namespace ML{


QWidget* LabelImages::make_widget(QWidget& parent, PanelHolder& holder){
    return new LabelImages_Widget(parent, *this, holder);
}


LabelImages_Widget::~LabelImages_Widget(){
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
{
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

    QPushButton* save_anno_button = new QPushButton("Save Annotation", scroll_inner);
    button_row->addWidget(save_anno_button, 1);

    QPushButton* open_anno_folder_button = new QPushButton("Open Saved Annotation Folder", scroll_inner);
    button_row->addWidget(open_anno_folder_button, 1);

    // add a row for user annotation
    // the user can annotate in two modes:
    // - set a pokemon form label
    // - load a predefined custom string list and select from the list
    // The custom list cannot contain pokemon form name. Otherwise it will be set to the pokemon form label
    // so underlying data is only a single string. The UI reflects on what dropdown menu is set.
    // the UI needs to have a 
    QHBoxLayout* annotation_row = new QHBoxLayout();
    scroll_layout->addLayout(annotation_row);

    // add a dropdown menu for user to pick whether to choose from pokemon form label or custom label
    ConfigWidget* label_type_widget = program.LABEL_TYPE.make_QtWidget(*scroll_inner);
    annotation_row->addWidget(&label_type_widget->widget(), 0);

    ConfigWidget* pokemon_label_widget = program.FORM_LABEL.make_QtWidget(*scroll_inner);
    annotation_row->addWidget(&pokemon_label_widget->widget(), 2);
    ConfigWidget* custom_label_widget = program.CUSTOM_SET_LABEL.make_QtWidget(*scroll_inner);
    annotation_row->addWidget(&custom_label_widget->widget(), 2);
    ConfigWidget* manual_input_label_widget = program.MANUAL_LABEL.make_QtWidget(*scroll_inner);
    annotation_row->addWidget(&manual_input_label_widget->widget(), 2);
    QPushButton* load_custom_set_button = new QPushButton("Load Custom Set", scroll_inner);
    annotation_row->addWidget(load_custom_set_button, 2);
    annotation_row->addWidget(new QLabel(scroll_inner), 10); // an empty label to push other UIs to the left

    // add compute embedding button

    QHBoxLayout* external_action_row = new QHBoxLayout();
    scroll_layout->addLayout(external_action_row);
    QPushButton* compute_embedding_button = new QPushButton("Compute Image Embeddings. SLOW! Don't Close Program!", scroll_inner);
    external_action_row->addWidget(compute_embedding_button, 10);

    QPushButton* export_to_yolo_button = new QPushButton("Export Image To YOLOv5", scroll_inner);
    external_action_row->addWidget(export_to_yolo_button, 5);

    // connect button signals to define button actions

    connect(delete_anno_button, &QPushButton::clicked, this, [this](bool){
        this->m_program.delete_selected_annotation();
    });

    connect(pre_anno_button, &QPushButton::clicked, this, [this](bool){
        this->m_program.select_prev_annotation();
    });
    connect(next_anno_button, &QPushButton::clicked, this, [this](bool){
        this->m_program.select_next_annotation();
    });
    connect(open_anno_folder_button, &QPushButton::clicked, this, [this](bool){
        this->m_program.save_annotation_to_file();

        // ensure the folder exists
        std::filesystem::create_directory(ML_ANNOTATION_PATH());
        QDesktopServices::openUrl(QUrl(QString::fromStdString("file:///" + ML_ANNOTATION_PATH())));
    });
    connect(save_anno_button, &QPushButton::clicked, this, [this](bool){
        this->m_program.save_annotation_to_file();
    });

    connect(load_custom_set_button, &QPushButton::clicked, this, [this](bool){
        const std::string& last_loaded_file_path = m_program.m_custom_label_set_file_path;
        std::string starting_dir = ".";
        if (last_loaded_file_path.size() > 0){
            starting_dir = std::filesystem::path(last_loaded_file_path).parent_path().string();
        }
        const std::string path = QFileDialog::getOpenFileName(
            nullptr, "Open JSON File", QString::fromStdString(starting_dir), "*.json"
        ).toStdString();
        if (path.size() > 0){
            cout << "File dialog returns JSON path " << path << endl;
            m_program.load_custom_label_set(path);
        }
    });

    connect(compute_embedding_button, &QPushButton::clicked, this, [this](bool){
        std::string folder_path = QFileDialog::getExistingDirectory(
            nullptr, "Open Image Folder", ".").toStdString();

        if (folder_path.size() > 0){
            this->m_program.compute_embeddings_for_folder(folder_path);
        }
    });

    connect(export_to_yolo_button, &QPushButton::clicked, this, [this](bool){
        std::string folder_path = QFileDialog::getExistingDirectory(
            nullptr, "Open Image Folder",
            QString::fromStdString(m_image_display_widget->image_folder_path())
            // QFileDialog::Option::DontUseNativeDialog
        ).toStdString();
        if (folder_path.size() == 0){
            return;
        }

        const std::string& last_loaded_file_path = m_program.m_yolo_config_file_path;
        std::string starting_dir = ".";
        if (last_loaded_file_path.size() > 0){
            starting_dir = std::filesystem::path(last_loaded_file_path).parent_path().string();
        }
        const std::string ds_config_path = QFileDialog::getOpenFileName(
            nullptr, "Open YOLOv5 Dataset Config YAML File", QString::fromStdString(starting_dir), "*.yaml"
        ).toStdString();
        if (ds_config_path.size() == 0){
            return;
        }

        cout << "Found image folder " << folder_path << endl;
        cout << "Found YOLO dataset config " << ds_config_path << endl;

        m_program.export_to_yolov5_dataset(folder_path, ds_config_path);
    });

    cout << "LabelImages_Widget built" << endl;
}


void LabelImages_Widget::on_config_value_changed(void* object){
}

// This callback function will be called whenever the display source (the image source) is loaded or reloaded:
void LabelImages_Widget::post_startup(VideoSource* source){
    const std::string& image_path = m_display_session.option().m_image_path;

    m_program.save_annotation_to_file();  // save the current annotation file
    m_program.clear_for_new_image();
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
}

void LabelImages_Widget::key_press(QKeyEvent* event){
    const auto key = Qt::Key(event->key());
    switch(key){
    case Qt::Key::Key_Shift:
        m_shift_pressed = true;
        break;
    case Qt::Key::Key_Control:
        #ifndef __APPLE__
        m_control_pressed = true;
        #endif
        break;
    case Qt::Key::Key_Meta:
        #if defined(__APPLE__)
        m_control_pressed = true;
        #endif
        break;
    default:;
    }
}

void LabelImages_Widget::key_release(QKeyEvent* event){
    const auto key = Qt::Key(event->key());
    switch(key){
    case Qt::Key::Key_Shift:
        m_shift_pressed = false;
        break;
    case Qt::Key::Key_Control:
        #ifndef __APPLE__
        m_control_pressed = false;
        #endif
        break;
    case Qt::Key::Key_Meta:
        #if defined(__APPLE__)
        m_control_pressed = false;
        #endif
        break;
    case Qt::Key::Key_Delete:
    case Qt::Key::Key_Backspace:
        m_program.delete_selected_annotation();
        break;
    case Qt::Key::Key_D:
        if (m_control_pressed){
            m_program.remove_segmentation_inclusion_point(m_cur_mouse_x, m_cur_mouse_y);
        } else if (m_shift_pressed){
            m_program.remove_segmentation_exclusion_point(m_cur_mouse_x, m_cur_mouse_y);
        }
        break;
    default:;
    }
}

void LabelImages_Widget::focus_in(QFocusEvent* event){
}

void LabelImages_Widget::focus_out(QFocusEvent* event){
}

void LabelImages_Widget::on_mouse_press(double x, double y){
    m_cur_mouse_x = x;
    m_cur_mouse_y = y;

    m_program.WIDTH.set(0);
    m_program.HEIGHT.set(0);
    m_program.X.set(x);
    m_program.Y.set(y);
    m_mouse_start.emplace();
    m_mouse_end.emplace();
    m_mouse_start->first = m_mouse_end->first = x;
    m_mouse_start->second = m_mouse_end->second = y;
    m_mouse_start_time = std::chrono::high_resolution_clock::now();
}

void LabelImages_Widget::on_mouse_release(double x, double y){
    m_cur_mouse_x = x;
    m_cur_mouse_y = y;

    const std::chrono::duration<double> duration = std::chrono::high_resolution_clock::now() - m_mouse_start_time;
    const double rel_x = std::fabs(m_mouse_start->first - m_mouse_end->first);
    const double rel_y = std::fabs(m_mouse_start->second - m_mouse_end->second);

    m_mouse_start.reset();
    m_mouse_end.reset();

    // cout << "Mouse release " << (rel_x) << " " << (rel_y) << " duration " << duration << " " <<
    //     (duration < std::chrono::milliseconds(150)) << endl;

    // user may have very small movement while doing quick clicking. To register this as a simple click, use relative
    // screen distance threshold 0.0015 and click duration threshold 0.15 second:
    if ((rel_x == 0 && rel_y == 0) || (rel_x < 0.0015 && rel_y < 0.0015 && duration < std::chrono::milliseconds(150))){
        if (m_control_pressed){
            // cout << "Ctrl pressed while at " << x << " " << y << endl;
            m_program.add_segmentation_inclusion_point(x, y);
        } else if (m_shift_pressed){
            // cout << "Shift pressed while at " << x << " " << y << endl;
            m_program.add_segmentation_exclusion_point(x, y);
        } else{
            // normal mouse clicking
            // change currently selected annotation
            // also change the option values in the UI
            m_program.change_annotation_selection_by_mouse(x, y);
        }
        return;
    }

    // not mouse clicking. So user draw a box:
    m_program.add_new_annotation_from_user_box();
}

void LabelImages_Widget::on_mouse_move(double x, double y){
    m_cur_mouse_x = x;
    m_cur_mouse_y = y;

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

    m_program.update_rendered_objects();
}


}
}