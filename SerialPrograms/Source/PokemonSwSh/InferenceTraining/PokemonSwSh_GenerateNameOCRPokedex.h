/*  Generate Pokemon Name OCR Data (Pokedex)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_GenerateNameOCRData_H
#define PokemonAutomation_PokemonSwSh_GenerateNameOCRData_H

#include "CommonFramework/Options/EnumDropdownOption.h"
#include "CommonFramework/OCR/OCR_LanguageOptionOCR.h"
#include "NintendoSwitch/Framework/NintendoSwitch_SingleSwitchProgram.h"


namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class GenerateNameOCRDataPokedex_Descriptor : public RunnableSwitchProgramDescriptor{
public:
    GenerateNameOCRDataPokedex_Descriptor();
};



class GenerateNameOCRDataPokedex : public SingleSwitchProgramInstance{
public:
    enum Mode{
        READ_AND_SAVE,
        GENERATE_TRAINING_DATA,
    };

public:
    GenerateNameOCRDataPokedex(const GenerateNameOCRDataPokedex_Descriptor& descriptor);

    virtual void program(SingleSwitchProgramEnvironment& env) override;

private:
    void read(
        QJsonArray& output,
        LoggerQt& logger,
        QImage image
    ) const;
    void dump_images(
        const std::vector<std::string>& expected,
        size_t index,
        QImage image
    ) const;

private:
    OCR::LanguageOCR LANGUAGE;
    EnumDropdownOption POKEDEX;
    EnumDropdownOption MODE;
};



}
}
}
#endif
