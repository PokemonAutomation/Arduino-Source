/*  Generate Location Name OCR Data
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLZA_GenerateNameOCRData_H
#define PokemonAutomation_PokemonLZA_GenerateNameOCRData_H

#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "CommonTools/Options/LanguageOCROption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"


namespace PokemonAutomation{
    class JsonArray;
    class ImageViewRGB32;
namespace NintendoSwitch{
namespace PokemonLZA{


class GenerateLocationNameOCR_Descriptor : public SingleSwitchProgramDescriptor{
public:
    GenerateLocationNameOCR_Descriptor();
};



class GenerateLocationNameOCR : public SingleSwitchProgramInstance{
public:
    GenerateLocationNameOCR();

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
