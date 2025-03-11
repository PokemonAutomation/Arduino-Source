/*  Audio File Loader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_AudioPipeline_AudioFileLoader_H
#define PokemonAutomation_AudioPipeline_AudioFileLoader_H

#include <string>
#include <tuple>
#include <QObject>
#include <QAudioFormat>
#include <QFile>
#include <QThread>

class QAudioBuffer;
class QAudioDecoder;
class QTimer;
class WavFile;


namespace PokemonAutomation{


class AudioDecoderWorker;

// Load .wav and .mp3 audio from disk and optionally play at desired sample rate.

// Note: for ease of implementation this class saves all the decoded raw samples to
// memory before sending them out when `start()` is called. For a large audio file this
// is problematic as it will consume lots of memory. But the purpose of this functionality
// is to load recorded audio for testing and developing audio inference programs.
// So the input audio files should be small.
// You can also call `loadFullAudio()` to load the audio directly into memory.
// The loader tries to convert the audio data into the format specified as `audioFormat`
// in the constructor. For .mp3 files this conversion is done by QtAudioDecoder, while
// for .wav files the converion is done in `AudioFileLoader`.
// Currently we hard code the code to only return float data because that's the rest of
// the audio pipeline uses.
// For .wav file the format conversion is not fully implement: we cannot convert sampling
// rate during playback (after 'start()' called), but support it in `loadFullAudio()`.
class AudioFileLoader: public QObject{
    Q_OBJECT

public:
    AudioFileLoader(QObject* parent, const std::string& filename, const QAudioFormat& audioFormat);
    virtual ~AudioFileLoader();

    // Start loading and decoding audio samples.
    // Send the sample buffers at the desired speed determined by sample rate in `audioFormat` passed
    // to the constructor.
    // Retrieve sample buffer by connecting to signal `bufferReady` from the same thread.
    // See comments of the class full more details.
    bool start();

    // Load and decode full audio from file. Return the internal buffer (pointer and size) holding
    // the decoded audio data. If loading fails, the returned pointer is nullptr.
    // Note: this is a blocking operation.
    // See comments of the class full more details.
    std::tuple<const char*, size_t> loadFullAudio();

    const QAudioFormat& audioFormat() const { return m_audioFormat; }

signals:
    // Send audio sample buffer at time interval `m_timer_interval_ms` after `start()` called
    // Pass raw pointer. So must be connected to objects in the same thread!
    void bufferReady(const char* data, size_t len);

    // When finished sending decoded audio frames after `start()` called
    void finished();

    // Used privately to launch audio decoder woker in a separate thread.
    void runAudioDecoderAsync();

private:
    void buildTimer();
    
    bool initWavFile();

    // Send audio samples decoded from m_audioDecoder on m_timer.
    void sendDecodedBufferOnTimer();

    // Convert raw samples read from m_wavFile (stored in m_rawBuffer) into
    // float type and return the pointer and length of the converted data.
    std::pair<const char*, size_t> convertRawWavSamples();

    // Send audio samples read fomr m_wavFile on m_timer.
    void sendBufferFromWavFileOnTimer();

private:
    std::string m_filename;

    QAudioFormat m_audioFormat;

    AudioDecoderWorker* m_audioDecoderWorker = nullptr;
    QThread m_audioDecoderThread;

    WavFile* m_wavFile = nullptr;

    // Buffer to store raw audio data from m_wavFile or output from m_audioDecoderWorker
    std::vector<char> m_rawBuffer;

    // Since m_wavFile only reads raw audio file, we need to do sample type conversion ourselves.
    // Therefore we need a buffer to store converted audio samples.
    std::vector<float> m_floatBuffer;

    // When reading m_rawBuffer, which index to start reading.
    size_t m_bufferNext = 0;

    // To playback the decoded audio frames at sample rate, we need a timer.
    QTimer* m_timer = nullptr;

    // The time interval (in milliseconds) that the timer will send a signal.
    size_t m_timer_interval_ms = 10;

    // The timer sends a signal, how many audio frames we need to prepare to
    // send to outside.
    // m_frames_per_timeout = <frame_rate (unit: frames per sec)> * m_timer_interval_ms / 1000
    size_t m_frames_per_timeout = 0;
};



// Used to run QAudioDecoder and collect decoded results
class AudioDecoderWorker: public QObject{
Q_OBJECT

public:
    AudioDecoderWorker(QObject* parent, const std::string& filename, const QAudioFormat& audioFormat, std::vector<char>& decodedBuffer);
    virtual ~AudioDecoderWorker();

    void start();

    bool startSucceeded() { return m_startSucceeded; }

    void stop();

signals:
    void errored();

    void finished();

public slots:
    // Read decoded buffer sent from m_audioDecoder and store it into m_decodedBuffer.
    void readAudioDecoderBuffer();

private:
    // Handle error from m_audioDecoder.
    void handleAudioDecoderError();

private:

    std::string m_filename;
    QAudioFormat m_audioFormat;

    QAudioDecoder* m_audioDecoder = nullptr;

    std::vector<char>& m_decodedBuffer;

    bool m_startSucceeded = true;
};





}

#endif
