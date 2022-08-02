/*  Camera Session
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Qt/Redispatch.h"
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
}
CameraSession::~CameraSession(){
}

void CameraSession::add_listener(Listener& listener){
    SpinLockGuard lg(m_lock);
    m_listeners.insert(&listener);
}
void CameraSession::remove_listener(Listener& listener){
    SpinLockGuard lg(m_lock);
    m_listeners.erase(&listener);
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
    SpinLockGuard lg(m_lock);
    return m_option.m_info;
}
Resolution CameraSession::resolution() const{
    SpinLockGuard lg(m_lock);
    if (m_camera){
        return m_option.m_current_resolution;
    }else{
        return Resolution();
    }
}


VideoSnapshot CameraSession::snapshot(){
    SpinLockGuard lg(m_lock);
    if (m_camera && m_allow_snapshots.load(std::memory_order_relaxed)){
        return m_camera->snapshot();
    }else{
        return VideoSnapshot();
    }
}


void CameraSession::set_info(const CameraInfo& info){
    run_on_main_thread_and_wait([=]{
        SpinLockGuard lg(m_lock);
        if (m_camera){
            push_camera_shutdown();
            m_camera.reset();
        }
        m_option.m_info = info;
        if (info){
            m_camera = make_camera(m_logger, m_option.m_info, m_option.m_current_resolution);
            m_option.m_current_resolution = m_camera->current_resolution();
            push_camera_startup();
        }
    });

}
void CameraSession::set_resolution(const Resolution& resolution){
    run_on_main_thread_and_wait([=]{
        SpinLockGuard lg(m_lock);
        if (m_camera){
            m_camera->set_resolution(resolution);
            m_option.m_current_resolution = m_camera->current_resolution();
        }
    });
}


void CameraSession::start_camera(){
    run_on_main_thread_and_wait([=]{
        SpinLockGuard lg(m_lock);
        if (m_camera){
            m_logger.log("Camera is already started.", COLOR_RED);
            return;
        }
        m_camera = make_camera(m_logger, m_option.m_info, m_option.m_current_resolution);
        m_option.m_current_resolution = m_camera->current_resolution();
        push_camera_startup();
    });
}
void CameraSession::stop_camera(){
    run_on_main_thread_and_wait([=]{
        SpinLockGuard lg(m_lock);
        if (m_camera){
            push_camera_shutdown();
            m_camera.reset();
        }
    });
}

void CameraSession::reset(){
    run_on_main_thread_and_wait([=]{
        SpinLockGuard lg(m_lock);
        if (m_camera){
            push_camera_shutdown();
            m_camera.reset();
        }
        m_camera = make_camera(m_logger, m_option.m_info, m_option.m_current_resolution);
        m_option.m_current_resolution = m_camera->current_resolution();
        push_camera_startup();
    });
}
void CameraSession::request_reset_video(){
    queue_on_main_thread([=]{ reset(); });
}


bool CameraSession::allow_snapshots() const{
    return m_allow_snapshots.load(std::memory_order_relaxed);
}
void CameraSession::set_allow_snapshots(bool allow){
    m_allow_snapshots.store(allow, std::memory_order_relaxed);
}





}
