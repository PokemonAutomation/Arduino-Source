/*  Stream History Tracker
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Implement by saving the last X seconds of frames. This is currently not
 *  viable because the QVideoFrames are uncompressed.
 *
 */

#ifndef PokemonAutomation_StreamHistoryTracker_SaveFrames_H
#define PokemonAutomation_StreamHistoryTracker_SaveFrames_H

#include <deque>
#include <QCoreApplication>
#include <QFileInfo>
#include <QUrl>
#include <QAudioBufferInput>
#include <QVideoFrameInput>
#include <QMediaFormat>
#include <QMediaRecorder>
#include <QMediaCaptureSession>
#include "Common/Cpp/AbstractLogger.h"
#include "Common/Cpp/Concurrency/SpinLock.h"
#include "CommonFramework/VideoPipeline/Backends/VideoFrameQt.h"


//#include <iostream>
//using std::cout;
//using std::endl;


namespace PokemonAutomation{


struct AudioBlock{
    WallClock timestamp;
    std::vector<float> samples;

    AudioBlock(const AudioBlock&) = delete;
    void operator=(const AudioBlock&) = delete;

    AudioBlock(WallClock p_timestamp, const float* p_samples, size_t p_count)
        : timestamp(p_timestamp)
        , samples(p_samples, p_samples + p_count)
    {}
};


class StreamHistoryTracker{
public:
    StreamHistoryTracker(
        size_t audio_samples_per_frame,
        size_t audio_frames_per_second,
        std::chrono::seconds window
    );
    void set_window(std::chrono::seconds window);

    bool save(Logger& logger, const std::string& filename) const;

public:
    void on_samples(const float* data, size_t frames);
    void on_frame(std::shared_ptr<VideoFrame> frame);

private:
    void clear_old();

private:
    mutable SpinLock m_lock;
    std::chrono::seconds m_window;

    const size_t m_audio_samples_per_frame;
    const size_t m_audio_frames_per_second;
    const size_t m_audio_samples_per_second;
    const double m_microseconds_per_sample;

