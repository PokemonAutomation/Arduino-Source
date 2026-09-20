/*  Read Encounter
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonFRLG_ReadEncounter_H
#define PokemonAutomation_PokemonFRLG_ReadEncounter_H

#include "CommonFramework/Tools/VideoStream.h"
#include "CommonTools/Options/LanguageOCROption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{

class ReadEncounter_Descriptor : public SingleSwitchProgramDescriptor{
public:
    ReadEncounter_Descriptor();
};

class ReadEncounter : public SingleSwitchProgramInstance{
public:
    using Descriptor = ReadEncounter_Descriptor;
    ReadEncounter();

    virtual void start_program_controller_check(SwitchSystemSession& session) override{}
    virtual void start_program_border_check(VideoStream &stream, FeedbackType feedback_type) override{}
    virtual void program(SingleSwitchProgramEnvironment& env, CancellableScope& scope) override;


private:
    OCR::LanguageOCROption LANGUAGE;

};

} // namespace PokemonFRLG
} // namespace NintendoSwitch
} // namespace PokemonAutomation
#endif

