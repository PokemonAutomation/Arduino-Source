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

class QAudioBuffer;
class QAudioDecoder;
class QTimer;
class WavFile;


namespace PokemonAutomation{


class AudioDecoderWorker;

// Load .wav and .mp3 audio from disk and play at the file's sample rate.

// Note: for ease of implementation this class saves all the decoded raw samples to
// memory before sending them out when `start()` is called. For a large audio file this
// is problematic as it will consume lots of memory. But the purpose of this functionality
// is to load recorded audio for testing and developing audio inference programs.
// So the input audio files should be small.
// You can also call `load_full_audio()` to load the audio directly into memory.
//
// For .wav files the loader streams the file's native format (rate, channels,
// sample type): the caller is expected to set `audio_format` to the file's own
// format (see probe_wav_native_format()), so the timer paces at the file's rate
// and the raw bytes are emitted unconverted.
// For .mp3 (and other compressed formats) the caller probes the decoder's
// native output format (see probe_decoded_native_format()) and sets
// `audio_format` to it, so QAudioDecoder decodes to that format (an identity
// conversion) and the decoded bytes are emitted at the file's native rate.
// load_full_audio() is the batch "convert to the requested format" API: it
// converts the sample type and channels to `audio_format`, but not the sample
// rate, which must already match (its consumer, load_audio_template, assumes the
// returned data is at the requested rate).
class AudioFileLoader: public QObject{
    Q_OBJECT

public:
    AudioFileLoader(
        QObject* parent,
        const std::string& filename,
        const QAudioFormat& audio_format
    );
    virtual ~AudioFileLoader();

    // Start loading and decoding audio samples.
    // Send the sample buffers at the desired speed determined by sample rate in `audio_format` passed
    // to the constructor.
    // Retrieve sample buffer by connecting to signal `buffer_ready` from the same thread.
    // See comments of the class full more details.
    bool start();

    // Load and decode full audio from file. Return the internal buffer (pointer and size) holding
    // the decoded audio data. If loading fails, the returned pointer is nullptr.
    // Note: this is a blocking operation.
    // See comments of the class full more details.
    std::tuple<const char*, size_t> load_full_audio();

    const QAudioFormat& audio_format() const { return m_audio_format; }

signals:
    // Send audio sample buffer at time interval `m_timer_interval_ms` after `start()` called
    // Pass raw pointer. So must be connected to objects in the same thread!
    void buffer_ready(const char* data, size_t len);

    // When finished sending decoded audio frames after `start()` called
    void finished();

    // Used privately to launch audio decoder woker in a separate thread.
    void run_audio_decoder_async();

private:
    void build_timer();
    
    bool init_wav_file();

    // Send audio samples decoded from m_audio_decoder on m_timer.
    void send_decoded_buffer_on_timer();

    // Convert raw samples read from m_wav_file (stored in m_raw_buffer) into
    // float type and return the pointer and length of the converted data.
    std::pair<const char*, size_t> convert_raw_wav_samples();

    // Send audio samples read fomr m_wav_file on m_timer.
    void send_buffer_from_wav_file_on_timer();

private:
    std::string m_filename;

    QAudioFormat m_audio_format;

    AudioDecoderWorker* m_audio_decoder_worker = nullptr;
//    QThread m_audioDecoderThread;

    WavFile* m_wav_file = nullptr;

    // Buffer to store raw audio data from m_wav_file or output from m_audio_decoder_worker
    std::vector<char> m_raw_buffer;

    // Since m_wav_file only reads raw audio file, we need to do sample type conversion ourselves.
    // Therefore we need a buffer to store converted audio samples.
    std::vector<float> m_float_buffer;

    // When reading m_raw_buffer, which index to start reading.
    size_t m_buffer_next = 0;

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
    AudioDecoderWorker(
        QObject* parent,
        const std::string& filename,
        const QAudioFormat& audio_format,
        std::vector<char>& decoded_buffer
    );
    virtual ~AudioDecoderWorker();

    void start(){
        QMetaObject::invokeMethod(this, &AudioDecoderWorker::internal_start);
    }

    bool start_succeeded(){ return m_start_succeeded; }

    void stop(){
        QMetaObject::invokeMethod(this, &AudioDecoderWorker::internal_stop);
    }

signals:
    void errored();

    void finished();

public slots:
    void internal_start();
    void internal_stop();

    // Read decoded buffer sent from m_audio_decoder and store it into m_decoded_buffer.
    void read_audio_decoder_buffer();

private:
    // Handle error from m_audio_decoder.
    void handle_audio_decoder_error();

private:

    std::string m_filename;
    QAudioFormat m_audio_format;

    QAudioDecoder* m_audio_decoder = nullptr;

    std::vector<char>& m_decoded_buffer;

    bool m_start_succeeded = true;
};





}

#endif
