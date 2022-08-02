/*  Camera Session
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "VideoWidget.h"
#include "CameraImplementations.h"
#include "CameraSession.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{



CameraSession::CameraSession(
    CameraOption& option,
    Logger& logger
)
     : m_option(option)
     , m_logger(logger)
     , m_allow_snapshots(true)
{
    if (option.m_info){
        m_camera = make_camera(logger, option.m_info, option.m_current_resolution);
    }
//    cout << "CameraSession() = " << option.m_current_resolution << endl;
//    cout << "CameraSession() = " << option.m_info.device_name() << endl;
}
CameraSession::~CameraSession(){
    push_camera_shutdown();

    //  Wait for all listeners to detach.
    std::unique_lock<std::mutex> lg(m_lock);
    m_cv.wait(lg, [=]{ return m_listeners.empty(); });
}

void CameraSession::add_listener(Listener& listener){
    std::lock_guard<std::mutex> lg(m_lock);
    m_listeners.insert(&listener);
}
void CameraSession::remove_listener(Listener& listener){
    std::lock_guard<std::mutex> lg(m_lock);
    m_listeners.erase(&listener);
    m_cv.notify_all();
}

void CameraSession::push_camera_startup(){
    for (Listener* listener : m_listeners){
        listener->camera_startup(*m_camera);
    }
}
void CameraSession::push_camera_shutdown(){
    for (Listener* listener : m_listeners){
        listener->camera_shutdown();
    }
}



CameraInfo CameraSession::info() const{
    std::lock_guard<std::mutex> lg(m_lock);
    return m_option.m_info;
}
Resolution CameraSession::resolution() const{
    std::lock_guard<std::mutex> lg(m_lock);
    if (m_camera){
        return m_camera->current_resolution();
    }else{
        return Resolution();
    }
}


VideoSnapshot CameraSession::snapshot(){
    std::lock_guard<std::mutex> lg(m_lock);
    if (m_camera && m_allow_snapshots.load(std::memory_order_relaxed)){
        return m_camera->snapshot();
    }else{
        return VideoSnapshot();
    }
}


void CameraSession::set_info(const CameraInfo& info){
    std::lock_guard<std::mutex> lg(m_lock);
    if (m_camera){
        push_camera_shutdown();
        m_camera.reset();
    }
    m_option.m_info = info;
    if (info){
        m_camera = make_camera(m_logger, m_option.m_info, m_option.m_current_resolution);
        m_option.m_current_resolution = m_camera->current_resolution();
        cout << "set_info() = " << m_option.m_current_resolution << endl;
        push_camera_startup();
    }
}
void CameraSession::set_resolution(const Resolution& resolution){
    std::lock_guard<std::mutex> lg(m_lock);
    if (m_camera){
        m_camera->set_resolution(resolution);
        m_option.m_current_resolution = m_camera->current_resolution();
        cout << "set_resolution() = " << m_option.m_current_resolution << endl;
    }
}


void CameraSession::start_camera(){
    std::lock_guard<std::mutex> lg(m_lock);
    if (m_camera){
        m_logger.log("Camera is already started.", COLOR_RED);
        return;
    }
    m_camera = make_camera(m_logger, m_option.m_info, m_option.m_current_resolution);
    m_option.m_current_resolution = m_camera->current_resolution();
    cout << "start_camera() = " << m_option.m_current_resolution << endl;
    push_camera_startup();
}
void CameraSession::stop_camera(){
    std::lock_guard<std::mutex> lg(m_lock);
    if (m_camera){
        push_camera_shutdown();
        m_camera.reset();
    }
}

void CameraSession::reset(){
    std::lock_guard<std::mutex> lg(m_lock);
    if (m_camera){
        push_camera_shutdown();
        m_camera.reset();
    }
    m_camera = make_camera(m_logger, m_option.m_info, m_option.m_current_resolution);
    m_option.m_current_resolution = m_camera->current_resolution();
    cout << "reset() = " << m_option.m_current_resolution << endl;
    push_camera_startup();
}


bool CameraSession::allow_snapshots() const{
    return m_allow_snapshots.load(std::memory_order_relaxed);
}
void CameraSession::set_allow_snapshots(bool allow){
    m_allow_snapshots.store(allow, std::memory_order_relaxed);
}





}
