/*  Nested Box Draw Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_NestedBoxDrawOption_H
#define PokemonAutomation_NestedBoxDrawOption_H

#include "Common/Cpp/Containers/Pimpl.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "BoxOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


class NestedBoxDrawOption : public BatchOption, public ConfigOption::Listener{
public:
    NestedBoxDrawOption(LockMode lock_while_program_is_running);

    class DrawnBox;
    Pimpl<DrawnBox> make_session(VideoOverlay& overlay);


public:
    BoxOption INFERENCE_BOX;
    BoxOption CONTENT_BOX;
};




}
}
#endif
