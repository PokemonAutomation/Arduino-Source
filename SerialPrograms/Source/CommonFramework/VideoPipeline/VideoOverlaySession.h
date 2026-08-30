/*  Video Overlay Session
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  This class holds the real-time state of the video overlays. You can
 *  asychronously add/remove objects to it.
 *
 *  This class is not responsible for any UI. However, any changes made to this
 *  class will be forwarded to any UI components that are attached to it.
 *
 */

#ifndef PokemonAutomation_VideoPipeline_VideoOverlaySession_H
#define PokemonAutomation_VideoPipeline_VideoOverlaySession_H

#include <memory>
#include <vector>
#include <list>
#include <set>
#include <map>
#include <deque>
#include "Common/Cpp/Logging/AbstractLogger.h"
#include "Common/Cpp/Color.h"
#include "Common/Cpp/ListenerSet.h"
#include "Common/Cpp/Concurrency/SpinLock.h"
#include "Common/Cpp/Concurrency/Mutex.h"
#include "Common/Cpp/Concurrency/ConditionVariable.h"
#include "Common/Cpp/Concurrency/AsyncTask.h"
#include "VideoOverlay.h"
#include "VideoOverlayOption.h"

namespace PokemonAutomation{

//  This class holds the real-time state of the video overlays. You can
//  asychronously add/remove objects to it.
//  This class is not responsible for any UI. However, any changes made to this
//  class will be forwarded to any UI components that are attached to it.
class VideoOverlaySession : public VideoOverlay
{
public:
    static constexpr size_t LOG_MAX_LINES = 20;

public:
    // Video overlay UI class (e.g. VideoOverlayWidget) inherits this listener to listen
    // to the overlay session.
    struct ContentListener{
        //  VideoOverlaySession will call these when they change.

        virtual void on_overlay_enabled_boxes  (bool enabled){}
        virtual void on_overlay_enabled_text   (bool enabled){}
        virtual void on_overlay_enabled_images (bool enabled){}
        virtual void on_overlay_enabled_log    (bool enabled){}
        virtual void on_overlay_enabled_stats  (bool enabled){}

        // Returns a copy to avoid the caller (UI object) accessing the data async while the data
        // is modified by VideoOverlaySession::add/remove_box().
        virtual void on_overlay_update_boxes  (const std::shared_ptr<const std::vector<OverlayBox>>& boxes){}
        // Returns a copy to avoid the caller (UI object) accessing the data async while the data
        // is modified by VideoOverlaySession::add/remove_text().
        virtual void on_overlay_update_text   (const std::shared_ptr<const std::vector<OverlayText>>& texts){}
        // Returns a copy to avoid the caller (UI object) accessing the data async while the data
        // is modified by VideoOverlaySession::add/remove_image().
        virtual void on_overlay_update_images (const std::shared_ptr<const std::vector<OverlayImage>>& images){}
        // Returns a copy to avoid the caller (UI object) accessing the data async while the data
        // is modified by VideoOverlaySession::add/remove_log().
        virtual void on_overlay_update_log    (const std::shared_ptr<const std::vector<OverlayLogLine>>& boxes){}

    };

    //  Add a UI class to listen to any overlay change. The UI class needs to inherit Listener.
    //  Must call `remove_listener()` before listener is destroyed.
    void add_content_listener(ContentListener& listener);
    //  Remove a UI class that listens to the overlay change, added by `add_listener()`.
    void remove_content_listener(ContentListener& listener);


public:
    ~VideoOverlaySession();
    VideoOverlaySession(Logger& logger, VideoOverlayOption& option);

    Logger& logger() const{ return m_logger; }

    void get(VideoOverlayOption& option) const;
    void set(const VideoOverlayOption& option);

    bool enabled_boxes () const{ return m_option.boxes.load(std::memory_order_relaxed); }
    bool enabled_text  () const{ return m_option.text.load(std::memory_order_relaxed); }
    bool enabled_images() const{ return m_option.images.load(std::memory_order_relaxed); }
    bool enabled_log   () const{ return m_option.log.load(std::memory_order_relaxed); }
    bool enabled_stats () const{ return m_option.stats.load(std::memory_order_relaxed); }

