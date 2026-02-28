/*  Stream History Tracker
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Implement by saving the last X seconds of frames. This is currently not
 *  viable because the QVideoFrames are uncompressed.
 *
 */

#include <opencv2/opencv.hpp>
// #include <QCoreApplication>
// #include <QFileInfo>
// #include <QUrl>
// #include <QAudioBufferInput>
// #include <QVideoFrameInput>
// #include <QMediaFormat>
// #include <QMediaRecorder>
// #include <QMediaCaptureSession>
// #include <QScopeGuard>
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Logging/AbstractLogger.h"
#include "Common/Cpp/Concurrency/SpinLock.h"
#include "CommonFramework/VideoPipeline/Backends/VideoFrameQt.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/Recording/StreamHistoryOption.h"
#include "CommonFramework/Tools/GlobalThreadPools.h"
#include "StreamHistoryTracker_SaveFrames.h"

#include <iostream>
using std::cout;
using std::endl;


namespace PokemonAutomation{



void simulate_cpu_load(int milliseconds) {
    auto start = std::chrono::steady_clock::now();
    while (std::chrono::steady_clock::now() - start < std::chrono::milliseconds(milliseconds)) {
        // Waste cycles with dummy math to prevent compiler optimization
        double d = 1.0;
        d = std::sqrt(d * 1.1);
    }
}

QImage decompress_video_frame(const std::vector<uchar> &compressed_buffer) {
    if (compressed_buffer.empty()) return {};

    // 1. Decompress JPEG buffer into a QImage
    // fromData handles the JPEG header and decompression automatically
    QImage img = QImage::fromData(compressed_buffer.data(), 
                                  static_cast<int>(compressed_buffer.size()), 
                                  "JPG");

    if (img.isNull()) return {};

    return img.convertToFormat(QImage::Format_BGR888);
}

std::vector<uchar> compress_video_frame(const QVideoFrame& const_frame) {
    // simulate_cpu_load(100);  // for testing, to see what happens when the CPU is overwhelmed, and needs to drop frames.


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

    int target_width;
    const StreamHistoryOption& settings = GlobalSettings::instance().STREAM_HISTORY;
    switch (settings.RESOLUTION){
    case StreamHistoryOption::Resolution::MATCH_INPUT:
        target_width = img.width();
        break;
    case StreamHistoryOption::Resolution::FORCE_720p:
        target_width = 1280;
        break;
    case StreamHistoryOption::Resolution::FORCE_1080p:
        target_width = 1920;
        break;
    default:
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Resolution: Unknown enum.");                
    }

    // scale to target resolution
    int target_height = img.height() * target_width / img.width();
    img = img.scaled(target_width, target_height, Qt::KeepAspectRatio, Qt::SmoothTransformation);


    // 3. Wrap QImage memory into a cv::Mat (No-copy)
    // Note: OpenCV expects BGR by default, but QImage is RGB. 
    // we use cv::cvtColor later to fix this
    cv::Mat mat(img.height(), img.width(), CV_8UC3, 
                const_cast<unsigned char*>(img.bits()), img.bytesPerLine());

    // 4. Compress using imencode
    std::vector<uchar> compressed_buffer;
    std::vector<int> params = {cv::IMWRITE_JPEG_QUALITY, settings.JPEG_QUALITY}; // 0-100
    
    // Convert RGB to BGR before encoding because imencode expects BGR
    cv::Mat bgr_Mat;
    cv::cvtColor(mat, bgr_Mat, cv::COLOR_RGB2BGR);
    
    cv::imencode(".jpg", bgr_Mat, compressed_buffer, params);

    return compressed_buffer; // Store this in the circular buffer
}

size_t get_target_fps(){
    const StreamHistoryOption& settings = GlobalSettings::instance().STREAM_HISTORY;
    size_t target_fps;
    switch (settings.VIDEO_FPS){
    case StreamHistoryOption::VideoFPS::FPS_30:
        target_fps = 30;
        break;
    case StreamHistoryOption::VideoFPS::FPS_15:
        target_fps = 15;
        break;
    case StreamHistoryOption::VideoFPS::FPS_10:
        target_fps = 10;
        break;
    case StreamHistoryOption::VideoFPS::FPS_05:
        target_fps = 5;
        break;
    case StreamHistoryOption::VideoFPS::FPS_01:
        target_fps = 1;
        break;
    default:
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "VideoFPS: Unknown enum.");                
    }

    return target_fps;
}


StreamHistoryTracker::~StreamHistoryTracker(){
    m_stopping = true;
    m_cv.notify_all();
    m_worker.wait_and_ignore_exceptions();
}

StreamHistoryTracker::StreamHistoryTracker(
    Logger& logger,
    std::chrono::seconds window,
    size_t audio_samples_per_frame,
    size_t audio_frames_per_second,
    bool has_video
)
    : m_logger(logger)
    , m_window(window)
    // , m_audio_samples_per_frame(audio_samples_per_frame)
    // , m_audio_frames_per_second(audio_frames_per_second)
    // , m_audio_samples_per_second(audio_samples_per_frame * audio_frames_per_second)
    // , m_microseconds_per_sample(1. / (m_audio_samples_per_second * 1000000.))
    // , m_has_video(has_video)
    , m_target_fps(get_target_fps())
    , m_frame_interval(1000000 / m_target_fps)
    , m_next_frame_time(WallClock::min())
{
    m_worker = GlobalThreadPools::unlimited_normal().dispatch_now_blocking(
        [this]{ worker_loop(); }
    );
}

void StreamHistoryTracker::set_window(std::chrono::seconds window){
    WriteSpinLock lg(m_lock, PA_CURRENT_FUNCTION);
    m_window = window;
    clear_old();
}
void StreamHistoryTracker::on_samples(const float* samples, size_t frames){
    #if 0
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
    #endif
}




