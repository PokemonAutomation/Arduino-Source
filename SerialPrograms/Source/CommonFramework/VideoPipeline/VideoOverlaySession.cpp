/*  Video Overlay Session
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "VideoOverlaySession.h"

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

void VideoOverlaySession::add_box(const ImageFloatBox& box, Color color){
    SpinLockGuard lg(m_lock, "VideoOverlay::add_box()");
    m_boxes[&box] = color;
    push_box_update();
}
void VideoOverlaySession::remove_box(const ImageFloatBox& box){
    SpinLockGuard lg(m_lock, "VideoOverlay::remove_box()");
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
        listeners->box_update(ptr);
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
    SpinLockGuard lg(m_lock, "VideoOverlay::add_text()");
    m_texts.insert(&text);
    push_text_update();
}
void VideoOverlaySession::remove_text(const OverlayText& text){
    SpinLockGuard lg(m_lock, "VideoOverlay::remove_text()");
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
        listeners->text_update(ptr);
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
    SpinLockGuard lg(m_lock, "VideoOverlay::add_log_text()");
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
    SpinLockGuard lg(m_lock, "VideoOverlay::clear_log_texts()");
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
        listeners->log_text_update(ptr);
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
        listeners->log_text_background_update(ptr);
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

}
