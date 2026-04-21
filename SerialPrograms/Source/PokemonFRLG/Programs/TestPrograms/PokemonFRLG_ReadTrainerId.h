/*  Read Trainer ID
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonFRLG_ReadTrainerId_H
#define PokemonAutomation_PokemonFRLG_ReadTrainerId_H

#include "CommonFramework/Tools/VideoStream.h"
#include "CommonTools/Options/LanguageOCROption.h"
#include "NintendoSwitch/Controllers/Procon/NintendoSwitch_ProController.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{

class ReadTrainerId_Descriptor : public SingleSwitchProgramDescriptor{
public:
    ReadTrainerId_Descriptor();
};

class ReadTrainerId : public SingleSwitchProgramInstance{
public:
    ReadTrainerId();
    virtual void program(
        SingleSwitchProgramEnvironment &env,
        ProControllerContext &context
    ) override;

    virtual void start_program_border_check(
        VideoStream &stream, FeedbackType feedback_type
    ) override{}

};

} // namespace PokemonFRLG
} // namespace NintendoSwitch
} // namespace PokemonAutomation
#endif

