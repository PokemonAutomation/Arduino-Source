/*  Box Draw
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <optional>
#include "Common/Cpp/Concurrency/Mutex.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "BoxDraw.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{








BoxDraw_Descriptor::BoxDraw_Descriptor()
    : SingleSwitchProgramDescriptor(
        "NintendoSwitch:BoxDraw",
        "Nintendo Switch", "Box Draw",
        "",
        "Test box coordinates for development.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::NONE,
        AllowCommandsWhenRunning::ENABLE_COMMANDS
    )
{}

BoxDraw::BoxDraw()
    : INFERENCE_BOX(
        "Inference Box",
        LockMode::UNLOCK_WHILE_RUNNING,
        GroupOption::EnableMode::ALWAYS_ENABLED,
        false
    )
    , CONTENT_BOX(
        "Content Box",
        LockMode::UNLOCK_WHILE_RUNNING,
        GroupOption::EnableMode::DEFAULT_DISABLED,
        true,
        {0.1, 0.1, .8, .8}
    )
{
    PA_ADD_OPTION(INFERENCE_BOX);
    PA_ADD_OPTION(CONTENT_BOX);
}

class BoxDraw::DrawnBox : public ConfigOption::Listener, public VideoOverlay::MouseListener{
public:
    ~DrawnBox(){
        detach();
    }
    DrawnBox(BoxDraw& parent, VideoOverlay& overlay)
        : m_parent(parent)
        , m_overlay(overlay)
        , m_overlay_set(overlay)
    {
        // DrawnBox listens to changes in the config option (X, Y, WIDTH, HEIGHT)
        // and mouse events on the video overlay layer.
        try{
            m_parent.INFERENCE_BOX.add_listener(*this);
            m_parent.CONTENT_BOX.add_listener(*this);
            overlay.add_mouse_listener(*this);
        }catch (...){
            detach();
            throw;
        }
    }
    virtual void on_config_value_changed(void* object) override{
        {
            std::lock_guard<Mutex> lg(m_lock);
            m_overlay_set.clear();
            double ix = m_parent.INFERENCE_BOX.X;
            double iy = m_parent.INFERENCE_BOX.Y;
            double iw = m_parent.INFERENCE_BOX.WIDTH;
            double ih = m_parent.INFERENCE_BOX.HEIGHT;
            if (m_parent.CONTENT_BOX.enabled()){
                double cx = m_parent.CONTENT_BOX.X;
                double cy = m_parent.CONTENT_BOX.Y;
                double cw = m_parent.CONTENT_BOX.WIDTH;
                double ch = m_parent.CONTENT_BOX.HEIGHT;
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
//        if (object != &m_parent.INFERENCE_BOX){
//            m_parent.INFERENCE_BOX.on_config_value_changed(object);
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

        if (m_parent.CONTENT_BOX.enabled()){
            double cx = m_parent.CONTENT_BOX.X;
            double cy = m_parent.CONTENT_BOX.Y;
            double cw = m_parent.CONTENT_BOX.WIDTH;
            double ch = m_parent.CONTENT_BOX.HEIGHT;

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

        m_parent.INFERENCE_BOX.X.set(xl);
        m_parent.INFERENCE_BOX.Y.set(yl);
        m_parent.INFERENCE_BOX.WIDTH.set(xh - xl);
        m_parent.INFERENCE_BOX.HEIGHT.set(yh - yl);
        // m_parent.update_box_coordinates();
    }

private:
    void detach(){
        m_overlay.remove_mouse_listener(*this);
        m_parent.CONTENT_BOX.remove_listener(*this);
        m_parent.INFERENCE_BOX.remove_listener(*this);
    }

private:
    BoxDraw& m_parent;
    VideoOverlay& m_overlay;
    VideoOverlaySet m_overlay_set;
    Mutex m_lock;

    std::optional<std::pair<double, double>> m_mouse_start;
};



void BoxDraw::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
//    update_individual_coordinates();
    DrawnBox drawn_box(*this, env.console.overlay());
    drawn_box.on_config_value_changed(this);
    context.wait_until_cancel();
}




}
}
