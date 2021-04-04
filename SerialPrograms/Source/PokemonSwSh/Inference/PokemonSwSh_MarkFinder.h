/*  Mark Finder
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_MarkFinder_H
#define PokemonAutomation_PokemonSwSh_MarkFinder_H

#include <vector>
#include <QImage>
#include "CommonFramework/Inference/InferenceTypes.h"
#include "CommonFramework/Inference/FillMatrix.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{



struct WhiteFilter{
    size_t count = 0;

    void operator()(FillMatrix::ObjectID& cell, const QImage& image, size_t x, size_t y);
};



size_t find_marks(
    const QImage& image,
    std::vector<PixelBox>* exclamation_marks,
    std::vector<PixelBox>* question_marks
);



}
}
}
#endif
