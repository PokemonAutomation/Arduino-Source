/*  Audio Worker
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_AudioPipeline_AudioWorker_H
#define PokemonAutomation_AudioPipeline_AudioWorker_H

#include <vector>
#include <chrono>
#include <QObject>
#include <QThread>
#include <QVector>
#include <QIODevice>
#include <QAudioFormat>
#include "Common/Compiler.h"
#include "Common/Cpp/AlignedVector.h"
#include "AudioInfo.h"
#include "AudioIODevice.h"


#if QT_VERSION_MAJOR == 5
    class QAudioInput;
    class QAudioOutput;
#elif QT_VERSION_MAJOR == 6
    class QAudioSource;
    class QAudioSink;
#endif



namespace PokemonAutomation{


// This is the main Audio IO class, which owns all the audio input/output component.
// This class will be moved into a QThread, separate from the main Qt UI thread so
// that audio processing does not block the UI and vice versa.
// The class, AudioThreadController is responsible for the management of the QThread.
class AudioWorker: public QObject{
    //  Need to define this Q_OBJECT to use Qt's extra features
    //  like signals and slots on this class.
    Q_OBJECT
public:
    AudioWorker(const AudioInfo& inputInfo, const AudioInfo& outputInfo, float outputVolume);
    virtual ~AudioWorker();

    // Initialize all the Qt audio components to start all audio work: read from audio input device,
    // save audio samples for FFT and pass audio to audio output device.
    void startAudio();

public slots:
    // Set volume of the audio output:
    void setVolume(float volume);

signals:
    // We would like to connect signals of FFT input from the audio thread to the FFT thread.
    // The main work that passes around audio data is in m_audioIODevice. To expose this signal
    // to AudioThreadController, we have this signal in AudioWorker that serves as relaying the
    // signal from m_audioIODevice to anywhere that sees this AudioWorker.
    void fftInputReady(const QVector<float>& fftInput);

private:
    QAudioFormat m_audioFormat;
    AudioInfo m_inputInfo;
    AudioInfo m_outputInfo;

    AudioIODevice* m_audioIODevice = nullptr;

#if QT_VERSION_MAJOR == 5
    QAudioInput* m_audioSource = nullptr;
    QAudioOutput* m_audioSink = nullptr;
#elif QT_VERSION_MAJOR == 6
    QAudioSource* m_audioSource = nullptr;
    QAudioSink* m_audioSink = nullptr;
#endif

    ChannelMode m_channelMode = ChannelMode::Mono;

    float m_volume = 1.0f;
};





}
#endif
