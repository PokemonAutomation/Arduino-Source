/*  Video Overlay Session
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "VideoOverlaySession.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{

namespace {

const float LOG_MIN_X = 0.025f;
const float LOG_WIDTH = 0.35f;
const size_t LOG_MAX_LINES = 20;
const float LOG_FONT_SIZE = 4.0f;
const float LOG_LINE_SPACING = 0.04f;
const float LOG_BORDER_X = 0.009f;
const float LOG_BORDER_Y = 0.016f;
const float LOG_MAX_Y = 0.95f;
const Color LOG_BACKGROUND_COLOR{200, 10, 10, 10};

}



void VideoOverlaySession::add_listener(Listener& listener){
    SpinLockGuard lg(m_lock);
    m_listeners.insert(&listener);
}
void VideoOverlaySession::remove_listener(Listener& listener){
    SpinLockGuard lg(m_lock);
    m_listeners.erase(&listener);
}


VideoOverlaySession::~VideoOverlaySession(){
    SpinLockGuard lg(m_lock);
    for (Listener* listeners : m_listeners){
        listeners->update_stats(nullptr);
    }
}
VideoOverlaySession::VideoOverlaySession(VideoOverlayOption& option)
    : m_option(option)
{}


void VideoOverlaySession::get(VideoOverlayOption& option){
    bool boxes = m_option.boxes.load(std::memory_order_relaxed);
    bool text = m_option.text.load(std::memory_order_relaxed);
    bool log = m_option.log.load(std::memory_order_relaxed);
    bool stats = m_option.stats.load(std::memory_order_relaxed);
    option.boxes.store(boxes, std::memory_order_relaxed);
    option.text.store(text, std::memory_order_relaxed);
    option.log.store(log, std::memory_order_relaxed);
    option.stats.store(stats, std::memory_order_relaxed);
}
void VideoOverlaySession::set(const VideoOverlayOption& option){
    SpinLockGuard lg(m_lock, "VideoOverlaySession::set_enabled_boxes()");
    bool boxes = option.boxes.load(std::memory_order_relaxed);
    bool text = option.text.load(std::memory_order_relaxed);
    bool log = option.log.load(std::memory_order_relaxed);
    bool stats = option.stats.load(std::memory_order_relaxed);
    m_option.boxes.store(boxes, std::memory_order_relaxed);
    m_option.text.store(text, std::memory_order_relaxed);
    m_option.log.store(log, std::memory_order_relaxed);
    m_option.stats.store(stats, std::memory_order_relaxed);
    for (Listener* listeners : m_listeners){
        listeners->enabled_boxes(boxes);
        listeners->enabled_text(text);
        listeners->enabled_log(log);
        listeners->enabled_stats(stats);
    }
}


void VideoOverlaySession::set_enabled_boxes(bool enabled){
    SpinLockGuard lg(m_lock, "VideoOverlaySession::set_enabled_boxes()");
    m_option.boxes.store(enabled, std::memory_order_relaxed);
    for (Listener* listeners : m_listeners){
        listeners->enabled_boxes(enabled);
    }
}
void VideoOverlaySession::set_enabled_text(bool enabled){
    SpinLockGuard lg(m_lock, "VideoOverlaySession::set_enabled_text()");
    m_option.text.store(enabled, std::memory_order_relaxed);
    for (Listener* listeners : m_listeners){
        listeners->enabled_text(enabled);
    }
}
void VideoOverlaySession::set_enabled_log(bool enabled){
    SpinLockGuard lg(m_lock, "VideoOverlaySession::set_enabled_log()");
    m_option.log.store(enabled, std::memory_order_relaxed);
    for (Listener* listeners : m_listeners){
        listeners->enabled_log(enabled);
    }
}
void VideoOverlaySession::set_enabled_stats(bool enabled){
    SpinLockGuard lg(m_lock, "VideoOverlaySession::set_enabled_stats()");
    m_option.stats.store(enabled, std::memory_order_relaxed);
    for (Listener* listeners : m_listeners){
        listeners->enabled_stats(enabled);
    }
}



void VideoOverlaySession::add_box(const ImageFloatBox& box, Color color){
    SpinLockGuard lg(m_lock, "VideoOverlaySession::add_box()");
    m_boxes[&box] = color;
    push_box_update();
}
void VideoOverlaySession::remove_box(const ImageFloatBox& box){
    SpinLockGuard lg(m_lock, "VideoOverlaySession::remove_box()");
    m_boxes.erase(&box);
    push_box_update();
}

void VideoOverlaySession::push_box_update(){
    if (m_listeners.empty()){
        return;
    }
    
    // We create a newly allocated Box vector to avoid listeners accessing `m_boxes` asynchronously.
    std::shared_ptr<std::vector<Box>> ptr = std::make_shared<std::vector<Box>>();
    for (const auto& item : m_boxes){
        ptr->emplace_back(*item.first, item.second);
    }
    for (Listener* listeners : m_listeners){
        listeners->update_boxes(ptr);
    }
}

std::vector<VideoOverlaySession::Box> VideoOverlaySession::boxes() const{
    SpinLockGuard lg(m_lock);
    std::vector<Box> ret;
    for (const auto& item : m_boxes){
        ret.emplace_back(*item.first, item.second);
    }
    return ret;
}

void VideoOverlaySession::add_text(const OverlayText& text){
    SpinLockGuard lg(m_lock, "VideoOverlaySession::add_text()");
    m_texts.insert(&text);
    push_text_update();
}
void VideoOverlaySession::remove_text(const OverlayText& text){
    SpinLockGuard lg(m_lock, "VideoOverlaySession::remove_text()");
    m_texts.erase(&text);
    push_text_update();
}

void VideoOverlaySession::push_text_update(){
    if (m_listeners.empty()){
        return;
    }
    
    // We create a newly allocated Box vector to avoid listeners accessing `m_texts` asynchronously.
    std::shared_ptr<std::vector<OverlayText>> ptr = std::make_shared<std::vector<OverlayText>>();
    for (const auto& item : m_texts){
        ptr->emplace_back(*item);
    }
    for (Listener* listeners : m_listeners){
        listeners->update_text(ptr);
    }
}

std::vector<OverlayText> VideoOverlaySession::texts() const{
    SpinLockGuard lg(m_lock);
    std::vector<OverlayText> ret;
    for (const auto& item : m_texts){
        ret.emplace_back(*item);
    }
    return ret;
}

void VideoOverlaySession::add_log_text(std::string message, Color color){
    SpinLockGuard lg(m_lock, "VideoOverlaySession::add_log_text()");
    const size_t old_num_lines = m_log_texts.size();
    m_log_texts.emplace_front(std::move(message), LOG_MIN_X + LOG_BORDER_X, 0.0f, LOG_FONT_SIZE, color);

    if (m_log_texts.size() > LOG_MAX_LINES){
        m_log_texts.pop_back();
    }

    float y = LOG_MAX_Y - LOG_BORDER_Y;
    for(auto& item : m_log_texts){
        item.y = y;
        y -= LOG_LINE_SPACING;
    }
    
    push_log_text_update();
    if (old_num_lines == 0){
        // update log text background
        push_text_background_update();
    }
}

void VideoOverlaySession::clear_log_texts(){
    SpinLockGuard lg(m_lock, "VideoOverlaySession::clear_log_texts()");
    const size_t old_num_lines = m_log_texts.size();
    m_log_texts.clear();
    push_text_update();
    if (old_num_lines != 0){
        push_text_background_update();
    }
}

void VideoOverlaySession::push_log_text_update(){
    if (m_listeners.empty()){
        return;
    }
    
    // We create a newly allocated Box vector to avoid listeners accessing `m_log_texts` asynchronously.
    std::shared_ptr<std::vector<OverlayText>> ptr = std::make_shared<std::vector<OverlayText>>();
    for(const auto& item : m_log_texts){
        ptr->emplace_back(item);
    }
    for (Listener* listeners : m_listeners){
        listeners->update_log_text(ptr);
    }
}

std::vector<OverlayText> VideoOverlaySession::log_texts() const{
    SpinLockGuard lg(m_lock);
    std::vector<OverlayText> ret;
    for(const auto& item : m_log_texts){
        ret.emplace_back(item);
    }
    return ret;
}

void VideoOverlaySession::push_text_background_update(){
    if (m_listeners.empty()){
        return;
    }

    std::shared_ptr<std::vector<Box>> ptr = std::make_shared<std::vector<Box>>();
    if (m_log_texts.size() > 0){
        const float log_bg_height = LOG_MAX_LINES * LOG_LINE_SPACING + 2*LOG_BORDER_Y;
        ImageFloatBox region(LOG_MIN_X, LOG_MAX_Y - log_bg_height, LOG_WIDTH, log_bg_height);
        ptr->emplace_back(region, LOG_BACKGROUND_COLOR);
    }
    for (Listener* listeners : m_listeners){
        listeners->update_log_background(ptr);
    }
}

std::vector<VideoOverlaySession::Box> VideoOverlaySession::log_text_background() const{
    SpinLockGuard lg(m_lock);
    std::vector<Box> ret;
    if (m_log_texts.size() > 0){
        const float log_bg_height = LOG_MAX_LINES * LOG_LINE_SPACING + 2*LOG_BORDER_Y;
        ImageFloatBox region(LOG_MIN_X, LOG_MAX_Y - log_bg_height, LOG_WIDTH, log_bg_height);
        ret.emplace_back(region, LOG_BACKGROUND_COLOR);
    }
    return ret;
}




void VideoOverlaySession::add_stat(OverlayStat& stat){
    SpinLockGuard lg(m_lock);
    auto map_iter = m_stats.find(&stat);
    if (map_iter != m_stats.end()){
        return;
    }

    //  Remove all stats so they aren't being referenced.
    for (Listener* listeners : m_listeners){
        listeners->update_stats(nullptr);
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

    //  Add all the stats back.
    for (Listener* listeners : m_listeners){
        listeners->update_stats(&m_stats_order);
    }

//    push_stats_update();
}
void VideoOverlaySession::remove_stat(OverlayStat& stat){
    SpinLockGuard lg(m_lock);
    auto iter = m_stats.find(&stat);
    if (iter == m_stats.end()){
        return;
    }

    //  Remove all stats so they aren't being referenced.
    for (Listener* listeners : m_listeners){
        listeners->update_stats(nullptr);
    }

    m_stats_order.erase(iter->second);
    m_stats.erase(iter);

    //  Add all the stats back.
    for (Listener* listeners : m_listeners){
        listeners->update_stats(&m_stats_order);
    }

//    push_stats_update();
}

#if 0
void VideoOverlaySession::push_stats_update(){
    if (m_listeners.empty()){
        return;
    }

    std::shared_ptr<std::vector<OverlayStatSnapshot>> ptr = std::make_shared<std::vector<OverlayStatSnapshot>>();
    for(const auto& item : m_stats_order){
        std::string text;
        Color color = item->get_text(text);
        ptr->emplace_back(OverlayStatSnapshot{text, color});
    }
    for (Listener* listeners : m_listeners){
        listeners->update_stats(&m_stats_order);
    }
}
#endif



























}
