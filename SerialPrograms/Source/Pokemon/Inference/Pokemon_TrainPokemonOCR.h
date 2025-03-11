/*  Train Pokemon Name OCR
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Pokemon_TrainPokemonOCR_H
#define PokemonAutomation_Pokemon_TrainPokemonOCR_H

#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/StringOption.h"
#include "CommonTools/Options/TrainOCRModeOption.h"
#include "ComputerPrograms/ComputerProgram.h"

namespace PokemonAutomation{
namespace Pokemon{


class TrainPokemonOCR_Descriptor : public ComputerProgramDescriptor{
public:
    TrainPokemonOCR_Descriptor();
};



class TrainPokemonOCR : public ComputerProgramInstance{
public:
    TrainPokemonOCR();

    virtual void program(ProgramEnvironment& env, CancellableScope& scope) override;

private:
    StringOption DIRECTORY;
    TrainOCRModeOption MODE;
    SimpleIntegerOption<uint32_t> THREADS;  //  Can't use "size_t" due to integer type ambiguity.

};



}
}
#endif
