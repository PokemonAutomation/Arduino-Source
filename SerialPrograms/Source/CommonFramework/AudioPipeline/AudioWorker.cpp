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


AudioIODevice::AudioIODevice(const QAudioFormat & audioFormat, AudioWorker::ChannelMode channelMode)
     : QIODevice(nullptr)
     , m_audioFormat(audioFormat)
     , m_channelMode(channelMode)
     , m_channelSwapBuffer(8192)
     , m_fftCircularBuffer(NUM_FFT_SAMPLES * 8)
     , m_fftInputVector(NUM_FFT_SAMPLES) {}

AudioIODevice::~AudioIODevice() {}

qint64 AudioIODevice::writeData(const char* data, qint64 len)
{
    const auto& audioFormat = m_audioFormat;
    // One audio frame consists of one or more channels.
    // Each channel in the frame has an audio sample that is made by one or
    // more bytes.
    int frameBytes = audioFormat.bytesPerFrame();
    int numChannels = audioFormat.channelCount();
    int sampleBytes = frameBytes / numChannels;
    size_t numSamples = len / sampleBytes;
    size_t numFrames = len / frameBytes;
    // We assert the data is always float with size of 4 bytes.
    const float * floatData = reinterpret_cast<const float *>(data);

    // Pass audio data to the audio sink for audio playback.
    if (m_audioSinkDevice){
        if (m_channelMode == AudioWorker::ChannelMode::Interleaved){
            // Interleaved mode: numChannels == 1 and sample rate should be 2 times the 
            // normal sample rate. The input device interleaved L and R stereo channels into
            // one mono channel. The incoming samples follow the R - L order.
            // Since the stereo format has L - R order. We need to swap L and R samples:
            
            // First we need to make sure there are equal number of L and R samples to swap:
            if(numSamples % 2 != 0){
                std::cout << "Error: audio in interleaved mode but the number of samples "
                    << numSamples << " is not even" << std::endl;
            }

            // Swapping samples:
            m_channelSwapBuffer.resize(numSamples);
            for(size_t i = 0; i < numSamples/2; i++){
                m_channelSwapBuffer[2*i] = floatData[2*i+1];
                m_channelSwapBuffer[2*i+1] = floatData[2*i];
            }
            m_audioSinkDevice->write(reinterpret_cast<const char *>(m_channelSwapBuffer.data()), len);
        } else{
            // In other modes, we can safely pass data to output directly.
            m_audioSinkDevice->write(data, len);
        }
    }

#define PASS_FFT
#ifdef PASS_FFT
    // Pass audio frames to FFT input buffer:
    // If the input audio is mono channel, then the samples are directly written to FFT buffer.
    // If the input audio is stero, then for each frame, average the left and right channel samples
    // in the frame and send to the FFT buffer.
    // If the input audio is interleaved mode, then its essentially the stereo mode, do the same
    // as stereo.
    if (m_channelMode == AudioWorker::ChannelMode::Interleaved){
        frameBytes *= 2;
        numChannels = 2;
        numFrames /= 2;
    }

    // numTotalFramesCopied: how many audio frames have been copied in the following loop
    size_t numTotalFramesCopied = 0;
    const size_t circularBufferSize = m_fftCircularBuffer.size();

    // while there are still samples not copied from audio source to the fft buffer:
    while(numTotalFramesCopied < numFrames){

        // First, let's calculate how many more samples we need to file a FFT.
        size_t nextFFTNumFramesNeeded = computeNextFFTSamplesNeeded();

        // Copy as much data as possible from the audio source until either the input samples are
        // all used or the circular buffer reaches the end.
        size_t curNumFramesCopied = std::min((size_t)m_fftCircularBuffer.size() - m_bufferNext,
            numFrames - numTotalFramesCopied);
        if (numChannels == 1){
            // Mono channel, directly copy data:
            memcpy(m_fftCircularBuffer.data() + m_bufferNext, floatData + numTotalFramesCopied,
                sizeof(float) * curNumFramesCopied);
        } else{
            for(size_t i = 0; i < curNumFramesCopied; i++){
                const float v0 = floatData[(numTotalFramesCopied+i) * numChannels];
                const float v1 = floatData[(numTotalFramesCopied+i) * numChannels + 1];
                m_fftCircularBuffer[m_bufferNext + i] = (v0 + v1) / 2.0f;
            }
        }
        
        // Update m_bufferNext to reflect new data added:
        m_bufferNext = (m_bufferNext + curNumFramesCopied) % circularBufferSize;
        // Update numTotalFramesCopied to reflect new data copied:
        numTotalFramesCopied += curNumFramesCopied;

        // With new data added, we need to check whether we can file one or more FFTs:

        size_t numNewDataForFFT = curNumFramesCopied;
        // While we have enough new data to file an FFT:
        while(nextFFTNumFramesNeeded <= numNewDataForFFT){
            // copy data from m_fftCircularBuffer to m_fftInputVector:
            moveDataToFFTInputVector();
            // std::cout << "FFT input copied " << std::endl;

            // emit signal for FFT:
            emit fftInputReady(m_fftInputVector);

            // Update numNewDataForFFT to account for fft samples consumed:
            numNewDataForFFT -= nextFFTNumFramesNeeded;
            // Initialize nextFFTNumFramesNeeded for next FFT:
            nextFFTNumFramesNeeded = FFT_SLIDING_WINDOW_STEP;
            // Move m_fftStart forward for next FFT:
            m_fftStart = (m_fftStart + FFT_SLIDING_WINDOW_STEP) % circularBufferSize;
        }
    }
#endif


#ifdef DEBUG_AUDIO_IO
    std::vector<float> meanChannelVol(numChannels);
    std::vector<float> minChannelVol(numChannels, FLT_MAX);
    std::vector<float> maxChannelVol(numChannels, -FLT_MAX);
    for(int i = 0; i < numFrames; i++){
        for(int j = 0; j < numChannels; j++){
            const float v = floatData[i*numChannels+j];
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

    const QAudioFormat defaultFormat = chosenAudioInputDevice.preferredFormat();
    std::cout << "Default input audio format: ";
    printAudioFormat(defaultFormat);
    const int defaultChannelCount = defaultFormat.channelCount();
    const int defaultSampleRate = defaultFormat.sampleRate();
    
    m_audioFormat = defaultFormat;
    // For now we let Qt handle the audio sample type conversion for us:
#if QT_VERSION_MAJOR == 5
        m_audioFormat.setSampleType(QAudioFormat::SampleType::Float);
        m_audioFormat.setSampleSize(32);
#elif QT_VERSION_MAJOR == 6
        m_audioFormat.setSampleFormat(QAudioFormat::SampleFormat::Float);
#endif

    QAudioFormat m_outputAudioFormat = m_audioFormat;

    if (false){
        m_audioFormat.setChannelCount(1);
        m_audioFormat.setChannelConfig(QAudioFormat::ChannelConfigMono);
        m_audioFormat.setSampleRate(48000);
        m_channelMode = ChannelMode::Mono;
        m_outputAudioFormat = m_audioFormat;
    }
    else if (defaultChannelCount == 1 && defaultSampleRate <= 50000){
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
        m_outputAudioFormat.setChannelConfig(QAudioFormat::ChannelConfig::ChannelConfigStereo);
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
