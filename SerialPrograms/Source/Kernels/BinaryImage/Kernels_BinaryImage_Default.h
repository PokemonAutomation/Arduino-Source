/*  Binary Image (u64)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Kernels_BinaryImage_u64_H
#define PokemonAutomation_Kernels_BinaryImage_u64_H

#include "Kernels_BinaryImageBase.h"

namespace PokemonAutomation{
namespace Kernels{


class BinaryImage_Default : public BinaryImageBase<uint64_t>{
    using Word = uint64_t;

public:
    using BinaryImageBase::BinaryImageBase;

    void set_zero(){
        size_t lc = m_data.size();
        Word* ptr = m_data.data();
        do{
            ptr[0] = 0;
            ptr++;
        }while (--lc);
    }
};


}
}
#endif
