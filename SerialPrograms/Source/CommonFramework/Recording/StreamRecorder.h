/*  Stream Recorder
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_StreamRecorder_H
#define PokemonAutomation_StreamRecorder_H

#include <vector>
#include <deque>
#include <QIODevice>
#include <QFile>
#include <QAudioFormat>
#include <QThread>
#include "Common/Cpp/Time.h"
#include "Common/Cpp/AbstractLogger.h"

//  REMOVE
#include <iostream>
using std::cout;
using std::endl;


class QAudioBufferInput;
class QVideoFrameInput;

namespace PokemonAutomation{

class VideoFrame;


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



class StreamRecording : public QThread{
public:
    StreamRecording(
        Logger& logger,
        std::chrono::milliseconds buffer_limit,
        size_t audio_samples_per_frame,
        size_t audio_frames_per_second,
        WallClock start_time
    );
    ~StreamRecording();

    void push_samples(WallClock timestamp, const float* data, size_t frames);
    void push_frame(std::shared_ptr<VideoFrame> frame);

    bool stop_and_save(const std::string& filename);

private:
    void internal_run();
    virtual void run() override;

private:
    Logger& m_logger;
    const std::chrono::milliseconds m_buffer_limit;
    const size_t m_audio_samples_per_frame;
    QAudioFormat m_audio_format;
    const WallClock m_start_time;
    std::string m_filename;

    std::mutex m_lock;
    std::condition_variable m_cv;

    enum class State{
        STARTING,
        ACTIVE,
        STOPPING,
    };

    State m_state;
    WallClock m_last_drop;
    std::deque<std::shared_ptr<AudioBlock>> m_buffered_audio;
    std::deque<std::shared_ptr<VideoFrame>> m_buffered_frames;
    QAudioBufferInput* m_audio_input;
    QVideoFrameInput* m_video_input;
    WriteBuffer m_write_buffer;
};



}
#endif
