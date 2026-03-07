/*  Box Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <vector>
#include "Common/Cpp/Strings/StringTools.h"
#include "BoxOption.h"

namespace PokemonAutomation{


BoxOption::~BoxOption(){
    BOX_COORDINATES.remove_listener(*this);
    HEIGHT.remove_listener(*this);
    WIDTH.remove_listener(*this);
    Y.remove_listener(*this);
    X.remove_listener(*this);
}
BoxOption::BoxOption(
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

    BoxOption::on_config_value_changed(&X);
    X.add_listener(*this);
    Y.add_listener(*this);
    WIDTH.add_listener(*this);
    HEIGHT.add_listener(*this);
    BOX_COORDINATES.add_listener(*this);
}

void BoxOption::on_config_value_changed(void* object){
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



}
