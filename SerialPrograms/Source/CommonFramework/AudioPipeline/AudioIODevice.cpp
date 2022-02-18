/*  Audio IO Device
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "AudioConstants.h"
#include "AudioIODevice.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{



AudioIODevice::AudioIODevice(const QAudioFormat& audioFormat, ChannelMode channelMode)
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
        if (m_channelMode == ChannelMode::Interleaved){
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
    if (m_channelMode == ChannelMode::Interleaved){
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



}
