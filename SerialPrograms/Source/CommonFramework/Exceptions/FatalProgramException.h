/*  Fatal Program Exception
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_FatalProgramException_H
#define PokemonAutomation_FatalProgramException_H

#include "ScreenshotException.h"

namespace PokemonAutomation{


//  A generic exception that should not be caught outside of infra.
class FatalProgramException : public ScreenshotException{
public:
    using ScreenshotException::ScreenshotException;
    FatalProgramException(ScreenshotException&& e);

    virtual const char* name() const override{ return "FatalProgramException"; }
};





}
#endif
