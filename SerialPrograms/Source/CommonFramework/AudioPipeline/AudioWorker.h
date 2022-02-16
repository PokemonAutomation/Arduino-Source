/*  Audio Worker
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_AudioWorker_H
#define PokemonAutomation_AudioWorker_H

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


#if QT_VERSION_MAJOR == 5
    class QAudioInput;
    class QAudioOutput;
#elif QT_VERSION_MAJOR == 6
    class QAudioSource;
    class QAudioSink;
#endif

namespace PokemonAutomation{

class AudioIODevice;


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

    enum class ChannelMode{
        Mono,
        Stereo,
        Interleaved,
    };

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


// QIODevice defines an interface for IO.
// QAudioSource reads from an audio input device and writes data to a QIODevice.
// The data is then passed to QAudioSink's internal QIODevice and sends to an audio output device.
// This class inherits a QIODevice to receive data from QAudioSource and push the data to
// QAudioSink.
// When enough audio samples are collected for the next FFT window, this class also emits a signal
// containing the FFT input samples so that the FFTWorker in the FFT thread can receive it
// to do FFT asynchronously, without blocking the audio thread.
class AudioIODevice : public QIODevice
{
    //  Need to define this Q_OBJECT to use Qt's extra features
    //  like signals and slots on this class.
    Q_OBJECT

public:
    AudioIODevice(const QAudioFormat& audioFormat, AudioWorker::ChannelMode channelMode);
    virtual ~AudioIODevice();

    // Called by QAudioSource to write data to AudioIODevice's buffer.
    // The function also does FFT if enough sample is collected on the next
    // windows.
    // if an QAudioSink's internal QIODevice is set via setAudioSinkDevice(),
    // also pass data to the audio sink.
    qint64 writeData(const char *data, qint64 len) override;

    qint64 readData(char *data, qint64 maxlen) override { return 0; }

    bool isSequential() const override { return true; }

    void setAudioSinkDevice(QIODevice* audioSinkDevice) { m_audioSinkDevice = audioSinkDevice; }

signals:
    // Signal filed whenever the FFT input buffer is filled.
    void fftInputReady(const QVector<float>& fftInput);

private:
    // how many more samples needed to file the next FFT
    size_t computeNextFFTSamplesNeeded() const;

    // move audio samples from m_fftCircularBuffer starting at m_fftStart to m_fftInputVector
    // to ready for next FFT signal.
    void moveDataToFFTInputVector();

    QAudioFormat m_audioFormat;
    AudioWorker::ChannelMode m_channelMode;

    // Used as a temporal buffer to swap L and R channels in the interleaved mode.
    std::vector<float> m_channelSwapBuffer;

    // A large circular buffer to store multiple sliding windows of FFT inputs.
    // Because FFT windows may overlap, it may be more efficient to store
    // them in this buffer.
    std::vector<float> m_fftCircularBuffer;

    // The index on m_fftCircularBuffer where to receive next incoming audio sample.
    size_t m_bufferNext = 0;
    
    // The index on the m_fftCircularBuffer as the next chunk of FFT input.
    size_t m_fftStart = 0;

    // The vector to store the input of one fft call.
    QVector<float> m_fftInputVector;

    QIODevice* m_audioSinkDevice = nullptr;

    using TimePoint = std::chrono::system_clock::time_point;
    // Used to measure the frequency of the audio loop for debugging.
    TimePoint m_lastWriteTimepoint;

#ifdef USE_FFTREAL
    FFTRealWrapper m_fft;
#endif
};



}
#endif
