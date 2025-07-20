/*  Stream Recorder
 *
 *  From: https://github.com/PokemonAutomation/
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
//#include "Common/Qt/Redispatch.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/VideoPipeline/Backends/VideoFrameQt.h"
#include "CommonFramework/Recording/StreamHistoryOption.h"
#include "StreamRecorder.h"


//  This doesn't work yet. QMediaRecorder writes different bytes with
//  setOutputDevice() vs. setOutputLocation().
//  So far I can't get setOutputDevice() to work. The bytes that it
//  spits out is not a valid .mp4 file.

//#define PA_STREAM_HISTORY_LOCAL_BUFFER


//  REMOVE
#include <iostream>
using std::cout;
using std::endl;


namespace PokemonAutomation{


StreamRecording::StreamRecording(
    Logger& logger,
    std::chrono::milliseconds buffer_limit,
    WallClock start_time,
    size_t audio_samples_per_frame,
    size_t audio_frames_per_second,
    bool has_video
)
    : m_logger(logger)
    , m_buffer_limit(buffer_limit)
    , m_start_time(start_time)
    , m_audio_samples_per_frame(audio_samples_per_frame)
    , m_has_video(has_video)
    , m_filename(GlobalSettings::instance().TEMP_FOLDER)
    , m_stopping(false)
    , m_last_drop(current_time())
    , m_last_frame_time(std::numeric_limits<qint64>::min())
{
    logger.log(
        "StreamRecording: Audio = " + std::to_string(audio_samples_per_frame) +
        ", Video = " + std::to_string(has_video)
    );
//    cout << "audio = " << audio_samples_per_frame << ", video = " << has_video << endl;

    if (audio_samples_per_frame == 0 && !has_video){
        return;
    }

    if (audio_samples_per_frame > 0){
        m_audio_format.setChannelCount((int)audio_samples_per_frame);
        m_audio_format.setChannelConfig(audio_samples_per_frame == 1 ? QAudioFormat::ChannelConfigMono : QAudioFormat::ChannelConfigStereo);
        m_audio_format.setSampleRate((int)audio_frames_per_second);
        m_audio_format.setSampleFormat(QAudioFormat::Float);
    }

#ifndef PA_STREAM_HISTORY_LOCAL_BUFFER
    QDir().mkdir(QString::fromStdString(m_filename));
#endif
    if (has_video){
        m_filename += now_to_filestring() + ".mp4";
    }else{
        m_filename += now_to_filestring() + ".m4a";
    }

    start();
}
StreamRecording::~StreamRecording(){
    {
        std::lock_guard<std::mutex> lg(m_lock);
        m_stopping = true;
//        cout << "signalling: ~StreamRecording()" << endl;
        m_cv.notify_all();
    }
    quit();
    wait();
#ifndef PA_STREAM_HISTORY_LOCAL_BUFFER
    if (!m_filename.empty()){
        QDir().remove(QString::fromStdString(m_filename));
    }
#endif
}

bool StreamRecording::stop_and_save(const std::string& filename){
    auto scope_check = m_santizer.check_scope();
    {
        std::lock_guard<std::mutex> lg(m_lock);
        m_stopping = true;
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
    auto scope_check = m_santizer.check_scope();
#if 1
    WallClock now = current_time();
    if (m_audio_samples_per_frame == 0 || now < m_start_time){
        return;
    }
    WallClock threshold = timestamp - m_buffer_limit;

    std::lock_guard<std::mutex> lg(m_lock);
    if (m_stopping){
        return;
    }

    do{
        if (m_buffered_audio.empty()){
            break;
        }

        //  Too much has been buffered. Drop the block.
        if (m_buffered_audio.front().timestamp < threshold){
            //  Throttle the prints.
            if (now - m_last_drop > std::chrono::seconds(5)){
                m_last_drop = now;
                m_logger.log("Unable to keep up with audio recording. Dropping samples.", COLOR_RED);
            }
            return;
        }

    }while (false);

    //  Enqueue the sample block.
    m_buffered_audio.emplace_back(
        timestamp,
        data, frames * m_audio_samples_per_frame
    );
    m_cv.notify_all();
#endif
}
void StreamRecording::push_frame(std::shared_ptr<const VideoFrame> frame){
    auto scope_check = m_santizer.check_scope();
#if 1
    WallClock now = current_time();
    if (!m_has_video || now < m_start_time){
        return;
    }
//    cout << "push_frame(): " << frame->frame.startTime() << " - " << frame->frame.endTime() << endl;
    WallClock threshold = frame->timestamp - m_buffer_limit;

    std::lock_guard<std::mutex> lg(m_lock);
    if (m_stopping){
        return;
    }

    qint64 frame_time = frame->frame.startTime();
    do{
        if (m_buffered_frames.empty()){
            break;
        }

        //  Too much has been buffered. Drop the frame.
        if (m_buffered_frames.front()->timestamp < threshold){
            //  Throttle the prints.
            if (now - m_last_drop > std::chrono::seconds(5)){
                m_last_drop = now;
                m_logger.log("Unable to keep up with video recording. Dropping samples.", COLOR_RED);
            }
            return;
        }

        //  Non-increasing timestamp. Drop possible duplicate frame.
        if (frame_time <= m_last_frame_time){
            return;
        }
    }while (false);

    //  Enqueue the frame.
    m_buffered_frames.emplace_back(std::move(frame));
    m_last_frame_time = frame_time;
    m_cv.notify_all();
#endif
}



std::unique_ptr<QAudioBufferInput> StreamRecording::initialize_audio(){
    std::unique_ptr<QAudioBufferInput> ret(new QAudioBufferInput());
    m_audio_input = ret.get();
    m_session->setAudioBufferInput(m_audio_input);

    connect(
        m_audio_input, &QAudioBufferInput::readyToSendAudioBuffer,
        m_recorder, [this](){
            std::lock_guard<std::mutex> lg(m_lock);
            m_cv.notify_all();
        },
        Qt::DirectConnection
    );

    return ret;
}
std::unique_ptr<QVideoFrameInput> StreamRecording::initialize_video(){
    std::unique_ptr<QVideoFrameInput> ret(new QVideoFrameInput());
    m_video_input = ret.get();
    m_session->setVideoFrameInput(m_video_input);

    const StreamHistoryOption& settings = GlobalSettings::instance().STREAM_HISTORY;

    switch (settings.RESOLUTION){
    case StreamHistoryOption::Resolution::MATCH_INPUT:
        break;
    case StreamHistoryOption::Resolution::FORCE_720p:
        m_recorder->setVideoResolution(1280, 720);
        break;
    case StreamHistoryOption::Resolution::FORCE_1080p:
        m_recorder->setVideoResolution(1920, 1080);
        break;
    }

    switch (settings.ENCODING_MODE){
    case StreamHistoryOption::EncodingMode::FIXED_QUALITY:
        switch (settings.VIDEO_QUALITY){
        case StreamHistoryOption::VideoQuality::VERY_LOW:
            m_recorder->setQuality(QMediaRecorder::VeryLowQuality);
            break;
        case StreamHistoryOption::VideoQuality::LOW:
            m_recorder->setQuality(QMediaRecorder::LowQuality);
            break;
        case StreamHistoryOption::VideoQuality::NORMAL:
            m_recorder->setQuality(QMediaRecorder::NormalQuality);
            break;
        case StreamHistoryOption::VideoQuality::HIGH:
            m_recorder->setQuality(QMediaRecorder::HighQuality);
            break;
        case StreamHistoryOption::VideoQuality::VERY_HIGH:
            m_recorder->setQuality(QMediaRecorder::VeryHighQuality);
            break;
        }
        break;
    case StreamHistoryOption::EncodingMode::FIXED_BITRATE:
        m_recorder->setVideoBitRate(settings.VIDEO_BITRATE * 1000);
        m_recorder->setEncodingMode(QMediaRecorder::AverageBitRateEncoding);
        break;
    }

    connect(
        m_video_input, &QVideoFrameInput::readyToSendVideoFrame,
        m_recorder, [this](){
            std::lock_guard<std::mutex> lg(m_lock);
            m_cv.notify_all();
        },
        Qt::DirectConnection
    );

    return ret;
}

void StreamRecording::internal_run(){
    QMediaCaptureSession session;
    QMediaRecorder recorder;
    m_session = &session;
    m_recorder = &recorder;
    m_session->setRecorder(m_recorder);

    std::unique_ptr<QAudioBufferInput> audio;
    std::unique_ptr<QVideoFrameInput> video;

    //  Only initialize the streams we intend to use.
    if (m_audio_samples_per_frame > 0){
        audio = initialize_audio();
    }
    if (m_has_video){
        video = initialize_video();
    }

    m_recorder->setMediaFormat(QMediaFormat::MPEG4);

    connect(
        m_recorder, &QMediaRecorder::recorderStateChanged,
        m_recorder, [this](QMediaRecorder::RecorderState state){
            if (state == QMediaRecorder::StoppedState){
                std::lock_guard<std::mutex> lg(m_lock);
//                cout << "signalling: StoppedState" << endl;
                m_stopping = true;
                m_cv.notify_all();
            }
        },
        Qt::DirectConnection
    );


#ifdef PA_STREAM_HISTORY_LOCAL_BUFFER
    m_recorder->setOutputDevice(&m_write_buffer);
#else
    QFileInfo file(QString::fromStdString(m_filename));
    m_recorder->setOutputLocation(
        QUrl::fromLocalFile(file.absoluteFilePath())
    );
#endif

//    cout << "Encoding Mode = " << (int)recorder.encodingMode() << endl;
//    cout << "Bit Rate = " << (int)recorder.videoBitRate() << endl;


#if 1
//    cout << "starting recording" << endl;
    m_recorder->record();

    AudioBlock current_audio;
    std::shared_ptr<const VideoFrame> current_frame;
    QAudioBuffer audio_buffer;

    while (true){
        QCoreApplication::processEvents();

        {
            std::unique_lock<std::mutex> lg(m_lock);
            if (m_stopping){
                break;
            }

            if (!current_audio.is_valid() && !m_buffered_audio.empty()){
                current_audio = std::move(m_buffered_audio.front());
                m_buffered_audio.pop_front();
            }
            if (!current_frame && !m_buffered_frames.empty()){
                current_frame = std::move(m_buffered_frames.front());
                m_buffered_frames.pop_front();
            }

            if (!current_audio.is_valid() && !current_frame){
//                cout << "sleeping 0..." << endl;
                m_cv.wait(lg);
//                cout << "waking 0..." << endl;
            }
        }

        bool progress_made = false;

        if (current_audio.is_valid()){
            if (!audio_buffer.isValid()){
                const std::vector<float>& samples = current_audio.samples;
                QByteArray bytes((const char*)samples.data(), samples.size() * sizeof(float));
                audio_buffer = QAudioBuffer(bytes, m_audio_format);
            }
            if (audio_buffer.isValid() && m_audio_input->sendAudioBuffer(audio_buffer)){
                current_audio.clear();
                audio_buffer = QAudioBuffer();
                progress_made = true;
            }
        }
//        cout << "Before: " << m_video_input << endl;
        if (current_frame && m_video_input->sendVideoFrame(current_frame->frame)){
//            cout << "push frame: " << current_frame->frame.startTime() << endl;
            current_frame.reset();
            progress_made = true;
        }
//        cout << "After: " << m_video_input << endl;

        if (!progress_made){
            std::unique_lock<std::mutex> lg(m_lock);
            if (m_stopping){
                break;
            }
//            cout << "sleeping 1..." << endl;
            m_cv.wait(lg);
//            cout << "waking 1..." << endl;
        }
    }

    m_recorder->stop();
//    cout << "recorder.stop()" << endl;
#endif


    while (m_recorder->recorderState() != QMediaRecorder::StoppedState){
//        cout << "StreamHistoryTracker: process" << endl;
        QCoreApplication::processEvents();
        pause();
    }

    exec();

}
void StreamRecording::run(){
    auto scope_check = m_santizer.check_scope();
    try{
        internal_run();
    }catch (...){
        m_logger.log("Exception thrown out of stream recorder...", COLOR_RED);
        std::lock_guard<std::mutex> lg(m_lock);
        m_stopping = true;
    }
}





StreamRecording2::StreamRecording2(
    Logger& logger,
    std::chrono::milliseconds buffer_limit,
    WallClock start_time,
    size_t audio_samples_per_frame,
    size_t audio_frames_per_second,
    bool has_video
)
    : m_logger(logger)
    , m_buffer_limit(buffer_limit)
    , m_start_time(start_time)
    , m_audio_samples_per_frame(audio_samples_per_frame)
    , m_has_video(has_video)
    , m_filename(GlobalSettings::instance().TEMP_FOLDER)
    , m_stopping(false)
    , m_last_drop(current_time())
    , m_last_frame_time(std::numeric_limits<qint64>::min())
{
    logger.log(
        "StreamRecording: Audio = " + std::to_string(audio_samples_per_frame) +
        ", Video = " + std::to_string(has_video)
    );
//    cout << "audio = " << audio_samples_per_frame << ", video = " << has_video << endl;

    if (audio_samples_per_frame == 0 && !has_video){
        return;
    }

    if (audio_samples_per_frame > 0){
        m_audio_format.setChannelCount((int)audio_samples_per_frame);
        m_audio_format.setChannelConfig(audio_samples_per_frame == 1 ? QAudioFormat::ChannelConfigMono : QAudioFormat::ChannelConfigStereo);
        m_audio_format.setSampleRate((int)audio_frames_per_second);
        m_audio_format.setSampleFormat(QAudioFormat::Float);
    }

#ifndef PA_STREAM_HISTORY_LOCAL_BUFFER
    QDir().mkdir(QString::fromStdString(m_filename));
#endif
    if (has_video){
        m_filename += now_to_filestring() + ".mp4";
    }else{
        m_filename += now_to_filestring() + ".m4a";
    }

    m_recorder.reset(new QMediaRecorder());
    m_session.setRecorder(m_recorder.get());

    //  Only initialize the streams we intend to use.
    if (m_audio_samples_per_frame > 0){
        initialize_audio();
    }
    if (m_has_video){
        initialize_video();
    }

    m_recorder->setMediaFormat(QMediaFormat::MPEG4);



#ifdef PA_STREAM_HISTORY_LOCAL_BUFFER
    m_recorder->setOutputDevice(&m_write_buffer);
#else
    QFileInfo file(QString::fromStdString(m_filename));
    m_recorder->setOutputLocation(
        QUrl::fromLocalFile(file.absoluteFilePath())
    );
#endif

    m_recorder->record();
}
StreamRecording2::~StreamRecording2(){
#if 1
    stop();
#else
    {
        std::unique_lock<std::mutex> lg(m_lock);
        if (m_stopping){
            return;
        }
        m_stopping = true;
    }

    m_recorder->stop();
    m_recorder.reset();
#endif

#ifndef PA_STREAM_HISTORY_LOCAL_BUFFER
    cout << QDir().remove(QString::fromStdString(m_filename)) << endl;  //  REMOVE
#endif
}

void StreamRecording2::stop(){
    auto scope_check = m_santizer.check_scope();

    {
        std::unique_lock<std::mutex> lg(m_lock);
        if (m_stopping){
            return;
        }
        m_stopping = true;
    }

#if 0
    run_on_main_thread_and_wait([this]{
        QEventLoop loop;
        m_recorder.connect(
            &m_recorder, &QMediaRecorder::recorderStateChanged,
            &m_recorder, [&](QMediaRecorder::RecorderState state){
                if (state == QMediaRecorder::StoppedState){
                    loop.quit();
                }
            }
        );

        emit m_recorder.stop();

        if (m_recorder.recorderState() != QMediaRecorder::StoppedState){
            loop.exec();
        }
    });
#endif

    m_recorder->stop();

    while (m_recorder->recorderState() != QMediaRecorder::StoppedState){
        cout << "Spin waiting..." << endl;  //  REMOVE
        pause();
    }
}


void StreamRecording2::initialize_audio(){
    m_audio_input.reset(new QAudioBufferInput());
    m_session.setAudioBufferInput(m_audio_input.get());

    m_recorder->connect(
        m_audio_input.get(), &QAudioBufferInput::readyToSendAudioBuffer,
        m_recorder.get(), [this](){
//            cout << "readyToSendAudioBuffer()" << endl;
//            std::lock_guard<std::mutex> lg(m_lock);
//            m_cv.notify_all();
            process();
        },
        Qt::DirectConnection
    );
}
void StreamRecording2::initialize_video(){
    m_video_input.reset(new QVideoFrameInput());
    m_session.setVideoFrameInput(m_video_input.get());

    const StreamHistoryOption& settings = GlobalSettings::instance().STREAM_HISTORY;

    switch (settings.RESOLUTION){
    case StreamHistoryOption::Resolution::MATCH_INPUT:
        break;
    case StreamHistoryOption::Resolution::FORCE_720p:
        m_recorder->setVideoResolution(1280, 720);
        break;
    case StreamHistoryOption::Resolution::FORCE_1080p:
        m_recorder->setVideoResolution(1920, 1080);
        break;
    }

    switch (settings.ENCODING_MODE){
    case StreamHistoryOption::EncodingMode::FIXED_QUALITY:
        switch (settings.VIDEO_QUALITY){
        case StreamHistoryOption::VideoQuality::VERY_LOW:
            m_recorder->setQuality(QMediaRecorder::VeryLowQuality);
            break;
        case StreamHistoryOption::VideoQuality::LOW:
            m_recorder->setQuality(QMediaRecorder::LowQuality);
            break;
        case StreamHistoryOption::VideoQuality::NORMAL:
            m_recorder->setQuality(QMediaRecorder::NormalQuality);
            break;
        case StreamHistoryOption::VideoQuality::HIGH:
            m_recorder->setQuality(QMediaRecorder::HighQuality);
            break;
        case StreamHistoryOption::VideoQuality::VERY_HIGH:
            m_recorder->setQuality(QMediaRecorder::VeryHighQuality);
            break;
        }
        break;
    case StreamHistoryOption::EncodingMode::FIXED_BITRATE:
        m_recorder->setVideoBitRate(settings.VIDEO_BITRATE * 1000);
        m_recorder->setEncodingMode(QMediaRecorder::AverageBitRateEncoding);
        break;
    }

    m_recorder->connect(
        m_video_input.get(), &QVideoFrameInput::readyToSendVideoFrame,
        m_recorder.get(), [this](){
//            cout << "readyToSendVideoFrame()" << endl;
//            std::lock_guard<std::mutex> lg(m_lock);
//            m_cv.notify_all();
            process();
        },
        Qt::DirectConnection
    );
}




void StreamRecording2::push_samples(WallClock timestamp, const float* data, size_t frames){
    auto scope_check = m_santizer.check_scope();
#if 1
    WallClock now = current_time();
    if (m_audio_samples_per_frame == 0 || now < m_start_time){
        return;
    }
    WallClock threshold = timestamp - m_buffer_limit;

    {
        std::lock_guard<std::mutex> lg(m_lock);
        if (m_stopping){
            return;
        }

        do{
            if (m_buffered_audio.empty()){
                break;
            }

            //  Too much has been buffered. Drop the block.
            if (m_buffered_audio.front().timestamp < threshold){
                //  Throttle the prints.
                if (now - m_last_drop > std::chrono::seconds(5)){
                    m_last_drop = now;
                    m_logger.log("Unable to keep up with audio recording. Dropping samples.", COLOR_RED);
                }
                return;
            }

        }while (false);

        //  Enqueue the sample block.
        m_buffered_audio.emplace_back(
            timestamp,
            data, frames * m_audio_samples_per_frame
        );
    }

    emit m_audio_input->readyToSendAudioBuffer();
#endif
}
void StreamRecording2::push_frame(std::shared_ptr<const VideoFrame> frame){
    auto scope_check = m_santizer.check_scope();
#if 1
    WallClock now = current_time();
    if (!m_has_video || now < m_start_time){
        return;
    }
//    cout << "push_frame(): " << frame->frame.startTime() << " - " << frame->frame.endTime() << endl;
    WallClock threshold = frame->timestamp - m_buffer_limit;

    {
        std::lock_guard<std::mutex> lg(m_lock);
        if (m_stopping){
            return;
        }

        qint64 frame_time = frame->frame.startTime();
        do{
            if (m_buffered_frames.empty()){
                break;
            }

            //  Too much has been buffered. Drop the frame.
            if (m_buffered_frames.front()->timestamp < threshold){
                //  Throttle the prints.
                if (now - m_last_drop > std::chrono::seconds(5)){
                    m_last_drop = now;
                    m_logger.log("Unable to keep up with video recording. Dropping samples.", COLOR_RED);
                }
                return;
            }

            //  Non-increasing timestamp. Drop possible duplicate frame.
            if (frame_time <= m_last_frame_time){
                return;
            }
        }while (false);

        //  Enqueue the frame.
        m_buffered_frames.emplace_back(std::move(frame));
        m_last_frame_time = frame_time;
    }

    emit m_video_input->readyToSendVideoFrame();
#endif
}


bool StreamRecording2::stop_and_save(const std::string& filename){
    stop();

#ifdef PA_STREAM_HISTORY_LOCAL_BUFFER
    return m_write_buffer.write(m_logger, filename);
#else
    bool ret = QDir().rename(QString::fromStdString(m_filename), QString::fromStdString(filename));
    m_filename.clear();
    return ret;
#endif
}

void StreamRecording2::process(){
    auto scope_check = m_santizer.check_scope();

//    cout << "StreamRecording2::process()" << endl;

    bool progress_made;
    do{
        {
            std::unique_lock<std::mutex> lg(m_lock);
            if (m_stopping){
                cout << "exit" << endl; //  REMOVE
                break;
            }

            if (!m_current_audio.is_valid() && !m_buffered_audio.empty()){
                m_current_audio = std::move(m_buffered_audio.front());
                m_buffered_audio.pop_front();
            }
            if (!m_current_frame && !m_buffered_frames.empty()){
                m_current_frame = std::move(m_buffered_frames.front());
                m_buffered_frames.pop_front();
            }

            if (!m_current_audio.is_valid() && !m_current_frame){
                return;
            }
        }

        progress_made = false;

        if (m_current_audio.is_valid()){
            if (!m_audio_buffer.isValid()){
                const std::vector<float>& samples = m_current_audio.samples;
                QByteArray bytes((const char*)samples.data(), samples.size() * sizeof(float));
                m_audio_buffer = QAudioBuffer(bytes, m_audio_format);
            }
            if (m_audio_buffer.isValid() && m_audio_input->sendAudioBuffer(m_audio_buffer)){
                m_current_audio.clear();
                m_audio_buffer = QAudioBuffer();
                progress_made = true;
            }
        }
//        cout << "Before: " << m_video_input << endl;
        if (m_current_frame && m_video_input->sendVideoFrame(m_current_frame->frame)){
//            cout << "push frame: " << current_frame->frame.startTime() << endl;
            m_current_frame.reset();
            progress_made = true;
        }

    }while (progress_made);
}







}
#endif
