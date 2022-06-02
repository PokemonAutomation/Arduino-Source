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
#include "Common/Cpp/Time.h"
#include "Common/Cpp/AlignedVector.h"
#include "AudioInfo.h"
#include "AudioStream.h"

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
    AudioIODevice(AudioFormat format);
    virtual ~AudioIODevice();

    // Called by QAudioSource to write data to AudioIODevice's buffer.
    // The function also does FFT if enough sample is collected on the next
    // windows.
    // if an QAudioSink's internal QIODevice is set via setAudioSinkDevice(),
    // also pass data to the audio sink.
    qint64 writeData(const char *data, qint64 len) override;

    qint64 readData(char *data, qint64 maxlen) override { return 0; }

    bool isSequential() const override { return true; }

    void setAudioSinkDevice(QIODevice* audioSinkDevice);

signals:
    // Signal filed whenever the FFT input buffer is filled.
    void fftInputReady(size_t sampleRate, std::shared_ptr<AlignedVector<float>> fftInput);

private:
    AudioFormat m_format;

    std::unique_ptr<AudioSourceReader> m_reader;
    std::unique_ptr<AudioSinkWriter> m_output;
    std::unique_ptr<FFTRunner> m_fft_runner;

    QIODevice* m_audioSinkDevice = nullptr;


};



}
#endif
