/*  Nested Box Draw Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_NestedBoxDrawOption_H
#define PokemonAutomation_NestedBoxDrawOption_H

#include "Common/Cpp/Concurrency/Mutex.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "BoxOption.h"

namespace PokemonAutomation{



class NestedBoxDrawOption
    : public BatchOption
    , public ConfigOption::Listener
    , public VideoDisplayHidListener
{
public:
    ~NestedBoxDrawOption();
    NestedBoxDrawOption(
        LockMode lock_while_program_is_running,
        VideoOverlay& overlay
    );

    virtual void on_config_value_changed(void* object) override;
    virtual void on_mouse_press(double x, double y) override;
    virtual void on_mouse_release(double x, double y) override;
    virtual void on_mouse_move(double x, double y) override;

public:
    BoxOption INFERENCE_BOX;
    BoxOption CONTENT_BOX;

private:
    VideoOverlay& m_overlay;
    Mutex m_lock;
    std::optional<std::pair<double, double>> m_mouse_start;
    VideoOverlaySet m_overlay_set;
};


}
#endif
