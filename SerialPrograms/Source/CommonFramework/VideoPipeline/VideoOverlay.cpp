/*  Video Overlay
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QKeyEvent>
#include "Common/Cpp/ListenerSet.h"
#include "Common/Cpp/Containers/Pimpl.tpp"
#include "VideoOverlay.h"

namespace PokemonAutomation{



struct VideoOverlay::DataMouseEvent{
    ListenerSet<MouseListener> m_mouseevent_listeners;
};


VideoOverlay::VideoOverlay()
    : m_data_mouseevent(CONSTRUCT_TOKEN)
    , m_data_keyevent(CONSTRUCT_TOKEN)
{}
VideoOverlay::~VideoOverlay() = default;



void VideoOverlay::add_mouse_listener(MouseListener& listener){
    m_data_mouseevent->m_mouseevent_listeners.add(listener);
}
void VideoOverlay::remove_mouse_listener(MouseListener& listener){
    m_data_mouseevent->m_mouseevent_listeners.remove(listener);
}
void VideoOverlay::issue_mouse_press(double x, double y){
    m_data_mouseevent->m_mouseevent_listeners.run_method_unique(&MouseListener::on_mouse_press, x, y);
}
void VideoOverlay::issue_mouse_release(double x, double y){
    m_data_mouseevent->m_mouseevent_listeners.run_method_unique(&MouseListener::on_mouse_release, x, y);
}
void VideoOverlay::issue_mouse_move(double x, double y){
    m_data_mouseevent->m_mouseevent_listeners.run_method_unique(&MouseListener::on_mouse_move, x, y);
}

struct VideoOverlay::DataKeyEvent{
    ListenerSet<KeyEventListener> m_keyevent_listeners;
};

void VideoOverlay::add_keyevent_listener(KeyEventListener& listener){
    m_data_keyevent->m_keyevent_listeners.add(listener);
}
void VideoOverlay::remove_keyevent_listener(KeyEventListener& listener){
    m_data_keyevent->m_keyevent_listeners.remove(listener);
}
void VideoOverlay::issue_key_press(QKeyEvent* event){
    m_data_keyevent->m_keyevent_listeners.run_method_unique(&KeyEventListener::on_key_press, event);
}
void VideoOverlay::issue_key_release(QKeyEvent* event){
    m_data_keyevent->m_keyevent_listeners.run_method_unique(&KeyEventListener::on_key_release, event);
}






}
