/*  Stream Recorder
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_StreamRecorder_H
#define PokemonAutomation_StreamRecorder_H

#include <vector>
#include <deque>
#include <QIODevice>
#include <QFile>
#include <QAudioFormat>
#include <QAudioBuffer>
#include <QThread>
#include <QMediaCaptureSession>
#include <QMediaRecorder>
#include "Common/Cpp/LifetimeSanitizer.h"
#include "Common/Cpp/Time.h"
#include "Common/Cpp/AbstractLogger.h"
#include "CommonFramework/VideoPipeline/Backends/VideoFrameQt.h"

//#include <iostream>
//using std::cout;
//using std::endl;


class QAudioBufferInput;
class QVideoFrameInput;
class QMediaCaptureSession;
class QMediaRecorder;

namespace PokemonAutomation{


struct AudioBlock{
    WallClock timestamp;
    std::vector<float> samples;

    AudioBlock(AudioBlock&&) = default;
    AudioBlock& operator=(AudioBlock&&) = default;
    AudioBlock(const AudioBlock&) = delete;
    void operator=(const AudioBlock&) = delete;

    AudioBlock()
        : timestamp(WallClock::min())
    {}
    AudioBlock(WallClock p_timestamp, const float* p_samples, size_t p_count)
        : timestamp(p_timestamp)
        , samples(p_samples, p_samples + p_count)
    {}

    void clear(){
        samples.clear();
    }
    bool is_valid() const{
        return !samples.empty();
    }
};



class WriteBuffer : public QIODevice{
public:
    ~WriteBuffer(){
        waitForBytesWritten(-1);
    }
    WriteBuffer(){
        setOpenMode(QIODeviceBase::WriteOnly);
    }

    virtual qint64 readData(char* data, qint64 maxlen){ return 0; }
    virtual qint64 writeData(const char* data, qint64 len){
        m_bytes += len;
//        cout << "total = " << m_bytes << ", current = " << len << endl;
        m_blocks.emplace_back(data, data + len);
//        cout << "data = " << (unsigned)data[0] << endl;
        return len;
    }

    bool write(Logger& logger, const std::string& filename) const{
//        cout << "write()" << endl;
        QFile file(QString::fromStdString(filename));
        if (!file.open(QFile::WriteOnly)){
            logger.log("Failed to write file: " + filename, COLOR_RED);
            return false;
        }
        for (const std::vector<char>& block : m_blocks){
            if ((size_t)file.write(block.data(), block.size()) != block.size()){
                logger.log("Failed to write file: " + filename, COLOR_RED);
                return false;
            }
        }
        file.close();
        return true;
    }

private:
    uint64_t m_bytes = 0;
    std::deque<std::vector<char>> m_blocks;
};




//
//  This one works, but it leaks a tiny bit of memory each time QMediaRecorder
//  is started and stopped.
//
class StreamRecording : public QThread{
public:
    StreamRecording(
        Logger& logger,
        std::chrono::milliseconds buffer_limit,
        WallClock start_time,
        size_t audio_samples_per_frame,
        size_t audio_frames_per_second,
        bool has_video
    );
    ~StreamRecording();

    void push_samples(WallClock timestamp, const float* data, size_t frames);
    void push_frame(std::shared_ptr<const VideoFrame> frame);

    bool stop_and_save(const std::string& filename);

private:
    std::unique_ptr<QAudioBufferInput> initialize_audio();
    std::unique_ptr<QVideoFrameInput> initialize_video();
    void internal_run();
    virtual void run() override;

private:
    Logger& m_logger;
    const std::chrono::milliseconds m_buffer_limit;
    const WallClock m_start_time;
    const size_t m_audio_samples_per_frame;
    const bool m_has_video;

    QAudioFormat m_audio_format;
    std::string m_filename;

    std::mutex m_lock;
    std::condition_variable m_cv;

    bool m_stopping = false;
    WallClock m_last_drop;

    std::deque<AudioBlock> m_buffered_audio;

    qint64 m_last_frame_time;
    std::deque<std::shared_ptr<const VideoFrame>> m_buffered_frames;

    WriteBuffer m_write_buffer;

    QAudioBufferInput* m_audio_input = nullptr;
    QVideoFrameInput* m_video_input = nullptr;
    QMediaCaptureSession* m_session = nullptr;
    QMediaRecorder* m_recorder = nullptr;

    LifetimeSanitizer m_santizer;
};


//
//  This one doesn't work. QMediaRecorder::stop() is an asynchronous operation
//  that you must execute event loop to finish it. But we cannot call event
//  loop because it leads to reentrancy issues with the destruction path.
//
class StreamRecording2{
public:
    StreamRecording2(
        Logger& logger,
        std::chrono::milliseconds buffer_limit,
        WallClock start_time,
        size_t audio_samples_per_frame,
        size_t audio_frames_per_second,
        bool has_video
    );
    ~StreamRecording2();

    void push_samples(WallClock timestamp, const float* data, size_t frames);
    void push_frame(std::shared_ptr<const VideoFrame> frame);

    void stop();
    bool stop_and_save(const std::string& filename);

private:
    void initialize_audio();
    void initialize_video();
    void process();


private:
    Logger& m_logger;
    const std::chrono::milliseconds m_buffer_limit;
    const WallClock m_start_time;
    const size_t m_audio_samples_per_frame;
    const bool m_has_video;

    QAudioFormat m_audio_format;
    std::string m_filename;

    std::mutex m_lock;
//    std::condition_variable m_cv;

    bool m_stopping = false;
    WallClock m_last_drop;

    std::deque<AudioBlock> m_buffered_audio;

    qint64 m_last_frame_time;
    std::deque<std::shared_ptr<const VideoFrame>> m_buffered_frames;

    WriteBuffer m_write_buffer;

    std::unique_ptr<QAudioBufferInput> m_audio_input;
    std::unique_ptr<QVideoFrameInput> m_video_input;
    QMediaCaptureSession m_session;
    std::unique_ptr<QMediaRecorder> m_recorder;

    AudioBlock m_current_audio;
    std::shared_ptr<const VideoFrame> m_current_frame;
    QAudioBuffer m_audio_buffer;

    LifetimeSanitizer m_santizer;
};




}
#endif
