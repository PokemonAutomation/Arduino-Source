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
#include <QAudio>
#include "Common/Compiler.h"
#include "Common/Cpp/AlignedVector.h"
#include "CommonFramework/Logging/LoggerQt.h"
#include "AudioInfo.h"
#include "IO/AudioSource.h"
#include "IO/AudioSink.h"
#include "Spectrum/FFTStreamer.h"
#include "AudioIODevice.h"


#if QT_VERSION_MAJOR == 5
    class QAudioInput;
    class QAudioOutput;
#elif QT_VERSION_MAJOR == 6
    class QAudioSource;
    class QAudioSink;
#endif


namespace PokemonAutomation{

class AudioFileLoader;


// This is the main Audio IO class, which owns all the audio input/output component.
// This class will be moved into a QThread, separate from the main Qt UI thread so
// that audio processing does not block the UI and vice versa.
// The class, AudioThreadController is responsible for the management of the QThread.
class AudioWorker: public QObject, private FFTListener{
    //  Need to define this Q_OBJECT to use Qt's extra features
    //  like signals and slots on this class.
    Q_OBJECT
public:
    // If `inputAbsoluteFilepath` is not empty, load audio from file, otherwise, from the audio
    // device specified by `inputInfo`.
    // Note: we currently hardcode the audio format played from `inputAbsoluteFilepath` to be
    // mono channel 48KHz.
    // Note: there is a program-crashing bug in Qt's audio decoder that will be triggered
    // when `inputAbsoluteFilepath` is a relative path. Do not pass in a relative path.
    AudioWorker(
        Logger& logger,
        const AudioDeviceInfo& inputInfo,
        AudioChannelFormat inputFormat,
        const std::string& inputAbsoluteFilepath,
        const AudioDeviceInfo& outputInfo,
        float outputVolume
    );

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
//    void fftInputReady(size_t sampleRate, std::shared_ptr<AlignedVector<float>> fftInput);
    void fftOutputReady(size_t sampleRate, std::shared_ptr<AlignedVector<float>> fftOutput);


private:
    virtual void on_fft(size_t sampleRate, std::shared_ptr<AlignedVector<float>> fftOutput) override;

private:
    Logger& m_logger;
    AudioDeviceInfo m_inputInfo;
    AudioChannelFormat m_inputFormat;
    std::string m_inputAbsoluteFilepath;
    AudioDeviceInfo m_outputInfo;

    AudioIODevice m_audioIODevice;

    float m_volume = 1.0f;
};





}
#endif
