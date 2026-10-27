/*  Audio File Loader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <iostream>
#include "3rdParty/QtWavFile/WavFile.h"
#include <QUrl>
#include <QAudioBuffer>
#include <QAudioDecoder>
#include <QAudioFormat>
#include <QTimer>
#include <QEventLoop>
#include "Common/Qt/StringToolsQt.h"
#include "Common/Qt/GlobalThreadPoolsQt.h"
#include "CommonFramework/AudioPipeline/Tools/AudioFormatUtils.h"
#include "AudioFileLoader.h"



// #define DEBUG_AUDIO_DECODER_WORKER


namespace PokemonAutomation{

AudioFileLoader::AudioFileLoader(
    QObject* parent,
    const std::string& filename,
    const QAudioFormat& audio_format
):
    QObject(parent), m_filename(filename), m_audio_format(audio_format) {}

AudioFileLoader::~AudioFileLoader(){
    if (m_audio_decoder_worker){
        m_audio_decoder_worker->stop();
    }

    GlobalThreadPools::qt_event_threadpool().remove_object(m_audio_decoder_worker);
//    m_audioDecoderThread.quit();
//    m_audioDecoderThread.wait();

//    if (m_audio_decoder_worker){
//        delete m_audio_decoder_worker;
        m_audio_decoder_worker = nullptr;
//    }

    if (m_wav_file){
        m_wav_file->close();
        delete m_wav_file;
        m_wav_file = nullptr;
    }
}

bool AudioFileLoader::start(){

    m_frames_per_timeout = m_audio_format.sampleRate() * m_timer_interval_ms / 1000;

    if (has_extension(m_filename, "wav")){
        // Use WavFile to load unencoded samples:
        if (!init_wav_file()){
            return false;
        }

        const QAudioFormat& wav_audio_format = m_wav_file->audioFormat();
        //  Up to 8 channels: that is the range with a standard channel
        //  layout (the mixer's fallback_channel_config), which is what the
        //  tree's mix matrix is built from. Beyond that the tree cannot
        //  mix the input, so the file is rejected up front.
        if (wav_audio_format.channelCount() > 8){
            std::cout << "Error: audio file input supports up to 8 channels; the wav file has "
                << wav_audio_format.channelCount() << " channels." << std::endl;
            return false;
        }

        //  The loader streams the wav's native format: the caller
        //  (AudioInputFile) sets m_audio_format to the file's own format
        //  (probed from the header), so m_frames_per_timeout paces the timer
        //  at the file's sample rate and the raw bytes are emitted as-is.
        build_timer();
        connect(m_timer, &QTimer::timeout, this, &AudioFileLoader::send_buffer_from_wav_file_on_timer);
        
        return true;
    }

    // Use QAudioDecoder to decode compressed audio file:
    m_audio_decoder_worker = static_cast<AudioDecoderWorker*>(
        GlobalThreadPools::qt_event_threadpool().add_object([this]{
            return std::make_unique<AudioDecoderWorker>(nullptr, m_filename, m_audio_format, m_raw_buffer);
        })
    );

    // When the decoder finishes decoding the entire file, launch a timer to send decoded audio
    // frames to outside at desired frame rate.
    // Note: here we save all the decoded raw samples to memory before sending them out.
    // For a large audio file this is problematic, will consume lots of memory. But for small audio
    // files this is OK.
    connect(m_audio_decoder_worker, &AudioDecoderWorker::finished, this, [&](){
        std::cout << "Audio decoder finishes decoding. Start playing audio at desired sample rate" << std::endl;
        if (m_raw_buffer.empty()){
            std::cout << "Error: audio decoder produced no audio data for " << m_filename
                << " (the decoder could not find usable audio in the file; nothing to play)." << std::endl;
            return;
        }
        build_timer();
        connect(m_timer, &QTimer::timeout, this, &AudioFileLoader::send_decoded_buffer_on_timer);
    });

    emit m_audio_decoder_worker->start();

    return m_audio_decoder_worker->start_succeeded();
}

void AudioFileLoader::build_timer(){
    m_timer = new QTimer(this);
    m_timer->setTimerType(Qt::PreciseTimer);
    m_timer->start((int)m_timer_interval_ms);
}

std::tuple<const char*, size_t> AudioFileLoader::load_full_audio(){
    if (has_extension(m_filename, "wav")){
        // Use WavFile to load unencoded samples:
        if (!init_wav_file()){
            return std::make_tuple<const char*, size_t>(nullptr, 0);
        }

        //  The batch API converts the sample type and channels to the
        //  requested format, but not the sample rate: its consumer
        //  (load_audio_template) assumes the returned data is at the requested
        //  rate.
        if (m_wav_file->audioFormat().sampleRate() != m_audio_format.sampleRate()){
            std::cout << "Error: load_full_audio() requires the wav file's sample rate ("
                << m_wav_file->audioFormat().sampleRate() << ") to match the requested sample rate ("
                << m_audio_format.sampleRate() << "); it does not resample." << std::endl;
            return std::make_tuple<const char*, size_t>(nullptr, 0);
        }

        const size_t num_bytes = m_wav_file->size() - m_wav_file->pos();
        m_raw_buffer.resize(num_bytes);
        const size_t bytes_read = m_wav_file->read(m_raw_buffer.data(), num_bytes);
        if (bytes_read != num_bytes){
            std::cout << "Error: failed to read all bytes from wav file: " << bytes_read << " < " << num_bytes << std::endl;
        }
        return convert_raw_wav_samples();
    }

    // Use QAudioDecoder to decode compressed audio file:
    m_audio_decoder_worker = static_cast<AudioDecoderWorker*>(
        GlobalThreadPools::qt_event_threadpool().add_object([this]{
            return std::make_unique<AudioDecoderWorker>(nullptr, m_filename, m_audio_format, m_raw_buffer);
        })
    );
//    m_audio_decoder_worker = new AudioDecoderWorker(nullptr, m_filename, m_audio_format, m_raw_buffer);
//    m_audio_decoder_worker->moveToThread(&m_audioDecoderThread);

    connect(this, &AudioFileLoader::run_audio_decoder_async, m_audio_decoder_worker, &AudioDecoderWorker::start);
//    connect(&m_audioDecoderThread, &QThread::finished, m_audio_decoder_worker, &QObject::deleteLater);

    QEventLoop loop;
    connect(m_audio_decoder_worker, &AudioDecoderWorker::errored, &loop, &QEventLoop::quit);
    connect(m_audio_decoder_worker, &AudioDecoderWorker::finished, &loop, &QEventLoop::quit);

    // m_audioDecoderThread takes care of m_audio_decoder_worker via finished -> deleteLater connection.
    // So we don't hold the pointer m_audio_decoder_worker anymore.
    m_audio_decoder_worker = nullptr;

//    m_audioDecoderThread.start();
    emit run_audio_decoder_async();

    // Block the current thread until the m_audio_decoder_worker errored or finished.
    loop.exec();
    if (m_raw_buffer.empty()){
        std::cout << "Error: audio decoder produced no audio data for " << m_filename
            << " (the decoder could not find usable audio in the file)." << std::endl;
    }
    
//    m_audioDecoderThread.quit();
//    m_audioDecoderThread.wait();
    GlobalThreadPools::qt_event_threadpool().remove_object(m_audio_decoder_worker);
    return std::make_tuple<const char*, size_t>(m_raw_buffer.data(), m_raw_buffer.size());
}

bool AudioFileLoader::init_wav_file(){
    std::cout << "Initialize WavFile on " << m_filename << std::endl;
    m_wav_file = new WavFile(this);
    if (m_wav_file->open(QString::fromStdString(m_filename)) == false){
        std::cout << "Error: WavFile cannot open file " << m_filename << std::endl;
        return false;
    }

    std::cout << "Wav file audio format: " << dump_audio_format(m_wav_file->audioFormat());

    //  The wav's native rate / channels / sample type are the stream's
    //  format: the caller (AudioInputFile) probes the header and sets
    //  m_audio_format to the file's own format, so the raw bytes stream
    //  through unconverted at the file's rate. (load_full_audio() keeps the
    //  "convert to the requested format" behavior for its batch consumer.)

    return true;
}


void AudioFileLoader::send_decoded_buffer_on_timer(){
    const int bytes_per_frame = m_audio_format.bytesPerFrame();
    const size_t bytes_to_send = bytes_per_frame * m_frames_per_timeout;

    const auto& buffer = m_raw_buffer;

    const size_t buffer_end = std::min(m_buffer_next + bytes_to_send, buffer.size());

//    const float* float_data = reinterpret_cast<const float*>(buffer.data() + m_buffer_next);
    // std::cout << "Timer: " << float_data[0] << " " << float_data[1] << "... " << buffer.size() << " " << m_buffer_next << std::endl;

    const size_t sent_len = buffer_end - m_buffer_next;
    emit buffer_ready(buffer.data() + m_buffer_next, sent_len);

    m_buffer_next = buffer_end;

    if (buffer_end == buffer.size()){
        if (m_timer){
            m_timer->stop();
        }
        emit finished();
    }
}

void AudioFileLoader::send_buffer_from_wav_file_on_timer(){
    const QAudioFormat& wav_audio_format = m_wav_file->audioFormat();
    const int wav_bytes_per_frame = wav_audio_format.bytesPerFrame();
    const int wav_bytes_to_read = (int)(wav_bytes_per_frame * m_frames_per_timeout);
    m_raw_buffer.resize(wav_bytes_to_read);
    const int64_t wav_bytes_read = m_wav_file->read(m_raw_buffer.data(), wav_bytes_to_read);
    m_raw_buffer.resize(wav_bytes_read);

    if (wav_bytes_read == 0){
        if (m_timer){
            m_timer->stop();
        }
        emit finished();
        return;
    }

    //  The file's native format is the stream's format, so the raw bytes are
    //  emitted as-is (no conversion).
    emit buffer_ready(m_raw_buffer.data(), (size_t)wav_bytes_read);
}

std::pair<const char*, size_t> AudioFileLoader::convert_raw_wav_samples(){
    const QAudioFormat& wav_audio_format = m_wav_file->audioFormat();

    const size_t wav_bytes_read = m_raw_buffer.size();
    
    // Simple case, the target audio format is the same as the format used in the wav file.
    if (m_audio_format == wav_audio_format){
        return {m_raw_buffer.data(), wav_bytes_read};
    }

    // Now we need to convert the audio samples to the target format:

    const size_t wav_bytes_per_frame = wav_audio_format.bytesPerFrame();
    const size_t wav_num_channels = wav_audio_format.channelCount();
    const size_t wav_bytes_per_sample = wav_bytes_per_frame / wav_num_channels;
    const size_t frames_read = wav_bytes_read / wav_bytes_per_frame;
    const size_t samples_read = wav_bytes_read / wav_bytes_per_sample;

    // m_float_buffer holds the converted float-type samples
    // TODO: design a general format conversion method
    m_float_buffer.resize(samples_read);
    convert_samples_to_float(wav_audio_format, m_raw_buffer.data(), wav_bytes_read, m_float_buffer.data());
    
    if (m_audio_format.channelCount() == wav_audio_format.channelCount()){
        return {
            reinterpret_cast<const char*>(m_float_buffer.data()),
            m_float_buffer.size() * sizeof(float)
        };
    }
    
    if(m_audio_format.channelCount() == 1){
        // Input wav file has stereo or more channels, but output format is mono,
        // average L and R channel samples per frame:
        for (size_t i = 0; i < frames_read; i++){
            m_float_buffer[i] = (m_float_buffer[2*i] + m_float_buffer[2*i+1]) / 2.0f;
        }
        return {reinterpret_cast<const char*>(m_float_buffer.data()), frames_read * sizeof(float)};
    }
    
    if(wav_audio_format.channelCount() == 1){
        // Input wav is mono but output format is stereo,
        // Duplicate samples for each channel:
        m_float_buffer.resize(samples_read * m_audio_format.channelCount());
        for (size_t i = samples_read; i-- > 0;){
            const float v = m_float_buffer[i];
            for (int j = m_audio_format.channelCount()-1; j >= 0; j--){
                m_float_buffer[m_audio_format.channelCount()*i+j] = v;
            }
        }
        return {
            reinterpret_cast<const char*>(m_float_buffer.data()),
            frames_read * m_audio_format.channelCount() * sizeof(float)
        };
    }
    
    std::cout << "Error format conversion" << std::endl;
    std::cout << "Wav file format: " << dump_audio_format(wav_audio_format);
    std::cout << "Audio format to convert to: " << dump_audio_format(m_audio_format);
    return {reinterpret_cast<const char*>(m_float_buffer.data()), 0};
}


AudioDecoderWorker::AudioDecoderWorker(
    QObject* parent,
    const std::string& filename,
    const QAudioFormat& audio_format,
    std::vector<char>& decoded_buffer
):
    QObject(parent), m_filename(filename), m_audio_format(audio_format), m_decoded_buffer(decoded_buffer){}


void AudioDecoderWorker::internal_start(){
    m_audio_decoder = new QAudioDecoder(this);

#if 0
    connect(
        m_audio_decoder, &QAudioDecoder::bufferAvailableChanged,
        this, [](bool available){
            std::cout << "QAudioDecoder::bufferAvailableChanged(): " << available << std::endl;
        }
    );
#endif

    // Whenever a new buffer of audo frames decoded, save them by calling read_audio_decoder_buffer().
    connect(m_audio_decoder, &QAudioDecoder::bufferReady, this, &AudioDecoderWorker::read_audio_decoder_buffer);

    // When the decoder finishes decoding the entire file, launch a timer to send decoded audio
    // frames to outside at desired frame rate.
    // Note: here we save all the decoded raw samples to memory before sending them out.
    // For a large audio file this is problematic, will consume lots of memory. But for small audio
    // files this is OK.
    connect(m_audio_decoder, &QAudioDecoder::finished, this, &AudioDecoderWorker::finished);

    m_audio_decoder->setAudioFormat(m_audio_format);
    if (m_audio_decoder->error() != QAudioDecoder::NoError){
        handle_audio_decoder_error();
        m_start_succeeded = false;
        return;
    }

    connect(m_audio_decoder, QOverload<QAudioDecoder::Error>::of(&QAudioDecoder::error), this, [&](QAudioDecoder::Error){
        handle_audio_decoder_error();
    });

    m_audio_decoder->setSource(QUrl::fromLocalFile(QString::fromStdString(m_filename)));

    m_audio_decoder->start();
    std::cout << "Start audio decoder." << std::endl;

    m_start_succeeded = true;
    return;
}

AudioDecoderWorker::~AudioDecoderWorker(){
    stop();
}

void AudioDecoderWorker::internal_stop(){
    if (m_audio_decoder){
        m_audio_decoder->stop();
    }
}

void AudioDecoderWorker::read_audio_decoder_buffer(){
    if (m_audio_decoder == nullptr){
        return;
    }

    const QAudioBuffer& buffer = m_audio_decoder->read();
    size_t old_size = m_decoded_buffer.size();
    m_decoded_buffer.resize(old_size + buffer.byteCount());
    memcpy(m_decoded_buffer.data() + old_size, buffer.data<char>(), buffer.byteCount());

#ifdef DEBUG_AUDIO_DECODER_WORKER
    std::cout << "Read QAudioBuffer (size " << buffer.byteCount() << ") from audio decoder, cur pos " <<
        m_audio_decoder->position() << " ms, total length " << m_audio_decoder->duration() << " ms, " << 
        "buffer length now " << m_decoded_buffer.size() << std::endl;
#endif
}


void AudioDecoderWorker::handle_audio_decoder_error(){
    if (m_audio_decoder){
        QAudioDecoder::Error error = m_audio_decoder->error();
        std::string error_str;
        switch(error){
        case QAudioDecoder::ResourceError:
            error_str = "ResourceError";
            break;
        case QAudioDecoder::AccessDeniedError:
            error_str = "AccessDeniedError";
            break;
        case QAudioDecoder::FormatError:
            error_str = "FormatError";
            break;
        case QAudioDecoder::NotSupportedError:
            error_str = "NotSupportedError";
            break;
        default:
            error_str = "UnownError";
        }

        std::cout << "QAudioDecoder error when loading file: " << m_filename << ", " <<
            error_str << ": " << m_audio_decoder->errorString().toStdString() << std::endl;

        emit errored();
    }
}

}
