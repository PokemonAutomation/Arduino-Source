/*  Audio Worker
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "AudioConstants.h"
#include "AudioInfo.h"
#include "AudioWorker.h"
#include "CommonFramework/Logging/Logger.h"

#include <QIODevice>
#include <QThread>

#include <iomanip>
#include <cfloat>
#include <chrono>
#include <memory>
#include <cassert>

#include <iostream>
using std::cout;
using std::endl;

//#define DEBUG_AUDIO_IO

#if QT_VERSION_MAJOR == 5
#include <QAudioDeviceInfo>
#include <QAudioInput>
#include <QAudioOutput>
#include <QtEndian>

void printAudioFormat(const QAudioFormat& format){
    std::string sampleTypeStr = "";
    switch(format.sampleType()){
        case QAudioFormat::SampleType::Float:
            sampleTypeStr="Float";
            break;
        case QAudioFormat::SampleType::SignedInt:
            sampleTypeStr="SignedInt";
            break;
        case QAudioFormat::SampleType::UnSignedInt:
            sampleTypeStr="UnSignedInt";
            break;
        default:
            sampleTypeStr="Error";
    }

    std::cout << "Audio format: sample type " << sampleTypeStr << 
        ", bytes per sample " << format.bytesPerFrame() / format.channelCount() << 
        ", num channels " << format.channelCount() << 
        ", sample rate " << format.sampleRate() << std::endl;
}
#elif QT_VERSION_MAJOR == 6
#include <QMediaDevices>
#include <QAudioSource>
#include <QAudioSink>
#include <QAudioDevice>

void printAudioFormat(const QAudioFormat& format){
    std::string sampleFormatStr = "";
    switch(format.sampleFormat()){
        case QAudioFormat::SampleFormat::Float:
            sampleFormatStr="Float";
            break;
        case QAudioFormat::SampleFormat::Int16:
            sampleFormatStr="Int16";
            break;
        case QAudioFormat::SampleFormat::Int32:
            sampleFormatStr="Int32";
            break;
        case QAudioFormat::SampleFormat::UInt8:
            sampleFormatStr="UInt8";
            break;
        default:
            sampleFormatStr="Error";
    }

    std::string channelConfigStr = "";
    switch(format.channelConfig()){
        case QAudioFormat::ChannelConfig::ChannelConfigMono:
            channelConfigStr = "Mono";
            break;
        case QAudioFormat::ChannelConfig::ChannelConfigStereo:
            channelConfigStr = "Stereo";
            break;
        case QAudioFormat::ChannelConfig::ChannelConfigUnknown:
            channelConfigStr = "Unkown";
            break;
        default:
            channelConfigStr = "Non Mono or Stereo";
    }
    std::cout << "sample format " << sampleFormatStr << 
        ", bytes per sample " << format.bytesPerSample() << 
        ", channel config " << channelConfigStr <<
        ", num channels " << format.channelCount() << 
        ", sample rate " << format.sampleRate() << std::endl;
}

#endif


namespace PokemonAutomation{



AudioWorker::AudioWorker(const AudioInfo& inputInfo, const AudioInfo& outputInfo, float outputVolume){
    m_inputInfo = inputInfo;
    m_outputInfo = outputInfo;
    m_volume = std::max(std::min(outputVolume, 1.0f), 0.0f);
}


void AudioWorker::startAudio(){
#ifdef DEBUG_AUDIO_IO
    std::cout << "T" << QThread::currentThread() << " AudioWorker::startAudio()" << std::endl;
#endif

    bool foundAudioInputInfo = false;
    bool foundAudioOutputInfo = false;

#if QT_VERSION_MAJOR == 5
    using AudioSource = QAudioInput;
    using AudioSink = QAudioOutput;

    const auto audioInputs = QAudioDeviceInfo::availableDevices(QAudio::AudioInput);
    QAudioDeviceInfo chosenAudioInputDevice;
    for (const auto &audioDevice : audioInputs){
        if (audioDevice.deviceName().toStdString() == m_inputInfo.device_name()){
            foundAudioInputInfo = true;
            chosenAudioInputDevice = audioDevice;
            break;
        }
    }

    const auto audioOutputs = QAudioDeviceInfo::availableDevices(QAudio::AudioOutput);
    QAudioDeviceInfo chosenAudioOutputDevice;
    for (const auto &audioDevice : audioOutputs){
        if (audioDevice.deviceName().toStdString() == m_outputInfo.device_name()){
            foundAudioOutputInfo = true;
            chosenAudioOutputDevice = audioDevice;
            break;
        }
    }
#elif QT_VERSION_MAJOR == 6
    using AudioSource = QAudioSource;
    using AudioSink = QAudioSink;

    const auto audioInputs = QMediaDevices::audioInputs();
    QAudioDevice chosenAudioInputDevice;
    for (const auto &audioDevice : audioInputs){
        if (audioDevice.id().toStdString() == m_inputInfo.device_name()){
                foundAudioInputInfo = true;
            chosenAudioInputDevice = audioDevice;
            break;
        }
    }

    const auto audioOutputs = QMediaDevices::audioOutputs();
    QAudioDevice chosenAudioOutputDevice;
    for (const auto &audioDevice : audioOutputs){
        if (audioDevice.id().toStdString() == m_outputInfo.device_name()){
                foundAudioOutputInfo = true;
            chosenAudioOutputDevice = audioDevice;
            break;
        }
    }
#endif

    if (foundAudioInputInfo == false){
        // std::cout << "Cannot build Qt6VideoWidget: cannot found audio device name matching: " << inputInfo.device_name() << std::endl;
        return;
    }

    const QAudioFormat defaultFormat = chosenAudioInputDevice.preferredFormat();
    std::cout << "Default input audio format: ";
    printAudioFormat(defaultFormat);
    const int defaultChannelCount = defaultFormat.channelCount();
    const int defaultSampleRate = defaultFormat.sampleRate();
    
    m_audioFormat = defaultFormat;
    m_audioFormat.setSampleRate(48000);

    // For now we let Qt handle the audio sample type conversion for us:
#if QT_VERSION_MAJOR == 5
        m_audioFormat.setSampleType(QAudioFormat::SampleType::Float);
        m_audioFormat.setSampleSize(32);
#elif QT_VERSION_MAJOR == 6
        m_audioFormat.setSampleFormat(QAudioFormat::SampleFormat::Float);
#endif

    QAudioFormat m_outputAudioFormat = m_audioFormat;

// #define FORCE_MONO
#ifdef FORCE_MONO
    if (true){
        m_audioFormat.setChannelCount(1);
        m_audioFormat.setSampleRate(48000);
        m_channelMode = ChannelMode::Mono;
        m_outputAudioFormat = m_audioFormat;
    } else
#endif
    if (defaultChannelCount == 1 && defaultSampleRate <= 50000){
        // The input audio device uses mono mode, nothing to change on the output format.
        m_channelMode = ChannelMode::Mono;
    } else if (defaultChannelCount >= 2 && defaultSampleRate <= 50000){
        // The input audio device uses stereo mode, nothing to change on the output format.
        m_channelMode = ChannelMode::Stereo;
        // Reduce the channel count to 2 in case the input sound is surrounded stereo.
        m_audioFormat.setChannelCount(2);
        m_outputAudioFormat = m_audioFormat;
    } else if (defaultChannelCount == 1 && defaultSampleRate >= 80000){
        // The input audio device uses interleaved mode, where the stereo samples are
        // interleaved into a mono channel.
        // This is the mode many capture cards use. If the Swith audio comes as stereo 48KHz,
        // the capture card generates a mono 96KHz audio stream.
        // To handle this, we need to set the output audio format to be stereo with the correct
        // sample rate.
#if QT_VERSION_MAJOR == 5        
        m_outputAudioFormat.setChannelCount(2);
#elif QT_VERSION_MAJOR == 6
        m_outputAudioFormat.setChannelConfig(QAudioFormat::ChannelConfig::ChannelConfigStereo);
#endif
        m_outputAudioFormat.setSampleRate(defaultSampleRate / 2);
        m_channelMode = ChannelMode::Interleaved;
    } else {
        std::cout << "Error: unkown input audio configuration:" << std::endl;
        return;
    }

    std::cout << "Set input audio format to: ";
    printAudioFormat(m_audioFormat);
    std::cout << "Set output audio format to: ";
    printAudioFormat(m_outputAudioFormat);
    
    if (!chosenAudioInputDevice.isFormatSupported(m_audioFormat)){
        std::cout << "Error: audio input device cannot support desired audio format" << std::endl;
        return;
    }

    const int bytesPerSample = m_audioFormat.bytesPerFrame() / m_audioFormat.channelCount();
    if (bytesPerSample != sizeof(float)){
        std::cout << "Error: audio format is wrong. Set its sample format to float but the bytesPerSample is "
            << bytesPerSample << ", different from float size " << sizeof(float) << std::endl;
        return;
    }

#ifdef DEBUG_AUDIO_IO
    std::cout << "Audio input format:" << std::endl;
    printAudioFormat(m_audioFormat);
#endif
    
    m_audioSource = new AudioSource(chosenAudioInputDevice, m_audioFormat, this);

    connect(m_audioSource, &AudioSource::stateChanged, this, [&](QAudio::State newState){
        // TODO connect logger output to it
        switch (newState) {
        case QAudio::StoppedState:
            switch (m_audioSource->error()) {
            case QAudio::NoError:
                std::cout << "AudioSource stopped normally" << std::endl;
                break;
            case QAudio::OpenError:
                std::cout << "AudioSource OpenError" << std::endl;
                break;
            case QAudio::IOError:
                std::cout << "AudioSource IOError" << std::endl;
                break;
            case QAudio::UnderrunError:
                std::cout << "AudioSource UnderrunError" << std::endl;
                break;
            case QAudio::FatalError:
                std::cout << "AudioSource FatalError" << std::endl;
                break;
            }
            break;

        case QAudio::ActiveState:
            // Started recording - read from IO device
            // std::cout << "Audio started" << std::endl;
            break;
        
        case QAudio::SuspendedState:
            std::cout << "AudioSource is suspended" << std::endl;
            break;

        case QAudio::IdleState:
            // std::cout << "AudioSource is idle, no input" << std::endl;
            break;
#if QT_VERSION_MAJOR == 5
        case QAudio::InterruptedState:
            std::cout << "AudioSource is interrupted, no input" << std::endl;
            break;
#endif
        }
    });

    m_audioIODevice = new AudioIODevice(m_audioFormat, m_channelMode);
    m_audioIODevice->open(QIODevice::ReadWrite | QIODevice::Unbuffered);
    
    m_audioSource->start(m_audioIODevice);

    if (foundAudioOutputInfo){
        bool outputSupported = chosenAudioOutputDevice.isFormatSupported(m_outputAudioFormat);
        if (!outputSupported){
            std::cout << "Error the audio output device does not support the audio format edited from the audio input device" << std::endl;
        } else{
            m_audioSink = new AudioSink(chosenAudioOutputDevice, m_outputAudioFormat, this);
            m_audioSink->setBufferSize(32768);
            m_audioSink->setVolume(m_volume);
            m_audioIODevice->setAudioSinkDevice(m_audioSink->start());

            connect(m_audioSink, &AudioSink::stateChanged, this, [&](QAudio::State newState){
                switch (newState) {
                case QAudio::StoppedState:
                    switch (m_audioSource->error()) {
                    case QAudio::NoError:
                        std::cout << "AudioSink stopped normally" << std::endl;
                        break;
                    case QAudio::OpenError:
                        std::cout << "AudioSink OpenError" << std::endl;
                        break;
                    case QAudio::IOError:
                        std::cout << "AudioSink IOError" << std::endl;
                        break;
                    case QAudio::UnderrunError:
                        // Underrun error happens when the audio thread is closing.
                        // std::cout << "AudioSink UnderrunError" << std::endl;
                        break;
                    case QAudio::FatalError:
                        std::cout << "AudioSink FatalError" << std::endl;
                        break;
                    }
                    break;

                case QAudio::ActiveState:
                    break;
                
                case QAudio::SuspendedState:
                    std::cout << "AudioSink is suspended" << std::endl;
                    break;

                case QAudio::IdleState:
                    // std::cout << "Audio is idle, no input" << std::endl;
                    break;
#if QT_VERSION_MAJOR == 5
                case QAudio::InterruptedState:
                    std::cout << "AudioSink is interrupted, no input" << std::endl;
                    break;
#endif
                }
            });
        }
    }

    connect(m_audioIODevice, &AudioIODevice::fftInputReady, this, &AudioWorker::fftInputReady);
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

    if (m_audioIODevice){
        delete m_audioIODevice;
        m_audioIODevice = nullptr;
    }
}

void AudioWorker::setVolume(float volume){
    volume = std::max(std::min(volume, 1.0f), 0.0f);
    m_volume = volume;
    if (m_audioSink){
        m_audioSink->setVolume(volume);
    }
}

}
