/*  Camera Selector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QJsonArray>
#include <QJsonObject>
#include <QHBoxLayout>
#include <QLabel>
#include "Common/Compiler.h"
#include "Common/Qt/QtJsonTools.h"
#include "Common/Qt/NoWheelComboBox.h"
#include "QtVideoWidget.h"
#include "CameraSelector.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{


const QString CameraSelector::JSON_CAMERA       = "Device";
const QString CameraSelector::JSON_RESOLUTION   = "Resolution";


CameraSelector::CameraSelector()
    : m_resolution(1920, 1080)
{}
CameraSelector::CameraSelector(const QJsonValue& json)
    : CameraSelector()
{
    load_json(json);
}

void CameraSelector::load_json(const QJsonValue& json){
    QJsonObject obj = json.toObject();
    QString name;
    if (!json_get_string(name, obj, JSON_CAMERA)){
        return;
    }
    for (const auto& item : QCameraInfo::availableCameras()){
        if (name == item.deviceName()){
            m_camera = item;
            break;
        }
    }
    QJsonArray res = json_get_array_nothrow(obj, JSON_RESOLUTION);
    if (res.size() == 2 && res[0].isDouble() && res[1].isDouble()){
        m_resolution = QSize(res[0].toInt(), res[1].toInt());
    }
}
QJsonValue CameraSelector::to_json() const{
    QJsonObject root;
    root.insert(JSON_CAMERA, m_camera.deviceName());
    QJsonArray res;
    res += m_resolution.width();
    res += m_resolution.height();
    root.insert(JSON_RESOLUTION, res);
    return root;
}

CameraSelectorUI* CameraSelector::make_ui(QWidget& parent, Logger& logger, VideoDisplayWidget& holder){
    return new CameraSelectorUI(parent, logger, *this, holder);
}





CameraSelectorUI::~CameraSelectorUI(){
//    cout << "~CameraSelectorUI()" << endl;
}
CameraSelectorUI::CameraSelectorUI(
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
            QCameraInfo& current_port = m_value.m_camera;
            if (index <= 0 || index > m_cameras.size()){
                current_port = QCameraInfo();
            }else{
                const QCameraInfo& camera = m_cameras[index - 1];
                if (!current_port.isNull() && current_port.deviceName() == camera.deviceName()){
                    return;
                }
                current_port = camera;
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
            m_value.m_resolution = resolution;
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
        this, &CameraSelectorUI::internal_async_reset_video,
        this, [=]{
            reset_video();
        }
    );
}
void CameraSelectorUI::refresh(){
    m_camera_box->clear();
    m_camera_box->addItem("(none)");

    m_cameras = QCameraInfo::availableCameras();
    QCameraInfo& current_camera = m_value.m_camera;

    int index = 0;
    for (int c = 0; c < m_cameras.size(); c++){
        const QCameraInfo& camera = m_cameras[c];
        m_camera_box->addItem(camera.description());

        if (!current_camera.isNull() && current_camera.deviceName() == camera.deviceName()){
            index = c + 1;
        }
    }
    if (index != 0){
        m_camera_box->setCurrentIndex(index);
    }else{
        current_camera = QCameraInfo();
        m_camera_box->setCurrentIndex(0);
    }
}
QString CameraSelectorUI::aspect_ratio(const QSize& size){
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
void CameraSelectorUI::reset_video(){
    std::lock_guard<std::mutex> lg(m_camera_lock);
    m_display.close_video();

    const QCameraInfo& info = m_value.m_camera;

    //  If we change video implementations, here's what we change.
    if (!info.isNull()){
        m_display.set_video(new QtVideoWidget(m_logger, info, m_value.m_resolution));
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
        if (size == m_value.m_resolution){
            resolution_match = true;
            index = c;
//            cout << "index0 = " << index << endl;
        }
        if (!resolution_match && size == resolution){
            index = c;
        }
    }
    if (index >= 0){
        m_value.m_resolution = m_resolutions[index];
        m_resolution_box->setCurrentIndex(index);
        m_resolution_box->activated(index);
    }else{
        m_value.m_resolution = QSize();
    }
}
void CameraSelectorUI::async_reset_video(){
    internal_async_reset_video();
}



void CameraSelectorUI::set_camera_enabled(bool enabled){
    m_camera_box->setEnabled(enabled);
}
void CameraSelectorUI::set_resolution_enabled(bool enabled){
    m_resolution_box->setEnabled(enabled);
}
void CameraSelectorUI::set_snapshots_allowed(bool enabled){
    m_snapshots_allowed.store(enabled, std::memory_order_release);
}
void CameraSelectorUI::set_overlay_enabled(bool enabled){
    m_display.overlay().setHidden(!enabled);
}

QImage CameraSelectorUI::snapshot(){
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
