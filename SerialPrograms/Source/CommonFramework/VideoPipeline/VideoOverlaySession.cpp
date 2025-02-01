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



void VideoOverlaySession::add_listener(Listener& listener){
    WriteSpinLock lg(m_lock);
    m_listeners.insert(&listener);
    listener.update_stats(&m_stats_order);
}
void VideoOverlaySession::remove_listener(Listener& listener){
    WriteSpinLock lg(m_lock);
//    listener.update_stats(nullptr);
    m_listeners.erase(&listener);
}


VideoOverlaySession::~VideoOverlaySession(){
    ReadSpinLock lg(m_lock);
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
    WriteSpinLock lg(m_lock, "VideoOverlaySession::set_enabled_boxes()");
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
    m_option.boxes.store(enabled, std::memory_order_relaxed);
    ReadSpinLock lg(m_lock, "VideoOverlaySession::set_enabled_boxes()");
    for (Listener* listeners : m_listeners){
        listeners->enabled_boxes(enabled);
    }
}
void VideoOverlaySession::set_enabled_text(bool enabled){
    m_option.text.store(enabled, std::memory_order_relaxed);
    ReadSpinLock lg(m_lock, "VideoOverlaySession::set_enabled_text()");
    for (Listener* listeners : m_listeners){
        listeners->enabled_text(enabled);
    }
}
void VideoOverlaySession::set_enabled_log(bool enabled){
    m_option.log.store(enabled, std::memory_order_relaxed);
    ReadSpinLock lg(m_lock, "VideoOverlaySession::set_enabled_log()");
    for (Listener* listeners : m_listeners){
        listeners->enabled_log(enabled);
    }
}
void VideoOverlaySession::set_enabled_stats(bool enabled){
    m_option.stats.store(enabled, std::memory_order_relaxed);
    ReadSpinLock lg(m_lock, "VideoOverlaySession::set_enabled_stats()");
    for (Listener* listeners : m_listeners){
        listeners->enabled_stats(enabled);
    }
}



void VideoOverlaySession::add_box(const OverlayBox& box){
    WriteSpinLock lg(m_lock, "VideoOverlaySession::add_box()");
    m_boxes.insert(&box);
    push_box_update();
}
void VideoOverlaySession::remove_box(const OverlayBox& box){
    WriteSpinLock lg(m_lock, "VideoOverlaySession::remove_box()");
    m_boxes.erase(&box);
    push_box_update();
}

void VideoOverlaySession::push_box_update(){
    if (m_listeners.empty()){
        return;
    }
    
    //  We create a newly allocated Box vector to avoid listeners accessing
    //  `m_boxes` asynchronously.
    std::shared_ptr<std::vector<OverlayBox>> ptr = std::make_shared<std::vector<OverlayBox>>();
    for (const auto& item : m_boxes){
        ptr->emplace_back(*item);
    }
    for (Listener* listeners : m_listeners){
        listeners->update_boxes(ptr);
    }
}

std::vector<OverlayBox> VideoOverlaySession::boxes() const{
    ReadSpinLock lg(m_lock);
    std::vector<OverlayBox> ret;
    for (const auto& item : m_boxes){
        ret.emplace_back(*item);
    }
    return ret;
}

void VideoOverlaySession::add_text(const OverlayText& text){
    WriteSpinLock lg(m_lock, "VideoOverlaySession::add_text()");
    m_texts.insert(&text);
    push_text_update();
}
void VideoOverlaySession::remove_text(const OverlayText& text){
    WriteSpinLock lg(m_lock, "VideoOverlaySession::remove_text()");
    m_texts.erase(&text);
    push_text_update();
}

void VideoOverlaySession::push_text_update(){
    if (m_listeners.empty()){
        return;
    }

    //  We create a newly allocated Box vector to avoid listeners accessing
    //  `m_texts` asynchronously.
    std::shared_ptr<std::vector<OverlayText>> ptr = std::make_shared<std::vector<OverlayText>>();
    for (const auto& item : m_texts){
        ptr->emplace_back(*item);
    }
    for (Listener* listeners : m_listeners){
        listeners->update_text(ptr);
    }
}

std::vector<OverlayText> VideoOverlaySession::texts() const{
    ReadSpinLock lg(m_lock);
    std::vector<OverlayText> ret;
    for (const auto& item : m_texts){
        ret.emplace_back(*item);
    }
    return ret;
}

void VideoOverlaySession::add_log(std::string message, Color color){
    WriteSpinLock lg(m_lock, "VideoOverlaySession::add_log_text()");
    m_log_texts.emplace_front(color, std::move(message));

    if (m_log_texts.size() > LOG_MAX_LINES){
        m_log_texts.pop_back();
    }

    push_log_text_update();
}

void VideoOverlaySession::clear_log(){
    WriteSpinLock lg(m_lock, "VideoOverlaySession::clear_log_texts()");
    m_log_texts.clear();
    push_log_text_update();
}

void VideoOverlaySession::push_log_text_update(){
    if (m_listeners.empty()){
        return;
    }
    
    //  We create a newly allocated Box vector to avoid listeners accessing
    //  `m_log_texts` asynchronously.
    std::shared_ptr<std::vector<OverlayLogLine>> ptr = std::make_shared<std::vector<OverlayLogLine>>();
    for(const auto& item : m_log_texts){
        ptr->emplace_back(item);
    }
    for (Listener* listeners : m_listeners){
        listeners->update_log(ptr);
    }
}

std::vector<OverlayLogLine> VideoOverlaySession::log_texts() const{
    ReadSpinLock lg(m_lock);
    std::vector<OverlayLogLine> ret;
    for(const auto& item : m_log_texts){
        ret.emplace_back(item);
    }
    return ret;
}




void VideoOverlaySession::add_stat(OverlayStat& stat){
    WriteSpinLock lg(m_lock);
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
}
void VideoOverlaySession::remove_stat(OverlayStat& stat){
    WriteSpinLock lg(m_lock);
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
}



























}
