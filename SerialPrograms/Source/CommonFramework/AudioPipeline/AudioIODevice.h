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
#include "IO/AudioSource.h"

class QObject;

namespace PokemonAutomation{

class AudioSink;


// QIODevice defines an interface for IO.
// QAudioSource reads from an audio input device and writes data to a QIODevice.
// The data is then passed to QAudioSink's internal QIODevice and sends to an audio output device.
// This class inherits a QIODevice to receive data from QAudioSource and push the data to
// QAudioSink.
// When enough audio samples are collected for the next FFT window, this class also emits a signal
// containing the FFT input samples so that the FFTWorker in the FFT thread can receive it
// to do FFT asynchronously, without blocking the audio thread.
class AudioIODevice : public QObject{
    //  Need to define this Q_OBJECT to use Qt's extra features
    //  like signals and slots on this class.
    Q_OBJECT

public:
    AudioIODevice(Logger& logger, const std::string& file, AudioFormat our_format, AudioSampleFormat input_format);
    AudioIODevice(Logger& logger, const AudioDeviceInfo& device, AudioFormat our_format, AudioSampleFormat input_format);
    virtual ~AudioIODevice();

    void setAudioSinkDevice(std::unique_ptr<AudioSink> writer);
    void set_volume(float volume);

signals:
    // Signal filed whenever the FFT input buffer is filled.
//    void fftInputReady(size_t sampleRate, std::shared_ptr<AlignedVector<float>> fftInput);
    void fftOutputReady(size_t sampleRate, std::shared_ptr<AlignedVector<float>> fftOutput);


private:
    void make_FFT_runner();

private:
    AudioFormat m_format;

    std::unique_ptr<AudioSource> m_reader;
    std::unique_ptr<FFTRunner> m_fft_runner;

    std::unique_ptr<AudioSink> m_writer;
    QIODevice* m_audioSinkDevice = nullptr;


};



}
#endif
