/*  Stream Recorder
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QtConfig>
#if (QT_VERSION_MAJOR == 6) && (QT_VERSION_MINOR >= 8)
#include <QFileInfo>
#include <QDir>
#include <QUrl>
#include <QAudioFormat>
#include <QMediaFormat>
#include <QCoreApplication>
#include <QAudioBufferInput>
#include <QVideoFrameInput>
#include <QMediaCaptureSession>
#include <QMediaRecorder>
#include "Common/Cpp/PrettyPrint.h"
#include "Common/Cpp/Concurrency/SpinPause.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/VideoPipeline/Backends/VideoFrameQt.h"
#include "CommonFramework/Recording/StreamHistoryOption.h"
#include "StreamRecorder.h"


//  This doesn't work yet. QMediaRecorder writes different bytes with
//  setOutputDevice() vs. setOutputLocation().
//  So far I can't get setOutputDevice() to work. The bytes that it
//  spits out is not a valid .mp4 file.

//#define PA_STREAM_HISTORY_LOCAL_BUFFER


namespace PokemonAutomation{


StreamRecording::StreamRecording(
    Logger& logger,
    std::chrono::milliseconds buffer_limit,
    size_t audio_samples_per_frame,
    size_t audio_frames_per_second,
    WallClock start_time
)
    : m_logger(logger)
    , m_buffer_limit(buffer_limit)
    , m_audio_samples_per_frame(audio_samples_per_frame)
    , m_start_time(start_time)
    , m_filename(GlobalSettings::instance().TEMP_FOLDER)
    , m_state(State::STARTING)
    , m_last_drop(current_time())
{
    m_audio_format.setChannelCount((int)audio_samples_per_frame);
    m_audio_format.setChannelConfig(audio_samples_per_frame == 1 ? QAudioFormat::ChannelConfigMono : QAudioFormat::ChannelConfigStereo);
    m_audio_format.setSampleRate((int)audio_frames_per_second);
    m_audio_format.setSampleFormat(QAudioFormat::Float);

#ifndef PA_STREAM_HISTORY_LOCAL_BUFFER
    QDir().mkdir(QString::fromStdString(m_filename));
#endif
    m_filename += now_to_filestring() + ".mp4";

    start();
}
StreamRecording::~StreamRecording(){
    {
        std::lock_guard<std::mutex> lg(m_lock);
        m_state = State::STOPPING;
//        cout << "signalling: ~StreamRecording()" << endl;
        m_cv.notify_all();
    }
    quit();
    wait();
#ifndef PA_STREAM_HISTORY_LOCAL_BUFFER
    QDir().remove(QString::fromStdString(m_filename));
#endif
}

bool StreamRecording::stop_and_save(const std::string& filename){
    {
        std::lock_guard<std::mutex> lg(m_lock);
        m_state = State::STOPPING;
//        cout << "signalling: stop_and_save()" << endl;
        m_cv.notify_all();
    }
    quit();
    wait();

#ifdef PA_STREAM_HISTORY_LOCAL_BUFFER
    return m_write_buffer.write(m_logger, filename);
#else
    bool ret = QDir().rename(QString::fromStdString(m_filename), QString::fromStdString(filename));
    m_filename.clear();
    return ret;
#endif
}


void StreamRecording::push_samples(WallClock timestamp, const float* data, size_t frames){
    WallClock now = current_time();
    if (now < m_start_time){
        return;
    }
    WallClock threshold = timestamp - m_buffer_limit;

    std::lock_guard<std::mutex> lg(m_lock);
    if (m_state != State::ACTIVE){
        return;
    }
    if (m_buffered_audio.empty() || m_buffered_audio.front()->timestamp > threshold){
        m_buffered_audio.emplace_back(std::make_shared<AudioBlock>(
            timestamp, data, frames * m_audio_samples_per_frame
        ));
        m_cv.notify_all();
        return;
    }
    if (now - m_last_drop > std::chrono::seconds(5)){
        m_last_drop = now;
        m_logger.log("Unable to keep up with audio recording. Dropping samples.", COLOR_RED);
    }
}
void StreamRecording::push_frame(std::shared_ptr<VideoFrame> frame){
    WallClock now = current_time();
    if (now < m_start_time){
        return;
    }
//        cout << "push_frame()" << endl;
    WallClock threshold = frame->timestamp - m_buffer_limit;

    std::lock_guard<std::mutex> lg(m_lock);
    if (m_state != State::ACTIVE){
        return;
    }
    if (m_buffered_frames.empty() || m_buffered_frames.front()->timestamp > threshold){
        m_buffered_frames.emplace_back(std::move(frame));
        m_cv.notify_all();
        return;
    }
    if (now - m_last_drop > std::chrono::seconds(5)){
        m_last_drop = now;
        m_logger.log("Unable to keep up with video recording. Dropping samples.", COLOR_RED);
    }
}



void StreamRecording::internal_run(){
    QAudioBufferInput audio_input;
    QVideoFrameInput video_input;
    m_audio_input = &audio_input;
    m_video_input = &video_input;

    QMediaCaptureSession session;
    QMediaRecorder recorder;
    session.setAudioBufferInput(&audio_input);
    session.setVideoFrameInput(&video_input);
    session.setRecorder(&recorder);
    recorder.setMediaFormat(QMediaFormat::MPEG4);
//    recorder.setQuality(QMediaRecorder::NormalQuality);
//    recorder.setQuality(QMediaRecorder::LowQuality);
//    recorder.setQuality(QMediaRecorder::VeryLowQuality);

    const StreamHistoryOption& settings = GlobalSettings::instance().STREAM_HISTORY;

    switch (settings.RESOLUTION){
    case StreamHistoryOption::Resolution::MATCH_INPUT:
        break;
    case StreamHistoryOption::Resolution::FORCE_720p:
        recorder.setVideoResolution(1280, 720);
        break;
    case StreamHistoryOption::Resolution::FORCE_1080p:
        recorder.setVideoResolution(1920, 1080);
        break;
    }

    switch (settings.ENCODING_MODE){
    case StreamHistoryOption::EncodingMode::FIXED_QUALITY:
        switch (settings.VIDEO_QUALITY){
        case StreamHistoryOption::VideoQuality::VERY_LOW:
            recorder.setQuality(QMediaRecorder::VeryLowQuality);
            break;
        case StreamHistoryOption::VideoQuality::LOW:
            recorder.setQuality(QMediaRecorder::LowQuality);
            break;
        case StreamHistoryOption::VideoQuality::NORMAL:
            recorder.setQuality(QMediaRecorder::NormalQuality);
            break;
        case StreamHistoryOption::VideoQuality::HIGH:
            recorder.setQuality(QMediaRecorder::HighQuality);
            break;
        case StreamHistoryOption::VideoQuality::VERY_HIGH:
            recorder.setQuality(QMediaRecorder::VeryHighQuality);
            break;
        }
        break;
    case StreamHistoryOption::EncodingMode::FIXED_BITRATE:
        recorder.setVideoBitRate(settings.VIDEO_BITRATE * 1000);
        recorder.setEncodingMode(QMediaRecorder::AverageBitRateEncoding);
        break;
    }


#ifdef PA_STREAM_HISTORY_LOCAL_BUFFER
    recorder.setOutputDevice(&m_write_buffer);
#else
    QFileInfo file(QString::fromStdString(m_filename));
    recorder.setOutputLocation(
        QUrl::fromLocalFile(file.absoluteFilePath())
    );
#endif

//    cout << "Encoding Mode = " << (int)recorder.encodingMode() << endl;
//    cout << "Bit Rate = " << (int)recorder.videoBitRate() << endl;

    connect(
        &audio_input, &QAudioBufferInput::readyToSendAudioBuffer,
        &recorder, [this](){
            std::lock_guard<std::mutex> lg(m_lock);
            m_cv.notify_all();
        },
        Qt::DirectConnection
    );
    connect(
        &video_input, &QVideoFrameInput::readyToSendVideoFrame,
        &recorder, [this](){
            std::lock_guard<std::mutex> lg(m_lock);
            m_cv.notify_all();
        },
        Qt::DirectConnection
    );
    connect(
        &recorder, &QMediaRecorder::recorderStateChanged,
        &recorder, [this](QMediaRecorder::RecorderState state){
            if (state == QMediaRecorder::StoppedState){
                std::lock_guard<std::mutex> lg(m_lock);
//                cout << "signalling: StoppedState" << endl;
                m_state = State::STOPPING;
                m_cv.notify_all();
            }
        },
        Qt::DirectConnection
    );

//    cout << "starting recording" << endl;
    recorder.record();

    std::shared_ptr<AudioBlock> current_audio;
    std::shared_ptr<VideoFrame> current_frame;
    QAudioBuffer audio_buffer;

    while (true){
//        cout << "recording loop" << endl;
        QCoreApplication::processEvents();

        {
            std::unique_lock<std::mutex> lg(m_lock);
            if (m_state == State::STOPPING){
                break;
            }
            m_state = State::ACTIVE;

            if (!current_audio && !m_buffered_audio.empty()){
                current_audio = std::move(m_buffered_audio.front());
                m_buffered_audio.pop_front();
            }
            if (!current_frame && !m_buffered_frames.empty()){
                current_frame = std::move(m_buffered_frames.front());
                m_buffered_frames.pop_front();
            }

            if (!current_audio && !current_frame){
//                cout << "sleeping 0..." << endl;
                m_cv.wait(lg);
//                cout << "waking 0..." << endl;
            }
        }

        bool progress_made = false;

        if (current_audio){
            if (!audio_buffer.isValid()){
                const std::vector<float>& samples = current_audio->samples;
                QByteArray bytes((const char*)samples.data(), samples.size() * sizeof(float));
                audio_buffer = QAudioBuffer(bytes, m_audio_format);
            }
            if (audio_buffer.isValid() && audio_input.sendAudioBuffer(audio_buffer)){
                current_audio.reset();
                audio_buffer = QAudioBuffer();
                progress_made = true;
            }
        }

        if (current_frame && video_input.sendVideoFrame(current_frame->frame)){
            current_frame.reset();
            progress_made = true;
        }

        if (!progress_made){
            std::unique_lock<std::mutex> lg(m_lock);
            if (m_state != State::ACTIVE){
                break;
            }
//            cout << "sleeping 1..." << endl;
            m_cv.wait(lg);
//            cout << "waking 1..." << endl;
        }
    }

    recorder.stop();
//    cout << "recorder.stop()" << endl;


    while (recorder.recorderState() != QMediaRecorder::StoppedState){
//        cout << "StreamHistoryTracker: process" << endl;
        QCoreApplication::processEvents();
        pause();
    }

    exec();

}
void StreamRecording::run(){
    try{
        internal_run();
    }catch (...){
        m_logger.log("Exception thrown out of stream recorder...", COLOR_RED);
        std::lock_guard<std::mutex> lg(m_lock);
        m_state = State::STOPPING;
    }
}





}
#endif
