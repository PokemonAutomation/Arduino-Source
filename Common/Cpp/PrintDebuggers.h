/*  Print Debuggers
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PrintDebuggers_H
#define PokemonAutomation_PrintDebuggers_H

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{


template <typename Type>
void print_array(const Type* ptr, size_t len){
    cout << "{";
    bool first = true;
    for (size_t c = 0; c < len; c++){
        if (!first){
            cout << ", ";
        }
        first = false;
        cout << ptr[c];
    }
    cout << "}" << endl;
}


inline void print_u8(const uint8_t* ptr, size_t len){
    cout << "{";
    bool first = true;
    for (size_t c = 0; c < len; c++){
        if (!first){
            cout << ", ";
        }
        first = false;
        cout << (unsigned)ptr[c];
    }
    cout << "}" << endl;
}



}
#endif
