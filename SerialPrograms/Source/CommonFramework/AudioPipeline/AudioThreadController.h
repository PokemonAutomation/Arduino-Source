/*  Audio Thread Controller
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_AudioThreadController_H
#define PokemonAutomation_AudioThreadController_H

#include <vector>
#include <chrono>
#include <QObject>
#include <QThread>


// #define USE_FFTREAL

namespace PokemonAutomation{

class AudioInfo;
class AudioDisplayWidget;
class AudioWorker;
class FFTWorker;


// The class is responsible for the audio loop.
// When this class is constructed, it launches a QThread to run a QT event loop
// handling audio input and output.
// The code that runs the actual audio is AudioWorker `m_AudioWorker` defined in
// "AudioWorker.h". It will be put into a thread `m_audioThread` to run continuously.
// The controller also launches a QThread to run FFT computataion asynchronously.
// It connects signals from the audio thread to the FFT thread to pass FFT input
// in a thread-safe way.
// It also connects signals from the FFT thread to the audio display widget (passed
// as `parent` in the constructor) to return the FFT results to the UI thread.
class AudioThreadController: public QObject{
    //  Need to define this Q_OBJECT to use Qt's extra features
    //  like signals and slots on this class.
    Q_OBJECT

public:
    AudioThreadController(AudioDisplayWidget* parent, const AudioInfo& inputInfo, const AudioInfo& outputInfo);
    virtual ~AudioThreadController();

signals:
    void operate();

private:
    QThread m_audioThread;

    AudioWorker* m_AudioWorker = nullptr;

    QThread m_fftThread;
    FFTWorker* m_fftWorker = nullptr;

};




}
#endif