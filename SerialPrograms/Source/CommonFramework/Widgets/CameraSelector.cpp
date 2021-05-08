/*  Camera Selector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QJsonArray>
#include <QJsonObject>
#include <QHBoxLayout>
#include "Common/Compiler.h"
#include "Common/Qt/StringException.h"
#include "Common/Qt/QtJsonTools.h"
#include "CameraSelector.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{


const QString CameraSelector::JSON_CAMERA       = "Device";
const QString CameraSelector::JSON_RESOLUTION   = "Resolution";


CameraSelector::CameraSelector(QString label)
    : m_label(std::move(label))
    , m_resolution(1280, 720)
{}
CameraSelector::CameraSelector(QString label, const QJsonValue& json)
    : CameraSelector(std::move(label))
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

CameraSelectorUI* CameraSelector::make_ui(QWidget& parent, QWidget& holder){
    return new CameraSelectorUI(parent, *this, holder);
}



CameraSelectorUI::~CameraSelectorUI(){
//    cout << "~CameraSelectorUI()" << endl;
}
CameraSelectorUI::CameraSelectorUI(QWidget& parent, CameraSelector& value, QWidget& holder)
    : QWidget(&parent)
    , m_value(value)
    , m_holder(holder)
    , m_camera_box(nullptr)
    , m_resolution_box(nullptr)
    , m_camera(nullptr)
    , m_camera_view(nullptr)
    , m_snapshots_allowed(true)
{
    QHBoxLayout* camera_row = new QHBoxLayout(this);
    camera_row->setMargin(0);

    camera_row->addWidget(new QLabel("<b>Camera:</b>", this), 1);
    camera_row->addSpacing(5);

    m_camera_box = new QComboBox(this);
    camera_row->addWidget(m_camera_box, 5);
    refresh();
    camera_row->addSpacing(5);

    m_resolution_box = new QComboBox(this);
    camera_row->addWidget(m_resolution_box, 3);
    camera_row->addSpacing(5);

    m_reset_button = new QPushButton("Reset Camera", this);
    camera_row->addWidget(m_reset_button, 1);

    m_overlay = new VideoOverlay(m_holder);

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
            if (index < 0 || index >= m_resolutions.size()){
                return;
            }
            QSize resolution = m_resolutions[index];
//            cout << "index1 = " << index << endl;
            QCameraViewfinderSettings settings = m_camera->viewfinderSettings();
            if (settings.resolution() == resolution){
                return;
            }

            settings.setResolution(resolution);
            m_value.m_resolution = resolution;

//            std::lock_guard<std::mutex> lg(m_camera_lock);
            m_camera->setViewfinderSettings(settings);
//            resizeEvent(nullptr);
//            on_state_changed();
            m_overlay->update_size(m_holder.size(), resolution);

//            m_capture_done = true;
//            m_cv.notify_all();
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
    QSize resolution;
    {
        std::lock_guard<std::mutex> lg(m_camera_lock);
        if (m_camera != nullptr){
            delete m_camera_view;
            m_camera_view = nullptr;

            delete m_capture;
            m_capture = nullptr;

            delete m_camera;
            m_camera = nullptr;

            for (auto& item : m_pending_captures){
                item.second.status = CaptureStatus::COMPLETED;
                item.second.cv.notify_all();
            }
        }


        const QCameraInfo& info = m_value.m_camera;
        if (info.isNull()){
            m_resolution_box->clear();
            return;
        }


        m_camera = new QCamera(info, &m_holder);

        m_capture = new QCameraImageCapture(m_camera, &m_holder);
        m_capture->setCaptureDestination(QCameraImageCapture::CaptureToBuffer);
        m_camera->setCaptureMode(QCamera::CaptureStillImage);
        connect(
            m_capture, &QCameraImageCapture::imageCaptured,
            this, [&](int id, const QImage& preview){
                std::lock_guard<std::mutex> lg(m_camera_lock);
//                cout << "finish = " << id << endl;
                auto iter = m_pending_captures.find(id);
                if (iter == m_pending_captures.end()){
                    cout << "QCameraImageCapture::imageCaptured(): Unable to find capture id: " << id << endl;
                    return;
                }
                iter->second.status = CaptureStatus::COMPLETED;
                iter->second.image = preview;
                iter->second.cv.notify_all();
            }
        );
        connect(
            m_capture, static_cast<void(QCameraImageCapture::*)(int, QCameraImageCapture::Error, const QString&)>(&QCameraImageCapture::error),
            this, [&](int id, QCameraImageCapture::Error error, const QString& errorString){
                std::lock_guard<std::mutex> lg(m_camera_lock);
//                cout << "error = " << id << endl;
                cout << "QCameraImageCapture::error(): " << errorString.toUtf8().data() << endl;
                auto iter = m_pending_captures.find(id);
                if (iter == m_pending_captures.end()){
                    return;
                }
                iter->second.status = CaptureStatus::COMPLETED;
                iter->second.cv.notify_all();
            }
        );

        m_camera_view = new QCameraViewfinder(&m_holder);
        m_holder.layout()->addWidget(m_camera_view);
        m_camera_view->setMinimumSize(80, 45);
    //    m_camera_view->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Maximum);
        m_camera->setViewfinder(m_camera_view);
        m_camera->start();

        m_resolutions = m_camera->supportedViewfinderResolutions();

        QCameraViewfinderSettings settings = m_camera->viewfinderSettings();
        resolution = settings.resolution();
        m_overlay->update_size(m_holder.size(), resolution);
    }

    m_resolution_box->clear();
    int index = -1;
    bool resolution_match = false;
    for (int c = 0; c < m_resolutions.size(); c++){
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

    if (m_camera_view == nullptr){
        return;
    }
//    cout << this->width() << " x " << this->height() << " | "
//         << m_video->width() << " x " << m_video->height() << endl;
    QSize resolution = m_camera->viewfinderSettings().resolution();
    int height = (int)((double)m_holder.width() / resolution.width() * resolution.height());


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
    m_holder.setMaximumHeight(height);
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
    if (m_camera_view == nullptr){
        return QImage();
    }

    m_camera->searchAndLock();
    int id = m_capture->capture();
    m_camera->unlock();

//    cout << "start = " << id << endl;

    auto iter = m_pending_captures.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(id),
        std::forward_as_tuple()
    );
    if (!iter.second){
        return QImage();
    }
    PendingCapture& capture = iter.first->second;

    capture.cv.wait_for(
        lg,
        std::chrono::milliseconds(1000),
        [&]{
            return capture.status != CaptureStatus::PENDING;
        }
    );

    if (capture.status != CaptureStatus::COMPLETED){
        cout << "Capture timed out." << endl;
    }

    QImage ret = std::move(capture.image);
    m_pending_captures.erase(iter.first);
    return ret;
}
void CameraSelectorUI::operator+=(const InferenceBox& box){
    *m_overlay += box;
}
void CameraSelectorUI::operator-=(const InferenceBox& box){
    *m_overlay -= box;
}

#if 0
void CameraSelectorUI::test_draw(){
#if 0
    QPainter painter(m_camera_view);
    painter.setBrush(Qt::DiagCrossPattern);
    QPen pen;
    pen.setColor(Qt::green);
    pen.setWidth(5);
    painter.setPen(5);
    painter.drawRect(QRect(80, 120, 200, 100));
#endif
    cout << m_camera_view->width() << " x " << m_camera_view->height() << endl;
//    m_camera_view->grab().toImage().save("test.jpg");
}
#endif





}
