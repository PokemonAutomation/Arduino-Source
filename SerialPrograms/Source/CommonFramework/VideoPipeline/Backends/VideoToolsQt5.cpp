/*  Video Tools (QT5)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QtGlobal>
#if QT_VERSION_MAJOR == 5

#include <QCameraInfo>
#include <QCoreApplication>
#include "Common/Cpp/PrettyPrint.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/ImageTools/ImageDiff.h"
#include "VideoToolsQt5.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{



std::vector<CameraInfo> qt5_get_all_cameras(){
    QList<QCameraInfo> cameras = QCameraInfo::availableCameras();
    std::vector<CameraInfo> ret;
    for (const QCameraInfo& info : cameras){
        ret.emplace_back(info.deviceName().toStdString());
    }
    return ret;
}

std::string qt5_get_camera_name(const CameraInfo& info){
    QCameraInfo qinfo(info.device_name().c_str());
    return qinfo.description().toStdString();
}




QImage frame_to_image(Logger& logger, QVideoFrame frame, bool flip_vertical){
    if (!frame.isValid()){
        return QImage();
    }
    if (!frame.map(QAbstractVideoBuffer::ReadOnly)){
        logger.log("Unable to map QVideoFrame.");
        return QImage();
    }

    try{
        if (frame.pixelFormat() == QVideoFrame::Format_Jpeg){
            QImage image;
            bool ok = image.loadFromData(frame.bits(0), frame.mappedBytes(), "JPG");
            frame.unmap();
            if (!ok){
//                cout << frame.mappedBytes() << " " << frame.width() << endl;
                return QImage();
            }
            if (flip_vertical){
                image = image.mirrored(false, flip_vertical);
            }
            return image;
        }

        QImage::Format format = QVideoFrame::imageFormatFromPixelFormat(frame.pixelFormat());
        QImage image(
            frame.bits(),
            frame.width(),
            frame.height(),
            frame.bytesPerLine(),
            format
        );
        int64_t key = image.cacheKey();
        image = image.convertToFormat(QImage::Format_RGB32);
        frame.unmap();
        image = image.mirrored(false, flip_vertical);
        if (key == image.cacheKey()){
            image = image.copy();
        }
        return image;
    }catch (...){
        frame.unmap();
    }
    return QImage();
}


bool determine_frame_orientation(
    Logger& logger, const ImageViewRGB32& reference, const ImageViewRGB32& image,
    bool& flip_vertical
){
    std::string str;
    str += "Attempting to determine frame orientation... ";

    if (!reference){
        str += "Image is null.";
        logger.log(str, COLOR_RED);
        return false;
    }
    if (!image){
        str += "Frame is null.";
        logger.log(str, COLOR_RED);
        return false;
    }

    ImageStats ref_stats = image_stats(reference);
    double ref_stddev = ref_stats.stddev.sum();
    if (ref_stddev < 10){
        str += "Image has too little detail. (rmsd = " + tostr_default(ref_stddev) + ")";
        logger.log(str, COLOR_RED);
        return false;
    }
    ImageStats img_stats = image_stats(image);
    double img_stddev = img_stats.stddev.sum();
    if (img_stddev < 10){
        str += "Frame has too little detail. (rmsd = " + tostr_default(img_stddev) + ")";
        logger.log(str, COLOR_RED);
        return false;
    }

    double stddev_inv = (ref_stddev + img_stddev) * 0.5;
    str += "\n";
    str += "    Stddev = " + tostr_default(stddev_inv) + "\n";

    stddev_inv = 1 / stddev_inv;

    double identity         = stddev_inv * ImageMatch::pixel_RMSD(reference, image);
    double flipped_vertical = stddev_inv * ImageMatch::pixel_RMSD(reference, image.to_QImage_ref().mirrored(false, true));
    str += "    Identity = " + tostr_default(identity) + "\n";
    str += "    Flipped Vertical = " + tostr_default(flipped_vertical) + "\n";

    str += "    Orientation: ";
    bool ok = false;
    do{
        if (identity < 0.05 && identity * 10 < flipped_vertical){
            flip_vertical = false;
            ok = true;
            str += "Identity";
            break;
        }
        if (flipped_vertical < 0.05 && flipped_vertical * 10 < identity){
            flip_vertical = true;
            ok = true;
            str += "Flipped Vertical";
            break;
        }
        str += "Unknown";
    }while (false);

    logger.log(str, ok ? COLOR_BLUE : COLOR_RED);

    return ok;
}






CameraScreenshotter::CameraScreenshotter(Logger& logger, QCamera& camera)
    : m_logger(logger)
    , m_camera(camera)
    , m_stats_conversion("Snapshot", "ms", 1000, std::chrono::seconds(10))
{
    m_capture = new QCameraImageCapture(&m_camera, &m_camera);
    m_capture->setCaptureDestination(QCameraImageCapture::CaptureToBuffer);
    m_camera.setCaptureMode(QCamera::CaptureStillImage);

    connect(
        m_capture, &QCameraImageCapture::imageCaptured,
        this, [&](int id, const QImage& preview){
            std::lock_guard<std::mutex> lg(m_lock);
            m_screenshot = preview;
            m_pending_screenshot = false;
            m_cv.notify_all();
        },
        Qt::ConnectionType::QueuedConnection
    );
    connect(
        m_capture, static_cast<void(QCameraImageCapture::*)(int, QCameraImageCapture::Error, const QString&)>(&QCameraImageCapture::error),
        this, [&](int id, QCameraImageCapture::Error error, const QString& errorString){
            std::lock_guard<std::mutex> lg(m_lock);
            m_logger.log(
                "QCameraImageCapture::error(): Capture ID: " + errorString.toStdString(),
                COLOR_RED
            );
            m_screenshot = QImage();
            m_pending_screenshot = false;
            m_cv.notify_all();
        },
        Qt::ConnectionType::QueuedConnection
    );
}
CameraScreenshotter::~CameraScreenshotter(){
    delete m_capture;
}
VideoSnapshot CameraScreenshotter::snapshot(){
    //  Only allow one snapshot at a time.
    WallClock timestamp = current_time();

    m_pending_screenshot = true;
    m_camera.searchAndLock();
    m_capture->capture();
    m_camera.unlock();

    QImage image;
    WallClock start = current_time();
    do{
        if (current_time() - start >= std::chrono::seconds(1)){
            m_logger.log("Capture timed out.");
            m_pending_screenshot = false;
            return VideoSnapshot{QImage(), timestamp};
        }
        QCoreApplication::instance()->processEvents(QEventLoop::AllEvents, 1);
        std::unique_lock<std::mutex> lg(m_lock);
        m_cv.wait_for(
            lg,
            std::chrono::milliseconds(1),
            [&]{ return !m_pending_screenshot; }
        );
        image = std::move(m_screenshot);
    }while (m_pending_screenshot);

    QImage::Format format = image.format();
    if (format != QImage::Format_ARGB32 && format != QImage::Format_RGB32){
        image = image.convertToFormat(QImage::Format_ARGB32);
    }

    WallClock time1 = current_time();
    m_stats_conversion.report_data(m_logger, std::chrono::duration_cast<std::chrono::microseconds>(time1 - timestamp).count());
    return VideoSnapshot(image, timestamp);
}
















}
#endif
