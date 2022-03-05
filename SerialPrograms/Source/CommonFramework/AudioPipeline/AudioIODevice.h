/*  Audio IO Device
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_AudioPipeline_AudioIODevice_H
#define PokemonAutomation_AudioPipeline_AudioIODevice_H

#include <memory>
#include <QVector>
#include <QAudioFormat>
#include <QIODevice>
#include "Common/Compiler.h"
#include "Common/Cpp/AlignedVector.h"
#include "AudioInfo.h"

namespace PokemonAutomation{


enum class ChannelMode{
    Mono,
    Stereo,
    Interleaved,
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
    AudioIODevice(const QAudioFormat& audioFormat, AudioFormat format);
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
    void fftInputReady(size_t sampleRate, std::shared_ptr<AlignedVector<float>> fftInput);

private:
    // how many more samples needed to file the next FFT
    size_t computeNextFFTSamplesNeeded() const;

    // move audio samples from m_fftCircularBuffer starting at m_fftStart to m_fftInputVector
    // to ready for next FFT signal.
    AlignedVector<float> moveDataToFFTInputVector();

    void write_output(const float* data, size_t samples);


private:
    QAudioFormat m_audioFormat;
    AudioFormat m_format;

    // Used as a temporal buffer to swap L and R channels in the interleaved mode.
    std::vector<float> m_channelSwapBuffer;

    std::vector<float> m_fft_input_buffer;

    // A large circular buffer to store multiple sliding windows of FFT inputs.
    // Because FFT windows may overlap, it may be more efficient to store
    // them in this buffer.
    std::vector<float> m_fftCircularBuffer;

    // The index on m_fftCircularBuffer where to receive next incoming audio sample.
    size_t m_bufferNext = 0;

    // The index on the m_fftCircularBuffer as the next chunk of FFT input.
    size_t m_fftStart = 0;

//    // The vector to store the input of one fft call.
//    QVector<float> m_fftInputVector;

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
