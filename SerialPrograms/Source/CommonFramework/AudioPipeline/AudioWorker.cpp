/*  Audio Worker
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "AudioConstants.h"
#include "AudioInfo.h"
#include "AudioWorker.h"
#include "AudioFileLoader.h"
#include "AudioFormatUtils.h"
#include "CommonFramework/Logging/Logger.h"

#include <QIODevice>
#include <QThread>

#if QT_VERSION_MAJOR == 5
#include <QAudioDeviceInfo>
#include <QAudioInput>
#include <QAudioOutput>
#include <QtEndian>
using AudioSource = QAudioInput;
using AudioSink = QAudioOutput;
#elif QT_VERSION_MAJOR == 6
#include <QMediaDevices>
#include <QAudioSource>
#include <QAudioSink>
#include <QAudioDevice>
using AudioSource = QAudioSource;
using AudioSink = QAudioSink;
#endif


#include <iomanip>
#include <cfloat>
#include <chrono>
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
    const AudioInfo& inputInfo,
    const QString& inputAbsoluteFilepath,
    const AudioInfo& outputInfo,
    float outputVolume
)
    : m_logger(logger)
    , m_inputInfo(inputInfo)
    , m_inputAbsoluteFilepath(inputAbsoluteFilepath)
    , m_outputInfo(outputInfo)
    , m_volume(std::max(std::min(outputVolume, 1.0f), 0.0f))
{}


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

    // If input filename is not empty, load audio from file:
    if (m_inputAbsoluteFilepath.size() > 0){
        // We hard code file audio format to be mono channel 48KHz.
        inputAudioFormat.setChannelCount(2);
#if QT_VERSION_MAJOR == 5
        inputAudioFormat.setCodec("audio/pcm");
#endif
        inputAudioFormat.setSampleRate(48000);
        setSampleFormatToFloat(inputAudioFormat);
        m_channelMode = ChannelMode::Stereo;

        // Note: m_inputAbsoluteFilepath must be an absolute file path. Otherwise it may trigger a bug
        // in QAudioDecoder used in AudioFileLoader, which will either stops the audio stream
        // halfway or crash the program when deleting QAudioDecoder.
        m_FileLoader = new AudioFileLoader(this, m_inputAbsoluteFilepath, inputAudioFormat);
        if (m_FileLoader->start() == false){
            return;
        }
        outputAudioFormat = m_FileLoader->audioFormat();
        m_logger.log("Set output audio format to: " + dumpAudioFormat(outputAudioFormat));

        connect(m_FileLoader, &AudioFileLoader::bufferReady, this, [&](const char* data, size_t len){
            if (m_audioIODevice){
                m_audioIODevice->writeData(data, len);
            }
        });

    } else{
        // Load from audio input device:

        if (foundAudioInputInfo == false){
            // std::cout << "Cannot build Qt6VideoWidget: cannot found audio device name matching: " << inputInfo.device_name() << std::endl;
            return;
        }

        const QAudioFormat defaultInputFormat = chosenAudioInputDevice.preferredFormat();
        m_logger.log("Default input audio format: " + dumpAudioFormat(defaultInputFormat));
        const int defaultChannelCount = defaultInputFormat.channelCount();
        const int defaultSampleRate = defaultInputFormat.sampleRate();
        
        inputAudioFormat = defaultInputFormat;
        
        // inputAudioFormat.setSampleRate(48000);

        // For now we let Qt handle the audio sample type conversion for us:
        setSampleFormatToFloat(inputAudioFormat);

        if (defaultSampleRate <= 40000){
            // Somehow Qt picks a suboptimal default format.
            // Force it to be the standard 48000
            inputAudioFormat.setSampleRate(48000);
        }

        outputAudioFormat = inputAudioFormat;

// #define FORCE_MONO
#ifdef FORCE_MONO
        if (true){
            inputAudioFormat.setChannelCount(1);
            inputAudioFormat.setSampleRate(48000);
            m_channelMode = ChannelMode::Mono;
            outputAudioFormat = inputAudioFormat;
        } else
#endif
        if (defaultChannelCount == 1 && defaultSampleRate <= 50000){
            // The input audio device uses mono mode, nothing to change on the output format.
            m_channelMode = ChannelMode::Mono;
        } else if (defaultChannelCount >= 2 && defaultSampleRate <= 50000){
            // The input audio device uses stereo mode, nothing to change on the output format.
            m_channelMode = ChannelMode::Stereo;
            // Reduce the channel count to 2 in case the input sound is surrounded stereo.
            inputAudioFormat.setChannelCount(2);
            outputAudioFormat = inputAudioFormat;
        } else if (defaultChannelCount == 1 && defaultSampleRate >= 80000){
            // The input audio device uses interleaved mode, where the stereo samples are
            // interleaved into a mono channel.
            // This is the mode many capture cards use. If the Swith audio comes as stereo 48KHz,
            // the capture card generates a mono 96KHz audio stream.
            // To handle this, we need to set the output audio format to be stereo with the correct
            // sample rate.
#if QT_VERSION_MAJOR == 5
            outputAudioFormat.setChannelCount(2);
#elif QT_VERSION_MAJOR == 6
            outputAudioFormat.setChannelConfig(QAudioFormat::ChannelConfig::ChannelConfigStereo);
#endif
            outputAudioFormat.setSampleRate(defaultSampleRate / 2);
            m_channelMode = ChannelMode::Interleaved;
        } else {
            std::cout << "Error: unkown input audio configuration:" << std::endl;
            return;
        }

        m_logger.log("Set input audio format to: " + dumpAudioFormat(inputAudioFormat));
        m_logger.log("Set output audio format to: " + dumpAudioFormat(outputAudioFormat));
        
        if (!chosenAudioInputDevice.isFormatSupported(inputAudioFormat)){
            std::cout << "Error: audio input device cannot support desired audio format" << std::endl;
            return;
        }

        const int bytesPerSample = inputAudioFormat.bytesPerFrame() / inputAudioFormat.channelCount();
        if (bytesPerSample != sizeof(float)){
            std::cout << "Error: audio format is wrong. Set its sample format to float but the bytesPerSample is "
                << bytesPerSample << ", different from float size " << sizeof(float) << std::endl;
            return;
        }
        

        m_audioSource = new AudioSource(chosenAudioInputDevice, inputAudioFormat, this);

        connect(m_audioSource, &AudioSource::stateChanged, this, [&](QAudio::State newState){
            this->handleDeviceErrorState(newState, m_audioSource->error(), "AudioSource");
        });
    } // end if load audio from input audio device
    
    m_audioIODevice = new AudioIODevice(inputAudioFormat, m_channelMode);
    m_audioIODevice->open(QIODevice::ReadWrite | QIODevice::Unbuffered);
    connect(m_audioIODevice, &AudioIODevice::fftInputReady, this, &AudioWorker::fftInputReady);
    
    if (m_audioSource){
        m_audioSource->start(m_audioIODevice);
    }

    if (foundAudioOutputInfo){
        bool outputSupported = chosenAudioOutputDevice.isFormatSupported(outputAudioFormat);
        if (!outputSupported){
            std::cout << "Error the audio output device does not support the requested audio format" << std::endl;
        } else{
            m_audioSink = new AudioSink(chosenAudioOutputDevice, outputAudioFormat, this);
            m_audioSink->setBufferSize(32768);
            m_audioSink->setVolume(m_volume);
            m_audioIODevice->setAudioSinkDevice(m_audioSink->start());

            connect(m_audioSink, &AudioSink::stateChanged, this, [&](QAudio::State newState){
                this->handleDeviceErrorState(newState, m_audioSink->error(), "AudioSink");
            });
        }
    }
}

AudioWorker::~AudioWorker(){
    if (m_audioIODevice){
        // Close the connection between m_audioIODevice and 
        // m_audioSink.
        m_audioIODevice->setAudioSinkDevice(nullptr);
    }

    if (m_audioSink){
        m_audioSink->stop();
        delete m_audioSink;

        m_audioSink = nullptr;
    }

    if (m_audioSource){
        m_audioSource->stop();
        delete m_audioSource;
        m_audioSource = nullptr;
    }

    if (m_FileLoader){
        delete m_FileLoader;
        m_FileLoader = nullptr;
    }

    if (m_audioIODevice){
        delete m_audioIODevice;
        m_audioIODevice = nullptr;
    }

    std::cout << "AudioWorker destroyed" << std::endl;
}

void AudioWorker::setVolume(float volume){
    volume = std::max(std::min(volume, 1.0f), 0.0f);
    m_volume = volume;
    if (m_audioSink){
        m_audioSink->setVolume(volume);
    }
}

void AudioWorker::handleDeviceErrorState(QAudio::State newState, QAudio::Error error, const char* deviceType){
    // TODO connect logger output to it
    switch (newState) {
    case QAudio::StoppedState:
        switch (error) {
        case QAudio::NoError:
            std::cout << deviceType << " stopped normally" << std::endl;
            break;
        case QAudio::OpenError:
            std::cout << deviceType << " OpenError" << std::endl;
            break;
        case QAudio::IOError:
            std::cout << deviceType <<  " IOError" << std::endl;
            break;
        case QAudio::UnderrunError:
            // Underrun error happens on audio sink when the audio thread is closing.
            // So we don't print this error if it's on audio sink.
            if (strcmp(deviceType, "AudioSink") != 0){
                std::cout << deviceType <<  " UnderrunError" << std::endl;
            }
            break;
        case QAudio::FatalError:
            std::cout << deviceType <<  " FatalError" << std::endl;
            break;
        }
        break;

    case QAudio::ActiveState:
        // Started recording - read from IO device
        // std::cout << "Audio started" << std::endl;
        break;
    
    case QAudio::SuspendedState:
        std::cout << deviceType <<  " is suspended" << std::endl;
        break;

    case QAudio::IdleState:
        // std::cout << "AudioSource is idle, no input" << std::endl;
        break;
#if QT_VERSION_MAJOR == 5
    case QAudio::InterruptedState:
        std::cout << "AudioSource is interrupted, no input" << std::endl;
        break;
#endif
    } // end switch newState
}


}
