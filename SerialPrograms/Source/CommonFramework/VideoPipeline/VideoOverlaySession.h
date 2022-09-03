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
        // `VideoOverlaySession` will call this function to give the newest boxes to the listener.
        virtual void box_update(const std::shared_ptr<const std::vector<Box>>& boxes){}

        // `VideoOverlaySession` will call this function to give the newest texts to the listener.
        virtual void text_update(const std::shared_ptr<const std::vector<OverlayText>>& boxes){}

        virtual void text_background_update(const std::shared_ptr<const std::vector<Box>>& boxes){}
    };

    // Add a UI class to listen to any overlay change. The UI class needs to inherit Listener.
    // Must call `remove_listener()` before listener is destroyed.
    void add_listener(Listener& listener);
    // remove a UI class that listens to the overlay change, added by `add_listener()`.
    void remove_listener(Listener& listener);

public:
    std::vector<Box> boxes() const;

    std::vector<OverlayText> texts() const;

    // Override `VideoOverlay::add_box()`. See the overridden function for more comments.
    virtual void add_box(const ImageFloatBox& box, Color color) override;
    // Override `VideoOverlay::remove_box()`. See the overridden function for more comments.
    virtual void remove_box(const ImageFloatBox& box) override;

    // Override `VideoOverlay::add_text()`. See the overridden function for more comments.
    virtual void add_text(const OverlayText& text) override;
    // Override `VideoOverlay::remove_text()`. See the overridden function for more comments.
    virtual void remove_text(const OverlayText& texxt) override;
    // Override `VideoOverlay::add_shell_text()`. See the overridden function for more comments.
    virtual void add_shell_text(std::string message, Color color) override;
    // Override `VideoOverlay::clear_shell_texts()`. See the overridden function for more comments.
    virtual void clear_shell_texts() override;

private:
    // Pass the current boxes to the listeners.
    // Called by `add_box()` and `remove_box()` so that when boxes change, the listeners know the
    // change.
    void push_box_update();
    // Pass the current texts to the listeners.
    // Called by `add_text()`, `remove_text()`, `add_shell_text()` and `clear_shell_texts()` so that
    // when texts change, the listeners know the change.
    void push_text_update();
    // Pass the shell text background box to the listeners.
    // Called by `add_shell_text()` and `clear_shell_texts()` to notify listeners of updating text
    // background.
    void push_text_background_update();

private:
    mutable SpinLock m_lock;

    std::map<const ImageFloatBox*, Color> m_boxes;

    std::set<const OverlayText*> m_texts;

    std::deque<OverlayText> m_shell_texts;

    std::set<Listener*> m_listeners;
};



}
#endif
