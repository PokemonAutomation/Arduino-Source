/*  Video Overlay Session
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "VideoOverlaySession.h"

namespace PokemonAutomation{

namespace {

const float shell_xmin = 0.025;
const float shell_width = 0.35;
const size_t shell_max_lines = 20;
const float shell_font_size = 30.0f;
const float shell_line_spacing = 0.04f;
const float shell_x_border = 0.009f;
const float shell_y_border = 0.016f;
const float shell_ymax = 0.95f;
const Color shell_bg_color{200, 10, 10, 10};

}



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
        ret.emplace_back(*item.first, item.second);
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
        ptr->emplace_back(*item.first, item.second);
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
    SpinLockGuard lg(m_lock, "VideoOverlay::add_shell_text()");
    const size_t old_num_lines = m_shell_texts.size();
    m_shell_texts.emplace_front(std::move(message), shell_xmin + shell_x_border, 0.0f, shell_font_size, color);

    if (m_shell_texts.size() > shell_max_lines){
        m_shell_texts.pop_back();
    }

    float y = shell_ymax - shell_y_border;
    for(auto& item : m_shell_texts){
        item.y = y;
        y -= shell_line_spacing;
    }
    
    push_text_update();
    if (old_num_lines == 0){
        // update shell text background
        push_text_background_update();
    }
}

void VideoOverlaySession::clear_shell_texts(){
    SpinLockGuard lg(m_lock, "VideoOverlay::clear_shell_texts()");
    const size_t old_num_lines = m_shell_texts.size();
    m_shell_texts.clear();
    push_text_update();
    if (old_num_lines != 0){
        push_text_background_update();
    }
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

void VideoOverlaySession::push_text_background_update(){
    if (m_listeners.empty()){
        return;
    }

    std::shared_ptr<std::vector<Box>> ptr = std::make_shared<std::vector<Box>>();
    if (m_shell_texts.size() > 0){
        const float shell_bg_height = shell_max_lines * shell_line_spacing + 2*shell_y_border;
        ImageFloatBox region(shell_xmin, shell_ymax - shell_bg_height, shell_width, shell_bg_height);
        ptr->emplace_back(region, shell_bg_color);
    }
    for (Listener* listeners : m_listeners){
        listeners->text_background_update(ptr);
    }
}



}
