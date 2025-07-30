/*  Camera Widget (Qt6.5)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_VideoPipeline_Qt65VideoWidget_H
#define PokemonAutomation_VideoPipeline_Qt65VideoWidget_H

#include <QtGlobal>
#if QT_VERSION_MAJOR == 6

//#include <set>
//#include <mutex>
#include <QCameraDevice>
#include <QMediaCaptureSession>
#include <QVideoFrame>
#include <QVBoxLayout>
#include <QVideoWidget>
#include <QGraphicsView>
#include <QGraphicsVideoItem>
#include <QKeyEvent>
#include "CommonFramework/Tools/StatAccumulator.h"
#include "CommonFramework/VideoPipeline/VideoSource.h"
#include "CommonFramework/VideoPipeline/CameraInfo.h"
#include "QCameraThread.h"
#include "QVideoFrameCache.h"
#include "SnapshotManager.h"
#include "CameraImplementations.h"

//#include <iostream>
//using std::cout;
//using std::endl;

class QCamera;
class QVideoSink;

namespace PokemonAutomation{
namespace CameraQt65QMediaCaptureSession{

class VideoDisplayWidget;


class CameraBackend : public PokemonAutomation::CameraBackend{
public:
    virtual std::vector<CameraInfo> get_all_cameras() const override;
    virtual std::string get_camera_name(const CameraInfo& info) const override;

    virtual std::unique_ptr<VideoSource> make_video_source(
        Logger& logger,
        const CameraInfo& info,
        Resolution resolution
    ) const override;
};


class StaticQGraphicsView : public QGraphicsView{
public:
    StaticQGraphicsView(QWidget* parent)
        : QGraphicsView(parent)
    {
        setContentsMargins(QMargins(0, 0, 0, 0));
        setFrameShape(QFrame::NoFrame);
        setStyleSheet("QGraphicsView { border-style: none; }");
        setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        setFocusPolicy(Qt::FocusPolicy::NoFocus);
    }

    virtual void wheelEvent(QWheelEvent* e) override{
        QFrame::wheelEvent(e);
    }
    virtual void resizeEvent(QResizeEvent* e) override{
        QFrame::resizeEvent(e);
    }

    //  QGraphicsView doesn't like to propagate mouse events.
    //  https://forum.qt.io/topic/152138/struggling-with-mouse-events/2
    virtual void mousePressEvent(QMouseEvent* event) override{
        QWidget::mousePressEvent(event);
    }
    virtual void mouseReleaseEvent(QMouseEvent* event) override{
        QWidget::mouseReleaseEvent(event);
    }
    virtual void mouseMoveEvent(QMouseEvent* event) override{
        QWidget::mouseMoveEvent(event);
    }

};





class CameraVideoSource : public QObject, public VideoSource{
public:
    virtual ~CameraVideoSource();
    CameraVideoSource(
        Logger& logger,
        const CameraInfo& info,
        Resolution desired_resolution
    );

    virtual Resolution current_resolution() const override{
        return m_resolution;
    }
    virtual const std::vector<Resolution>& supported_resolutions() const override{
        return m_resolutions;
    }

    virtual VideoSnapshot snapshot_latest_blocking() override{
        return m_snapshot_manager.snapshot_latest_blocking();
    }
    virtual VideoSnapshot snapshot_recent_nonblocking(WallClock min_time) override{
        return m_snapshot_manager.snapshot_recent_nonblocking(min_time);
    }

    virtual QWidget* make_display_QtWidget(QWidget* parent) override;

private:
    void set_video_output(QGraphicsVideoItem& item);


private:
    friend class CameraVideoDisplay;

    Logger& m_logger;
    Resolution m_resolution;

    std::unique_ptr<QCameraThread> m_camera;
    std::unique_ptr<QVideoSink> m_video_sink;
    std::unique_ptr<QMediaCaptureSession> m_capture_session;

    std::vector<Resolution> m_resolutions;


private:
    QVideoFrameCache m_last_frame;
    SnapshotManager m_snapshot_manager;
};


class CameraVideoDisplay : public QWidget{
public:
    CameraVideoDisplay(QWidget* parent, CameraVideoSource& source);

private:
    virtual void resizeEvent(QResizeEvent* event) override;

private:
    CameraVideoSource& m_source;

    StaticQGraphicsView* m_view;
    QGraphicsScene m_scene;
    QGraphicsVideoItem m_video;

    LifetimeSanitizer m_sanitizer;
};






}
}
#endif
#endif
