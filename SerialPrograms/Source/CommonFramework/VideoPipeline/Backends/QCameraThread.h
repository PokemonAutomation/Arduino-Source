/*  QCameraThread
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_VideoPipeline_QCameraThread_H
#define PokemonAutomation_VideoPipeline_QCameraThread_H

#include <QThread>
#include <QCamera>
#include "Common/Cpp/AbstractLogger.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{


class QCameraThread : public QThread{
    Q_OBJECT

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
        start();

        std::unique_lock<std::mutex> lg(m_lock);
        m_cv.wait(lg, [this]{ return m_camera != nullptr; });
    }
    ~QCameraThread(){
        quit();
        wait();
    }

    QCamera& camera() const{
        return *m_camera;
    }


private:
    virtual void run() override{
        QCamera camera(m_device);
        m_camera = &camera;
        camera.setCameraFormat(m_format);

        connect(&camera, &QCamera::errorOccurred, this, [&](){
            if (camera.error() == QCamera::NoError){
                return;
            }
            m_logger.log("QCamera error: " + camera.errorString().toStdString(), COLOR_RED);
        });

        camera.start();

        {
            std::lock_guard<std::mutex> lg(m_lock);
        }
        m_cv.notify_all();

//        cout << "start" << endl;
        exec();
//        cout << "end" << endl;

        m_camera = nullptr;
    }


private:
    Logger& m_logger;
    QCameraDevice m_device;
    QCameraFormat m_format;
    QCamera* m_camera;

    std::mutex m_lock;
    std::condition_variable m_cv;
};



}
#endif
