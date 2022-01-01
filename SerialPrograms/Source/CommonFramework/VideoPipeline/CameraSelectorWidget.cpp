/*  Camera Selector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include "Common/Qt/NoWheelComboBox.h"
#include "VideoDisplayWidget.h"
#include "CameraImplementations.h"
#include "CameraSelectorWidget.h"

namespace PokemonAutomation{



CameraSelectorWidget::~CameraSelectorWidget(){
//    cout << "~CameraSelectorUI()" << endl;
}
CameraSelectorWidget::CameraSelectorWidget(
    QWidget& parent,
    Logger& logger,
    CameraSelector& value,
    VideoDisplayWidget& holder
)
    : QWidget(&parent)
    , m_logger(logger)
    , m_value(value)
    , m_display(holder)
    , m_camera_box(nullptr)
    , m_resolution_box(nullptr)
    , m_snapshots_allowed(true)
{
    QHBoxLayout* camera_row = new QHBoxLayout(this);
    camera_row->setContentsMargins(0, 0, 0, 0);

    camera_row->addWidget(new QLabel("<b>Camera:</b>", this), 1);
    camera_row->addSpacing(5);

    m_camera_box = new NoWheelComboBox(this);
    camera_row->addWidget(m_camera_box, 5);
    refresh();
    camera_row->addSpacing(5);

    m_resolution_box = new NoWheelComboBox(this);
    camera_row->addWidget(m_resolution_box, 3);
    camera_row->addSpacing(5);

    m_reset_button = new QPushButton("Reset Camera", this);
    camera_row->addWidget(m_reset_button, 1);

    connect(
        m_camera_box, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
        this, [=](int index){
            CameraInfo& current = m_value.m_camera;
            if (index <= 0 || index > (int)m_cameras.size()){
                current = CameraInfo();
            }else{
                const CameraInfo& camera = m_cameras[index - 1];
                if (current == camera){
                    return;
                }
                current = camera;
            }
            reset_video();
        }
    );
    connect(
        m_resolution_box, static_cast<void(QComboBox::*)(int)>(&QComboBox::activated),
        this, [=](int index){
            if (!m_display){
                return;
            }
            if (index < 0 || index >= (int)m_resolutions.size()){
                return;
            }
            QSize resolution = m_resolutions[index];
            m_value.m_current_resolution = resolution;
            m_display.set_resolution(resolution);
        }
    );
    connect(
        m_reset_button, &QPushButton::clicked,
        this, [=](bool){
            reset_video();
        }
    );
    connect(
        this, &CameraSelectorWidget::internal_async_reset_video,
        this, [=]{
            reset_video();
        }
    );
}
void CameraSelectorWidget::refresh(){
    m_camera_box->clear();
    m_camera_box->addItem("(none)");

    m_cameras = get_all_cameras();
    CameraInfo& current_camera = m_value.m_camera;

    size_t index = 0;
    for (size_t c = 0; c < m_cameras.size(); c++){
        const CameraInfo& camera = m_cameras[c];
        m_camera_box->addItem(get_camera_name(camera));

        if (current_camera == camera){
            index = c + 1;
        }
    }
    if (index != 0){
        m_camera_box->setCurrentIndex((int)index);
    }else{
        current_camera = CameraInfo();
        m_camera_box->setCurrentIndex(0);
    }
}
QString CameraSelectorWidget::aspect_ratio(const QSize& size){
    int w = size.width();
    int h = size.height();
    if (w <= 0 || h <= 0){
        return "";
    }
    int gcd;
    while (true){
        if (h == 0){
            gcd = w;
            break;
        }
        w %= h;
        if (w == 0){
            gcd = h;
            break;
        }
        h %= w;
    }
    w = size.width();
    h = size.height();
    w /= gcd;
    h /= gcd;
    return "(" + QString::number(w) + ":" + QString::number(h) + ")";
}




void CameraSelectorWidget::reset_video(){
    std::lock_guard<std::mutex> lg(m_camera_lock);
    m_display.close_video();

    const CameraInfo& info = m_value.m_camera;
    if (info){
        m_display.set_video(make_video_factory(m_logger, info, m_value.m_current_resolution));
    }

    QSize resolution = m_display.resolution();

    //  Update resolutions dropdown.
    m_resolution_box->clear();
    m_resolutions = m_display.resolutions();

    int index = -1;
    bool resolution_match = false;
    for (int c = 0; c < (int)m_resolutions.size(); c++){
        const QSize& size = m_resolutions[c];
        m_resolution_box->addItem(
            QString::number(size.width()) + " x " + QString::number(size.height()) + " " + aspect_ratio(size)
        );
        if (size == m_value.m_current_resolution){
            resolution_match = true;
            index = c;
//            cout << "index0 = " << index << endl;
        }
        if (!resolution_match && size == resolution){
            index = c;
        }
    }
    if (index >= 0){
        m_value.m_current_resolution = m_resolutions[index];
        m_resolution_box->setCurrentIndex(index);
        m_resolution_box->activated(index);
    }else{
        //  Reset to default resolution.
        m_value.m_current_resolution = m_value.m_default_resolution;
    }
}
void CameraSelectorWidget::async_reset_video(){
    internal_async_reset_video();
}



void CameraSelectorWidget::set_camera_enabled(bool enabled){
    m_camera_box->setEnabled(enabled);
}
void CameraSelectorWidget::set_resolution_enabled(bool enabled){
    m_resolution_box->setEnabled(enabled);
}
void CameraSelectorWidget::set_snapshots_allowed(bool enabled){
    m_snapshots_allowed.store(enabled, std::memory_order_release);
}
void CameraSelectorWidget::set_overlay_enabled(bool enabled){
    m_display.overlay().setHidden(!enabled);
}

QImage CameraSelectorWidget::snapshot(){
    if (!m_snapshots_allowed.load(std::memory_order_acquire)){
        return QImage();
    }

    std::unique_lock<std::mutex> lg(m_camera_lock);
    if (!m_display){
        return QImage();
    }

    return m_display.snapshot();
}





}
