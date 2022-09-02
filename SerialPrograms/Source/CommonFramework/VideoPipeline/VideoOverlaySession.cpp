/*  Video Overlay Session
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "VideoOverlaySession.h"

namespace PokemonAutomation{



void VideoOverlaySession::add_listener(Listener& listener){
    SpinLockGuard lg(m_lock);
    m_listeners.insert(&listener);
}
void VideoOverlaySession::remove_listener(Listener& listener){
    SpinLockGuard lg(m_lock);
    m_listeners.erase(&listener);
}


std::vector<VideoOverlaySession::Box> VideoOverlaySession::boxes() const{
    SpinLockGuard lg(m_lock);
    std::vector<Box> ret;
    for (const auto& item : m_boxes){
        ret.emplace_back(Box{*item.first, item.second});
    }
    return ret;
}

std::vector<OverlayText> VideoOverlaySession::texts() const{
    SpinLockGuard lg(m_lock);
    std::vector<OverlayText> ret;
    for (const auto& item : m_texts){
        ret.emplace_back(*item);
    }
    return ret;
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
        ptr->emplace_back(Box{*item.first, item.second});
    }
    for (Listener* listeners : m_listeners){
        listeners->box_update(ptr);
    }
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

void VideoOverlaySession::add_shell_text(std::string message, Color color){
    const float x = 0.03f;
    const size_t max_lines = 20;
    const float font_size = 30.0f;

    m_shell_texts.emplace_front(std::move(message), x, 0.0f, font_size, color);

    if (m_shell_texts.size() > max_lines){
        m_shell_texts.pop_back();
    }

    float y = 0.95f;
    for(auto& item : m_shell_texts){
        item.y = y;
        y -= 0.04f;
    }
    
    push_text_update();
}

void VideoOverlaySession::clear_shell_texts(){
    m_shell_texts.clear();
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
    for(const auto& item : m_shell_texts){
        ptr->emplace_back(item);
    }
    for (Listener* listeners : m_listeners){
        listeners->text_update(ptr);
    }
}



}
