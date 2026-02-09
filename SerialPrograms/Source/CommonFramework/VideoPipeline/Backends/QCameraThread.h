/*  QCameraThread
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_VideoPipeline_QCameraThread_H
#define PokemonAutomation_VideoPipeline_QCameraThread_H

#include <QCamera>
#include "Common/Cpp/Logging/AbstractLogger.h"
#include "Common/Qt/GlobalThreadPoolsQt.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{


class QCameraThread : public QObject{
public:
    QCameraThread(
        Logger& logger,
        QCameraDevice device,
        QCameraFormat format
    )
        : m_logger(logger)
        , m_device(std::move(device))
        , m_format(std::move(format))
        , m_camera(nullptr)
    {
        m_camera = static_cast<QCamera*>(GlobalThreadPools::qt_event_threadpool().add_object(
            [this]{ return make_camera(); }
        ));
    }
    ~QCameraThread(){
        GlobalThreadPools::qt_event_threadpool().remove_object(m_camera);
    }

    QCamera& camera() const{
        return *m_camera;
    }


private:
    std::unique_ptr<QCamera> make_camera(){
        auto camera = std::make_unique<QCamera>(m_device);
        camera->setCameraFormat(m_format);
        connect(
            camera.get(), &QCamera::errorOccurred,
            this, [&](){
                if (camera->error() == QCamera::NoError){
                    return;
                }
                m_logger.log("QCamera error: " + camera->errorString().toStdString(), COLOR_RED);
            }
        );

        camera->start();
        return camera;
    }


private:
    Logger& m_logger;
    QCameraDevice m_device;
    QCameraFormat m_format;
    QCamera* m_camera;
};



}
#endif
