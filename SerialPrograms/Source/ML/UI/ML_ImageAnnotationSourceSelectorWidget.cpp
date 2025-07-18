/*  Video Source Selector Widget
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QFileDialog>
#include "Common/Qt/NoWheelComboBox.h"
#include "CommonFramework/VideoPipeline/Backends/CameraImplementations.h"
#include "ML_ImageAnnotationSourceSelectorWidget.h"
#include "ML_ImageAnnotationDisplaySession.h"

#include "CommonFramework/VideoPipeline/VideoSources/VideoSource_Null.h"
#include "CommonFramework/VideoPipeline/VideoSources/VideoSource_Camera.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace ML{


ImageAnnotationSourceSelectorWidget::~ImageAnnotationSourceSelectorWidget(){
    m_session.video_session().remove_state_listener(*this);
}
ImageAnnotationSourceSelectorWidget::ImageAnnotationSourceSelectorWidget(ImageAnnotationDisplaySession& session)
    : m_session(session)
{
    QHBoxLayout* camera_row = new QHBoxLayout(this);
    camera_row->setContentsMargins(0, 0, 0, 0);

    // camera_row->addWidget(new QLabel("<b>Video Input:</b>", this), 1);
    // camera_row->addSpacing(5);

    m_source_file_path_label = new QLabel(this);

    camera_row->addWidget(m_source_file_path_label, 5);
    camera_row->addSpacing(5);

    // m_resolution_box = new NoWheelComboBox(this);
    // m_resolution_box->setMaxVisibleItems(20);
    // camera_row->addWidget(m_resolution_box, 3);
    // camera_row->addSpacing(5);

    m_reset_button = new QPushButton("Load File", this);
    camera_row->addWidget(m_reset_button, 1);

    //  Set the action for the video reset button
    connect(
        m_reset_button, &QPushButton::clicked,
        this, [this](bool){
            std::string path = QFileDialog::getOpenFileName(
                nullptr, "Open image file", ".", "*.png *.jpg *.jpeg"
            ).toStdString();

            if (path.size() > 0){
                m_source_file_path_label->setText(QString::fromStdString(path));
                m_session.set_image_source(path);
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




void ImageAnnotationSourceSelectorWidget::update_source_list(){
}


void ImageAnnotationSourceSelectorWidget::post_startup(VideoSource* source){
    update_source_list();
}







}
}
