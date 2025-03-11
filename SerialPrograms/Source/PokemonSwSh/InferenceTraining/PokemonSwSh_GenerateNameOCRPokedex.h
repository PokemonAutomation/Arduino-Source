/*  Generate Pokemon Name OCR Data (Pokedex)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_GenerateNameOCRData_H
#define PokemonAutomation_PokemonSwSh_GenerateNameOCRData_H

#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "CommonTools/Options/LanguageOCROption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"


namespace PokemonAutomation{
    class JsonArray;
    class ImageViewRGB32;
namespace NintendoSwitch{
namespace PokemonSwSh{


class GenerateNameOCRDataPokedex_Descriptor : public SingleSwitchProgramDescriptor{
public:
    GenerateNameOCRDataPokedex_Descriptor();
};



class GenerateNameOCRDataPokedex : public SingleSwitchProgramInstance{
public:
    GenerateNameOCRDataPokedex();

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    void read(
        JsonArray& output,
        Logger& logger,
        const ImageViewRGB32& image
    ) const;
    void dump_images(
        const std::vector<std::string>& expected,
        size_t index,
        const ImageViewRGB32& image
    ) const;

private:
    OCR::LanguageOCROption LANGUAGE;

    enum class Pokedex{
        Galar,
        IsleOfArmor,
        CrownTundra,
    };
    EnumDropdownOption<Pokedex> POKEDEX;

    enum class Mode{
        SaveToJson,
        GenerateTrainingData,
    };
    EnumDropdownOption<Mode> MODE;
};



}
}
}
#endif
