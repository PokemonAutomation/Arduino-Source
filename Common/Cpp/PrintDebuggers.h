/*  Print Debuggers
 *
 *  From: https://github.com/PokemonAutomation/
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

void print_bits(uint64_t m){
    for (int i = 0; i < 8; i++){
        for (int j = 0; j < 8; j++){
            cout << (m & 1);
            m >>= 1;
        }
        cout << endl;
    }
    cout << endl;
}
void print_8x8(uint64_t m){
    for (int i = 0; i < 8; i++){
        for (int j = 0; j < 8; j++){
            cout << (m & 1);
            m >>= 1;
        }
        cout << " ";
    }
    cout << endl;
}





}
#endif
