/*  Nested Box Draw Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Containers/Pimpl.tpp"
#include "Common/Cpp/Concurrency/Mutex.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "NestedBoxDrawOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{



NestedBoxDrawOption::NestedBoxDrawOption(LockMode lock_while_program_is_running)
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
{
    PA_ADD_OPTION(INFERENCE_BOX);
    PA_ADD_OPTION(CONTENT_BOX);
}


class NestedBoxDrawOption::DrawnBox final : public ConfigOption::Listener, public VideoOverlay::MouseListener{
public:
    ~DrawnBox(){
        detach();
    }
    DrawnBox(
        VideoOverlay& overlay,
        BoxOption& content_box,
        BoxOption& inference_box
    )
        : m_content_box(content_box)
        , m_inference_box(inference_box)
        , m_overlay(overlay)
        , m_overlay_set(overlay)
    {
        // DrawnBox listens to changes in the config option (X, Y, WIDTH, HEIGHT)
        // and mouse events on the video overlay layer.
        try{
            m_inference_box.add_listener(*this);
            m_content_box.add_listener(*this);
            overlay.add_mouse_listener(*this);
            on_config_value_changed(nullptr);
        }catch (...){
            detach();
            throw;
        }
    }
    virtual void on_config_value_changed(void* object) override{
        {
            std::lock_guard<Mutex> lg(m_lock);
            m_overlay_set.clear();
            double ix = m_inference_box.X;
            double iy = m_inference_box.Y;
            double iw = m_inference_box.WIDTH;
            double ih = m_inference_box.HEIGHT;
            if (m_content_box.enabled()){
                double cx = m_content_box.X;
                double cy = m_content_box.Y;
                double cw = m_content_box.WIDTH;
                double ch = m_content_box.HEIGHT;
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
//        if (object != &m_inference_box){
//            m_inference_box.on_config_value_changed(object);
//        }
    }
    virtual void on_mouse_press(double x, double y) override{
        // m_parent.WIDTH.set(0);
        // m_parent.HEIGHT.set(0);
        // m_parent.X.set(x);
        // m_parent.Y.set(y);
        m_mouse_start.emplace();
        m_mouse_start->first = x;
        m_mouse_start->second = y;
    }
    virtual void on_mouse_release(double x, double y) override{
        m_mouse_start.reset();
    }
    virtual void on_mouse_move(double x, double y) override{
        if (!m_mouse_start){
            return;
        }

        double xl = m_mouse_start->first;
        double xh = x;
        double yl = m_mouse_start->second;
        double yh = y;

        if (m_content_box.enabled()){
            double cx = m_content_box.X;
            double cy = m_content_box.Y;
            double cw = m_content_box.WIDTH;
            double ch = m_content_box.HEIGHT;

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

        m_inference_box.X.set(xl);
        m_inference_box.Y.set(yl);
        m_inference_box.WIDTH.set(xh - xl);
        m_inference_box.HEIGHT.set(yh - yl);
        // m_parent.update_box_coordinates();
    }

private:
    void detach(){
        m_overlay.remove_mouse_listener(*this);
        m_content_box.remove_listener(*this);
        m_inference_box.remove_listener(*this);
    }

private:
    BoxOption& m_content_box;
    BoxOption& m_inference_box;
    VideoOverlay& m_overlay;
    VideoOverlaySet m_overlay_set;
    Mutex m_lock;

    std::optional<std::pair<double, double>> m_mouse_start;
};



Pimpl<NestedBoxDrawOption::DrawnBox> NestedBoxDrawOption::make_session(VideoOverlay& overlay){
    return Pimpl<DrawnBox>(CONSTRUCT_TOKEN, overlay, CONTENT_BOX, INFERENCE_BOX);
}


}


//template Pimpl<NintendoSwitch::NestedBoxDrawOption::DrawnBox>::Pimpl();
template Pimpl<NintendoSwitch::NestedBoxDrawOption::DrawnBox>::~Pimpl();


}










