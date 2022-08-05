/*  Audio Thread Controller
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_AudioPipeline_AudioThreadController_H
#define PokemonAutomation_AudioPipeline_AudioThreadController_H

#include <string>
#include <QObject>
#include "AudioInfo.h"
#include "Spectrum/FFTStreamer.h"
#include "AudioPassthroughPairQtThread.h"

namespace PokemonAutomation{

class AudioDeviceInfo;
class AudioDisplayWidget;
class AudioWorker;



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
class AudioThreadController: public QObject, private FFTListener{
public:
    AudioThreadController(
        Logger& logger,
        AudioDisplayWidget* parent,
        const AudioDeviceInfo& inputInfo,
        AudioChannelFormat inputFormat,
        const std::string& inputAbsoluteFilepath,
        const AudioDeviceInfo& outputInfo,
        float outputVolume
    );
    virtual ~AudioThreadController();

private:
    virtual void on_fft(size_t sample_rate, std::shared_ptr<AlignedVector<float>> fft_output) override;

private:
    AudioDisplayWidget& m_parent;
    AudioPassthroughPairQtThread m_devices;

};




}
#endif
