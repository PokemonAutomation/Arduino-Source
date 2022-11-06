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
#include <set>
#include <map>
#include <deque>
#include "Common/Compiler.h"
#include "Common/Cpp/Color.h"
#include "Common/Cpp/Concurrency/SpinLock.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "VideoOverlay.h"

namespace PokemonAutomation{

// This class holds the real-time state of the video overlays. You can
// asychronously add/remove objects to it.
// This class is not responsible for any UI. However, any changes made to this
// class will be forwarded to any UI components that are attached to it.
class VideoOverlaySession : public VideoOverlay{
public:
    struct Box{
        ImageFloatBox box;
        Color color;
        Box(const ImageFloatBox& box, Color color) : box(box), color(color) {}
        Box(const Box&) = default;
        Box(Box&&) = default;
    };

public:
    // Video overlay UI class (e.g. VideoOverlayWidget) inherits this listener to listen
    // to the overlay session.
    struct Listener{
        //  VideoOverlaySession will call these when they change.

        virtual void update_boxes(const std::shared_ptr<const std::vector<Box>>& boxes){}
        virtual void update_text(const std::shared_ptr<const std::vector<OverlayText>>& texts){}
        virtual void update_log_text(const std::shared_ptr<const std::vector<OverlayText>>& boxes){}
        virtual void update_log_background(const std::shared_ptr<const std::vector<Box>>& boxes){}
        virtual void update_stats(const std::shared_ptr<const std::vector<OverlayStat>>& stats){}

    };

    // Add a UI class to listen to any overlay change. The UI class needs to inherit Listener.
    // Must call `remove_listener()` before listener is destroyed.
    void add_listener(Listener& listener);
    // remove a UI class that listens to the overlay change, added by `add_listener()`.
    void remove_listener(Listener& listener);

public:
    std::vector<Box> boxes() const;
    std::vector<OverlayText> texts() const;
    std::vector<OverlayText> log_texts() const;
    std::vector<Box> log_text_background() const;

    // Override `VideoOverlay::add_box()`. See the overridden function for more comments.
    virtual void add_box(const ImageFloatBox& box, Color color) override;
    // Override `VideoOverlay::remove_box()`. See the overridden function for more comments.
    virtual void remove_box(const ImageFloatBox& box) override;

    // Override `VideoOverlay::add_text()`. See the overridden function for more comments.
    virtual void add_text(const OverlayText& text) override;
    // Override `VideoOverlay::remove_text()`. See the overridden function for more comments.
    virtual void remove_text(const OverlayText& texxt) override;
    // Override `VideoOverlay::add_log_text()`. See the overridden function for more comments.
    virtual void add_log_text(std::string message, Color color) override;
    // Override `VideoOverlay::clear_log_texts()`. See the overridden function for more comments.
    virtual void clear_log_texts() override;

private:
    // Pass the current boxes to the listeners.
    // Called by `add_box()` and `remove_box()` so that when boxes change, the listeners know the
    // change.
    void push_box_update();
    // Pass the current texts to the listeners.
    // Called by `add_text()`, `remove_text()` so that when texts change, the listeners know the
    // change.
    void push_text_update();
    // Pass the log texts to the listeners.
    // Called by `add_log_text()` and `clear_log_texts()` to notify listeners of updating the log
    // texts.
    void push_log_text_update();
    // Pass the log text background box to the listeners.
    // Called by `add_log_text()` and `clear_log_texts()` to notify listeners of updating the log
    // texts background.
    void push_text_background_update();

private:
    mutable SpinLock m_lock;

    std::map<const ImageFloatBox*, Color> m_boxes;
    std::set<const OverlayText*> m_texts;
    std::deque<OverlayText> m_log_texts;

    std::set<Listener*> m_listeners;
};



}
#endif
