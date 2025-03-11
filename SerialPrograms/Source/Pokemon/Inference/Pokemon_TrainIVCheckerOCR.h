/*  Train IV Checker OCR Data
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_TrainIVCheckerOCR_H
#define PokemonAutomation_PokemonSwSh_TrainIVCheckerOCR_H

#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/StringOption.h"
#include "CommonTools/Options/TrainOCRModeOption.h"
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
    TrainOCRModeOption MODE;
    SimpleIntegerOption<uint32_t> THREADS;  //  Can't use "size_t" due to integer type ambiguity.

};



}
}
#endif
