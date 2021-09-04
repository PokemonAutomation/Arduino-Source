/*  Train Pokemon Name OCR
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Pokemon_TrainPokemonOCR_H
#define PokemonAutomation_Pokemon_TrainPokemonOCR_H

#include "CommonFramework/Options/SimpleInteger.h"
#include "CommonFramework/Options/String.h"
#include "CommonFramework/Options/EnumDropdown.h"
#include "CommonFramework/Panels/RunnableComputerProgram.h"

namespace PokemonAutomation{
namespace Pokemon{


class TrainPokemonOCR_Descriptor : public RunnableComputerProgramDescriptor{
public:
    TrainPokemonOCR_Descriptor();
};



class TrainPokemonOCR : public RunnableComputerProgramInstance{
public:
    TrainPokemonOCR(const TrainPokemonOCR_Descriptor& descriptor);

    virtual void program(ProgramEnvironment& env) override;

private:
    String DIRECTORY;
    EnumDropdown MODE;
    SimpleInteger<uint16_t> THREADS;

};



}
}
#endif
