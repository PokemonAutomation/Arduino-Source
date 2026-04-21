/*  Read Battle Level Up
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonFRLG_ReadBattleLevelUp_H
#define PokemonAutomation_PokemonFRLG_ReadBattleLevelUp_H

#include "CommonFramework/Tools/VideoStream.h"
#include "CommonTools/Options/LanguageOCROption.h"
#include "NintendoSwitch/Controllers/Procon/NintendoSwitch_ProController.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "PokemonFRLG/Inference/Dialogs/PokemonFRLG_BattleDialogs.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{

class ReadBattleLevelUp_Descriptor : public SingleSwitchProgramDescriptor{
public:
    ReadBattleLevelUp_Descriptor();
};

class ReadBattleLevelUp : public SingleSwitchProgramInstance{
public:
    ReadBattleLevelUp();
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

