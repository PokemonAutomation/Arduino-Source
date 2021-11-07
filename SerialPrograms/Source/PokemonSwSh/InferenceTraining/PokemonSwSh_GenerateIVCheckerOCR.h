/*  Generate IV Checker OCR Data
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_GenerateIVCheckerOCR_H
#define PokemonAutomation_PokemonSwSh_GenerateIVCheckerOCR_H

#include "CommonFramework/Options/EnumDropdownOption.h"
#include "CommonFramework/OCR/OCR_LanguageOptionOCR.h"
#include "PokemonSwSh/Inference/PokemonSwSh_IVCheckerReader.h"
#include "NintendoSwitch/Framework/SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class IVCheckerOptionOCR : public EnumDropdownOption{
public:
    static const std::string TOKENS[];

public:
    IVCheckerOptionOCR(QString label, size_t default_index = 0)
        : EnumDropdownOption(
            std::move(label),
            {
                "No Good (0)",
                "Decent (0-15)",
                "Pretty Good (16-25)",
                "Very Good (26-29)",
                "Fantastic (30)",
                "Best (31)",
                "Hyper trained!",
            },
            default_index
        )
    {}
};



class GenerateIVCheckerOCR_Descriptor : public RunnableSwitchProgramDescriptor{
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
    GenerateIVCheckerOCR(const GenerateIVCheckerOCR_Descriptor& descriptor);

    virtual void program(SingleSwitchProgramEnvironment& env) override;

private:
    IVCheckerReader m_reader;

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
