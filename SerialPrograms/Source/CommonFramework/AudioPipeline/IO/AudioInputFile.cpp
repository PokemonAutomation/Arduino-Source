/*  Audio Input File
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <memory>
#include <QString>
#include <QUrl>
#include <QAudioBuffer>
#include <QAudioDecoder>
#include <QEventLoop>
#include <QTimer>
#include "3rdParty/QtWavFile/WavFile.h"
#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/AudioPipeline/AudioInfo.h"
#include "CommonFramework/AudioPipeline/Tools/AudioFormatUtils.h"
#include "AudioFileLoader.h"
#include "AudioInputFile.h"

namespace PokemonAutomation{



namespace{

Kernels::AudioPipeline::SampleFormat sample_format_of(const QAudioFormat& format){
    QAudioFormat copy = format;
    return to_sample_format(get_sample_format(copy));
}


}



bool probe_wav_native_format(const std::string& filename, QAudioFormat& out_format){
    WavFile wav_file;
    if (!wav_file.open(QString::fromStdString(filename))){
        return false;
    }
    out_format = wav_file.audioFormat();
    wav_file.close();
    if (out_format.channelCount() < 1 || out_format.sampleRate() < 1
        || out_format.sampleFormat() == QAudioFormat::SampleFormat::Unknown){
        return false;
    }
    return true;
}


bool probe_decoded_native_format(const std::string& filename, QAudioFormat& out_format){
    //  A QAudioDecoder started without a target format decodes in the
    //  source's native format, and the first decoded buffer carries that
    //  format. The probe runs a nested event loop on the calling thread
    //  until the decoder reaches the end of the file, errors, or a 3 s
    //  timeout. The decoder is throwaway: only the first buffer's format is
    //  kept. It is not stopped after the first buffer, because a decoder
    //  that is still decoding cannot be deleted safely (see
    //  delete_audio_decoder_when_done()).
    QAudioDecoder* decoder = new QAudioDecoder();
    if (!decoder->isSupported()){
        delete decoder;
        return false;
    }
    decoder->setSource(QUrl::fromLocalFile(QString::fromStdString(filename)));

    QEventLoop loop;
    bool got_format = false;
    bool running = true;
    bool reached_end = false;
    QObject::connect(
        decoder, &QAudioDecoder::bufferReady,
        &loop, [decoder, &out_format, &got_format]{
            const QAudioBuffer buffer = decoder->read();
            if (!got_format && buffer.isValid() && buffer.format().isValid()){
                out_format = buffer.format();
                got_format = true;
            }
        }
    );
    QObject::connect(
        decoder, QOverload<QAudioDecoder::Error>::of(&QAudioDecoder::error),
        &loop, [&running, &loop](QAudioDecoder::Error){
            running = false;
            loop.quit();
        }
    );
    QObject::connect(
        decoder, &QAudioDecoder::finished,
        &loop, [&running, &reached_end, &loop]{
            running = false;
            reached_end = true;
            loop.quit();
        }
    );

    QTimer timer;
    timer.setSingleShot(true);
    QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    decoder->start();
    //  A backend may emit error() from inside start(), and a quit() before
    //  exec() is lost.
    if (running){
        timer.start(3000);
        loop.exec();
        timer.stop();
    }

    const QAudioDecoder::Error decode_error = decoder->error();
    const std::string error_string = decoder->errorString().toStdString();

    //  The handlers above point into this stack frame.
    QObject::disconnect(decoder, nullptr, &loop, nullptr);
    delete_audio_decoder_when_done(decoder, running);

    if (got_format){
        return true;
    }
    if (decode_error != QAudioDecoder::NoError){
        std::cout << "probe_decoded_native_format: decoder error for " << filename
            << ": " << error_string << std::endl;
    }else if (reached_end){
        std::cout << "probe_decoded_native_format: decoder finished without producing any audio for "
            << filename << " (the decoder found no usable audio in the file)." << std::endl;
    }else{
        std::cout << "probe_decoded_native_format: no decoded buffer within 3 s for " << filename
            << std::endl;
    }
    return false;
}



AudioInputFile::AudioInputFile(
    Logger& logger,
    const std::string& file, const QAudioFormat& format
)
    : m_source(std::make_unique<AudioFileLoader>(nullptr, file, format))
    , m_sample_type(sample_format_of(format))
    , m_sample_rate(format.sampleRate())
    , m_channels(format.channelCount())
    , m_frame_bytes(format.bytesPerFrame())
{
    logger.log("AudioInputFile(): " + dump_audio_format(format));
    connect(
        m_source.get(), &AudioFileLoader::buffer_ready,
        this, [this](const char* data, size_t len){
            on_buffer_ready(data, len);
        }
    );
    m_source->start();
}
AudioInputFile::~AudioInputFile(){}

void AudioInputFile::add_listener(Kernels::AudioPipeline::RawAudioStreamListener& listener){
    auto scope_check = m_sanitizer.check_scope();
    if (listener.sample_type() != m_sample_type){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Sample type doesn't match.");
    }
    if (listener.sample_rate() != m_sample_rate){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Sample rate doesn't match.");
    }
    if (listener.channels() != m_channels){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Channel count doesn't match.");
    }
    m_listeners.add(listener);
}
void AudioInputFile::remove_listener(Kernels::AudioPipeline::RawAudioStreamListener& listener){
    auto scope_check = m_sanitizer.check_scope();
    m_listeners.remove(listener);
}

void AudioInputFile::on_buffer_ready(const char* data, size_t len){
    //  AudioFileLoader only emits whole multiples of the frame size, so the
    //  raw listeners can take the data as-is.
    m_listeners.run_method(
        &Kernels::AudioPipeline::RawAudioStreamListener::push_samples,
        len / m_frame_bytes,
        data
    );
}



}
