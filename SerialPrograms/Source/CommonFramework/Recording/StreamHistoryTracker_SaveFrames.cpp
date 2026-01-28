/*  Stream History Tracker
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Implement by saving the last X seconds of frames. This is currently not
 *  viable because the QVideoFrames are uncompressed.
 *
 */

#include <opencv2/opencv.hpp>
// #include <deque>
#include <QCoreApplication>
#include <QFileInfo>
#include <QUrl>
#include <QAudioBufferInput>
#include <QVideoFrameInput>
#include <QMediaFormat>
#include <QMediaRecorder>
#include <QMediaCaptureSession>
#include <QScopeGuard>
#include "Common/Cpp/Logging/AbstractLogger.h"
#include "Common/Cpp/Concurrency/SpinLock.h"
#include "CommonFramework/VideoPipeline/Backends/VideoFrameQt.h"
#include "StreamHistoryTracker_SaveFrames.h"

#include <iostream>
using std::cout;
using std::endl;


namespace PokemonAutomation{



QVideoFrame decompress_video_frame(const std::vector<uchar> &compressed_buffer) {
    if (compressed_buffer.empty()) return {};

    // 1. Decompress JPEG buffer into a QImage
    // fromData handles the JPEG header and decompression automatically
    QImage img = QImage::fromData(compressed_buffer.data(), 
                                  static_cast<int>(compressed_buffer.size()), 
                                  "JPG");

    if (img.isNull()) return {};

    // 2. Use the new Qt 6.8 constructor
    // This wraps the QImage into a QVideoFrame efficiently.
    // If the format is compatible (like RGB888), it minimizes copies.
    return QVideoFrame(img);
}

std::vector<uchar> compress_video_frame(const QVideoFrame& const_frame) {
    // Create a local non-const copy (cheap, uses explicit sharing)
    QVideoFrame frame = const_frame;

    // 1. Map the frame to CPU memory
    if (!frame.map(QVideoFrame::ReadOnly)) {
        return {};
    }

    // Ensure unmap() is called when this function exits (success or failure)
    auto guard = qScopeGuard([&frame] { frame.unmap(); });

    // 2. Convert to QImage (Qt 6.8+ handles internal conversions efficiently)
    // For circular buffers, using a 3-channel RGB888 is common for OpenCV
    QImage img = frame.toImage().convertToFormat(QImage::Format_RGB888);

    // 3. Wrap QImage memory into a cv::Mat (No-copy)
    // Note: OpenCV expects BGR by default, but QImage is RGB. 
    // If color accuracy matters, use cv::cvtColor later or img.rgbSwapped().
    cv::Mat mat(img.height(), img.width(), CV_8UC3, 
                const_cast<unsigned char*>(img.bits()), img.bytesPerLine());

    // 4. Compress using imencode
    std::vector<uchar> compressed_buffer;
    std::vector<int> params = {cv::IMWRITE_JPEG_QUALITY, 80}; // 0-100
    
    // Convert RGB to BGR before encoding because imencode expects BGR
    cv::Mat bgr_Mat;
    cv::cvtColor(mat, bgr_Mat, cv::COLOR_RGB2BGR);
    
    cv::imencode(".jpg", bgr_Mat, compressed_buffer, params);

    return compressed_buffer; // Store this in your circular buffer
}


#if 0
class VideoGenerator : public QObject {
    // Q_OBJECT
public:
    VideoGenerator(QVideoFrameInput *input, std::deque<CompressedVideoFrame> frames)
        : m_input(input), m_frames(frames){
        
        // Listen for the signal that the recorder's buffer has space
        connect(m_input, &QVideoFrameInput::readyToSendVideoFrame, this, &VideoGenerator::sendNextFrame);
    }

public slots:
    void sendNextFrame() {
        // Send frames in a loop until the buffer is full or we run out of images
        if (!m_frames.empty())
        {
            QVideoFrame frame = decompress_video_frame(m_frames.front().compressed_frame); //  m_frameQueue.dequeue();

            // Set timestamp (e.g., 30 FPS = 33333 microseconds per frame)
            // frame.setStartTime(m_index * 33333);

            // 2. ONLY send the frame if the input is ready
            bool success = m_input->sendVideoFrame(frame);

            // 3. Optional: Check if the frame was dropped (shouldn't happen
            //    often if you are responding to readyToSendVideoFrame)
            if (!success) {
                cout << "Frame was rejected, stopping frame emission." << endl;;
                // You might re-enqueue the frame or just discard it based on your needs.
            }
        }
        else {
            cout << "Queue empty, waiting for more frames...";
            // Finalize: Sending an empty frame signals the end of the stream
            m_input->sendVideoFrame(QVideoFrame());
            emit finished();
        }
        
    }

signals:
    void finished();

private:
    QVideoFrameInput *m_input;
    std::deque<CompressedVideoFrame> m_frames;
};
#endif

StreamHistoryTracker::StreamHistoryTracker(
    Logger& logger,
    std::chrono::seconds window,
    size_t audio_samples_per_frame,
    size_t audio_frames_per_second,
    bool has_video
)
    : m_logger(logger)
    , m_window(window)
    , m_audio_samples_per_frame(audio_samples_per_frame)
    , m_audio_frames_per_second(audio_frames_per_second)
    , m_audio_samples_per_second(audio_samples_per_frame * audio_frames_per_second)
    , m_microseconds_per_sample(1. / (m_audio_samples_per_second * 1000000.))
    , m_has_video(has_video)
{}

void StreamHistoryTracker::set_window(std::chrono::seconds window){
    WriteSpinLock lg(m_lock, PA_CURRENT_FUNCTION);
    m_window = window;
    clear_old();
}
void StreamHistoryTracker::on_samples(const float* samples, size_t frames){
    if (frames == 0){
        return;
    }
    WallClock now = current_time();
    WriteSpinLock lg(m_lock, PA_CURRENT_FUNCTION);
//    cout << "on_samples() = " << m_audio.size() << endl;
    m_audio.emplace_back(std::make_shared<AudioBlock>(
        now, samples, frames * m_audio_samples_per_frame
    ));
    clear_old();
}




void StreamHistoryTracker::on_frame(std::shared_ptr<const VideoFrame> frame){
    //  TODO: Find a more efficient way to buffer the frames.
    //  It takes almost 10GB of memory to store 30 seconds of QVideoFrames
    //  due to them caching uncompressed bitmaps.
//    return;   //  TODO

    WriteSpinLock lg(m_lock, PA_CURRENT_FUNCTION);
//    cout << "on_frame() = " << m_frames.size() << endl;
    auto compressed_frame = compress_video_frame(frame->frame);
    m_compressed_frames.emplace_back(CompressedVideoFrame{frame->timestamp, compressed_frame});
    // m_frames.emplace_back(std::move(frame));
    clear_old();
}



void StreamHistoryTracker::clear_old(){
    //  Must call under lock.
    WallClock now = current_time();
    WallClock threshold = now - m_window;

//    WriteSpinLock lg(m_lock, PA_CURRENT_FUNCTION);
//    cout << "enter" << endl;
    while (!m_audio.empty()){
//        cout << "audio.size() = " << m_audio.size() << endl;
        AudioBlock& block = *m_audio.front();

        WallClock end_block = block.timestamp;
        end_block += std::chrono::microseconds(
            static_cast<std::chrono::microseconds::rep>((double)block.samples.size() * m_microseconds_per_sample)
        );

        if (end_block < threshold){  // todo: confirm if the audio deque clears properly
            m_audio.pop_front();
        }else{
            break;
        }
    }
//    cout << "exit" << endl;

    while (!m_compressed_frames.empty()){
        // if (m_frames.front()->timestamp < threshold){
        //     m_frames.pop_front();
        // }else{
        //     break;
        // }

        if (m_compressed_frames.front().timestamp < threshold){
            m_compressed_frames.pop_front();
        }else{
            break;
        }
    }
}


bool StreamHistoryTracker::save(const std::string& filename) const{
    m_logger.log("Saving stream history...", COLOR_BLUE);

    std::deque<CompressedVideoFrame> frames;
    {
        //  Fast copy the current state of the stream.
        WriteSpinLock lg(m_lock, PA_CURRENT_FUNCTION);
        if (m_compressed_frames.empty()){
            return false;
        }
        frames = m_compressed_frames;
    }

    int width = 1920;
    int height = 1080;

    // 1. Initialize VideoWriter (e.g., MP4 with 30 FPS)
    cv::VideoWriter writer(filename, cv::VideoWriter::fourcc('m', 'p', '4', 'v'), 
                           30.0, cv::Size(width, height), true);

    if (!writer.isOpened()) {
        throw std::runtime_error("Could not open video file for writing.");
    }

    // 2. Loop through your memory pointers
    for (CompressedVideoFrame frame : frames) {
        QVideoFrame video_frame = decompress_video_frame(frame.compressed_frame);
        QImage img = video_frame.toImage().convertToFormat(QImage::Format_BGR888);
         
        cv::Mat mat(height, width, CV_8UC3, (void*)img.bits(), img.bytesPerLine());
        
        // 3. Write to video (Encoding happens here)
        writer.write(mat);
    }
    // Writer automatically releases when going out of scope

    m_logger.log("Done saving stream history...", COLOR_BLUE);
    return true;
}


// bool StreamHistoryTracker::save(const std::string& filename) const{
//     m_logger.log("Saving stream history...", COLOR_BLUE);

//     std::deque<CompressedVideoFrame> frames;
//     {
//         //  Fast copy the current state of the stream.
//         WriteSpinLock lg(m_lock, PA_CURRENT_FUNCTION);
//         if (m_compressed_frames.empty()){
//             return false;
//         }
//         frames = m_compressed_frames;
//     }

//     //  Now that the lock is released, we can take our time encoding it.

//     //  TODO

// #if 0
//     WallClock start = WallClock::max();
//     if (!frames.empty()){
//         start = std::min(start, frames.front()->timestamp);
//     }

// #endif


// //    run_on_main_thread_and_wait([&]{

//     QVideoFrameFormat format(QSize(1920, 1080), QVideoFrameFormat::Format_ARGB8888);
//     QVideoFrameInput videoInput(format);

// //    cout << "frames = " << frames.size() << endl;

//     QMediaCaptureSession session;
//     QMediaRecorder recorder;
//     session.setVideoFrameInput(&videoInput);
//     session.setRecorder(&recorder);
// #if 1
//     recorder.setMediaFormat(QMediaFormat::MPEG4);
// #else
//     QMediaFormat video_format;
//     video_format.setAudioCodec(QMediaFormat::AudioCodec::AAC);
// //    video_format.setVideoCodec(QMediaFormat::VideoCodec::H264);
//     video_format.setFileFormat(QMediaFormat::MPEG4);
//     recorder.setMediaFormat(video_format);
// #endif
//     recorder.setQuality(QMediaRecorder::NormalQuality);

//     QFileInfo file(QString::fromStdString(filename));
//     recorder.setOutputLocation(
//         QUrl::fromLocalFile(file.absoluteFilePath())
//     );

//     VideoGenerator generator(&videoInput, frames);

//     QObject::connect(&generator, &VideoGenerator::finished, &recorder, &QMediaRecorder::stop);
//     QObject::connect(&recorder, &QMediaRecorder::recorderStateChanged, [](QMediaRecorder::RecorderState state){
//         if (state == QMediaRecorder::StoppedState) qApp->quit();
//     });

//     recorder.record();
// #if 0
//     WallClock last_change = current_time();
//     bool success = true;

//     while (!frames.empty()){
// #if 1
//         while (true){
//             if (frames.empty()){
// //                video_input.sendVideoFrame(QVideoFrame());
// //                session.setVideoFrameInput(nullptr);
//                 break;
//             }
//             if (!video_input.sendVideoFrame((*frames.begin())->frame)){
// //                cout << "Failed Video: " << frames.size() << endl;
//                 break;
//             }
//             frames.pop_front();
//             last_change = current_time();
// //            cout << "Pushed Video: " << frames.size() << endl;
//         }
// #endif
//         if (current_time() - last_change > std::chrono::seconds(10)){
//             m_logger.log("Failed to record stream history: No progress made after 10 seconds.", COLOR_RED);
//             success = false;
//             break;
//         }

//         QCoreApplication::processEvents();
//     }
// #endif

//     recorder.stop();
//     m_logger.log("Done saving stream history...", COLOR_BLUE);
// //    cout << recorder.duration() << endl;


// //    });
//     return true;
// }


// #if 0
// bool StreamHistoryTracker::save(const std::string& filename) const{
//     m_logger.log("Saving stream history...", COLOR_BLUE);

//     std::deque<std::shared_ptr<AudioBlock>> audio;
//     std::deque<std::shared_ptr<const VideoFrame>> frames;
//     {
//         //  Fast copy the current state of the stream.
//         WriteSpinLock lg(m_lock, PA_CURRENT_FUNCTION);
//         if (m_audio.empty() && m_frames.empty()){
//             return false;
//         }
//         audio = m_audio;
//         frames = m_frames;
//     }

//     //  Now that the lock is released, we can take our time encoding it.

//     //  TODO

// #if 0
//     WallClock start = WallClock::max();
//     if (!audio.empty()){
//         start = std::min(start, audio.front()->timestamp);
//     }
//     if (!frames.empty()){
//         start = std::min(start, frames.front()->timestamp);
//     }

// #endif


// //    run_on_main_thread_and_wait([&]{

//     QAudioFormat audio_format;
//     audio_format.setChannelCount((int)m_audio_samples_per_frame);
//     audio_format.setChannelConfig(m_audio_samples_per_frame == 1 ? QAudioFormat::ChannelConfigMono : QAudioFormat::ChannelConfigStereo);
//     audio_format.setSampleRate((int)m_audio_frames_per_second);
//     audio_format.setSampleFormat(QAudioFormat::Float);

// //    cout << "audio_format = " << audio_format.isValid() << endl;

//     QAudioBufferInput audio_input;
//     QVideoFrameInput video_input;

// //    cout << "audio = " << audio.size() << endl;
// //    cout << "frames = " << frames.size() << endl;

//     QMediaCaptureSession session;
//     QMediaRecorder recorder;
//     session.setAudioBufferInput(&audio_input);
//     session.setVideoFrameInput(&video_input);
//     session.setRecorder(&recorder);
// #if 1
//     recorder.setMediaFormat(QMediaFormat::MPEG4);
// #else
//     QMediaFormat video_format;
//     video_format.setAudioCodec(QMediaFormat::AudioCodec::AAC);
// //    video_format.setVideoCodec(QMediaFormat::VideoCodec::H264);
//     video_format.setFileFormat(QMediaFormat::MPEG4);
//     recorder.setMediaFormat(video_format);
// #endif
//     recorder.setQuality(QMediaRecorder::NormalQuality);

//     QFileInfo file(QString::fromStdString(filename));
//     recorder.setOutputLocation(
//         QUrl::fromLocalFile(file.absoluteFilePath())
//     );

//     recorder.record();

//     WallClock last_change = current_time();
//     QAudioBuffer audio_buffer;
//     bool success = true;
//     while (audio_buffer.isValid() || !frames.empty()){
// #if 1
//         while (true){
//             if (frames.empty()){
// //                video_input.sendVideoFrame(QVideoFrame());
// //                session.setVideoFrameInput(nullptr);
//                 break;
//             }
//             if (!video_input.sendVideoFrame((*frames.begin())->frame)){
// //                cout << "Failed Video: " << frames.size() << endl;
//                 break;
//             }
//             frames.pop_front();
//             last_change = current_time();
// //            cout << "Pushed Video: " << frames.size() << endl;
//         }
// #endif
// #if 1
//         while (true){
//             if (!audio_buffer.isValid()){
//                 if (audio.empty()){
// //                    audio_input.sendAudioBuffer(QAudioBuffer());
// //                    session.setAudioBufferInput(nullptr);
//                     break;
//                 }
// //                cout << "constructing audio buffer: " << audio.size() << endl;
//                 const std::vector<float>& samples = audio.front()->samples;
//                 QByteArray bytes((const char*)samples.data(), samples.size() * sizeof(float));
//                 audio_buffer = QAudioBuffer(
//                     bytes, audio_format//,
// //                    std::chrono::duration_cast<std::chrono::microseconds>(audio.front()->timestamp - start).count()
//                 );
// //                cout << "audio_buffer = " << audio_buffer.isValid() << endl;
//                 audio.pop_front();
//             }
//             if (!audio_buffer.isValid()){
//                 break;
//             }
//             if (!audio_input.sendAudioBuffer(audio_buffer)){
// //                cout << "Failed Audio: " << audio.size() << endl;
// //                cout << audio_input.captureSession() << endl;
//                 break;
//             }
//             audio_buffer = QAudioBuffer();
//             last_change = current_time();
// //            cout << "Pushed audio: " << audio.size() << endl;
//         }
// #endif

//         if (current_time() - last_change > std::chrono::seconds(10)){
//             m_logger.log("Failed to record stream history: No progress made after 10 seconds.", COLOR_RED);
//             success = false;
//             break;
//         }

//         QCoreApplication::processEvents();
//     }

//     recorder.stop();
//     m_logger.log("Done saving stream history...", COLOR_BLUE);
// //    cout << recorder.duration() << endl;


// //    });
//     return success;
// }
// #endif






}

// #include "StreamHistoryTracker_SaveFrames.moc" 