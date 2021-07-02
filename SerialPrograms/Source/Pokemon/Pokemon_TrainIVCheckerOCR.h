/*  Train IV Checker OCR Data
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_TrainIVCheckerOCR_H
#define PokemonAutomation_PokemonSwSh_TrainIVCheckerOCR_H

#include "CommonFramework/Options/SimpleInteger.h"
#include "CommonFramework/Options/String.h"
#include "CommonFramework/Options/EnumDropdown.h"
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

    virtual void program(ProgramEnvironment& env) override;

private:
    String DIRECTORY;
    EnumDropdown MODE;
    SimpleInteger<uint16_t> THREADS;

};



}
}
#endif
