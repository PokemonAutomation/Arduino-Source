/*  Program Dumper
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Compiler.h"
#include "ProgramDumper.h"


#if _WIN32 && _MSC_VER
#include "ProgramDumper_Windows.tpp"

#else
namespace PokemonAutomation{
    void setup_crash_handler(){
        //  Not supported
    }
    bool program_dump(Logger* logger, const std::string& filename){
        return false;
    }
}
#endif
