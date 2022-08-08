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
    std::vector<Box>* boxes = new std::vector<Box>();
    std::shared_ptr<std::vector<Box>> ptr(boxes);
    for (const auto& item : m_boxes){
        boxes->emplace_back(Box{*item.first, item.second});
    }
    for (Listener* listeners : m_listeners){
        listeners->box_update(ptr);
    }
}




}
