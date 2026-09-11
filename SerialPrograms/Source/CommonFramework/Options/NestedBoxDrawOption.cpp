/*  Nested Box Draw Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "NestedBoxDrawOption.h"

namespace PokemonAutomation{



NestedBoxDrawOption::~NestedBoxDrawOption(){
    m_overlay.remove_hid_listener(*this);
    CONTENT_BOX.remove_listener(*this);
    INFERENCE_BOX.remove_listener(*this);
}
NestedBoxDrawOption::NestedBoxDrawOption(
    LockMode lock_while_program_is_running,
    VideoOverlay& overlay
)
    : BatchOption(lock_while_program_is_running)
    , INFERENCE_BOX(
        "Inference Box",
        lock_while_program_is_running,
        GroupOption::EnableMode::ALWAYS_ENABLED,
        false
    )
    , CONTENT_BOX(
        "Content Box",
        lock_while_program_is_running,
        GroupOption::EnableMode::DEFAULT_DISABLED,
        true,
        {0.1, 0.1, .8, .8}
    )
    , m_overlay(overlay)
    , m_overlay_set(overlay)
{
    PA_ADD_OPTION(INFERENCE_BOX);
    PA_ADD_OPTION(CONTENT_BOX);

    INFERENCE_BOX.add_listener(*this);
    CONTENT_BOX.add_listener(*this);
    overlay.add_hid_listener(*this);
}

void NestedBoxDrawOption::on_config_value_changed(void* object){
    if (this->visibility() != ConfigOptionState::ENABLED){
        return;
    }

    std::lock_guard<Mutex> lg(m_lock);
    m_overlay_set.clear();
    double ix = INFERENCE_BOX.X;
    double iy = INFERENCE_BOX.Y;
    double iw = INFERENCE_BOX.WIDTH;
    double ih = INFERENCE_BOX.HEIGHT;
    if (CONTENT_BOX.enabled()){
        double cx = CONTENT_BOX.X;
        double cy = CONTENT_BOX.Y;
        double cw = CONTENT_BOX.WIDTH;
        double ch = CONTENT_BOX.HEIGHT;
        m_overlay_set.add(COLOR_GREEN, {cx, cy, cw, ch});
        ix *= cw;
        iy *= ch;
        iw *= cw;
        ih *= ch;
        ix += cx;
        iy += cy;
    }
    m_overlay_set.add(COLOR_RED, {ix, iy, iw, ih});
}
void NestedBoxDrawOption::on_mouse_press(double x, double y){
    if (this->visibility() != ConfigOptionState::ENABLED){
        return;
    }
    m_mouse_start.emplace();
    m_mouse_start->first = x;
    m_mouse_start->second = y;
}
void NestedBoxDrawOption::on_mouse_release(double x, double y){
    if (this->visibility() != ConfigOptionState::ENABLED){
        return;
    }
    m_mouse_start.reset();
}
void NestedBoxDrawOption::on_mouse_move(double x, double y){
    if (this->visibility() != ConfigOptionState::ENABLED){
        return;
    }
    if (!m_mouse_start){
        return;
    }

    double xl = m_mouse_start->first;
    double xh = x;
    double yl = m_mouse_start->second;
    double yh = y;

    if (CONTENT_BOX.enabled()){
        double cx = CONTENT_BOX.X;
        double cy = CONTENT_BOX.Y;
        double cw = CONTENT_BOX.WIDTH;
        double ch = CONTENT_BOX.HEIGHT;

        xl = (xl - cx) / cw;
        xl = std::max(xl, 0.);
        xl = std::min(xl, 1.);

        yl = (yl - cy) / ch;
        yl = std::max(yl, 0.);
        yl = std::min(yl, 1.);

        xh = (xh - cx) / cw;
        xh = std::max(xh, 0.);
        xh = std::min(xh, 1.);

        yh = (yh - cy) / ch;
        yh = std::max(yh, 0.);
        yh = std::min(yh, 1.);
    }

    if (xl > xh){
        std::swap(xl, xh);
    }
    if (yl > yh){
        std::swap(yl, yh);
    }

    INFERENCE_BOX.X.set(xl);
    INFERENCE_BOX.Y.set(yl);
    INFERENCE_BOX.WIDTH.set(xh - xl);
    INFERENCE_BOX.HEIGHT.set(yh - yl);
}







}










