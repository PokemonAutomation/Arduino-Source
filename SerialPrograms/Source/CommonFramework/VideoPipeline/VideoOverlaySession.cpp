/*  Video Overlay Session
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "VideoOverlaySession.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{



void VideoOverlaySession::add_listener(ContentListener& listener){
    m_listeners.add(listener);
}
void VideoOverlaySession::remove_listener(ContentListener& listener){
    m_listeners.remove(listener);
}


VideoOverlaySession::~VideoOverlaySession(){
    {
        std::lock_guard<std::mutex> lg(m_stats_lock);
        m_stopping = true;
    }
    m_stats_cv.notify_all();
    m_stats_updater.join();
}
VideoOverlaySession::VideoOverlaySession(Logger& logger, VideoOverlayOption& option)
    : m_logger(logger)
    , m_option(option)
    , m_stats_updater(&VideoOverlaySession::stats_thread, this)
{}


void VideoOverlaySession::get(VideoOverlayOption& option){
    bool boxes = m_option.boxes.load(std::memory_order_relaxed);
    bool text = m_option.text.load(std::memory_order_relaxed);
    bool images = m_option.images.load(std::memory_order_relaxed);
    bool log = m_option.log.load(std::memory_order_relaxed);
    bool stats = m_option.stats.load(std::memory_order_relaxed);
    option.boxes.store(boxes, std::memory_order_relaxed);
    option.text.store(text, std::memory_order_relaxed);
    option.images.store(images, std::memory_order_relaxed);
    option.log.store(log, std::memory_order_relaxed);
    option.stats.store(stats, std::memory_order_relaxed);
}
void VideoOverlaySession::set(const VideoOverlayOption& option){
    bool boxes = option.boxes.load(std::memory_order_relaxed);
    bool text = option.text.load(std::memory_order_relaxed);
    bool images = option.images.load(std::memory_order_relaxed);
    bool log = option.log.load(std::memory_order_relaxed);
    bool stats = option.stats.load(std::memory_order_relaxed);
    m_option.boxes.store(boxes, std::memory_order_relaxed);
    m_option.text.store(text, std::memory_order_relaxed);
    m_option.images.store(images, std::memory_order_relaxed);
    m_option.log.store(log, std::memory_order_relaxed);
    m_option.stats.store(stats, std::memory_order_relaxed);
    m_listeners.run_method_unique(&ContentListener::on_overlay_enabled_boxes, boxes);
    m_listeners.run_method_unique(&ContentListener::on_overlay_enabled_text, text);
    m_listeners.run_method_unique(&ContentListener::on_overlay_enabled_images, images);
    m_listeners.run_method_unique(&ContentListener::on_overlay_enabled_log, log);
    m_listeners.run_method_unique(&ContentListener::on_overlay_enabled_stats, stats);
}


void VideoOverlaySession::stats_thread(){
    std::unique_lock<std::mutex> lg(m_stats_lock);
    while (!m_stopping){
        {
            std::vector<OverlayStatSnapshot> lines;
            WriteSpinLock lg0(m_lock);
            for (const auto& stat : m_stats_order){
                OverlayStatSnapshot snapshot = stat->get_current();
                if (!snapshot.text.empty()){
                    lines.emplace_back(std::move(snapshot));
                }
            }
            m_stat_lines = std::move(lines);
        }
        m_stats_cv.wait_for(lg, std::chrono::milliseconds(100));
    }
}


void VideoOverlaySession::set_enabled_boxes(bool enabled){
    m_option.boxes.store(enabled, std::memory_order_relaxed);
    m_listeners.run_method_unique(&ContentListener::on_overlay_enabled_boxes, enabled);
}
void VideoOverlaySession::set_enabled_text(bool enabled){
    m_option.text.store(enabled, std::memory_order_relaxed);
    m_listeners.run_method_unique(&ContentListener::on_overlay_enabled_text, enabled);
}
void VideoOverlaySession::set_enabled_images(bool enabled){
    m_option.images.store(enabled, std::memory_order_relaxed);
    m_listeners.run_method_unique(&ContentListener::on_overlay_enabled_images, enabled);
}
void VideoOverlaySession::set_enabled_log(bool enabled){
    m_option.log.store(enabled, std::memory_order_relaxed);
    m_listeners.run_method_unique(&ContentListener::on_overlay_enabled_log, enabled);
}
void VideoOverlaySession::set_enabled_stats(bool enabled){
    m_option.stats.store(enabled, std::memory_order_relaxed);
    m_listeners.run_method_unique(&ContentListener::on_overlay_enabled_stats, enabled);
}


//
//  Boxes
//

void VideoOverlaySession::add_box(const OverlayBox& box){
    std::shared_ptr<std::vector<OverlayBox>> ptr = std::make_shared<std::vector<OverlayBox>>();
    {
        WriteSpinLock lg(m_lock, "VideoOverlaySession::add_box()");
        m_boxes.insert(&box);

        //  We create a newly allocated Box vector to avoid listener accessing
        //  `m_boxes` asynchronously.
        for (const auto& item : m_boxes){
            ptr->emplace_back(*item);
        }
    }
    m_listeners.run_method_unique(&ContentListener::on_overlay_update_boxes, ptr);
}
void VideoOverlaySession::remove_box(const OverlayBox& box){
    std::shared_ptr<std::vector<OverlayBox>> ptr = std::make_shared<std::vector<OverlayBox>>();
    {
        WriteSpinLock lg(m_lock, "VideoOverlaySession::remove_box()");
        m_boxes.erase(&box);

        //  We create a newly allocated Box vector to avoid listener accessing
        //  `m_boxes` asynchronously.
        for (const auto& item : m_boxes){
            ptr->emplace_back(*item);
        }
    }
    m_listeners.run_method_unique(&ContentListener::on_overlay_update_boxes, ptr);
}
std::vector<OverlayBox> VideoOverlaySession::boxes() const{
    ReadSpinLock lg(m_lock);
    std::vector<OverlayBox> ret;
    for (const auto& item : m_boxes){
        ret.emplace_back(*item);
    }
    return ret;
}


//
//  Texts
//

void VideoOverlaySession::add_text(const OverlayText& text){
    std::shared_ptr<std::vector<OverlayText>> ptr = std::make_shared<std::vector<OverlayText>>();
    {
        WriteSpinLock lg(m_lock, "VideoOverlaySession::add_text()");
        m_texts.insert(&text);

        //  We create a newly allocated Box vector to avoid listener accessing
        //  `m_texts` asynchronously.
        for (const auto& item : m_texts){
            ptr->emplace_back(*item);
        }
    }
    m_listeners.run_method_unique(&ContentListener::on_overlay_update_text, ptr);
}
void VideoOverlaySession::remove_text(const OverlayText& text){
    std::shared_ptr<std::vector<OverlayText>> ptr = std::make_shared<std::vector<OverlayText>>();
    {
        WriteSpinLock lg(m_lock, "VideoOverlaySession::remove_text()");
        m_texts.erase(&text);

        //  We create a newly allocated Box vector to avoid listener accessing
        //  `m_texts` asynchronously.
        for (const auto& item : m_texts){
            ptr->emplace_back(*item);
        }
    }
    m_listeners.run_method_unique(&ContentListener::on_overlay_update_text, ptr);
}
std::vector<OverlayText> VideoOverlaySession::texts() const{
    ReadSpinLock lg(m_lock);
    std::vector<OverlayText> ret;
    for (const auto& item : m_texts){
        ret.emplace_back(*item);
    }
    return ret;
}


//
//  Images
//

void VideoOverlaySession::add_image(const OverlayImage& image){
    std::shared_ptr<std::vector<OverlayImage>> ptr = std::make_shared<std::vector<OverlayImage>>();
    {
        WriteSpinLock lg(m_lock, "VideoOverlaySession::add_image()");
        m_images.insert(&image);

        //  We create a newly allocated Box vector to avoid listener accessing
        //  `m_images` asynchronously.
        for (const auto& item : m_images){
            ptr->emplace_back(*item);
        }
    }
    m_listeners.run_method_unique(&ContentListener::on_overlay_update_images, ptr);
}
void VideoOverlaySession::remove_image(const OverlayImage& image){
    std::shared_ptr<std::vector<OverlayImage>> ptr = std::make_shared<std::vector<OverlayImage>>();
    {
        WriteSpinLock lg(m_lock, "VideoOverlaySession::remove_image()");
        m_images.erase(&image);

        //  We create a newly allocated Box vector to avoid listener accessing
        //  `m_images` asynchronously.
        for (const auto& item : m_images){
            ptr->emplace_back(*item);
        }
    }
    m_listeners.run_method_unique(&ContentListener::on_overlay_update_images, ptr);
}
std::vector<OverlayImage> VideoOverlaySession::images() const{
    ReadSpinLock lg(m_lock);
    std::vector<OverlayImage> ret;
    for (const auto& item : m_images){
        ret.emplace_back(*item);
    }
    return ret;
}


//
//  Log
//

void VideoOverlaySession::add_log(std::string message, Color color){
    std::shared_ptr<std::vector<OverlayLogLine>> ptr = std::make_shared<std::vector<OverlayLogLine>>();
    {
        WriteSpinLock lg(m_lock, "VideoOverlaySession::add_log_text()");
        m_log_texts.emplace_front(color, std::move(message));

        if (m_log_texts.size() > LOG_MAX_LINES){
            m_log_texts.pop_back();
        }

        //  We create a newly allocated Box vector to avoid listener accessing
        //  `m_log_texts` asynchronously.
        for(const auto& item : m_log_texts){
            ptr->emplace_back(item);
        }
    }
    m_listeners.run_method_unique(&ContentListener::on_overlay_update_log, ptr);
}
void VideoOverlaySession::clear_log(){
    std::shared_ptr<std::vector<OverlayLogLine>> ptr = std::make_shared<std::vector<OverlayLogLine>>();
    {
        WriteSpinLock lg(m_lock, "VideoOverlaySession::clear_log_texts()");
        m_log_texts.clear();

        //  We create a newly allocated Box vector to avoid listener accessing
        //  `m_log_texts` asynchronously.
        for(const auto& item : m_log_texts){
            ptr->emplace_back(item);
        }
    }
    m_listeners.run_method_unique(&ContentListener::on_overlay_update_log, ptr);
}
std::vector<OverlayLogLine> VideoOverlaySession::log_texts() const{
    ReadSpinLock lg(m_lock);
    std::vector<OverlayLogLine> ret;
    for (const auto& item : m_log_texts){
        ret.emplace_back(item);
    }
    return ret;
}


//
//  Stats
//

void VideoOverlaySession::add_stat(OverlayStat& stat){
    WriteSpinLock lg(m_lock);
    auto map_iter = m_stats.find(&stat);
    if (map_iter != m_stats.end()){
        return;
    }

    m_stats_order.emplace_back(&stat);
    auto list_iter = m_stats_order.end();
    --list_iter;
    try{
        m_stats.emplace(&stat, list_iter);
    }catch (...){
        m_stats_order.pop_back();
        throw;
    }
}
void VideoOverlaySession::remove_stat(OverlayStat& stat){
    WriteSpinLock lg(m_lock);
    auto iter = m_stats.find(&stat);
    if (iter == m_stats.end()){
        return;
    }

    m_stats_order.erase(iter->second);
    m_stats.erase(iter);
}

std::vector<OverlayStatSnapshot> VideoOverlaySession::stats() const{
    ReadSpinLock lg(m_lock);
    return m_stat_lines;
}


























}
