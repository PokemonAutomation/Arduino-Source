/*  Video Overlay
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_VideoOverlaySet_H
#define PokemonAutomation_VideoOverlaySet_H

#include <string>
#include "Common/Compiler.h"
#include "Common/Cpp/Color.h"
#include "Common/Cpp/Containers/Pimpl.h"
#include "VideoOverlayTypes.h"

namespace PokemonAutomation{

// Interface to add overlay objects (e.g. bounding boxes and texts) on top of rendered
// video stream
// The implementation is in CommonFramework/VideoPipeline/VideoOverlaySession.h:VideoOverlaySession
// The reason to create this interface is to reduce the header dependency on other code that relies
// on the overlay.
class VideoOverlay{
public:
    VideoOverlay();
    virtual ~VideoOverlay();

public:
    // Asychronously, add an inference box as part of the video overlay.
    // Once added, `box` cannot be destroyed until after `VideoOverlay::remove_box()` is called to remove it.
    // If a `box` with the same address is added, it will override the old box's position, shape and color. You only need to
    // call `remove_box()` once to remove it.
    //
    // Can use `InferenceBoxScope: public ImageFloatBox` to handle box removal automatically when it's destroyed.
    // Can also use `VideoOverlayScopes.h:VideoOverlaySet` to manage multiple boxes.
    virtual void add_box(const OverlayBox& box) = 0;

    // Asychronously, remove an added inference box.
    // The box must be already added.
    // See `add_box()` for more info on managing boxes.
    virtual void remove_box(const OverlayBox& box) = 0;

    // Asychronously, add a text, `OverlayText` as part of the video overlay.
    // Once added, `text` cannot be destroyed until after `VideoOverlay::remove_text()` is called to remove it.
    // If a `text` with the same address is added, it will override the old text's position, content and color.
    // You only need to call `remove_text()` once to remove it.
    //
    // Can use `OverlayTextScope: public OverlayText` to handle text removal automatically when it's destroyed.
    virtual void add_text(const OverlayText& text) = 0;

    // Asychronously, remove an added `OverlayText`.
    // The OverlayText must be already added.
    // See `add_text()` for more info on managing texts.
    virtual void remove_text(const OverlayText& text) = 0;

    // Asychronously, add an image, `OverlayImage` as part of the video overlay.
    // Allow transparency to create mask overlay.
    // Once added, `image` cannot be destroyed until after `VideoOverlay::remove_image()` is called to remove it.
    // If an `image` with the same address is added, it will override the old content. You only need
    // to call `remove_text()` once to remove it.
    virtual void add_image(const OverlayImage& image) = 0;
    // Asychronously, remove an added `OverlayImage`.
    // The OverlayImage must be already added.
    // See `add_image()` for more info on managing overlay images.
    virtual void remove_image(const OverlayImage& image) = 0;

    // Asynchronously, add a log message to the screen. The older messages added via `add_log_text()`
    // will be placed higher like in the logging window.
    // Use `OverlayLogTextScope` to remove the log messages automatically. 
    virtual void add_log(std::string message, Color color = COLOR_WHITE) = 0;
    // Remove all messages added by `add_log_text()`.
    // Use `OverlayLogTextScope` to remove log messages automatically.
    virtual void clear_log() = 0;

    virtual void add_stat(OverlayStat& stat) = 0;
    virtual void remove_stat(OverlayStat& stat) = 0;


public:
    struct MouseListener{
        virtual void on_mouse_press(double x, double y){}
        virtual void on_mouse_release(double x, double y){};
        virtual void on_mouse_move(double x, double y){};
    };
    void add_listener(MouseListener& listener);
    void remove_listener(MouseListener& listener);
    // Called by VideoDisplayWidget to call attached mouse listeners' on_mouse_press().
    void issue_mouse_press(double x, double y);
    // Called by VideoDisplayWidget to call attached mouse listeners' on_mouse_release().
    void issue_mouse_release(double x, double y);
    // Called by VideoDisplayWidget to call attached mouse listeners' on_mouse_move().
    void issue_mouse_move(double x, double y);

private:
    struct Data;
    Pimpl<Data> m_data;
};




}
#endif
