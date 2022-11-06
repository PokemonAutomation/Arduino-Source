/*  Video Overlay Set
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_VideoOverlaySet_H
#define PokemonAutomation_VideoOverlaySet_H

#include <deque>
#include <string>
#include <vector>
#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"

namespace PokemonAutomation{

struct OverlayText;


class VideoOverlay{
public:
    
    // Asychronously, add an inference box as part of the video overlay.
    // Once added, `box` cannot be destroyed until after `VideoOverlay::remove_box()` is called to remove it.
    // If a `box` with the same address is added, it will override the old box's position, shape and color. You only need to
    // call `remove_box()` once to remove it.
    //
    // Can use `InferenceBoxScope: public ImageFloatBox` to handle box removal automatically when it's destroyed.
    // Can also use `VideoOverlay.h:VideoOverlaySet` to manage multiple boxes.
    virtual void add_box(const ImageFloatBox& box, Color color) = 0;
    
    // Asychronously, remove an added inference box.
    // The box must be already added.
    // See `add_box()` for more info on managing boxes.
    virtual void remove_box(const ImageFloatBox& box) = 0;

    // Asychronously, add a text, `OverlayText` as part of the video overlay.
    // Once added, `text` cannot be destroyed until after `VideoOverlay::remove_text()` is called to remove it.
    // If a `text` with the same address is added, it will override the old text's position, content and color. You only need
    // to call `remove_text()` once to remove it.
    //
    // Can use `OverlayTextScope: public OverlayText` to handle text removal automatically when it's destroyed.
    virtual void add_text(const OverlayText& text) = 0;

    // Asychronously, remove an added `OverlayText`.
    // The OverlayText must be already added.
    // See `add_text()` for more info on managing texts.
    virtual void remove_text(const OverlayText& text) = 0;

    // Asynchronously, add a log message to the screen. The older messages added via `add_log_text()`
    // will be placed higher like in the logging window.
    // Use `OverlayLogTextScope` to remove the log messages automatically. 
    virtual void add_log_text(std::string message, Color color) = 0;
    // Remove all messages added by `add_log_text()`.
    // Use `OverlayLogTextScope` to remove log messages automatically.
    virtual void clear_log_texts() = 0;
};


// A box as part of the video overlay.
// It handles its own life time on video overlay: once it's destroyed, it removes itself from VideoOverlay.
class InferenceBoxScope : public ImageFloatBox{
public:
    ~InferenceBoxScope(){
        m_overlay.remove_box(*this);
    }
    InferenceBoxScope(const InferenceBoxScope&) = delete;
    void operator=(const InferenceBoxScope&) = delete;

public:
    InferenceBoxScope(
        VideoOverlay& overlay,
        const ImageFloatBox& box,
        Color color = COLOR_RED
    )
        : ImageFloatBox(box)
        , m_color(color)
        , m_overlay(overlay)
    {
        overlay.add_box(*this, color);
    }
    InferenceBoxScope(
        VideoOverlay& overlay,
        double p_x, double p_y,
        double p_width, double p_height,
        Color color = COLOR_RED
    )
        : ImageFloatBox(p_x, p_y, p_width, p_height)
        , m_color(color)
        , m_overlay(overlay)
    {
        overlay.add_box(*this, color);
    }

private:
    Color m_color;
    VideoOverlay& m_overlay;
};



// Used by video inference sessions to manage inference boxes.
// VideoOverlaySet will be passed to the inference callbacks in a session
// to store inference boxes. When the session ends, VideoOverlaySet::clear()
// is called to release those inference boxes. The boxes are also cleared
// automatically when VideoOverlaySet is destroyed.
// In this way, the user will see the inference boxes on the video overlay UI
// and those boxes will leave the UI after the session ends.
//
// TODO: shall we add text management to this class, or rename this class to BoxOverleySet?
class VideoOverlaySet{
public:
    VideoOverlaySet(VideoOverlay& overlay)
        : m_overlay(overlay)
    {}

    void clear(){
        m_boxes.clear();
    }
    void add(Color color, const ImageFloatBox& box){
        m_boxes.emplace_back(m_overlay, box, color);
    }

private:
    VideoOverlay& m_overlay;
    std::deque<InferenceBoxScope> m_boxes;
};



// A text as part of the video overlay.
struct OverlayText{
    // Text content.
    // Note overlay cannot handle newline character "\n".
    std::string message;
    // x coordinate of the text start, range: 0.0-1.0.
    double x;
    // y coordinate of the text start, range: 0.0-1.0.
    double y;
    // Font point size. This value is relative to the video overlay widget height. So you can
    // set it without considering overlay widget resolution.
    // Value of 4.0 gives a large, comfortable font size while not too large to clutter the screen.
    double font_size;
    // Text color.
    Color color;

public:
    OverlayText(std::string message,
        double x,
        double y,
        double font_size = 1.0,
        Color color = COLOR_BLUE
    )
        : message(std::move(message))
        , x(x)
        , y(y)
        , font_size(font_size)
        , color(color) {}
    
    OverlayText(const OverlayText& other) = default;
    OverlayText(OverlayText&& other) = default;
};

// A text as part of the video overlay.
// It handles its own life time on video overlay: once it's destroyed, it removes itself from VideoOverlay.
class OverlayTextScope : public OverlayText{
    OverlayTextScope(const OverlayTextScope&) = delete;
    void operator=(const OverlayTextScope&) = delete;

public:
    ~OverlayTextScope(){
        m_overlay.remove_text(*this);
    }

public:
    OverlayTextScope(
        VideoOverlay& overlay,
        std::string message,
        double x,
        double y,
        double font_size = 1.0,
        Color color = COLOR_BLUE
    )
        : OverlayText(message, x, y, font_size, color)
        , m_overlay(overlay)
    {
        overlay.add_text(*this);
    }

private:
    VideoOverlay& m_overlay;
};



// Used to clear log messages on video overlay automatically.
// Place this at the beginning of a program, so that when the program exits, it will
// clear the log messages from the overlay automatically.
class OverlayLogTextScope{
    OverlayLogTextScope(const OverlayLogTextScope&) = delete;
    void operator=(const OverlayLogTextScope&) = delete;

public:
    OverlayLogTextScope(VideoOverlay& overlay)
        : m_overlay(overlay)
    {}
    ~OverlayLogTextScope(){
        m_overlay.clear_log_texts();
    }

private:
    VideoOverlay& m_overlay;
};



}
#endif
