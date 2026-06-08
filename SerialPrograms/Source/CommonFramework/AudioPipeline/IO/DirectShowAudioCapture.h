/*  DirectShow Audio Capture (Windows-only)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *      Captures audio from a DirectShow video capture device's audio pin.
 *  This is needed for capture cards like the AVerMedia GC550 where the
 *  HDMI audio is embedded in the DirectShow video filter and not exposed
 *  as a standalone Windows audio endpoint.
 *
 */

#ifndef PokemonAutomation_AudioPipeline_DirectShowAudioCapture_H
#define PokemonAutomation_AudioPipeline_DirectShowAudioCapture_H

#ifdef _WIN32

#include <atomic>
#include <thread>
#include <string>

namespace PokemonAutomation{

class Logger;
class AudioStreamToFloat;


class DirectShowAudioCapture{
public:
    ~DirectShowAudioCapture();

    //  device_name: friendly name of the DirectShow video capture device.
    //  sample_rate: desired sample rate (e.g. 48000).
    //  channels:    desired channel count (e.g. 2).
    DirectShowAudioCapture(
        Logger& logger,
        AudioStreamToFloat& reader,
        const std::string& device_name,
        int sample_rate,
        int channels
    );

private:
    void capture_thread(
        const std::string& device_name,
        int sample_rate,
        int channels
    );

    Logger& m_logger;
    AudioStreamToFloat& m_reader;
    std::atomic<bool> m_stopping{false};
    std::thread m_thread;
};


}

#endif // _WIN32
#endif