    void set_enabled_boxes (bool enabled);
    void set_enabled_text  (bool enabled);
    void set_enabled_images(bool enabled);
    void set_enabled_log   (bool enabled);
    void set_enabled_stats (bool enabled);

    // Called by rendering infra to access a copy of the stored overlay boxes.
    // Returns a copy to avoid the caller (UI object) accessing the data async while the data
    // is modified by VideoOverlaySession::add/remove_box().
    std::vector<OverlayBox> boxes() const;
    // Called by rendering infra to access the overlay texts.
    // Returns a copy to avoid the caller (UI object) accessing the data async while the data
    // is modified by VideoOverlaySession::add/remove_text().
    std::vector<OverlayText> texts() const;
    // Called by rendering infra to access the overlay images.
    // Returns a copy to avoid the caller (UI object) accessing the data async while the data
    // is modified by VideoOverlaySession::add/remove_image().
    std::vector<OverlayImage> images() const;
    // Called by rendering infra to access the overlay logs.
    // Returns a copy to avoid the caller (UI object) accessing the data async while the data
    // is modified by VideoOverlaySession::add/remove_log().
    std::vector<OverlayLogLine> log_texts() const;

    std::vector<OverlayStatSnapshot> stats() const;


    virtual void add_box(const OverlayBox& box) override;
    virtual void remove_box(const OverlayBox& box) override;

    virtual void add_text(const OverlayText& text) override;
    virtual void remove_text(const OverlayText& text) override;

    virtual void add_image(const OverlayImage& image) override;
    virtual void remove_image(const OverlayImage& image) override;

    virtual void add_log(std::string message, Color color = COLOR_WHITE) override;
    virtual void clear_log() override;

    virtual void add_stat(OverlayStat& stat) override;
    virtual void remove_stat(OverlayStat& stat) override;


public:
    virtual void add_hid_listener(VideoDisplayHidListener& listener) override;
    virtual void remove_hid_listener(VideoDisplayHidListener& listener) override;

    virtual void on_focus_in() override;
    virtual void on_focus_out() override;

    virtual void on_mouse_press(double x, double y) override;
    virtual void on_mouse_release(double x, double y) override;
    virtual void on_mouse_move(double x, double y) override;

    virtual void on_key_press(void* key) override;
    virtual void on_key_release(void* key) override;

    void report_focus_in(){
        m_hid_listeners.run_method(&VideoDisplayHidListener::on_focus_in);
    }
    void report_focus_out(){
        m_hid_listeners.run_method(&VideoDisplayHidListener::on_focus_out);
    }
    void report_mouse_press(double x, double y){
        m_hid_listeners.run_method(&VideoDisplayHidListener::on_mouse_press, x, y);
    }
    void report_mouse_release(double x, double y){
        m_hid_listeners.run_method(&VideoDisplayHidListener::on_mouse_release, x, y);
    }
    void report_mouse_move(double x, double y){
        m_hid_listeners.run_method(&VideoDisplayHidListener::on_mouse_move, x, y);
    }
    void report_key_press(void* key){
        m_hid_listeners.run_method(&VideoDisplayHidListener::on_key_press, key);
    }
    void report_key_release(void* key){
        m_hid_listeners.run_method(&VideoDisplayHidListener::on_key_release, key);
    }


private:
    void stats_thread();


private:
    Logger& m_logger;
    VideoOverlayOption& m_option;

    mutable SpinLock m_lock;

    std::set<const OverlayBox*> m_boxes;
    std::set<const OverlayText*> m_texts;
    std::set<const OverlayImage*> m_images;
    std::deque<OverlayLogLine> m_log_texts;

    ListenerSet<OverlayStat> m_stats;

    ListenerSet<ContentListener> m_content_listeners;
    ListenerSet<VideoDisplayHidListener> m_hid_listeners;

    bool m_stopping = false;
    std::vector<OverlayStatSnapshot> m_stat_lines;
    Mutex m_stats_lock;
    ConditionVariable m_stats_cv;
    AsyncTask m_stats_updater;
};



}
#endif
