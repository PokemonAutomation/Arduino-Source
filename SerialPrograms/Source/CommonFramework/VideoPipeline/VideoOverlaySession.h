/*  Video Overlay Session
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
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
#include "Common/Cpp/Color.h"
#include "Common/Cpp/Concurrency/SpinLock.h"
#include "VideoOverlay.h"
#include "VideoOverlayOption.h"

namespace PokemonAutomation{

//  This class holds the real-time state of the video overlays. You can
//  asychronously add/remove objects to it.
//  This class is not responsible for any UI. However, any changes made to this
//  class will be forwarded to any UI components that are attached to it.
class VideoOverlaySession : public VideoOverlay{
public:
    static constexpr size_t LOG_MAX_LINES = 20;

public:
    // Video overlay UI class (e.g. VideoOverlayWidget) inherits this listener to listen
    // to the overlay session.
    struct Listener{
        //  VideoOverlaySession will call these when they change.

        virtual void enabled_boxes(bool enabled){}
        virtual void enabled_text (bool enabled){}
        virtual void enabled_log  (bool enabled){}
        virtual void enabled_stats(bool enabled){}

        virtual void update_boxes(const std::shared_ptr<const std::vector<OverlayBox>>& boxes){}
        virtual void update_text (const std::shared_ptr<const std::vector<OverlayText>>& texts){}
        virtual void update_log  (const std::shared_ptr<const std::vector<OverlayLogLine>>& boxes){}

        //  This one is different from the others. The listeners will store this
        //  pointer and access it directly and asynchronously. If you need to
        //  change the structure of the list itself, you must first call this
        //  with null to remove it from all the listeners. Then add the updated
        //  one back when you're done.
        //  This is called immediately when attaching a listener to give the
        //  current stats. The listener must drop all references to the stats
        //  before detaching.
        virtual void update_stats(const std::list<OverlayStat*>* stats){}

    };

    //  Add a UI class to listen to any overlay change. The UI class needs to inherit Listener.
    //  Must call `remove_listener()` before listener is destroyed.
    void add_listener(Listener& listener);
    //  Remove a UI class that listens to the overlay change, added by `add_listener()`.
    void remove_listener(Listener& listener);

public:
    ~VideoOverlaySession();
    VideoOverlaySession(VideoOverlayOption& option);

    void get(VideoOverlayOption& option);
    void set(const VideoOverlayOption& option);

    bool enabled_boxes() const{ return m_option.boxes.load(std::memory_order_relaxed); }
    bool enabled_text () const{ return m_option.text.load(std::memory_order_relaxed); }
    bool enabled_log  () const{ return m_option.log.load(std::memory_order_relaxed); }
    bool enabled_stats() const{ return m_option.stats.load(std::memory_order_relaxed); }

    void set_enabled_boxes(bool enabled);
    void set_enabled_text (bool enabled);
    void set_enabled_log  (bool enabled);
    void set_enabled_stats(bool enabled);

    std::vector<OverlayBox> boxes() const;
    std::vector<OverlayText> texts() const;
    std::vector<OverlayLogLine> log_texts() const;

    virtual void add_box(const OverlayBox& box) override;
    virtual void remove_box(const OverlayBox& box) override;

    virtual void add_text(const OverlayText& text) override;
    virtual void remove_text(const OverlayText& text) override;

    virtual void add_log(std::string message, Color color = COLOR_WHITE) override;
    virtual void clear_log() override;

    virtual void add_stat(OverlayStat& stat) override;
    virtual void remove_stat(OverlayStat& stat) override;

private:
    //  Push updates to the various listeners.
    void push_box_update();
    void push_text_update();
    void push_log_text_update();

private:
    mutable SpinLock m_lock;

    VideoOverlayOption& m_option;

    std::set<const OverlayBox*> m_boxes;
    std::set<const OverlayText*> m_texts;
    std::deque<OverlayLogLine> m_log_texts;

    std::list<OverlayStat*> m_stats_order;
    std::map<OverlayStat*, std::list<OverlayStat*>::iterator> m_stats;

    std::set<Listener*> m_listeners;
};



}
#endif
