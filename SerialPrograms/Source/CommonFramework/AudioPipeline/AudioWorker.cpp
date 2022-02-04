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

#include <iostream>
#include <iomanip>
#include <cfloat>
#include <chrono>
#include <memory>
#include <cassert>

// #define DEBUG_AUDIO_IO

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
            channelConfigStr = "Error";
            break;
        default:
            channelConfigStr = "Non Mono or Stereo";
    }
    std::cout << "Audio format: sample format " << sampleFormatStr << 
        ", bytes per sample " << format.bytesPerSample() << 
        ", channel config " << channelConfigStr <<
        ", num channels " << format.channelCount() << 
        ", sample rate " << format.sampleRate() << std::endl;
}

#endif


namespace PokemonAutomation{


AudioIODevice::AudioIODevice(const QAudioFormat & audioFormat)
     : QIODevice(nullptr)
     , m_audioFormat(audioFormat)
     , m_fftCircularBuffer(NUM_FFT_SAMPLES * 8)
     , m_fftInputVector(NUM_FFT_SAMPLES) {}

AudioIODevice::~AudioIODevice() {}

qint64 AudioIODevice::writeData(const char* data, qint64 len)
{
    const auto& audioFormat = m_audioFormat;
    // One audio frame consists of one or more channels.
    // Each channel in the frame has an audio sample that is made by one or
    // more bytes.
    const int frameBytes = audioFormat.bytesPerFrame();
    const int numChannels = audioFormat.channelCount();
    const int sampleBytes = frameBytes / numChannels;
    const size_t numSamples = len / frameBytes;
    const size_t numFrames = len / frameBytes;
    // We assert the data is always float with size of 4 bytes.
    const float * floatData = reinterpret_cast<const float *>(data);

    // We assert audio channel count is 1. This has the benefit that we can use memcpy
    // to pass audio data to the FFT input buffer. The drawback is the user can not hear
    // stereo sound.
    // Note: I tried to do stereo sound but it seems QAudioSink somehow still plays it as
    // mono sound on my machine.
    if (numChannels != 1){
        std::cout << "Error: AudioIODevice receives audio format with more than one channel count: "
            << numChannels << ". We currently only support one channel" << std::endl;
        return 0;
    }

    // Pass audio samples to FFT input buffer:

    // numTotalSamplesCopied: how many samples have been copied in the following loop
    size_t numTotalSamplesCopied = 0;
    const size_t circularBufferSize = m_fftCircularBuffer.size();

    // while there are still samples not copied from audio source to the fft buffer:
    while(numTotalSamplesCopied < numSamples){

        // First, let's calculate how many more samples we need to file a FFT.
        size_t nextFFTNumSamplesNeeded = computeNextFFTSamplesNeeded();

        // Copy as much data as possible from the audio source until either the input samples are
        // all used or the circular buffer reaches the end.
        size_t curNumSamplesCopied = std::min((size_t)m_fftCircularBuffer.size() - m_bufferNext, numSamples - numTotalSamplesCopied);
        memcpy(m_fftCircularBuffer.data() + m_bufferNext, floatData + numTotalSamplesCopied, sizeof(float) * curNumSamplesCopied);
        
        // Update m_bufferNext to reflect new data added:
        m_bufferNext = (m_bufferNext + curNumSamplesCopied) % circularBufferSize;
        // Update numTotalSamplesCopied to reflect new data copied:
        numTotalSamplesCopied += curNumSamplesCopied;

        // With new data added, we need to check whether we can file one or more FFTs:

        size_t numNewDataForFFT = curNumSamplesCopied;
        // While we have enough new data to file an FFT:
        while(nextFFTNumSamplesNeeded <= numNewDataForFFT){
            // copy data from m_fftCircularBuffer to m_fftInputVector:
            moveDataToFFTInputVector();
            // std::cout << "FFT input copied " << std::endl;

            // emit signal for FFT:
            emit fftInputReady(m_fftInputVector);

            // Update numNewDataForFFT to account for fft samples consumed:
            numNewDataForFFT -= nextFFTNumSamplesNeeded;
            // Initialize nextFFTNumSamplesNeeded for next FFT:
            nextFFTNumSamplesNeeded = FFT_SLIDING_WINDOW_STEP;
            // Move m_fftStart forward for next FFT:
            m_fftStart = (m_fftStart + FFT_SLIDING_WINDOW_STEP) % circularBufferSize;
        }
    }

    // Pass audio data to the audio sink for audio playback.
    if (m_audioSinkDevice){
        m_audioSinkDevice->write(data, len);
    }

#ifdef DEBUG_AUDIO_IO
    std::vector<float> meanChannelVol(numChannels);
    std::vector<float> minChannelVol(numChannels, FLT_MAX);
    std::vector<float> maxChannelVol(numChannels, -FLT_MAX);
    for(int i = 0; i < numFrames; i++){
        for(int j = 0; j < numChannels; j++){
            const float v = floatData[i*numChannels+j]
            meanChannelVol[j] += v;
            minChannelVol[j] = std::min(minChannelVol[j], v);
            maxChannelVol[j] = std::max(maxChannelVol[j], v);
        }
    }
    for(int j = 0; j < numChannels; j++){
        meanChannelVol[j] /= float(numSamples);
    }
    auto currentTimepoint = std::chrono::system_clock::now();
    const auto dur = std::chrono::duration_cast<std::chrono::microseconds>(currentTimepoint-m_lastWriteTimepoint);
    m_lastWriteTimepoint = currentTimepoint;
    std::cout << "T" << QThread::currentThread() << " Wrote " << numSamples << " audio frames after " << 
        dur.count() << " mircoseconds, mean/min/max volume per channel: [";
    for(int j = 0; j < audioFormat.channelCount(); j++){
        std::cout << std::setprecision(3) << meanChannelVol[j] << "/" << 
            std::setprecision(3) << minChannelVol[j] << "/" << 
            std::setprecision(3) << maxChannelVol[j] << ", ";
    }
    std::cout << "]" << std::endl;
#endif
    return len;
}

size_t AudioIODevice::computeNextFFTSamplesNeeded() const{
    // nextFFTNumSamplesFilled: how many samples already in the buffer to be used for the next FFT:
    size_t nextFFTNumSamplesFilled = 0;
    if (m_bufferNext >= m_fftStart){
        nextFFTNumSamplesFilled = m_bufferNext - m_fftStart;
    } else{
        nextFFTNumSamplesFilled = m_bufferNext + m_fftCircularBuffer.size() - m_fftStart;
    }
    if (nextFFTNumSamplesFilled >= NUM_FFT_SAMPLES){
        std::cout << "ERROR: internal buffer error, stored more than enough FFT samples but did not launch FFT "
            << nextFFTNumSamplesFilled << " " << m_bufferNext << " " << m_fftStart << std::endl;
    }
    assert(nextFFTNumSamplesFilled < NUM_FFT_SAMPLES);

    return NUM_FFT_SAMPLES - nextFFTNumSamplesFilled;
}

void AudioIODevice::moveDataToFFTInputVector(){
    size_t num = std::min(m_fftCircularBuffer.size() - m_fftStart, (size_t)NUM_FFT_SAMPLES);
    memcpy(m_fftInputVector.data(), m_fftCircularBuffer.data() + m_fftStart, sizeof(float) * num);
    if (num < NUM_FFT_SAMPLES){
        // We reach the end of the circular buffer, start from its beginning to copy the remaining part:
        memcpy(m_fftInputVector.data() + num, m_fftCircularBuffer.data(), sizeof(float) * (NUM_FFT_SAMPLES - num));
    }   
}

AudioWorker::AudioWorker(const AudioInfo& inputInfo, const AudioInfo& outputInfo){
    m_inputInfo = inputInfo;
    m_outputInfo = outputInfo;
}


void AudioWorker::startAudio(){
#ifdef DEBUG_AUDIO_IO
    std::cout << "T" << QThread::currentThread() << " AudioWorker::startAudio()" << std::endl;
#endif

    bool foundAudioInputInfo = false;
    bool foundAudioOutputInfo = false;
    
#if QT_VERSION_MAJOR == 5
    // We need QVector<float> to be able to pass around by signals and slots mechanism to pass
    // FFT input and output data across threads.
    // In Qt6 it automatically registers QVector<float>. But in Qt5 we need to manually register
    // the class.
    qRegisterMetaType<QVector<float>>("QVector<float>");

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

    m_audioFormat = chosenAudioInputDevice.preferredFormat();
    m_audioFormat.setSampleRate(AUDIO_SAMPLE_RATE);
#if QT_VERSION_MAJOR == 5
    m_audioFormat.setChannelCount(1);
    m_audioFormat.setSampleType(QAudioFormat::SampleType::Float);
    m_audioFormat.setSampleSize(32);
#elif QT_VERSION_MAJOR == 6
    m_audioFormat.setChannelConfig(QAudioFormat::ChannelConfig::ChannelConfigMono);
    m_audioFormat.setSampleFormat(QAudioFormat::SampleFormat::Float);
#endif
    
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

    m_audioIODevice = new AudioIODevice(m_audioFormat);
    m_audioIODevice->open(QIODevice::ReadWrite | QIODevice::Unbuffered);
    
    m_audioSource->start(m_audioIODevice);

    if (foundAudioOutputInfo){
        bool outputSupported = chosenAudioOutputDevice.isFormatSupported(m_audioFormat);
        if (!outputSupported){
            std::cout << "Error the audio output device does not support the audio format used by the audio input device" << std::endl;
        } else{
            m_audioSink = new AudioSink(chosenAudioOutputDevice, m_audioFormat, this);
            m_audioIODevice->setAudioSinkDevice(m_audioSink->start());
#ifdef DEBUG_AUDIO_IO
            std::cout << "Audio output format: " << std::endl;
            printAudioFormat(m_audioSink->format());
#endif
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


}