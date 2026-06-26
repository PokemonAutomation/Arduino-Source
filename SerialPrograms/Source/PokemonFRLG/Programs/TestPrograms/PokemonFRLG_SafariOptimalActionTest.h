/*  Safari Optimal Action Test
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonFRLG_SafariOptimalActionTest_H
#define PokemonAutomation_PokemonFRLG_SafariOptimalActionTest_H

#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "CommonFramework/Tools/VideoStream.h"
#include "CommonTools/Options/LanguageOCROption.h"
#include "NintendoSwitch/Controllers/Procon/NintendoSwitch_ProController.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{

class SafariOptimalActionTest_Descriptor : public SingleSwitchProgramDescriptor{
public:
    SafariOptimalActionTest_Descriptor();
};

class SafariOptimalActionTest : public SingleSwitchProgramInstance{
public:
    SafariOptimalActionTest();

    virtual void start_program_border_check(VideoStream& stream, FeedbackType feedback_type) override{}
    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    OCR::LanguageOCROption LANGUAGE;
    SimpleIntegerOption<uint8_t> BALLS_REMAINING;
};

}
}
}
#endif
