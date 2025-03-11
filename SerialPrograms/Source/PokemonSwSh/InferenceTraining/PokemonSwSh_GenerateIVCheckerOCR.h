/*  Generate IV Checker OCR Data
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_GenerateIVCheckerOCR_H
#define PokemonAutomation_PokemonSwSh_GenerateIVCheckerOCR_H

#include "CommonTools/Options/LanguageOCROption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "Pokemon/Pokemon_IvJudge.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

using namespace Pokemon;



class IVCheckerOptionOCR : public EnumDropdownOption<IvJudgeValue>{
public:
    static const std::string TOKENS[];

public:
    IVCheckerOptionOCR(std::string label, IvJudgeValue default_value)
        : EnumDropdownOption<IvJudgeValue>(
            std::move(label),
            IvJudgeValue_Database(),
            LockMode::LOCK_WHILE_RUNNING,
            default_value
        )
    {}
};



class GenerateIVCheckerOCR_Descriptor : public SingleSwitchProgramDescriptor{
public:
    GenerateIVCheckerOCR_Descriptor();
};



class GenerateIVCheckerOCR : public SingleSwitchProgramInstance{
public:
    enum Mode{
        READ_AND_SAVE,
        GENERATE_TRAINING_DATA,
    };

public:
    GenerateIVCheckerOCR();

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    OCR::LanguageOCROption LANGUAGE;
    IVCheckerOptionOCR HP;
    IVCheckerOptionOCR ATTACK;
    IVCheckerOptionOCR DEFENSE;
    IVCheckerOptionOCR SPATK;
    IVCheckerOptionOCR SPDEF;
    IVCheckerOptionOCR SPEED;
};



}
}
}
#endif
