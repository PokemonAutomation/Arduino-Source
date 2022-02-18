/*  Audio Thread Controller
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "AudioConstants.h"
#include "AudioDisplayWidget.h"
#include "AudioInfo.h"
#include "AudioWorker.h"
#include "AudioThreadController.h"
#include "FFTWorker.h"
#include "CommonFramework/Logging/Logger.h"

#include <QIODevice>
#include <QThread>
#include <QString>

#include <iostream>

namespace PokemonAutomation{


AudioThreadController::AudioThreadController(
    AudioDisplayWidget* parent,
    const AudioInfo& inputInfo,
    const QString& inputAbsoluteFilepath,
    const AudioInfo& outputInfo,
    float outputVolume
){
    QObject::setParent(parent);

    // std::cout << "Controller thread " << QThread::currentThread() << " " << inputAbsoluteFilepath.toStdString() << std::endl;

    // Note: there is no audio initialization work in AudioWorker constructor. This is intended.
    // Starting the audio will register internal QT audio code to the current thread. If the QT audio
    // classes are started AudioWorker constructor, they are registered to the main UI thread,
    // causing problems when it's moved to and run in m_audioThread.
    // So AudioWorker constructor is very light. The work to initialize and start audio processing
    // is done in AudioWorker::startAudio(). It will be called using a signal AudioThreadController::operate()
    // which is sent after the worker thread starts.
    m_AudioWorker = new AudioWorker(inputInfo, inputAbsoluteFilepath, outputInfo, outputVolume);
    m_AudioWorker->moveToThread(&m_audioThread);
    connect(&m_audioThread, &QThread::finished, m_AudioWorker, &QObject::deleteLater);

    // Connect this controller to the audio worker to start the audio initialization on the audio thread.
    connect(this, &AudioThreadController::operate, m_AudioWorker, &AudioWorker::startAudio);

    m_fftWorker = new FFTWorker(FFT_LENGTH_POWER_OF_TWO);
    m_fftWorker->moveToThread(&m_fftThread);
    connect(&m_fftThread, &QThread::finished, m_fftWorker, &QObject::deleteLater);

    // Connect the audio thread and fft thread to pass fft inputs.
    connect(m_AudioWorker, &AudioWorker::fftInputReady, m_fftWorker, &FFTWorker::computeFFT);

    // Connect fft thread to audio display widget to pass fft outputs.
    connect(m_fftWorker, &FFTWorker::FFTFinished, parent, &AudioDisplayWidget::loadFFTOutput);

    connect(parent, &AudioDisplayWidget::volumeChanged, m_AudioWorker, &AudioWorker::setVolume);
    
    m_audioThread.start();
    m_fftThread.start();

    // Send the signal to start audio processing after the worker thread is started.
    emit operate();
}

AudioThreadController::~AudioThreadController(){
    m_audioThread.quit();
    m_fftThread.quit();

    m_audioThread.wait();
    m_fftThread.wait();

    // We shouldn't need to delete AudioWorker here as its deletion is connected to
    // QThread::finished.
    // TODO: if this `deleteLater` handlement is not 100% safe, we should send a signal
    // from the controller to the AudioWorker to let it close the audio pipeline before
    // the worker thread is stopped.

    // Same goes to FFTWorker.
}





}
