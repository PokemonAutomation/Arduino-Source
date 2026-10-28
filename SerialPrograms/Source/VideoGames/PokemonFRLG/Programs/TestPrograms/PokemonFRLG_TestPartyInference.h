/*  Test Party Inference
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonFRLG_TestPartyInference_H
#define PokemonAutomation_PokemonFRLG_TestPartyInference_H

#include "CommonTools/Options/LanguageOCROption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{

class TestPartyInference_Descriptor : public SingleSwitchProgramDescriptor{
public:
    TestPartyInference_Descriptor();
};

class TestPartyInference : public SingleSwitchProgramInstance{
public:
    using Descriptor = TestPartyInference_Descriptor;
    TestPartyInference();

    virtual void start_program_border_check(VideoStream&, FeedbackType) override{}
    virtual void program(SingleSwitchProgramEnvironment& env, CancellableScope& scope) override;

private:
    OCR::LanguageOCROption LANGUAGE;
};

}
}
}
#endif
