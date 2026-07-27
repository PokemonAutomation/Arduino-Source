/*  Box Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_BoxOption_H
#define PokemonAutomation_BoxOption_H

#include "Common/Cpp/PrettyPrint.h"
#include "Common/Cpp/Options/GroupOption.h"
#include "Common/Cpp/Options/FloatingPointOption.h"
#include "Common/Cpp/Options/StringOption.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"

namespace PokemonAutomation{


class BoxOption : public GroupOption, public ConfigOption::Listener{
public:
    ~BoxOption();
    BoxOption(
        std::string label,
        LockMode lock_while_program_is_running,
        EnableMode enable_mode = EnableMode::ALWAYS_ENABLED,
        bool show_restore_defaults_button = false,
        ImageFloatBox box = {0.3, 0.3, 0.4, 0.4}
    );

    operator ImageFloatBox() const{
        return {X, Y, WIDTH, HEIGHT};
    }
    ImageFloatBox inner_to_outer(const ImageFloatBox& inner_box) const{
        return translate_to_parent(*this, inner_box);
    }

    virtual void on_config_value_changed(void* object) override;

private:
    std::string make_full_str() const{
        return
            tostr_fixed_no_trailing_zero(double(X), 6) + ", " +
            tostr_fixed_no_trailing_zero(double(Y), 6) + ", " +
            tostr_fixed_no_trailing_zero(double(WIDTH), 6) + ", " +
            tostr_fixed_no_trailing_zero(double(HEIGHT), 6);
    }

public:
    FloatingPointOption X;
    FloatingPointOption Y;
    FloatingPointOption WIDTH;
    FloatingPointOption HEIGHT;
    StringOption BOX_COORDINATES;
};



}
#endif
