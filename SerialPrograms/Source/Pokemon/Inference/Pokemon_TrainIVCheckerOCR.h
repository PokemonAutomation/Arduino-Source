/*  Train IV Checker OCR Data
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_TrainIVCheckerOCR_H
#define PokemonAutomation_PokemonSwSh_TrainIVCheckerOCR_H

#include "CommonFramework/Options/SimpleIntegerOption.h"
#include "CommonFramework/Options/StringOption.h"
#include "CommonFramework/Options/EnumDropdownOption.h"
#include "ComputerPrograms/ComputerProgram.h"

namespace PokemonAutomation{
namespace Pokemon{


class TrainIVCheckerOCR_Descriptor : public ComputerProgramDescriptor{
public:
    TrainIVCheckerOCR_Descriptor();
};



class TrainIVCheckerOCR : public ComputerProgramInstance{
public:
    TrainIVCheckerOCR();

    virtual void program(ProgramEnvironment& env, CancellableScope& scope) override;

private:
    StringOption DIRECTORY;
    EnumDropdownOption MODE;
    SimpleIntegerOption<uint32_t> THREADS;  //  Can't use "size_t" due to integer type ambiguity.

};



}
}
#endif