void StreamHistoryTracker::on_frame(std::shared_ptr<const VideoFrame> frame){
    {
        WriteSpinLock lg(m_lock, PA_CURRENT_FUNCTION);
    //    cout << "on_frame() = " << m_frames.size() << endl;

        // Initialize on first frame
        if (m_next_frame_time == WallClock::min()){
            m_next_frame_time = frame->timestamp;
        }

        // don't save every frame. only save frames as per m_target_fps
        // Only save when we've crossed the next sampling boundary
        if (frame->timestamp < m_next_frame_time){
            return; // skip frame
        }

        // Advance by fixed intervals
        // Next frame time is anchored relative to the first frame's time, with increments by a multiple of m_frame_interval, 
        // instead of being relative to the current frame's time. This prevents timing drift.

        // If there is a massive jump in time (e.g. the stream pauses for 5 seconds), 
        // the while loop advances the schedule multiple times until it is once again ahead of the 
        // current timestamp. If this happens, there will be a matching gap in the saved frames. 
        // We handle this gap by duplicating frames in the save() function, so that we maintain a constant frame rate.
        while (m_next_frame_time <= frame->timestamp){
            m_next_frame_time += std::chrono::microseconds(m_frame_interval);
        }
    } // Release SpinLock before hitting the queue mutex


    // auto compressed_frame = compress_video_frame(frame->frame);
    // m_compressed_frames.emplace_back(CompressedVideoFrame{frame->timestamp, std::move(compressed_frame)});
    // // m_frames.emplace_back(std::move(frame));
    // clear_old();

    {
        std::lock_guard<Mutex> lock(m_queue_lock);

        // Drop oldest if we are falling behind
        if (m_pending_frames.size() >= MAX_PENDING_FRAMES) {
            m_pending_frames.pop_front(); 
            m_logger.log("Worker thread lagging: Frame dropped.", COLOR_RED);
        }
        m_pending_frames.emplace_back(std::move(frame));

    }
    m_cv.notify_one();

}



void StreamHistoryTracker::clear_old(){
    //  Must call under lock.
    WallClock latest_frame = m_compressed_frames.back().timestamp;
    WallClock threshold = latest_frame - m_window;

    #if 0
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
    #endif
//    cout << "exit" << endl;

    while (!m_compressed_frames.empty()){
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

    m_logger.log("Total frames to save: " + std::to_string(frames.size()));

    if (frames.empty()) return false;

    // Use first frame to get size
    QImage first_img = decompress_video_frame(frames.front().compressed_frame);
    int width = first_img.width();
    int height = first_img.height();

    m_logger.log("Frame size: " + std::to_string(width) + " x " + std::to_string(height));

    // 1. Initialize VideoWriter (e.g., MP4 with 30 FPS)
    cv::VideoWriter writer(filename, cv::VideoWriter::fourcc('m', 'p', '4', 'v'), 
                           m_target_fps, cv::Size(width, height), true);

    if (!writer.isOpened()) {
        throw std::runtime_error("Could not open video file for writing.");
    }

    std::vector<unsigned char> last_good_buffer = frames[0].compressed_frame;

    size_t frame_index = 0;
    size_t frames_inserted = 0;
    WallClock start_time = frames[0].timestamp;

    // 2. Loop through frames
    for (CompressedVideoFrame frame : frames) {
        if (frame_index % 100 == 0) {
            m_logger.log("Saving frame " + std::to_string(frame_index) + " / " + std::to_string(frames.size()));
        }

        // Insert duplicate frames if there is a gap due to dropping frames.
        // Because VideoWriter can only handle a fixed frame rate.

        // calculates the frame index that this timestamp SHOULD be at
        double elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(frame.timestamp - start_time).count();
        double interval = std::chrono::duration_cast<std::chrono::milliseconds>(m_frame_interval).count();
        size_t target_frame_index = (size_t)std::round(elapsed/interval);
        // fill the gap with duplicate frames until we reach the target index
        while (frames_inserted < target_frame_index) {
            // Decompress last known good frame and write again
            QImage img = decompress_video_frame(last_good_buffer);
            cv::Mat mat(height, width, CV_8UC3, (void*)img.bits(), img.bytesPerLine());
            writer.write(mat);
            frames_inserted++;
        }

        // 3. decompress frame and write to video
        QImage img = decompress_video_frame(frame.compressed_frame);
        cv::Mat mat(height, width, CV_8UC3, (void*)img.bits(), img.bytesPerLine());
        writer.write(mat);

        last_good_buffer = frame.compressed_frame;
        frames_inserted++;
        frame_index++;
    }
    // Writer automatically releases when going out of scope

    m_logger.log("Done saving stream history...", COLOR_BLUE);
    return true;
}


void StreamHistoryTracker::worker_loop(){
    while (!m_stopping) {
        std::shared_ptr<const VideoFrame> frame;

        // 1. Wait for a frame to process
        {
            std::unique_lock<Mutex> lock(m_queue_lock);
            m_cv.wait(lock, [this] { return !m_pending_frames.empty() || m_stopping; });
            
            if (m_stopping && m_pending_frames.empty()) return;

            frame = std::move(m_pending_frames.front());
            m_pending_frames.pop_front();
        }

        // 2. Perform the expensive compression (Outside the lock)
        auto compressed_data = compress_video_frame(frame->frame);

        // 3. Move the result into the main storage
        {
            WriteSpinLock lg(m_lock, PA_CURRENT_FUNCTION);
            m_compressed_frames.emplace_back(CompressedVideoFrame{
                frame->timestamp, 
                std::move(compressed_data)
            });
            clear_old(); // Cleanup happens here
        }
    }
}



}

// #include "StreamHistoryTracker_SaveFrames.moc" 
