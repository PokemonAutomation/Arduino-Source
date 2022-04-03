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
#include "CommonFramework/Panels/RunnableComputerProgram.h"

namespace PokemonAutomation{
namespace Pokemon{


class TrainIVCheckerOCR_Descriptor : public RunnableComputerProgramDescriptor{
public:
    TrainIVCheckerOCR_Descriptor();
};



class TrainIVCheckerOCR : public RunnableComputerProgramInstance{
public:
    TrainIVCheckerOCR(const TrainIVCheckerOCR_Descriptor& descriptor);

    virtual void program(ProgramEnvironment& env, CancellableScope& scope) override;

private:
    StringOption DIRECTORY;
    EnumDropdownOption MODE;
    SimpleIntegerOption<uint16_t> THREADS;

};



}
}
#endif
