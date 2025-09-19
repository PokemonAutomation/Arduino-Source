/*  Box Draw
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <optional>
#include <mutex>
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "Common/Cpp/StringTools.h"
#include "BoxDraw.h"

// #include <iostream>
// using std::cout;
// using std::endl;

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
    : X("<b>X Coordinate:</b>", LockMode::UNLOCK_WHILE_RUNNING, 0.3, 0.0, 1.0)
    , Y("<b>Y Coordinate:</b>", LockMode::UNLOCK_WHILE_RUNNING, 0.3, 0.0, 1.0)
    , WIDTH("<b>Width:</b>", LockMode::UNLOCK_WHILE_RUNNING, 0.4, 0.0, 1.0)
    , HEIGHT("<b>Height:</b>", LockMode::UNLOCK_WHILE_RUNNING, 0.4, 0.0, 1.0)
    , BOX_COORDINATES(false, "ImageFloatBox coordinates", LockMode::UNLOCK_WHILE_RUNNING, "0.3, 0.3, 0.4, 0.4", "0.3, 0.3, 0.4, 0.4")
{
    PA_ADD_OPTION(X);
    PA_ADD_OPTION(Y);
    PA_ADD_OPTION(WIDTH);
    PA_ADD_OPTION(HEIGHT);
    PA_ADD_OPTION(BOX_COORDINATES);
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
            m_parent.BOX_COORDINATES.add_listener(*this);
            overlay.add_listener(*this);
        }catch (...){
            detach();
            throw;
        }
    }
    virtual void on_config_value_changed(void* object) override{
        {
            std::lock_guard<std::mutex> lg(m_lock);
            m_overlay_set.clear();
            m_overlay_set.add(COLOR_RED, {m_parent.X, m_parent.Y, m_parent.WIDTH, m_parent.HEIGHT});
        }
        
        if (object == &m_parent.X || object == &m_parent.Y || object == &m_parent.WIDTH || object == &m_parent.HEIGHT){
            m_parent.update_box_coordinates();
        }
        else if(object == &m_parent.BOX_COORDINATES){
            m_parent.update_individual_coordinates();    
        }

        
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
        // m_parent.update_box_coordinates();
    }

private:
    void detach(){
        m_overlay.remove_listener(*this);
        m_parent.X.remove_listener(*this);
        m_parent.Y.remove_listener(*this);
        m_parent.WIDTH.remove_listener(*this);
        m_parent.HEIGHT.remove_listener(*this);
        m_parent.BOX_COORDINATES.remove_listener(*this);
    }

private:
    BoxDraw& m_parent;
    VideoOverlay& m_overlay;
    VideoOverlaySet m_overlay_set;
    std::mutex m_lock;

    std::optional<std::pair<double, double>> m_mouse_start;
};

void BoxDraw::update_box_coordinates(){
    std::string box_coord_string = std::to_string(X) + ", " + std::to_string(Y) + ", " + std::to_string(WIDTH) + ", " + std::to_string(HEIGHT);
    BOX_COORDINATES.set(box_coord_string);
}



void BoxDraw::update_individual_coordinates(){
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
}

void BoxDraw::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    update_individual_coordinates();
    DrawnBox drawn_box(*this, env.console.overlay());
    drawn_box.on_config_value_changed(this);
    context.wait_until_cancel();
}




}
}