    //  We use shared_ptr here so it's fast to snapshot when we need to copy
    //  everything asynchronously.
    std::deque<std::shared_ptr<AudioBlock>> m_audio;
    std::deque<std::shared_ptr<VideoFrame>> m_frames;
};




StreamHistoryTracker::StreamHistoryTracker(
    size_t audio_samples_per_frame,
    size_t audio_frames_per_second,
    std::chrono::seconds window
)
    : m_window(window)
    , m_audio_samples_per_frame(audio_samples_per_frame)
    , m_audio_frames_per_second(audio_frames_per_second)
    , m_audio_samples_per_second(audio_samples_per_frame * audio_frames_per_second)
    , m_microseconds_per_sample(1. / (m_audio_samples_per_second * 1000000.))
{}

void StreamHistoryTracker::set_window(std::chrono::seconds window){
    WriteSpinLock lg(m_lock);
    m_window = window;
    clear_old();
}
void StreamHistoryTracker::on_samples(const float* samples, size_t frames){
    if (frames == 0){
        return;
    }
    WallClock now = current_time();
    WriteSpinLock lg(m_lock);
//    cout << "on_samples() = " << m_audio.size() << endl;
    m_audio.emplace_back(std::make_shared<AudioBlock>(
        now, samples, frames * m_audio_samples_per_frame
    ));
    clear_old();
}
void StreamHistoryTracker::on_frame(std::shared_ptr<VideoFrame> frame){
    //  TODO: Find a more efficient way to buffer the frames.
    //  It takes almost 10GB of memory to store 30 seconds of QVideoFrames
    //  due to them caching uncompressed bitmaps.
//    return;   //  TODO

    WriteSpinLock lg(m_lock);
//    cout << "on_frame() = " << m_frames.size() << endl;
    m_frames.emplace_back(std::move(frame));
    clear_old();
}



void StreamHistoryTracker::clear_old(){
    //  Must call under lock.
    WallClock now = current_time();
    WallClock threshold = now - m_window;

//    WriteSpinLock lg(m_lock);
//    cout << "enter" << endl;
    while (!m_audio.empty()){
//        cout << "audio.size() = " << m_audio.size() << endl;
        AudioBlock& block = *m_audio.front();

        WallClock end_block = block.timestamp;
        end_block += std::chrono::microseconds(
            static_cast<std::chrono::microseconds::rep>((double)block.samples.size() * m_microseconds_per_sample)
        );

        if (end_block < threshold){
            m_audio.pop_front();
        }else{
            break;
        }
    }
//    cout << "exit" << endl;

    while (!m_frames.empty()){
        if (m_frames.front()->timestamp < threshold){
            m_frames.pop_front();
        }else{
            break;
        }
    }
}





bool StreamHistoryTracker::save(Logger& logger, const std::string& filename) const{
    logger.log("Saving stream history...", COLOR_BLUE);

    std::deque<std::shared_ptr<AudioBlock>> audio;
    std::deque<std::shared_ptr<VideoFrame>> frames;
    {
        //  Fast copy the current state of the stream.
        WriteSpinLock lg(m_lock);
        if (m_audio.empty() && m_frames.empty()){
            return false;
        }
        audio = m_audio;
        frames = m_frames;
    }

    //  Now that the lock is released, we can take our time encoding it.

    //  TODO

#if 0
    WallClock start = WallClock::max();
    if (!audio.empty()){
        start = std::min(start, audio.front()->timestamp);
    }
    if (!frames.empty()){
        start = std::min(start, frames.front()->timestamp);
    }

#endif


//    run_on_main_thread_and_wait([&]{

    QAudioFormat audio_format;
    audio_format.setChannelCount((int)m_audio_samples_per_frame);
    audio_format.setChannelConfig(m_audio_samples_per_frame == 1 ? QAudioFormat::ChannelConfigMono : QAudioFormat::ChannelConfigStereo);
    audio_format.setSampleRate((int)m_audio_frames_per_second);
    audio_format.setSampleFormat(QAudioFormat::Float);

//    cout << "audio_format = " << audio_format.isValid() << endl;

    QAudioBufferInput audio_input;
    QVideoFrameInput video_input;

//    cout << "audio = " << audio.size() << endl;
//    cout << "frames = " << frames.size() << endl;

    QMediaCaptureSession session;
    QMediaRecorder recorder;
    session.setAudioBufferInput(&audio_input);
    session.setVideoFrameInput(&video_input);
    session.setRecorder(&recorder);
#if 1
    recorder.setMediaFormat(QMediaFormat::MPEG4);
#else
    QMediaFormat video_format;
    video_format.setAudioCodec(QMediaFormat::AudioCodec::AAC);
//    video_format.setVideoCodec(QMediaFormat::VideoCodec::H264);
    video_format.setFileFormat(QMediaFormat::MPEG4);
    recorder.setMediaFormat(video_format);
#endif
    recorder.setQuality(QMediaRecorder::NormalQuality);

    QFileInfo file(QString::fromStdString(filename));
    recorder.setOutputLocation(
        QUrl::fromLocalFile(file.absoluteFilePath())
    );

    recorder.record();

    WallClock last_change = current_time();
    QAudioBuffer audio_buffer;
    bool success = true;
    while (audio_buffer.isValid() || !frames.empty()){
#if 1
        while (true){
            if (frames.empty()){
//                video_input.sendVideoFrame(QVideoFrame());
//                session.setVideoFrameInput(nullptr);
                break;
            }
            if (!video_input.sendVideoFrame((*frames.begin())->frame)){
//                cout << "Failed Video: " << frames.size() << endl;
                break;
            }
            frames.pop_front();
            last_change = current_time();
//            cout << "Pushed Video: " << frames.size() << endl;
        }
#endif
#if 1
        while (true){
            if (!audio_buffer.isValid()){
                if (audio.empty()){
//                    audio_input.sendAudioBuffer(QAudioBuffer());
//                    session.setAudioBufferInput(nullptr);
                    break;
                }
//                cout << "constructing audio buffer: " << audio.size() << endl;
                const std::vector<float>& samples = audio.front()->samples;
                QByteArray bytes((const char*)samples.data(), samples.size() * sizeof(float));
                audio_buffer = QAudioBuffer(
                    bytes, audio_format//,
//                    std::chrono::duration_cast<std::chrono::microseconds>(audio.front()->timestamp - start).count()
                );
//                cout << "audio_buffer = " << audio_buffer.isValid() << endl;
                audio.pop_front();
            }
            if (!audio_buffer.isValid()){
                break;
            }
            if (!audio_input.sendAudioBuffer(audio_buffer)){
//                cout << "Failed Audio: " << audio.size() << endl;
//                cout << audio_input.captureSession() << endl;
                break;
            }
            audio_buffer = QAudioBuffer();
            last_change = current_time();
//            cout << "Pushed audio: " << audio.size() << endl;
        }
#endif

        if (current_time() - last_change > std::chrono::seconds(10)){
            logger.log("Failed to record stream history: No progress made after 10 seconds.", COLOR_RED);
            success = false;
            break;
        }

        QCoreApplication::processEvents();
    }

    recorder.stop();
    logger.log("Done saving stream history...", COLOR_BLUE);
//    cout << recorder.duration() << endl;


//    });
    return success;
}







}
#endif
