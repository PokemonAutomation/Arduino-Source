/*  Stream History Tracker
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Implement by recording in real-time.
 *
 */

#ifndef PokemonAutomation_StreamHistoryTracker_RecordOnTheFly_H
#define PokemonAutomation_StreamHistoryTracker_RecordOnTheFly_H

#include <mutex>
#include <QCoreApplication>
#include <QFileInfo>
#include <QUrl>
//#include <QThread>
#include <QAudioBufferInput>
#include <QVideoFrameInput>
#include <QMediaFormat>
#include <QMediaRecorder>
#include <QMediaCaptureSession>
#include "Common/Cpp/LifetimeSanitizer.h"
#include "Common/Cpp/PrettyPrint.h"
#include "Common/Cpp/AbstractLogger.h"
#include "Common/Cpp/Concurrency/SpinPause.h"
#include "Common/Cpp/Concurrency/SpinLock.h"
#include "CommonFramework/VideoPipeline/Backends/VideoFrameQt.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{


class RollingStream : public QIODevice{
public:
    ~RollingStream(){
        waitForBytesWritten(-1);
//        cout << "~RollingStream()" << endl;
    }
    RollingStream(){
        setOpenMode(QIODeviceBase::WriteOnly);
    }
    virtual qint64 readData(char* data, qint64 maxlen){ return 0; }
    virtual qint64 writeData(const char* data, qint64 len){
        auto scope_check = m_sanitizer.check_scope();
        m_bytes += len;
        cout << "total = " << m_bytes << ", current = " << len << endl;
        return len;
    }

private:
    uint64_t m_bytes = 0;

    LifetimeSanitizer m_sanitizer;
};



class StreamHistoryTracker{
public:
    ~StreamHistoryTracker();
    StreamHistoryTracker(
        size_t audio_samples_per_frame,
        size_t audio_frames_per_second,
        std::chrono::seconds window
    );
    void set_window(std::chrono::seconds window);

    bool save(Logger& logger, const std::string& filename) const;

public:
//    void push_frame(QVideoFrame frame);

    void on_samples(const float* data, size_t frames);
    void on_frame(std::shared_ptr<VideoFrame> frame);

private:
//    void clear_old();

private:
    mutable SpinLock m_lock;
    std::chrono::seconds m_window;

    const size_t m_audio_samples_per_frame;
//    const size_t m_audio_frames_per_second;
//    const size_t m_audio_samples_per_second;
//    const double m_microseconds_per_sample;

//    mutable std::mutex m_lock;
//    std::condition_variable m_cv;

    RollingStream m_stream;

    QAudioFormat m_audio_format;
    QAudioBufferInput m_audio_input;
    QVideoFrameInput m_video_input;
    QMediaCaptureSession m_session;

    QMediaRecorder m_recorder;
};


StreamHistoryTracker::~StreamHistoryTracker(){
    m_recorder.stop();
    while (m_recorder.recorderState() != QMediaRecorder::StoppedState){
//        cout << "StreamHistoryTracker: process" << endl;
        try{
            QCoreApplication::processEvents();
            pause();
        }catch (...){}
    }
//    cout << "~StreamHistoryTracker()" << endl;
}
StreamHistoryTracker::StreamHistoryTracker(
    size_t audio_samples_per_frame,
    size_t audio_frames_per_second,
    std::chrono::seconds window
)
    : m_window(window)
    , m_audio_samples_per_frame(audio_samples_per_frame)
//    , m_audio_frames_per_second(audio_frames_per_second)
//    , m_audio_samples_per_second(audio_samples_per_frame * audio_frames_per_second)
//    , m_microseconds_per_sample(1. / (m_audio_samples_per_second * 1000000.))
{
    m_audio_format.setChannelCount((int)audio_samples_per_frame);
    m_audio_format.setChannelConfig(audio_samples_per_frame == 1 ? QAudioFormat::ChannelConfigMono : QAudioFormat::ChannelConfigStereo);
    m_audio_format.setSampleRate((int)audio_frames_per_second);
    m_audio_format.setSampleFormat(QAudioFormat::Float);

    m_session.setAudioBufferInput(&m_audio_input);
    m_session.setVideoFrameInput(&m_video_input);
    m_session.setRecorder(&m_recorder);
    m_recorder.setMediaFormat(QMediaFormat::MPEG4);
    m_recorder.setQuality(QMediaRecorder::HighQuality);

#if 0
    m_recorder.connect(
        &m_recorder, &QMediaRecorder::recorderStateChanged,
        &m_recorder, [](QMediaRecorder::RecorderState state){

        }
    );
#endif

    QFileInfo file(QString::fromStdString("capture-" + now_to_filestring() + ".mp4"));
#if 1
    m_recorder.setOutputLocation(
        QUrl::fromLocalFile(file.absoluteFilePath())
    );
#else
    m_recorder.setOutputDevice(&m_stream);
#endif

    m_recorder.record();
}

void StreamHistoryTracker::set_window(std::chrono::seconds window){
    WriteSpinLock lg(m_lock);
    m_window = window;
}


void StreamHistoryTracker::on_samples(const float* samples, size_t frames){
    if (frames == 0){
        return;
    }
    QByteArray buffer((const char*)samples, m_audio_samples_per_frame * frames * sizeof(float));
    QAudioBuffer audio_buffer(buffer, m_audio_format);
    m_audio_input.sendAudioBuffer(audio_buffer);
}
void StreamHistoryTracker::on_frame(std::shared_ptr<VideoFrame> frame){
    m_video_input.sendVideoFrame(frame->frame);
}

bool StreamHistoryTracker::save(Logger& logger, const std::string& filename) const{
    logger.log("Cannot save stream history: Not implemented.", COLOR_RED);

    //  TODO

    return false;
}







}
#endif
