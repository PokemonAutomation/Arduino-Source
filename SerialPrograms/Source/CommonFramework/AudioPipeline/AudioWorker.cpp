/*  Audio Worker
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "AudioConstants.h"
#include "AudioInfo.h"
#include "AudioWorker.h"
#include "Tools/AudioFormatUtils.h"
#include "IO/AudioFileLoader.h"
#include "CommonFramework/Logging/LoggerQt.h"

#include <QIODevice>
#include <QThread>

#if QT_VERSION_MAJOR == 5
#include <QAudioDeviceInfo>
#include <QAudioInput>
#include <QAudioOutput>
#include <QtEndian>
using AudioSink = QAudioOutput;
#elif QT_VERSION_MAJOR == 6
#include <QMediaDevices>
#include <QAudioSource>
#include <QAudioSink>
#include <QAudioDevice>
using NativeAudioSink = QAudioSink;
#endif


#include <iomanip>
#include <cfloat>
#include <memory>
#include <cassert>
#include <cstring>
#include <set>

#include <iostream>
using std::cout;
using std::endl;

//#define DEBUG_AUDIO_IO

namespace PokemonAutomation{



AudioWorker::AudioWorker(
    Logger& logger,
    const AudioDeviceInfo& inputInfo,
    AudioChannelFormat inputFormat,
    const std::string& inputAbsoluteFilepath,
    const AudioDeviceInfo& outputInfo,
    float outputVolume
)
    : m_logger(logger)
    , m_inputInfo(inputInfo)
    , m_inputFormat(inputFormat)
    , m_inputAbsoluteFilepath(inputAbsoluteFilepath)
    , m_outputInfo(outputInfo)
    , m_audioIODevice(logger)
    , m_volume(std::max(std::min(outputVolume, 1.0f), 0.0f))
{
    qRegisterMetaType<std::shared_ptr<AlignedVector<float>>>("std::shared_ptr<AlignedVector<float>>");
    qRegisterMetaType<std::shared_ptr<const AlignedVector<float>>>("std::shared_ptr<const AlignedVector<float>>");
    m_audioIODevice.add_listener(*this);
}


void AudioWorker::startAudio(){
    if (m_inputAbsoluteFilepath.size() > 0){
        m_audioIODevice.set_audio_source(m_inputAbsoluteFilepath);
    }else{
        m_audioIODevice.set_audio_source(m_inputInfo, m_inputFormat);
    }
    m_audioIODevice.set_audio_sink(m_outputInfo, m_volume);
}

AudioWorker::~AudioWorker(){
    m_audioIODevice.remove_listener(*this);
    m_logger.log("AudioWorker destroyed");
}

void AudioWorker::on_fft(size_t sampleRate, std::shared_ptr<AlignedVector<float>> fftOutput){
    emit fftOutputReady(sampleRate, std::move(fftOutput));
}

void AudioWorker::setVolume(float volume){
    volume = std::max(std::min(volume, 1.0f), 0.0f);
    m_volume = volume;
    m_audioIODevice.set_sink_volume(volume);
}







}
