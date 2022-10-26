/*  Generate IV Checker OCR Data
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_GenerateIVCheckerOCR_H
#define PokemonAutomation_PokemonSwSh_GenerateIVCheckerOCR_H

#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "CommonFramework/Options/LanguageOCROption.h"
#include "PokemonSwSh/Inference/PokemonSwSh_IVCheckerReader.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class IVCheckerOptionOCR : public EnumDropdownOption<IVCheckerValue>{
public:
    static const std::string TOKENS[];

public:
    IVCheckerOptionOCR(std::string label, IVCheckerValue default_value)
        : EnumDropdownOption<IVCheckerValue>(
            std::move(label),
            IVCheckerValue_Database(),
            LockWhileRunning::LOCK_WHILE_RUNNING,
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

    virtual void program(SingleSwitchProgramEnvironment& env, BotBaseContext& context) override;

private:
    OCR::LanguageOCR LANGUAGE;
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
