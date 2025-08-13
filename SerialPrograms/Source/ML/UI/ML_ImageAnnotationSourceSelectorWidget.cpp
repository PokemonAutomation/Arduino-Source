/*  Video Source Selector Widget
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QFileDialog>
#include <filesystem>
#include "Common/Qt/NoWheelComboBox.h"
#include "CommonFramework/VideoPipeline/Backends/CameraImplementations.h"

// #include "CommonFramework/VideoPipeline/VideoSources/VideoSource_Null.h"
// #include "CommonFramework/VideoPipeline/VideoSources/VideoSource_Camera.h"

#include "ML_ImageAnnotationSourceSelectorWidget.h"
#include "ML_ImageAnnotationDisplaySession.h"

#include "ML/DataLabeling/ML_AnnotationIO.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace ML{


ImageAnnotationSourceSelectorWidget::~ImageAnnotationSourceSelectorWidget(){
    m_session.video_session().remove_state_listener(*this);
}
ImageAnnotationSourceSelectorWidget::ImageAnnotationSourceSelectorWidget(ImageAnnotationDisplaySession& session)
    : m_session(session)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    
    QHBoxLayout* image_path_row = new QHBoxLayout();
    layout->addLayout(image_path_row);

    image_path_row->setContentsMargins(0, 0, 0, 0);

    // image_path_row->addWidget(new QLabel("<b>Video Input:</b>", this), 1);
    
    image_path_row->addWidget(new QLabel("<b>Image File Path:</b> ", this));
    // image_path_row->addSpacing(5);

    m_source_file_path_label = new QLabel(this);

    image_path_row->addWidget(m_source_file_path_label, 5);
    image_path_row->addSpacing(5);

    QPushButton* load_image_button = new QPushButton("Open Image", this);
    image_path_row->addWidget(load_image_button, 0);

    QPushButton* reload_image_button = new QPushButton("Reload Image", this);
    image_path_row->addWidget(reload_image_button, 0);

    // add folder info row to show the index of the current image in the folder and buttons to move between images in this folder
    QHBoxLayout* folder_info_row = new QHBoxLayout();
    layout->addLayout(folder_info_row);

    m_folder_info_label = new QLabel(this);
    folder_info_row->addWidget(m_folder_info_label, 1);

    QPushButton* prev_image_button = new QPushButton("Prev Image in Folder", this);
    QPushButton* next_image_button = new QPushButton("Next Image in Folder", this);
    folder_info_row->addWidget(prev_image_button, 2);
    folder_info_row->addSpacing(2);
    folder_info_row->addWidget(next_image_button, 2);
    folder_info_row->addSpacing(10);
    folder_info_row->addWidget(new QLabel("     ", this), 10);  // empty label to push the buttons above to the left


    //  Set the action for the video reset button
    connect(
        load_image_button, &QPushButton::clicked,
        this, [this](bool){
            const std::string& image_path = m_session.option().m_image_path;
            std::string starting_dir = ".";
            if (image_path.size() > 0){
                starting_dir = std::filesystem::path(image_path).parent_path().string();
            }
            const std::string path = QFileDialog::getOpenFileName(
                nullptr, "Open image file", QString::fromStdString(starting_dir), "*.png *.jpg *.jpeg"
            ).toStdString();
            cout << "File dialog returns path " << path << endl;

            if (path.size() > 0){
                m_session.set_image_source(path);
            }
        }
    );
    connect(
        reload_image_button, &QPushButton::clicked,
        this, [this](bool){
            const std::string& image_path = m_session.option().m_image_path;
            if (image_path.size()){
                m_session.set_image_source(image_path);
            }
        }  
    );
    connect(
        prev_image_button, &QPushButton::clicked,
        this, [this](bool){
            if (m_cur_image_file_idx_in_folder != SIZE_MAX && m_cur_image_file_idx_in_folder > 0){
                m_cur_image_file_idx_in_folder--;
                m_session.set_image_source(m_image_paths_in_folder[m_cur_image_file_idx_in_folder]);
            }
        }  
    );
    connect(
        next_image_button, &QPushButton::clicked,
        this, [this](bool){
            if (m_cur_image_file_idx_in_folder != SIZE_MAX && m_cur_image_file_idx_in_folder + 1 < m_image_paths_in_folder.size()){
                m_cur_image_file_idx_in_folder++;
                m_session.set_image_source(m_image_paths_in_folder[m_cur_image_file_idx_in_folder]);
            }
        }  
    );

    m_session.video_session().add_state_listener(*this);

    const std::string& image_path = m_session.option().m_image_path;
    if(image_path.size() > 0){
        m_source_file_path_label->setText(QString::fromStdString(image_path));
        m_session.set_image_source(image_path);
    }
}


// This callback function will be called whenever the display source (the image source) is loaded or reloaded:
void ImageAnnotationSourceSelectorWidget::post_startup(VideoSource* source){
    const std::string& image_path = m_session.option().m_image_path;
    m_source_file_path_label->setText(QString::fromStdString(image_path));

    const auto path = std::filesystem::path(image_path);
    const auto filename = path.filename();
    const std::string folder_path = path.parent_path().string();
    const bool recursive_search = false;
    m_image_folder_path = folder_path;
    m_image_paths_in_folder = find_images_in_folder(folder_path, recursive_search);
    const size_t num_images = m_image_paths_in_folder.size();
    m_cur_image_file_idx_in_folder = SIZE_MAX;
    for (size_t i = 0; i < num_images; i++){
        if (std::filesystem::path(m_image_paths_in_folder[i]).filename() == filename){
            m_cur_image_file_idx_in_folder = i;
        }
    }
    if (m_cur_image_file_idx_in_folder == SIZE_MAX){
        std::cerr << "When searching the loaded image folder " << folder_path << ", cannot find the loaded image filename "
            << filename << "." << std::endl;
        m_folder_info_label->setText(QString::fromStdString("\?\?\?/" + std::to_string(num_images) + " in folder"));
    } else{
        m_folder_info_label->setText(QString::fromStdString(
            std::to_string(m_cur_image_file_idx_in_folder+1) + "/" + std::to_string(num_images) + " in folder"));
    }
}







}
}
