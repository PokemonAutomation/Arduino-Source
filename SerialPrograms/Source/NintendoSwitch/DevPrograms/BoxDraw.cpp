/*  Box Draw
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <mutex>
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
        FeedbackType::NONE, AllowCommandsWhenRunning::ENABLE_COMMANDS,
        {ControllerFeature::NintendoSwitch_ProController}
    )
{}

BoxDraw::BoxDraw()
    : X("<b>X Coordinate:</b>", LockMode::UNLOCK_WHILE_RUNNING, 0.3, 0.0, 1.0)
    , Y("<b>Y Coordinate:</b>", LockMode::UNLOCK_WHILE_RUNNING, 0.3, 0.0, 1.0)
    , WIDTH("<b>Width:</b>", LockMode::UNLOCK_WHILE_RUNNING, 0.4, 0.0, 1.0)
    , HEIGHT("<b>Height:</b>", LockMode::UNLOCK_WHILE_RUNNING, 0.4, 0.0, 1.0)
{
    PA_ADD_OPTION(X);
    PA_ADD_OPTION(Y);
    PA_ADD_OPTION(WIDTH);
    PA_ADD_OPTION(HEIGHT);
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
            m_parent.X.add_listener(*this);
            m_parent.Y.add_listener(*this);
            m_parent.WIDTH.add_listener(*this);
            m_parent.HEIGHT.add_listener(*this);
            overlay.add_listener(*this);
        }catch (...){
            detach();
            throw;
        }
    }
    virtual void on_config_value_changed(void* object) override{
        std::lock_guard<std::mutex> lg(m_lock);
        m_overlay_set.clear();
        m_overlay_set.add(COLOR_RED, {m_parent.X, m_parent.Y, m_parent.WIDTH, m_parent.HEIGHT});
    }
    virtual void on_mouse_press(double x, double y) override{
        m_parent.WIDTH.set(0);
        m_parent.HEIGHT.set(0);
        m_parent.X.set(x);
        m_parent.Y.set(y);
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

        if (xl > xh){
            std::swap(xl, xh);
        }
        if (yl > yh){
            std::swap(yl, yh);
        }

        m_parent.X.set(xl);
        m_parent.Y.set(yl);
        m_parent.WIDTH.set(xh - xl);
        m_parent.HEIGHT.set(yh - yl);
    }

private:
    void detach(){
        m_overlay.remove_listener(*this);
        m_parent.X.remove_listener(*this);
        m_parent.Y.remove_listener(*this);
        m_parent.WIDTH.remove_listener(*this);
        m_parent.HEIGHT.remove_listener(*this);
    }

private:
    BoxDraw& m_parent;
    VideoOverlay& m_overlay;
    VideoOverlaySet m_overlay_set;
    std::mutex m_lock;

    std::optional<std::pair<double, double>> m_mouse_start;
};

void BoxDraw::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    DrawnBox drawn_box(*this, env.console.overlay());
    drawn_box.on_config_value_changed(this);
    context.wait_until_cancel();
}




}
}
