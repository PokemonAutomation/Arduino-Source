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
    AudioFormat inputFormat,
    const std::string& inputAbsoluteFilepath,
    const AudioDeviceInfo& outputInfo,
    float outputVolume
)
    : m_logger(logger)
    , m_inputInfo(inputInfo)
    , m_inputFormat(inputFormat)
    , m_inputAbsoluteFilepath(inputAbsoluteFilepath)
    , m_outputInfo(outputInfo)
    , m_volume(std::max(std::min(outputVolume, 1.0f), 0.0f))
{
    qRegisterMetaType<std::shared_ptr<AlignedVector<float>>>("std::shared_ptr<AlignedVector<float>>");
    qRegisterMetaType<std::shared_ptr<const AlignedVector<float>>>("std::shared_ptr<const AlignedVector<float>>");
}


void AudioWorker::startAudio(){
#ifdef DEBUG_AUDIO_IO
    std::cout << "T" << QThread::currentThread() << " AudioWorker::startAudio()" << std::endl;
#endif

    bool foundAudioInputInfo = false;
    bool foundAudioOutputInfo = false;

    QAudioFormat inputAudioFormat, outputAudioFormat;

    NativeAudioInfo chosenAudioInputDevice = m_inputInfo.native_info();
    foundAudioInputInfo = m_inputInfo;

    NativeAudioInfo chosenAudioOutputDevice = m_outputInfo.native_info();
    foundAudioOutputInfo = m_outputInfo;

    AudioFormat outputFormat = m_inputFormat;

    // If input filename is not empty, load audio from file:
    if (m_inputAbsoluteFilepath.size() > 0){
        m_audioIODevice = new AudioIODevice(m_logger, m_inputAbsoluteFilepath, AudioFormat::DUAL_48000, AudioSampleFormat::FLOAT32);
        setSampleFormatToFloat(outputAudioFormat);
        set_format(outputAudioFormat, AudioFormat::DUAL_48000);
        outputFormat = AudioFormat::DUAL_48000;
    }else{
        // Load from audio input device:

        if (foundAudioInputInfo == false){
            // std::cout << "Cannot build Qt6VideoWidget: cannot find audio device name matching: " << inputInfo.device_name() << std::endl;
            return;
        }

        inputAudioFormat = chosenAudioInputDevice.preferredFormat();

        m_logger.log("Default input audio format: " + dumpAudioFormat(inputAudioFormat));

        set_format(inputAudioFormat, m_inputFormat);

        //  See if the default format is good.
        if (get_sample_format(inputAudioFormat) == AudioSampleFormat::INVALID){
            //  If not, force it to float.
            setSampleFormatToFloat(inputAudioFormat);
        }

        if (!chosenAudioInputDevice.isFormatSupported(inputAudioFormat)){
            m_logger.log("Error: audio input device cannot support desired audio format", COLOR_RED);
            return;
        }

        m_audioIODevice = new AudioIODevice(m_logger, m_inputInfo, m_inputFormat, get_sample_format(inputAudioFormat));
    } // end if load audio from input audio device


    if (m_audioIODevice == nullptr){
        return;
    }

    switch (outputFormat){
    case AudioFormat::MONO_48000:
    case AudioFormat::MONO_96000:
    case AudioFormat::DUAL_44100:
    case AudioFormat::DUAL_48000:
        break;
    case AudioFormat::INTERLEAVE_LR_96000:
    case AudioFormat::INTERLEAVE_RL_96000:
        outputFormat = AudioFormat::DUAL_48000;
        break;
    default:
        m_logger.log(std::string("Invalid AudioFormat: ") + AUDIO_FORMAT_LABELS[(size_t)m_inputFormat], COLOR_RED);
        return;
    }


    connect(m_audioIODevice, &AudioIODevice::fftOutputReady, this, &AudioWorker::fftOutputReady);


    if (foundAudioOutputInfo){
        m_audioIODevice->setAudioSinkDevice(std::make_unique<AudioSink>(
            m_logger, m_outputInfo, outputFormat
        ));
    }
}

AudioWorker::~AudioWorker(){
    if (m_audioIODevice){
        // Close the connection between m_audioIODevice and 
        // m_audioSink.
        m_audioIODevice->setAudioSinkDevice(nullptr);
    }

    if (m_FileLoader){
        delete m_FileLoader;
        m_FileLoader = nullptr;
    }

    if (m_audioIODevice){
        delete m_audioIODevice;
        m_audioIODevice = nullptr;
    }

    m_logger.log("AudioWorker destroyed");
}

void AudioWorker::setVolume(float volume){
    volume = std::max(std::min(volume, 1.0f), 0.0f);
    m_volume = volume;
//    if (m_audioSink){
//        m_audioSink->setVolume(volume);
//    }
    m_audioIODevice->set_volume(volume);
}







}
