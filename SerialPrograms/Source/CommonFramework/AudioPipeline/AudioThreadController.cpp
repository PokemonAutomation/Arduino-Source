/*  Audio Thread Controller
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <iostream>
#include <QIODevice>
#include <QThread>
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/Logging/LoggerQt.h"
#include "AudioConstants.h"
#include "AudioDisplayWidget.h"
#include "AudioInfo.h"
#include "AudioWorker.h"
#include "AudioThreadController.h"

namespace PokemonAutomation{


AudioThreadController::AudioThreadController(
    LoggerQt& logger,
    AudioDisplayWidget* parent,
    const AudioDeviceInfo& inputInfo,
    AudioFormat inputFormat,
    const std::string& inputAbsoluteFilepath,
    const AudioDeviceInfo& outputInfo,
    float outputVolume
){
    QObject::setParent(parent);

//    std::cout << "Controller thread " << QThread::currentThread() << " " << inputAbsoluteFilepath.toStdString() << std::endl;

    // Note: there is no audio initialization work in AudioWorker constructor. This is intended.
    // Starting the audio will register internal QT audio code to the current thread. If the QT audio
    // classes are started AudioWorker constructor, they are registered to the main UI thread,
    // causing problems when it's moved to and run in m_audioThread.
    // So AudioWorker constructor is very light. The work to initialize and start audio processing
    // is done in AudioWorker::startAudio(). It will be called using a signal AudioThreadController::operate()
    // which is sent after the worker thread starts.
    m_AudioWorker = new AudioWorker(logger, inputInfo, inputFormat, inputAbsoluteFilepath, outputInfo, outputVolume);
    m_AudioWorker->moveToThread(&m_audioThread);
    connect(&m_audioThread, &QThread::finished, m_AudioWorker, &QObject::deleteLater);

    // Connect this controller to the audio worker to start the audio initialization on the audio thread.
    connect(this, &AudioThreadController::operate, m_AudioWorker, &AudioWorker::startAudio);

    connect(m_AudioWorker, &AudioWorker::fftOutputReady, parent, &AudioDisplayWidget::loadFFTOutput);

    connect(parent, &AudioDisplayWidget::volumeChanged, m_AudioWorker, &AudioWorker::setVolume);

    m_audioThread.start();

    GlobalSettings::instance().REALTIME_THREAD_PRIORITY0.set_on_qthread(m_audioThread);

    // Send the signal to start audio processing after the worker thread is started.
    emit operate();
}

AudioThreadController::~AudioThreadController(){
    m_audioThread.quit();
    m_audioThread.wait();

    // We shouldn't need to delete AudioWorker here as its deletion is connected to
    // QThread::finished.
    // TODO: if this `deleteLater` handlement is not 100% safe, we should send a signal
    // from the controller to the AudioWorker to let it close the audio pipeline before
    // the worker thread is stopped.

    // Same goes to FFTWorker.
}





}
