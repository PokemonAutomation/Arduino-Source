/*  Audio Thread Controller
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_AudioPipeline_FFTWorker_H
#define PokemonAutomation_AudioPipeline_FFTWorker_H

#include <vector>
#include <chrono>
#include <QObject>
#include <QVector>
#include <QAudioFormat>
#include "Common/Cpp/AlignedVector.h"
#include "FFTWorker.h"

namespace PokemonAutomation{



// This is the main class to compute FFT. It receives signals from the audio IO code
// to get FFT input audio samples, and send signals of the computed FFT results to
// the audio UI in the UI thread for visualization.
// This class will be moved into a QThread, separate from the main Qt UI thread and
// audio IO thread so that FFT computation does not block them.
// The class, AudioThreadController is responsible for the management of the QThread.
class FFTWorker: public QObject{
    //  Need to define this Q_OBJECT to use Qt's extra features
    //  like signals and slots on this class.
    Q_OBJECT

public:
    FFTWorker(int fftLengthPowerOfTwo);
    virtual ~FFTWorker();

    const float* fftKernel(const float* input);

    size_t outputSize() const { return m_fftOutputBuffer.size(); }

public slots:
    // Will be connected to the audio IO code to receive fft input audio sample vector.
    void computeFFT(const QVector<float>& rawAudioSamples);

signals:
    // Will be connected to the audio UI widget to display FFT results.
    void FFTFinished(const QVector<float>& fftOutput);

private:

    int m_fftLengthPowerOfTwo = 0;
    size_t m_fftLength;

    // The buffer to store fft input
    AlignedVector<float> m_fftInputBuffer;
    // The buffer to save fft output.
    AlignedVector<float> m_fftOutputBuffer;

    QVector<float> m_outputVector;
};



}
#endif
