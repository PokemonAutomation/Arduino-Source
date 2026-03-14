/*  Read Stats
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonFRLG_ReadStats_H
#define PokemonAutomation_PokemonFRLG_ReadStats_H

#include "CommonFramework/Tools/VideoStream.h"
#include "CommonTools/Options/LanguageOCROption.h"
#include "NintendoSwitch/Controllers/Procon/NintendoSwitch_ProController.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{

class ReadStats_Descriptor : public SingleSwitchProgramDescriptor{
public:
    ReadStats_Descriptor();
};

class ReadStats : public SingleSwitchProgramInstance{
public:
    ReadStats();
    virtual void program(
        SingleSwitchProgramEnvironment &env,
        ProControllerContext &context
    ) override;

    virtual void start_program_border_check(
        VideoStream &stream, FeedbackType feedback_type
    ) override{}

private:
    OCR::LanguageOCROption LANGUAGE;
};

} // namespace PokemonFRLG
} // namespace NintendoSwitch
} // namespace PokemonAutomation
#endif

