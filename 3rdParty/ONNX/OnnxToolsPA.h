/*  Simple Integer Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *      This option is thread-safe.
 *
 */

#ifndef PokemonAutomation_3rdParty_OnnxToolsPA_H
#define PokemonAutomation_3rdParty_OnnxToolsPA_H

#include <string>
#include <onnxruntime_c_api.h>
#include "Common/Cpp/Unicode.h"

namespace PokemonAutomation{

#ifdef _WIN32

inline std::wstring str_to_onnx_str(const std::string& str){
    return utf8_to_wstr(str);
}


#else


inline std::string str_to_onnx_str(std::string str){
    return str;
}


#endif



}
#endif
