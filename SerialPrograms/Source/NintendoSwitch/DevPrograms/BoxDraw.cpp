/*  Box Draw
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <optional>
#include "Common/Cpp/Concurrency/Mutex.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "Common/Cpp/Strings/StringTools.h"
#include "BoxDraw.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{



BoxDrawGroup::~BoxDrawGroup(){
    BOX_COORDINATES.remove_listener(*this);
    HEIGHT.remove_listener(*this);
    WIDTH.remove_listener(*this);
    Y.remove_listener(*this);
    X.remove_listener(*this);
}
BoxDrawGroup::BoxDrawGroup(
    std::string label,
    LockMode lock_while_program_is_running,
    EnableMode enable_mode,
    bool show_restore_defaults_button,
    ImageFloatBox box
)
    : GroupOption(
        std::move(label),
        lock_while_program_is_running,
        enable_mode,
        show_restore_defaults_button
    )
    , X("<b>X Coordinate:</b>", LockMode::UNLOCK_WHILE_RUNNING, box.x, 0.0, 1.0)
    , Y("<b>Y Coordinate:</b>", LockMode::UNLOCK_WHILE_RUNNING, box.y, 0.0, 1.0)
    , WIDTH("<b>Width:</b>", LockMode::UNLOCK_WHILE_RUNNING, box.width, 0.0, 1.0)
    , HEIGHT("<b>Height:</b>", LockMode::UNLOCK_WHILE_RUNNING, box.height, 0.0, 1.0)
    , BOX_COORDINATES(
        false,
        "ImageFloatBox coordinates",
        LockMode::UNLOCK_WHILE_RUNNING,
        make_full_str(),
        "0.3, 0.3, 0.4, 0.4"
    )
{
    PA_ADD_OPTION(X);
    PA_ADD_OPTION(Y);
    PA_ADD_OPTION(WIDTH);
    PA_ADD_OPTION(HEIGHT);
    PA_ADD_OPTION(BOX_COORDINATES);

    BoxDrawGroup::on_config_value_changed(&X);
    X.add_listener(*this);
    Y.add_listener(*this);
    WIDTH.add_listener(*this);
    HEIGHT.add_listener(*this);
    BOX_COORDINATES.add_listener(*this);
}

void BoxDrawGroup::on_config_value_changed(void* object){
    if (object == &X || object == &Y || object == &WIDTH || object == &HEIGHT){
        BOX_COORDINATES.set(make_full_str());
        report_value_changed(this);
    }else if(object == &BOX_COORDINATES){
        std::string box_coord_string = BOX_COORDINATES;
        std::vector<std::string> all_coords = StringTools::split(box_coord_string, ", ");

        std::string x_string = all_coords[0];
        std::string y_string = all_coords[1];
        std::string width_string = all_coords[2];
        std::string height_string = all_coords[3];

        double x_coord = std::stod(x_string);
        double y_coord = std::stod(y_string);
        double width_coord = std::stod(width_string);
        double height_coord = std::stod(height_string);

        // cout << box_coord_string << endl;
        // cout << std::to_string(x_coord) << endl;
        // cout << std::to_string(y_coord) << endl;
        // cout << std::to_string(width_coord) << endl;
        // cout << std::to_string(height_coord) << endl;

        X.set(x_coord);
        Y.set(y_coord);
        WIDTH.set(width_coord);
        HEIGHT.set(height_coord);
    }else{
        report_value_changed(object);
    }
}






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
