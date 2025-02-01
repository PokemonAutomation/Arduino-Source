/*  Unexpected Battle Exception
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_UnexpectedBattleException_H
#define PokemonAutomation_UnexpectedBattleException_H

#include "ScreenshotException.h"

namespace PokemonAutomation{

class FatalProgramException;


//  Thrown by subroutines if caught in an wild battle in-game unexpectedly.
//  These include recoverable errors which can be consumed by the program.
class UnexpectedBattleException : public ScreenshotException{
public:
    using ScreenshotException::ScreenshotException;

    virtual const char* name() const override{ return "UnexpectedBattleException"; }
};





}
#endif
