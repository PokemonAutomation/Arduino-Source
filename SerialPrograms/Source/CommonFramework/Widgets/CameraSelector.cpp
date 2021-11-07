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

CameraSelectorUI* CameraSelector::make_ui(QWidget& parent, Logger& logger, QWidget& holder){
    return new CameraSelectorUI(parent, logger, *this, holder);
}





CameraSelectorUI::~CameraSelectorUI(){
//    cout << "~CameraSelectorUI()" << endl;
}
CameraSelectorUI::CameraSelectorUI(
    QWidget& parent,
    Logger& logger,
    CameraSelector& value,
    QWidget& holder
)
    : QWidget(&parent)
    , m_logger(logger)
    , m_value(value)
    , m_holder(holder)
    , m_camera_box(nullptr)
    , m_resolution_box(nullptr)
    , m_snapshots_allowed(true)
{
    QHBoxLayout* camera_row = new QHBoxLayout(this);
    camera_row->setMargin(0);

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

    m_overlay = new VideoOverlayWidget(m_holder);

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
            if (m_video == nullptr){
                return;
            }
            std::vector<QSize> resolutions = m_video->resolutions();
            if (index < 0 || index >= (int)resolutions.size()){
                return;
            }

            QSize resolution = resolutions[index];
            m_video->set_resolution(resolution);
            m_overlay->update_size(m_holder.size(), resolution);
        }
    );
    connect(
        m_reset_button, &QPushButton::clicked,
        this, [=](bool){
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
    if (m_video != nullptr){
        delete m_video;
        m_video = nullptr;
    }

    const QCameraInfo& info = m_value.m_camera;

    //  If we change video implementations, here's what we change.
    m_video = new QtVideoWidget(m_holder, m_logger, info, m_value.m_resolution);

    m_holder.layout()->addWidget(m_video);

    QSize resolution = m_video->resolution();
    m_overlay->update_size(m_holder.size(), resolution);

    //  Update resolutions dropdown.
    m_resolution_box->clear();
    std::vector<QSize> resolutions = m_video->resolutions();

    int index = -1;
    bool resolution_match = false;
    for (int c = 0; c < (int)resolutions.size(); c++){
        const QSize& size = resolutions[c];
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
        m_value.m_resolution = resolutions[index];
        m_resolution_box->setCurrentIndex(index);
        m_resolution_box->activated(index);
    }else{
        m_value.m_resolution = QSize();
    }

    m_overlay->raise();
//    update_size();
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
    if (m_overlay == nullptr){
        return;
    }
    m_overlay->setHidden(!enabled);
}
void CameraSelectorUI::update_size(){
    std::lock_guard<std::mutex> lg(m_camera_lock);
    if (m_video == nullptr){
        return;
    }

    QSize resolution = m_video->resolution();
//    cout << m_holder.width() << " x " << m_holder.height() << " | "
//         << resolution.width() << " x " << resolution.height() << endl;

    int height = (int)((double)m_holder.width() / resolution.width() * resolution.height());
//    cout << "desired height = " << height << endl;


    //  Safeguard against a resizing loop where the UI bounces between larger
    //  height with scroll bar and lower height with no scroll bar.
    auto iter = m_recent_heights.find(height);
    if (iter != m_recent_heights.end() && std::abs(height - m_holder.maximumHeight()) < 50){
//        cout << "Supressing potential infinite resizing loop." << endl;
        m_overlay->update_size(m_holder.size(), resolution);
        return;
    }

    m_height_history.push_back(height);
    m_recent_heights.insert(height);
    if (m_height_history.size() > 10){
        m_recent_heights.erase(m_height_history[0]);
        m_height_history.pop_front();
    }
//    cout << "before = " << m_video->height() << ", " << height << endl;
//    m_holder.setMaximumHeight(height);
    m_holder.setFixedHeight(height);
//    cout << "after  = " << m_video->height() << endl;
    m_overlay->update_size(m_holder.size(), resolution);

//    m_camera_view->frameGeometry()
//    QRect rect = m_camera_view->frameGeometry();
//    cout << rect.x() << ", " << rect.y() << " - " << rect.width() << " x " << rect.height() << endl;
//    QSize size = m_video->baseSize();
//    cout << size.width() << " x " << size.height() << endl;

//    m_overlay->update_size(m_video->size(), );
}

QImage CameraSelectorUI::snapshot(){
    if (!m_snapshots_allowed.load(std::memory_order_acquire)){
        return QImage();
    }

    std::unique_lock<std::mutex> lg(m_camera_lock);
    if (m_video == nullptr){
        return QImage();
    }

    return m_video->snapshot();
}
void CameraSelectorUI::add_box(const ImageFloatBox& box, QColor color){
    m_overlay->add_box(box, color);
}
void CameraSelectorUI::remove_box(const ImageFloatBox& box){
    m_overlay->remove_box(box);
}






}
