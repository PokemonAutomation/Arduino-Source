/*  Read Box Info
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonFRLG_ReadBoxInfo_H
#define PokemonAutomation_PokemonFRLG_ReadBoxInfo_H

#include "CommonTools/Options/LanguageOCROption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{

class ReadBoxInfo_Descriptor : public SingleSwitchProgramDescriptor{
public:
    ReadBoxInfo_Descriptor();
};

class ReadBoxInfo : public SingleSwitchProgramInstance{
public:
    using Descriptor = ReadBoxInfo_Descriptor;
    ReadBoxInfo();

    virtual void start_program_border_check(VideoStream& stream, FeedbackType feedback_type) override{}
    virtual void program(SingleSwitchProgramEnvironment& env, CancellableScope& scope) override;

private:
    OCR::LanguageOCROption LANGUAGE;
};

}
}
}
#endif
