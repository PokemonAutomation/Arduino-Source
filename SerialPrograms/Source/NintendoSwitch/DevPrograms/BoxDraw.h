/*  Box Draw
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_BoxDraw_H
#define PokemonAutomation_NintendoSwitch_BoxDraw_H

#include "Common/Cpp/Options/FloatingPointOption.h"
#include "Common/Cpp/Options/StringOption.h"
#include "Common/Cpp/Options/GroupOption.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


class BoxDrawGroup : public GroupOption, public ConfigOption::Listener{
public:
    ~BoxDrawGroup();
    BoxDrawGroup(
        std::string label,
        LockMode lock_while_program_is_running,
        EnableMode enable_mode = EnableMode::ALWAYS_ENABLED,
        bool show_restore_defaults_button = false,
        ImageFloatBox box = {0.3, 0.3, 0.4, 0.4}
    );

    virtual void on_config_value_changed(void* object) override;

private:
    std::string make_full_str() const{
        return
            std::to_string(X) + ", " +
            std::to_string(Y) + ", " +
            std::to_string(WIDTH) + ", " +
            std::to_string(HEIGHT);
    }

public:
    FloatingPointOption X;
    FloatingPointOption Y;
    FloatingPointOption WIDTH;
    FloatingPointOption HEIGHT;
    StringOption BOX_COORDINATES;
};



class BoxDraw_Descriptor : public SingleSwitchProgramDescriptor{
public:
    BoxDraw_Descriptor();
};


// Draw box on the video stream
class BoxDraw : public SingleSwitchProgramInstance{
public:
    BoxDraw();

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    class DrawnBox;

private:
    BoxDrawGroup INFERENCE_BOX;
    BoxDrawGroup CONTENT_BOX;
};





}
}
#endif
