/*  Camera Selector Widget
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include "Common/Qt/Redispatch.h"
#include "Common/Qt/NoWheelComboBox.h"
#include "CommonFramework/VideoPipeline/Backends/CameraImplementations.h"
#include "VideoDisplayWidget.h"
#include "CameraSelectorWidget.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{



CameraSelectorWidget::~CameraSelectorWidget(){
//    cout << "~CameraSelectorUI()" << endl;
    m_session.remove_state_listener(*this);
}
CameraSelectorWidget::CameraSelectorWidget(
    CameraSession& session,
    Logger& logger,
    VideoDisplayWidget& holder
)
    : m_logger(logger)
    , m_session(session)
    , m_display(holder)
    , m_camera_box(nullptr)
    , m_resolution_box(nullptr)
{
    QHBoxLayout* camera_row = new QHBoxLayout(this);
    camera_row->setContentsMargins(0, 0, 0, 0);

    camera_row->addWidget(new QLabel("<b>Camera:</b>", this), 1);
    camera_row->addSpacing(5);

    m_camera_box = new NoWheelComboBox(this);
    camera_row->addWidget(m_camera_box, 5);
    camera_row->addSpacing(5);

    m_resolution_box = new NoWheelComboBox(this);
    camera_row->addWidget(m_resolution_box, 3);
    camera_row->addSpacing(5);

    m_reset_button = new QPushButton("Reset Camera", this);
    camera_row->addWidget(m_reset_button, 1);

    update_camera_list();
    update_resolution_list();

    connect(
        m_camera_box, static_cast<void(QComboBox::*)(int)>(&QComboBox::activated),
        this, [this](int index){
//            cout << "m_camera_box" << endl;
            if (index <= 0 || index > (int)m_cameras.size()){
                m_session.set_source(CameraInfo());
            }else{
                m_session.set_source(m_cameras[index - 1]);
            }
        }
    );
    connect(
        m_resolution_box, static_cast<void(QComboBox::*)(int)>(&QComboBox::activated),
        this, [this](int index){
//            cout << "m_resolution_box" << endl;
            if (!m_display){
                return;
            }
            if (index < 0 || index >= (int)m_resolutions.size()){
                return;
            }
            Resolution resolution = m_resolutions[index];
            m_session.set_resolution(resolution);
            m_display.set_aspect_ratio(resolution.aspect_ratio());
        }
    );
    connect(
        m_reset_button, &QPushButton::clicked,
        this, [this](bool){
            update_camera_list();
            m_session.reset();
        }
    );

    m_session.add_state_listener(*this);
}
void CameraSelectorWidget::update_camera_list(){
    m_camera_box->clear();
    m_camera_box->addItem("(none)");

    m_cameras = get_all_cameras();
    CameraInfo info = m_session.current_device();

    size_t index = 0;
    for (size_t c = 0; c < m_cameras.size(); c++){
        const CameraInfo& camera = m_cameras[c];
        m_camera_box->addItem(QString::fromStdString(get_camera_name(camera)));

        if (info == camera){
            index = c + 1;
        }
    }
    if (index != 0){
        m_camera_box->setCurrentIndex((int)index);
    }else{
        info = CameraInfo();
        m_camera_box->setCurrentIndex(0);
    }
}
void CameraSelectorWidget::update_resolution_list(){
    Resolution camera_resolution = m_session.current_resolution();
//    cout << "camera_resolution = " << camera_resolution << endl;

    m_resolution_box->clear();
    m_resolutions.clear();

    m_resolutions = m_session.supported_resolutions();
//    cout << "CameraSelectorWidget::resolutions = " << m_resolutions.size() << endl;
    if (m_resolutions.empty()){
        return;
    }

    int camera_index = -1;
//    int current_index = -1;
    for (int c = 0; c < (int)m_resolutions.size(); c++){
        const Resolution& size = m_resolutions[c];
        m_resolution_box->addItem(
            QString::fromStdString(
                std::to_string(size.width) + " x " +
                std::to_string(size.height) + " " +
                aspect_ratio_as_string(size)
            )
        );
//        cout << "size = " << camera_resolution << endl;
        if (size == camera_resolution){
            camera_index = c;
        }
//        if (size == m_session.){
//            camera_index = c;
//        }
    }
    if (camera_index >= 0){
        m_resolution_box->setCurrentIndex(camera_index);
    }else{
        m_logger.log("Unable to find entry for this resolution.", COLOR_RED);
    }
}

void CameraSelectorWidget::pre_shutdown(){
//    cout << "CameraSelectorWidget::shutdown()" << endl;
    QMetaObject::invokeMethod(this, [&]{
        m_resolution_box->clear();
        m_resolutions.clear();
    }, Qt::QueuedConnection);
}
void CameraSelectorWidget::post_new_source(const CameraInfo& device, Resolution resolution){
//    cout << "CameraSelectorWidget::new_source()" << endl;
    QMetaObject::invokeMethod(this, [&]{
        //  See if it's in the cached list.
        for (size_t c = 0; c < m_cameras.size(); c++){
            if (m_cameras[c] == device){
                m_camera_box->setCurrentIndex((int)c + 1);
                update_resolution_list();
                return;
            }
        }
        update_camera_list();
        update_resolution_list();
    }, Qt::QueuedConnection);
}
void CameraSelectorWidget::post_resolution_change(Resolution resolution){
    //  See if it's in the cached list.
    for (size_t c = 0; c < m_resolutions.size(); c++){
        if (m_resolutions[c] == resolution){
            m_resolution_box->setCurrentIndex((int)c);
            return;
        }
    }
    QMetaObject::invokeMethod(this, [&]{
        update_resolution_list();
    }, Qt::QueuedConnection);
}










}
