/*  Video Overlay Set
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_VideoOverlaySet_H
#define PokemonAutomation_VideoOverlaySet_H

#include <string>
#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "VideoOverlayTypes.h"

namespace PokemonAutomation{


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




}
#endif
